#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include "chat_user.h"
#include "socket_udp.h"

#include <QJsonObject>
#include <QListWidget>
#include <QMap>


class ChatClient
{
private:
    bool loggedIn;

public:
    ChatClient();
    ~ChatClient();

    const short fetchNewContentIntervalMs = 1000;

    ChatUser* user;
    SocketUDP* socketUDP;

    QMap<QString, QListWidget*>* chatDialogs;

    bool Login(quint16 id, QString password);
    bool LoggedIn();
    void SetLoginStatus(bool);
    bool Logout();

    bool SendText(QString textContent, quint16 targetUserID);
    bool SendFile(QString& fileNameWithPath, QHostAddress targetAddr, quint16 targetPort, quint16 targetUserID);
    bool SendFile(QString& fileNameWithPath, quint16 targetUserID);

    bool SendChatRequest();
};

#endif // CHAT_CLIENT_H
