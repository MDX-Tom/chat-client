#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include "chat_user.h"
#include "socket_tcp.h"

#include <QJsonObject>
#include <QListWidget>
#include <QMap>

namespace ChatMessage
{
    enum ChatContentType
    {
        TEXT = 0,
    };

    enum ClientMsgType
    {
        CHAT_CONTENT_CLIENT = 0,
        LOGIN_REQUEST = 2, LOGOUT_REQUEST = 3,
        CHAT_REQUEST = 99,
    };

    enum ServerMsgType
    {
        // CHAT_CONTENT_SERVER = 0,
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

    const QString serverIP = "10.128.206.236";
    const short serverPort = 8002;
    const short fetchNewContentIntervalMs = 2000;

    ChatUser* user;
    SocketTCP* socketTCP;
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
