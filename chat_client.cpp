#include "chat_client.h"

#include "chat_user.h"
#include "socket_udp.h"
#include "chat_packet_udp.h"

#include <QFile>

ChatClient::ChatClient(MainWindow* mainWindow)
{
    this->socketUDP = new SocketUDP();
    this->user = new ChatUser(00000, "pswd_00000", "name_00000", new QVector<quint16>());
    this->chatDialogs = new QMap<quint16, QListWidget*>();

    this->mainWindow = mainWindow;

    // 定时请求新消息
    startTimer(this->fetchNewContentIntervalMs); // ms
}


ChatClient::~ChatClient()
{
    delete this->chatDialogs;
    delete this->user;
    delete this->socketUDP;
}


/// 定时向服务器获取新消息
void ChatClient::timerEvent(QTimerEvent *event)
{
    if (!this->LoggedIn())
    {
        // 未登录，不请求
        return;
    }

    this->SendChatRequest();
}


/// 向服务器发送LOGIN_REQUEST请求
bool ChatClient::Login(quint16 id, QString password)
{
    if (this->user->LoggedIn())
    {
        // 已经登录
        throw QString("已经登录！登录操作无效！");
        return false;
    }

    // 写入Header信息
    ChatPacketUDP::LoginRequestHeader header;
    header.thisUserID = id;
    QByteArray passwordBytes = password.toLatin1();
    passwordBytes.resize(sizeof(header.password));
    memcpy(header.password, passwordBytes.data(), sizeof(header.password));

    // 打包Header
    QByteArray bytesPacket;
    bytesPacket.resize(header.packetSize);
    memcpy(bytesPacket.data(), &header, sizeof(header));

    // 通过Socket发送请求Packet
    return this->socketUDP->SendPackedBytes(bytesPacket);
}


/// 向服务器发送LOGOUT_REQUEST请求
bool ChatClient::Logout()
{
    if (!this->user->LoggedIn())
    {
        // 还没有登录
        throw QString("还未登录！退出操作无效！");
        return false;
    }

    // 写入Header信息
    ChatPacketUDP::LogoutRequestHeader header;
    header.thisUserID = this->user->getID();

    // 打包Header
    QByteArray bytesPacket;
    bytesPacket.resize(header.packetSize);
    memcpy(bytesPacket.data(), &header, sizeof(header));

    // 通过Socket发送请求Packet
    return this->socketUDP->SendPackedBytes(bytesPacket);
}


/// 向服务器（对象用户）发送CHAT_CONTENT(TEXT)
bool ChatClient::SendText(QString Text, quint16 targetUserID)
{
    if (!this->user->LoggedIn())
    {
        // 尚未登录，不发送
        throw QString("尚未登录！发送消息无效！");
        return false;
    }

    // 将QString转化为QBytearray（UTF-8）
    QByteArray bytesText = Text.toLocal8Bit();

    // 准备Header
    ChatPacketUDP::TextMsgHeader header;
    header.packetSize = sizeof(header) + bytesText.size();
    if (header.packetSize > this->socketUDP->MaxPacketSize())
    {
        // 是否要分包处理超长信息？
        throw QString("消息过长...");
        return false;
    }
    header.fromUserID = this->user->getID();
    header.targetUserID = targetUserID;

    // 打包Header
    QByteArray bytesToSend;
    bytesToSend.resize(header.packetSize);
    memcpy(bytesToSend.data(), &header, sizeof(header));
    memcpy(bytesToSend.data() + sizeof(header), bytesText.data(), bytesText.size());

    // 通过Socket发送请求Packet
    return this->socketUDP->SendPackedBytes(bytesToSend);
}


/// 向服务器（对象用户）发送文件
bool ChatClient::SendFile
                        (QString &fileNameWithPath,
                         QHostAddress targetAddr,
                         quint16 targetPort,
                         quint16 targetUserID = 0)
{
    qDebug().noquote() << "Selected file: " << fileNameWithPath << Qt::endl;

    // 打开文件
    QFile file(fileNameWithPath);
    if (!file.open(QIODevice::ReadOnly))
    {
        throw "打开文件失败...";
        file.close();
        return false;
    }

    // 计算分包个数
    qint64 bytesTotal = file.size(); // 文件总字节数
    if (bytesTotal >= INT32_MAX)
    {
        throw "文件过大，最大只支持4GB...";
        file.close();
        return false;
    }
    qint16 bytesCountPerPacket = this->socketUDP->MaxPacketSize() - sizeof(ChatPacketUDP::FileMsgHeader); // 单包包含的文件字节数
    qint16 packetCountTotal = bytesTotal / bytesCountPerPacket + 1; // 分包数量

    // 分包发送
    for (int i = 0; i < packetCountTotal; i++)
    {
        // 处理包头
        ChatPacketUDP::FileMsgHeader header;
        header.packetSize = header.headerSize + bytesCountPerPacket; // == this->maxPayloadSize
        header.fromUserID = this->user->getID();
        header.targetUserID = targetUserID;
        header.packetCountTotal = packetCountTotal;
        header.packetCountCurrent = i + 1;

        // 写入包头到QByteArray
        QByteArray bytesPacket;
        bytesPacket.resize(header.packetSize);
        memcpy(bytesPacket.data(), &header, sizeof(header)); // sizeof(header) == header.headerSize

        // 将文件内容写入到QByteArray
        QByteArray bytesFile = file.read(bytesCountPerPacket);
        memcpy(bytesFile.data() + sizeof(header), &header, bytesFile.size()); // bytesFile.size() == bytesCountPerPacket

        // 更新进度指示信号
        float progress = 1.0 * i / packetCountTotal;
        emit this->sendFileProgress(progress);

        this->socketUDP->SendPackedBytes(bytesPacket, targetAddr, targetPort, 0, true);
    }

    file.close();

    return true;
}
bool ChatClient::SendFile
                        (QString &fileNameWithPath,
                         quint16 targetUserID = 0)
{
    QHostAddress a = this->socketUDP->ServerAddr();
    quint16 p = this->socketUDP->ServerPort();
    return SendFile(fileNameWithPath, a, p, targetUserID);
}



/// 向服务器发送CHAT_REQUEST请求
bool ChatClient::SendChatRequest()
{
    if (!this->LoggedIn())
    {
        // 尚未登录，不发送
        throw QString("尚未登录！获取新消息指令无效！");
        return false;
    }

    // 准备Header
    ChatPacketUDP::ChatRequestHeader header;
    header.thisUserID = this->user->getID();

    // 打包Header
    QByteArray bytesToSend;
    bytesToSend.resize(header.packetSize);
    memcpy(bytesToSend.data(), &header, sizeof(header));

    // 通过Socket发送请求Packet
    return this->socketUDP->SendPackedBytes(bytesToSend);
}
