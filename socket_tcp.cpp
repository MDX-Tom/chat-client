#include "socket_tcp.h"

#include <QByteArray>
#include <QHostAddress>
#include <QDebug>


SocketTCP::SocketTCP(QString serverIP, short serverPort)
{
    this->socket = new QTcpSocket();

    this->receivedData = "";

    this->serverIP = serverIP;
    this->serverPort = serverPort;
}


SocketTCP::~SocketTCP()
{
    this->socket->close();
    delete this;
}


QString SocketTCP::GetReceivedData()
{
    return this->receivedData;
}


/// readyRead()的槽函数
void SocketTCP::on_receiveData()
{
    this->receivedData = QString(this->socket->readAll());

    if (this->receivedData.isEmpty())
    {
        // 忽略空信息
        return;
    }
    // 通知mainWindow，服务器data已经被读入
    emit this->on_receivedData();
}


bool SocketTCP::ConnectToServer(int timeoutMs = 2000)
{
    if (this->socket->state() != QTcpSocket::UnconnectedState)
    {
        throw QString("连接占线！");
        return false;
    }

    this->socket->connectToHost(this->serverIP, this->serverPort);

    if (!this->socket->waitForConnected(timeoutMs))
    {
        throw QString("连接超时！");
        return false;
    }

    connect(this->socket, SIGNAL(readyRead()), this, SLOT(on_receiveData()));
    qDebug().noquote().nospace()
                       << "成功连接到："
                       << this->serverIP << ":" << this->serverPort
                       << "，接收进程已建立！" << endl;

    return true;
}


bool SocketTCP::SendTextMsg(QString msg)
{
    if (this->socket->state() == QTcpSocket::UnconnectedState)
    {
        if (!ConnectToServer())
        {
            return false;
        }
    }

    if (!this->socket->isWritable())
    {
        throw QString("socket不可发送！");
        return false;
    }

    QByteArray bytesToSend = msg.toUtf8();

    this->socket->write(bytesToSend);
    return true;
}

