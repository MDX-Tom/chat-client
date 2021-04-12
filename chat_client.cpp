#include "chat_client.h"

#include "chat_user.h"
#include "socket_tcp.h"

#include <QJsonDocument>

using ChatMessage::ChatContentType;
using ChatMessage::ClientMsgType;
using ChatMessage::ServerMsgType;

ChatClient::ChatClient()
{
    this->socketTCP = new SocketTCP(this->serverIP, this->serverPort);
    this->user = new ChatUser("99999", "pswd_99999", "name_99999", new QVector<QString>());
    this->chatDialogs = new QMap<QString, QListWidget*>();

    this->loggedIn = false;

    // jsonMsg模版
    this->jsonSendMsg = new QJsonObject();
    this->jsonSendMsg->insert("thisUserID", QJsonValue("00001"));
    this->jsonSendMsg->insert("msgType", QJsonValue(ClientMsgType::CHAT_CONTENT_CLIENT));

    this->jsonSendMsg->insert("targetUserID", QJsonValue("00002"));
    this->jsonSendMsg->insert("contentType", QJsonValue(ChatContentType::TEXT));
    this->jsonSendMsg->insert("content", QJsonValue("This is a test message from user 00001."));
}


ChatClient::~ChatClient()
{
    delete this->chatDialogs;
    delete this->jsonSendMsg;
    delete this->user;
    delete this->socketTCP;
}


bool ChatClient::LoggedIn()
{
    return this->loggedIn;
}


void ChatClient::SetLoginStatus(bool status)
{
    this->loggedIn = status;
}


/// 向服务器发送LOGIN_REQUEST请求
bool ChatClient::Login(QString id, QString password)
{
    if (this->loggedIn)
    {
        // 已经登录
        qDebug() << "已经登录！登录操作无效！" << Qt::endl;
        return false;
    }

    // 准备要发送的Json串
    delete this->jsonSendMsg;
    this->jsonSendMsg = new QJsonObject();

    this->jsonSendMsg->insert("thisUserID", QJsonValue(id));
    this->jsonSendMsg->insert("msgType", QJsonValue(ClientMsgType::LOGIN_REQUEST));

    this->jsonSendMsg->insert("password", QJsonValue(password));

    QJsonDocument jsonDoc = QJsonDocument(*this->jsonSendMsg);
    QString jsonStr = QString(jsonDoc.toJson(QJsonDocument::Compact)); // 紧凑型输出，节省网络资源

    // 通过Socket发送Json请求串
    return this->socketTCP->SendTextMsg(jsonStr);
}


/// 向服务器发送LOGOUT_REQUEST请求
bool ChatClient::Logout()
{
    if (!this->loggedIn)
    {
        // 还没有登录
        qDebug() << "还未登录！退出操作无效！" << Qt::endl;
        return false;
    }

    // 准备要发送的Json串
    delete this->jsonSendMsg;
    this->jsonSendMsg = new QJsonObject();

    this->jsonSendMsg->insert("thisUserID", QJsonValue(this->user->getID()));
    this->jsonSendMsg->insert("msgType", QJsonValue(ClientMsgType::LOGOUT_REQUEST));

    QJsonDocument jsonDoc = QJsonDocument(*this->jsonSendMsg);
    QString jsonStr = QString(jsonDoc.toJson(QJsonDocument::Compact)); // 紧凑型输出，节省网络资源

    // 通过Socket发送Json请求串
    return this->socketTCP->SendTextMsg(jsonStr);
}


/// 向服务器（对象用户）发送CHAT_CONTENT
bool ChatClient::SendChatContent(QString targetUserID, int contentType, QString content)
{
    if (!this->loggedIn)
    {
        // 尚未登录，不发送
        qDebug() << "尚未登录！发送消息无效！" << Qt::endl;
        return false;
    }

    // 准备要发送的Json串
    delete this->jsonSendMsg;
    this->jsonSendMsg = new QJsonObject();

    this->jsonSendMsg->insert("thisUserID", QJsonValue(this->user->getID()));
    this->jsonSendMsg->insert("targetUserID", targetUserID);
    this->jsonSendMsg->insert("msgType", QJsonValue(ClientMsgType::CHAT_CONTENT_CLIENT));

    this->jsonSendMsg->insert("contentType", contentType);
    this->jsonSendMsg->insert("content", content);

    QJsonDocument jsonDoc = QJsonDocument(*this->jsonSendMsg);
    QString jsonStr = QString(jsonDoc.toJson(QJsonDocument::Compact)); // 紧凑型输出，节省网络资源

    // 通过Socket发送Json请求串
    return this->socketTCP->SendTextMsg(jsonStr);
}


/// 向服务器发送CHAT_REQUEST请求
bool ChatClient::SendChatRequest()
{
    if (!this->loggedIn)
    {
        // 尚未登录，不发送
        qDebug() << "尚未登录！获取新消息指令无效！" << Qt::endl;
        return false;
    }

    // 准备要发送的Json串
    delete this->jsonSendMsg;
    this->jsonSendMsg = new QJsonObject();

    this->jsonSendMsg->insert("thisUserID", QJsonValue(this->user->getID()));
    this->jsonSendMsg->insert("msgType", QJsonValue(ClientMsgType::CHAT_REQUEST));

    QJsonDocument jsonDoc = QJsonDocument(*this->jsonSendMsg);
    QString jsonStr = QString(jsonDoc.toJson(QJsonDocument::Compact)); // 紧凑型输出，节省网络资源

    // 通过Socket发送Json请求串
    return this->socketTCP->SendTextMsg(jsonStr);
}
