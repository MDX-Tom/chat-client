#include "socket_udp.h"

#include <QByteArray>
#include <QHostInfo>
#include <QHostAddress>
#include <QNetworkDatagram>
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
        qDebug() << "Not connected..." << Qt::endl;
    }

    // 创建UDPsocket并启用监听
    this->uSocket = new QUdpSocket();
    this->uSocket->bind(this->clientPort); // 绑定本机任意IP:8002
    this->thisPort = this->clientPort;

    this->receivedData = "";
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
    QNetworkDatagram dgram = this->uSocket->receiveDatagram(this->maxPayloadSize);
    this->receivedAddr = dgram.senderAddress();
    this->receivedPort = dgram.senderPort();
    this->receivedData = dgram.data();

    if (this->receivedData.isEmpty())
    {
        // 忽略空信息
        return;
    }

    // 发送信号通知mainWindow，服务器data已经被读入
    emit this->on_receivedData();
}


