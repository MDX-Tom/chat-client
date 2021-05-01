#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include "chat_user.h"
#include "socket_udp.h"
#include "main_window.h"

#include <QTimerEvent>
#include <QListWidget>
#include <QMap>


class ChatClient : public QObject
{
    Q_OBJECT

signals:
    void sendFileProgress(float); // 指示文件发送进度

public:
    ChatClient(MainWindow* mainWindow);
    ~ChatClient();

    const short fetchNewContentIntervalMs = 1000;

    virtual void timerEvent(QTimerEvent* event);

    ChatUser* user;
    SocketUDP* socketUDP;
    MainWindow* mainWindow;

    QMap<quint16, QListWidget*>* chatDialogs;

    bool LoggedIn() { return this->user->LoggedIn(); }
    void SetLoginStatus(bool status) { this->user->SetLoginStatus(status); }

    bool Login(quint16 id, QString password);
    bool Logout();

    bool SendText(QString textContent, quint16 targetUserID);

    bool SendFile(QString& fileNameWithPath, QHostAddress targetAddr, quint16 targetPort, quint16 targetUserID);
    bool SendFile(QString& fileNameWithPath, quint16 targetUserID);

    bool SendChatRequest();
};

#endif // CHAT_CLIENT_H
