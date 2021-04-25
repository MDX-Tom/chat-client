#ifndef CHAT_PACKET_UDP_H
#define CHAT_PACKET_UDP_H

#include <QObject>

namespace ChatPacketUDP
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

    struct TextMsgHeader
    {
        quint16 headerSize = 12; // in bytes
        quint16 packetSize; // in bytes (= headerSize + payloadSize)

        quint8 fromUserID;
        quint8 targetUserID;
        quint8 contentType = ChatContentType::TEXT;

        quint32 contentCrc32;
    };

    struct FileMsgHeader
    {
        quint16 headerSize = 20; // in bytes
        quint16 packetSize; // in bytes (= headerSize + payloadSize)

        quint8 fromUserID;
        quint8 targetUserID;
        quint8 contentType = ChatContentType::FILE;

        // 分包信息
        quint32 packetCountTotal;
        quint32 packetCountCurrent;

        quint32 contentCrc32;
    };
}

#endif // CHAT_PACKET_UDP_H
