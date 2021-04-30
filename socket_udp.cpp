#include "socket_udp.h"
#include "chat_packet_udp.h"
#include "crc32.h"

#include <QObject>
#include <QByteArray>
#include <QFile>
#include <QHostInfo>
#include <QHostAddress>
#include <QNetworkDatagram>
#include <QTimer>
#include <QDebug>


SocketUDP::SocketUDP()
{
    // 获取本机IPv4地址
    QHostInfo info = QHostInfo::fromName(QHostInfo::localHostName());
    try
    {
        foreach (QHostAddress addr, info.addresses())
        {
            if (addr.protocol() == QAbstractSocket::IPv4Protocol)
            {
                this->thisAddr = addr;
            }
        }
    }
    catch (...)
    {
        qDebug() << "Network not connected..." << Qt::endl;
    }

    // 创建UDPsocket并启用监听
    this->uSocket = new QUdpSocket();
    this->uSocket->bind(this->clientPort); // 绑定本机任意IP:8002
    this->thisPort = this->clientPort;

    this->receivedData = "";

    // 处理接收信号槽
    QObject::connect(this->uSocket, SIGNAL(readyRead()), this, SLOT(on_ReceiveData()));
}


SocketUDP::~SocketUDP()
{
    this->uSocket->close();
    delete this;
}


QHostAddress SocketUDP::GetReceivedAddr()
{
    return this->receivedAddr;
}


quint16 SocketUDP::GetReceivedPort()
{
    return this->receivedPort;
}


QByteArray SocketUDP::GetReceivedData()
{
    return this->receivedData;
}


/// readyRead()的槽函数
void SocketUDP::on_receiveData()
{
    if (!this->uSocket->hasPendingDatagrams())
    {
        // 忽略假信号
        return;
    }

    // 读入信息
    this->receivedData.resize(this->uSocket->pendingDatagramSize());
    this->uSocket->readDatagram(this->receivedData.data(), this->receivedData.size(),
                                &this->receivedAddr, &this->receivedPort);

    if (this->receivedData.isEmpty())
    {
        // 忽略空信息
        return;
    }

    // CHAT_CONTENT ACK信息
    if (ChatPacketUDP::isPacketReplyMsg(this->receivedData))
    {
        ChatPacketUDP::PacketReplyHeader ackHeader = *(ChatPacketUDP::PacketReplyHeader*)this->receivedData.data();
        emit this->on_receivedACK(ackHeader.md5Hash);
        return;
    }

    // 发送信号，服务器data已经被读入
    emit this->on_receivedData();
}


void SocketUDP::on_receivedACK(unsigned char* md5Hash)
{
    this->receivedACKHash.resize(16);
    memcpy(this->receivedACKHash.data(), md5Hash, 16);
}


/// 直接发送字节流（单包）
bool SocketUDP::SendPackedBytes
                         (QByteArray& bytes,
                          QHostAddress targetAddr,
                          quint16 targetPort,
                          quint8 retrySeq,
                          bool requireACK = false)
{
    // 重发次数超过MAX
    if (retrySeq >= this->retryCountMax)
    {
        throw "网络错误：连接超时。重传次数已达：" + QString(this->retryCountMax);
        return false;
    }

    // 确认为单包
    if (bytes.size() > this->maxPayloadSize)
    {
        throw "SendBytes() can only send QByteArray with MAXIMUM SIZE OF: " +
                QString(this->maxPayloadSize) + " Bytes...";
        return false;
    }



    // 发送
    this->uSocket->writeDatagram(bytes, targetAddr, targetPort);

    // 接收回包
    if (requireACK)
    {
        // 发送前MD5Hash
        QByteArray sentHash = QCryptographicHash::hash(bytes, QCryptographicHash::Md5);

        // 等待回包
        bool toggleTimeout = false;
        bool ackValid = false;
        QTimer::singleShot(this->waitForReplyMs, [toggleTimeout] () mutable { toggleTimeout = true; });
        while (!toggleTimeout)
        {
            // 校验收到的包
            if (this->receivedACKHash == sentHash)
            {
                ackValid = true;
                break;
            }
        }

        // 判断是否需要重传
        if (!ackValid)
        {
            return this->SendPackedBytes(bytes, targetAddr, targetPort, retrySeq + 1);
        }
    }

    return true;
}
bool SocketUDP::SendPackedBytes(QByteArray &bytes)
{
    QHostAddress a = this->serverAddr;
    quint16 p = this->serverPort;
    return SendPackedBytes(bytes, a, p, 0);
}


/// 发送文件
bool SocketUDP::SendFile
                        (QString &fileNameWithPath,
                         QHostAddress targetAddr,
                         quint16 targetPort,
                         quint16 targetUserID = 0,
                         quint16 fromUserID = 1)
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
    qint16 bytesCountPerPacket = this->maxPayloadSize - sizeof(ChatPacketUDP::FileMsgHeader); // 单包包含的文件字节数
    qint16 packetCountTotal = bytesTotal / bytesCountPerPacket + 1; // 分包数量

    // 分包发送
    for (int i = 0; i < packetCountTotal; i++)
    {
        // 处理包头
        ChatPacketUDP::FileMsgHeader header;
        header.packetSize = header.headerSize + bytesCountPerPacket; // == this->maxPayloadSize
        header.fromUserID = fromUserID;
        header.targetUserID = targetUserID;
        header.packetCountTotal = packetCountTotal;
        header.packetCountCurrent = i + 1;

        // 写入包头到QByteArray
        QByteArray bytesPacket;
        bytesPacket.resize(header.packetSize);
        memcpy(bytesPacket.data(), &header, sizeof(header)); // sizeof(header) == header.headerSize

        // 将文件内容写入到QByteArray
        QByteArray bytesFile = file.read(bytesCountPerPacket);
        memcpy(bytesFile.data(), &header, bytesFile.size()); // bytesFile.size() == bytesCountPerPacket

        // 更新进度指示信号
        float progress = 1.0 * i / packetCountTotal;
        emit this->sendFileProgress(progress);

        this->SendPackedBytes(bytesPacket, targetAddr, targetPort, 0, true);
    }

    file.close();

    return true;
}
bool SocketUDP::SendFile
                        (QString &fileNameWithPath,
                         quint16 targetUserID = 0,
                         quint16 thisUserID = 1)
{
    QHostAddress a = this->serverAddr;
    quint16 p = this->serverPort;
    return SendFile(fileNameWithPath, a, p, targetUserID, thisUserID);
}

