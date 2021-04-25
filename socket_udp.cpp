#include "socket_udp.h"
#include "chat_packet_udp.h"
#include "crc32.h"

#include <QObject>
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


/// 发送消息
void SocketUDP::SendFilePacketThread::run()
{

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

    // 发送信号通知mainWindow，服务器data已经被读入
    emit this->on_receivedData();
}


/// 发送字节（单包！）
bool SocketUDP::SendBytes(QByteArray& bytes, QHostAddress& targetAddr, quint16& targetPort)
{
    if (bytes.size() >= this->maxPayloadSize)
    {
        qDebug().noquote() << "SendBytes() can only send QByteArray with MAXIMUM SIZE OF: "
                           << this->maxPayloadSize << " Bytes..." << Qt::endl;
        return false;
    }



    return true;
}
bool SocketUDP::SendBytes(QByteArray &bytes)
{
    QHostAddress a = this->serverAddr;
    quint16 p = this->serverPort;
    return SendBytes(bytes, a, p);
}


/// 发送文件
bool SocketUDP::SendFile(QString &fileNameWithPath, QHostAddress &targetAddr, quint16 &targetPort)
{

}
bool SocketUDP::SendFile(QString &fileNameWithPath)
{
    QHostAddress a = this->serverAddr;
    quint16 p = this->serverPort;
    return SendFile(fileNameWithPath, a, p);
}

