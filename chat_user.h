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

public:
    ChatUser(quint16 id, QString password, QString nickName, QVector<quint16>* friends);
    ~ChatUser();

    quint16 getID() { return this->id; }
    QString getNickName() { return this->nickName; }
    QVector<quint16>* getFriends() { return this->friends; }
};

#endif // CHATUSER_H
