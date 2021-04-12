#include "chat_user.h"

ChatUser::ChatUser(QString id, QString password, QString nickName, QVector<QString>* friends)
{
    this->id = id;
    this->password = password;

    this->nickName = nickName;
    this->friends = friends;
}

ChatUser::~ChatUser()
{
    delete this->friends;
}
