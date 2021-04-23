#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include "chat_user.h"
#include "socket_udp.h"

#include <QJsonObject>
#include <QListWidget>
#include <QMap>

namespace ChatMessage
{
    enum ChatContentType
    {
        TEXT = 0, FILE = 1,
    };

    enum ClientMsgType
    {
        CHAT_CONTENT_CLIENT = 0,
        LOGIN_REQUEST = 2, LOGOUT_REQUEST = 3,
        CHAT_REQUEST = 99,
    };

    enum ServerMsgType
    {
        CHAT_CONTENT_SERVER = 0,
        LOGIN_REPLY = 2, LOGOUT_REPLY = 3,
        CHAT_REQUEST_REPLY = 99
    };
}

class ChatClient
{
private:
    bool loggedIn;

public:
    ChatClient();
    ~ChatClient();

    const short fetchNewContentIntervalMs = 2000;

    ChatUser* user;
    SocketUDP* socketUDP;
    QJsonObject* jsonSendMsg;
    QMap<QString, QListWidget*>* chatDialogs;

    bool Login(QString id, QString password);
    bool LoggedIn();
    void SetLoginStatus(bool);
    bool Logout();

    bool SendChatContent(QString targetUserID, int contentType, QString content);
    bool SendChatRequest();
};

#endif // CHAT_CLIENT_H
