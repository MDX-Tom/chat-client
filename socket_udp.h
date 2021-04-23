#ifndef SOCKET_UDP_H
#define SOCKET_UDP_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>

class SocketUDP : public QObject
{
    Q_OBJECT

private:
    const QString serverIP = "10.128.206.236";
    const quint16 serverPort = 8002;
    const quint16 clientPort = 8002;
    const quint16 maxPayloadSize = 65500; // Bytes

    QHostAddress thisAddr;
    quint16 thisPort;

    QUdpSocket* uSocket;

    QHostAddress receivedAddr;
    quint16 receivedPort;
    QByteArray receivedData;

signals:
    void on_receivedData(); // 信号先由socket传递给本类，从缓冲区读取完成后再发信号通知mainWindow取走

private slots:
    void on_receiveData();

public:
    SocketUDP();
    ~SocketUDP();

    QHostAddress GetReceivedAddr();
    quint16 GetReceivedPort();
    QByteArray GetReceivedData();
};

#endif // SOCKET_UDP_H
