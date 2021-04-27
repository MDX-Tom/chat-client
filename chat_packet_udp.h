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
        MSG_CLIENT_ACK = 1, // Reliable UDP

        CHAT_CONTENT_CLIENT = 0,
        LOGIN_REQUEST = 2, LOGOUT_REQUEST = 3,
        CHAT_REQUEST = 99,
    };

    enum ServerMsgType
    {
        MSG_SERVER_ACK = 1, // Reliable UDP

        CHAT_CONTENT_SERVER = 0,
        LOGIN_REPLY = 2, LOGOUT_REPLY = 3,
        CHAT_REQUEST_REPLY = 99
    };

    struct HeaderBase
    {
        quint16 headerSize;
        quint16 packetSize;
        quint8 msgType;
    };

    struct TextMsgHeader
    {
        quint16 headerSize = sizeof(TextMsgHeader); // in bytes
        quint16 packetSize; // in bytes (= headerSize + payloadSize)
        quint8 msgType = ClientMsgType::CHAT_CONTENT_CLIENT;

        quint16 fromUserID;
        quint16 targetUserID;
        quint8 contentType = ChatContentType::TEXT;
    };

    struct FileMsgHeader
    {
        quint16 headerSize = sizeof(FileMsgHeader); // in bytes
        quint16 packetSize; // in bytes (= headerSize + payloadSize)
        quint8 msgType = ClientMsgType::CHAT_CONTENT_CLIENT;

        quint16 fromUserID;
        quint16 targetUserID;
        quint8 contentType = ChatContentType::FILE;

        // 分包信息
        quint32 packetCountTotal;
        quint32 packetCountCurrent;
    };

    struct PacketReplyHeader
    {
        quint16 headerSize = sizeof(PacketReplyHeader);
        quint16 packetSize = sizeof(PacketReplyHeader);
        quint8 msgType = ServerMsgType::MSG_SERVER_ACK;

        unsigned char md5Hash[16];
    };

    bool isPacketReplyMsg(QByteArray& bytes)
    {
        HeaderBase headerBase = *(HeaderBase*)bytes.data();
        return (headerBase.headerSize == sizeof(PacketReplyHeader) &&
                headerBase.packetSize == sizeof(PacketReplyHeader) &&
                headerBase.msgType == ServerMsgType::MSG_SERVER_ACK
                );
    }
}

#endif // CHAT_PACKET_UDP_H
