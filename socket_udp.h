#ifndef SOCKET_UDP_H
#define SOCKET_UDP_H

#include <QObject>
#include <QQueue>
#include <QThread>
#include <QUdpSocket>
#include <QHostAddress>
#include <QCryptographicHash>

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

    // 超时重传等待时间
    quint16 waitForReplyMs = 300; // 可以添加自适应算法

    // 超时重传次数
    const quint8 retryCountMax = 3;

    // 收到的ACK哈希值
    QByteArray receivedACKHash;

    // 本机信息
    QHostAddress thisAddr;
    quint16 thisPort;

    // 接收信息
    QHostAddress receivedAddr;
    quint16 receivedPort;
    QByteArray receivedData;

signals:
    void on_receivedData(); // 信号先由socket传递给本类，从缓冲区读取完成后再发信号通知mainWindow取走
    void sendFileProgress(float); // 指示文件发送进度
    void on_receivedACK(unsigned char*); // 收到了ACK信息

private slots:
    void on_receiveData();

public:
    SocketUDP();
    ~SocketUDP();

    QHostAddress GetReceivedAddr();
    quint16 GetReceivedPort();
    QByteArray GetReceivedData();

    bool SendPackedBytes(QByteArray& bytesPacked, QHostAddress targetAddr, quint16 targetPort, quint8 retrySeq, bool requireACK = false);
    bool SendPackedBytes(QByteArray& bytesPacked);

    bool SendFile(QString& fileNameWithPath, QHostAddress targetAddr, quint16 targetPort, quint16 targetUserID, quint16 thisUserID);
    bool SendFile(QString& fileNameWithPath, quint16 targetUserID, quint16 thisUserID);
};


#endif // SOCKET_UDP_H
