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

        CHAT_REQUEST_REPLY = 99,
    };

    enum Status
    {
        SUCCESS = 1,

        ERROR_PASSWORD_WRONG = 10,
        ERROR_CONFLICT = 11, // 重复操作

        ERROR = 99
    };

    struct HeaderBase
    {
        quint16 headerSize;
        quint16 packetSize;
        quint8 msgType;
    };

    //------------------------CLIENT HEADERS-------------------------//

    struct LoginRequestHeader
    {
        quint16 headerSize = sizeof(LoginRequestHeader); // in bytes
        quint16 packetSize; // in bytes (= headerSize + payloadSize)
        quint8 msgType = ClientMsgType::LOGIN_REQUEST;

        quint16 thisUserID;
        quint8 password[25];
    };

    struct LogoutRequestHeader
    {
        quint16 headerSize = sizeof(LogoutRequestHeader); // in bytes
        quint16 packetSize; // in bytes (= headerSize + payloadSize)
        quint8 msgType = ClientMsgType::LOGOUT_REQUEST;

        quint16 thisUserID;
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

    struct ChatRequestHeader
    {
        quint16 headerSize = sizeof(ChatRequestHeader); // in bytes
        quint16 packetSize; // in bytes (= headerSize + payloadSize)
        quint8 msgType = ClientMsgType::CHAT_REQUEST;

        quint16 thisUserID;
    };

    //------------------------SERVER HEADERS-------------------------//

    struct PacketReplyHeader
    {
        const quint16 headerSize = sizeof(PacketReplyHeader);
        const quint16 packetSize = sizeof(PacketReplyHeader);
        const quint8 msgType = ServerMsgType::MSG_SERVER_ACK;

        unsigned char md5Hash[16];
    };

    struct LoginReplyHeader
    {
        const quint16 headerSize = sizeof(LoginReplyHeader);
        const quint16 packetSize = sizeof(LoginReplyHeader);
        const quint8 msgType = ServerMsgType::LOGIN_REPLY;

        quint16 loginUserID;
        quint8 status;
    };

    struct LogoutReplyHeader
    {
        const quint16 headerSize = sizeof(LogoutReplyHeader);
        const quint16 packetSize = sizeof(LogoutReplyHeader);
        const quint8 msgType = ServerMsgType::LOGOUT_REPLY;

        quint16 logoutUserID;
        quint8 status;
    };

    struct ChatRequestReplyHeader
    {
        const quint16 headerSize = sizeof(ChatRequestReplyHeader);
        quint16 packetSize;
        const quint8 msgType = ServerMsgType::CHAT_REQUEST_REPLY;

        quint16 thisUserID;
        quint16 pendingMsgTotalCount;
    };

    struct ChatContentServerHeader
    {
        // 为ChatRequestReplyHeader的次级Header
        const quint16 headerSize = sizeof(ChatContentServerHeader);
        quint16 currentPacketSize;
        const quint8 msgType = ServerMsgType::CHAT_CONTENT_SERVER;
        quint8 contentType = ChatContentType::TEXT;

        quint16 fromUserID;
        quint16 pendingMsgTotalCount;
        quint16 currentMsgCount;
    };

    //------------------------HEADER FUNCTIONS-------------------------//

    bool isPacketReplyMsg(QByteArray& bytes)
    {
        HeaderBase headerBase = *(HeaderBase*)bytes.data();
        return (headerBase.msgType == ServerMsgType::MSG_SERVER_ACK &&
                headerBase.packetSize == sizeof(PacketReplyHeader) &&
                headerBase.headerSize == sizeof(PacketReplyHeader));
    }

    bool isLoginReplyMsg(QByteArray& bytes)
    {
        HeaderBase headerBase = *(HeaderBase*)bytes.data();
        return (headerBase.msgType == ServerMsgType::LOGIN_REPLY &&
                headerBase.headerSize == sizeof(LoginReplyHeader) &&
                headerBase.packetSize == sizeof(LoginReplyHeader));
    }

    bool isLogoutReplyMsg(QByteArray& bytes)
    {
        HeaderBase headerBase = *(HeaderBase*)bytes.data();
        return (headerBase.msgType == ServerMsgType::LOGOUT_REPLY &&
                headerBase.headerSize == sizeof(LogoutReplyHeader) &&
                headerBase.packetSize == sizeof(LogoutReplyHeader));
    }

    bool isChatRequestReplyMsg(QByteArray& bytes)
    {
        HeaderBase headerBase = *(HeaderBase*)bytes.data();
        return (headerBase.msgType == ServerMsgType::CHAT_REQUEST_REPLY &&
                headerBase.headerSize == sizeof(ChatRequestReplyHeader));
    }
}

#endif // CHAT_PACKET_UDP_H
