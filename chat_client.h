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

    bool Login(QString id, QString password);
    bool LoggedIn();
    void SetLoginStatus(bool);
    bool Logout();

    bool SendChatContent(QString targetUserID, int contentType, QString content);
    bool SendChatRequest();
};

#endif // CHAT_CLIENT_H
