#include "socket_udp.h"
#include "chat_packet_udp.h"
#include "crc32.h"

#include <QObject>
#include <QByteArray>
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


/// 收到ACK信息
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
                          bool requireACK,
                          quint8 retrySeq)
{
    // 重发次数超过MAX
    if (retrySeq >= this->retryCountMax)
    {
        throw QString("网络错误：连接超时。重传次数已达：") + QString(this->retryCountMax);
        return false;
    }

    // 确认为单包
    if (bytes.size() > this->maxPayloadSize)
    {
        throw QString("SendBytes() can only send QByteArray with MAXIMUM SIZE OF: ") +
                QString(this->maxPayloadSize) + QString(" Bytes...");
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
    return SendPackedBytes(bytes, a, p);
}

