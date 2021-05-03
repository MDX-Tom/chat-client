#include "socket_udp.h"
#include "chat_packet_udp.h"

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
    QObject::connect(this->uSocket, SIGNAL(readyRead()),
                     this, SLOT(on_receiveData()));
}


SocketUDP::~SocketUDP()
{
    this->uSocket->close();
    delete this;
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

    // 发送信号，服务器data已经被读入
    emit this->on_receivedData();
}


/// 直接发送字节流（单包）
bool SocketUDP::SendPackedBytes
                         (QByteArray& bytes,
                          QHostAddress targetAddr,
                          quint16 targetPort)
{
    // 确认为单包
    if (bytes.size() > this->maxPayloadSize)
    {
        throw QString("SendBytes() can only send QByteArray with MAXIMUM SIZE OF: ") +
                QString(this->maxPayloadSize) + QString(" Bytes...");
        return false;
    }

    // 发送
    this->uSocket->writeDatagram(bytes, targetAddr, targetPort);

    qDebug().noquote() << "Sent to: " << targetAddr.toString() << ":"
                       << QString::number(targetPort) << " : " << QString(bytes) << Qt::endl;

    return true;
}
bool SocketUDP::SendPackedBytes(QByteArray &bytes)
{
    QHostAddress a = this->serverAddr;
    quint16 p = this->serverPort;
    return SendPackedBytes(bytes, a, p);
}

