#ifndef CHATUSER_H
#define CHATUSER_H

#define MAX_FRIEND_COUNT 31

#include <QString>
#include <QVector>

class ChatUser
{
private:
    QString id;
    QString password;
    QString nickName;
    QVector<QString>* friends;

public:
    ChatUser(QString id, QString password, QString nickName, QVector<QString>* friends);
    ~ChatUser();

    QString getID() { return this->id; }
    QString getNickName() { return this->nickName; }
    QVector<QString>* getFriends() { return this->friends; }
};

#endif // CHATUSER_H
