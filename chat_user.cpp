#include "chat_user.h"

ChatUser::ChatUser(quint16 id, QString password, QString nickName, QVector<quint16>* friends)
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
