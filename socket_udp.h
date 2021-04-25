#ifndef SOCKET_UDP_H
#define SOCKET_UDP_H

#include <QObject>
#include <QQueue>
#include <QThread>
#include <QUdpSocket>
#include <QHostAddress>

#include "chat_packet_udp.h"

class SocketUDP : public QObject
{
    Q_OBJECT

private:
    QUdpSocket* uSocket;

    // 预置信息
    const QHostAddress serverAddr = QHostAddress("10.128.206.236");
    const quint16 serverPort = 8002;
    const quint16 clientPort = 8002;

    // 最大单包内容大小
    // UDP max payloadSize is 65507 Bytes (packetSize <= 65535)
    // but payloadSize <= 548 (packetSize <= 576) will not cause fragmentation
    const quint16 maxPayloadSize = 548;

    // 本机信息
    QHostAddress thisAddr;
    quint16 thisPort;

    // 发送队列信息
    struct SendFilePacket
    {
        ChatPacketUDP::FileMsgHeader* header;
        QByteArray* payload;
    };
    QQueue<SendFilePacket> packetsToSend;

    // 发送队列处理线程
    class SendFilePacketThread: public QThread
    {
        Q_OBJECT
    public:
        SendFilePacketThread(QObject* parent = nullptr);
    protected:
        void run();
    };

    // 接收信息
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

    bool SendBytes(QByteArray& bytes, QHostAddress targetAddr, quint16 targetPort);
    bool SendBytes(QByteArray& bytes);

    bool SendFile(QString& fileNameWithPath, QHostAddress targetAddr, quint16 targetPort);
    bool SendFile(QString& fileNameWithPath);
};

#endif // SOCKET_UDP_H
