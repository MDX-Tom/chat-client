#ifndef SOCKET_TCP_H
#define SOCKET_TCP_H

#include <QObject>
#include <QTcpSocket>

class SocketTCP : public QObject
{
    Q_OBJECT

private:
    QTcpSocket* socket;
    QString serverIP;
    short serverPort;

    QString receivedData;

    bool ConnectToServer(int timeout);

signals:
    void on_receivedData(); // 信号先由socket传递给本类，从缓冲区读取完成后再发信号通知mainWindow取走

private slots:
    void on_receiveData();

public:
    SocketTCP(QString serverIP, short serverPort);
    ~SocketTCP();

    QString GetReceivedData();
    bool SendTextMsg(QString msg);
};

#endif // SOCKET_TCP_H
