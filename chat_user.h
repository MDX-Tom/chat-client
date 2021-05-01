#ifndef CHATUSER_H
#define CHATUSER_H

#include <QString>
#include <QVector>

class ChatUser
{
private:
    quint16 id;
    QString password;
    QString nickName;
    QVector<quint16>* friends;

    bool loggedIn;

public:
    ChatUser(quint16 id, QString password, QString nickName, QVector<quint16>* friends);
    ~ChatUser();

    quint16 getID() { return this->id; }
    QString getNickName() { return this->nickName; }
    QVector<quint16>* getFriends() { return this->friends; }

    bool LoggedIn() { return this->loggedIn; }
    void SetLoginStatus(bool status) { this->loggedIn = status; }
};

#endif // CHATUSER_H
