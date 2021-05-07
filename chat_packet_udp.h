#ifndef CHAT_PACKET_UDP_H
#define CHAT_PACKET_UDP_H

#include <QObject>

class ChatPacketUDP
{
public:
    enum ChatContentType
    {
        TEXT = 0, FILE = 1,
    };

    enum ClientMsgType
    {
        MSG_CLIENT_ACK = 10, // Reliable UDP

        CHAT_CONTENT_CLIENT = 20,

        LOGIN_REQUEST = 30, LOGOUT_REQUEST = 40,

        CHAT_REQUEST = 100,
    };

    enum ServerMsgType
    {
        MSG_SERVER_ACK = 11, // Reliable UDP

        CHAT_CONTENT_SERVER = 21,

        LOGIN_REPLY = 31, LOGOUT_REPLY = 41,

        CHAT_REQUEST_REPLY = 101,
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
        const quint16 headerSize = sizeof(LoginRequestHeader); // in bytes
        const quint16 packetSize = sizeof(LoginRequestHeader); // in bytes (= headerSize + payloadSize)
        const quint8 msgType = ClientMsgType::LOGIN_REQUEST;

        quint16 thisUserID;
        quint8 password[28];
    };

    struct LogoutRequestHeader
    {
        const quint16 headerSize = sizeof(LogoutRequestHeader); // in bytes
        const quint16 packetSize = sizeof(LogoutRequestHeader); // in bytes (= headerSize + payloadSize)
        const quint8 msgType = ClientMsgType::LOGOUT_REQUEST;

        quint16 thisUserID;
    };

    struct TextMsgHeader
    {
        const quint16 headerSize = sizeof(TextMsgHeader); // in bytes
        quint16 packetSize; // in bytes (= headerSize + payloadSize)
        const quint8 msgType = ClientMsgType::CHAT_CONTENT_CLIENT;
        // BLANK BYTE

        quint16 fromUserID;
        quint16 targetUserID;
        const quint8 contentType = ChatContentType::TEXT;
        // BLANK BYTE

        quint16 packetSeq;
    };

    struct FileMsgHeader
    {
        const quint16 headerSize = sizeof(FileMsgHeader); // in bytes
        quint16 packetSize; // in bytes (= headerSize + payloadSize)
        const quint8 msgType = ClientMsgType::CHAT_CONTENT_CLIENT;
        // BLANK BYTE

        quint16 fromUserID;
        quint16 targetUserID;
        const quint8 contentType = ChatContentType::FILE;
        // BLACK BYTE

        quint16 packetSeq;
        quint8 fileNameLength;
        // BLANK BYTE

        // 分包信息
        quint32 packetCountTotal;
        quint32 packetCountCurrent;

        // 包后紧跟fineName
        // 然后才是payload
    };

    struct ChatRequestHeader
    {
        const quint16 headerSize = sizeof(ChatRequestHeader); // in bytes
        const quint16 packetSize = sizeof(ChatRequestHeader); // in bytes (= headerSize + payloadSize)
        const quint8 msgType = ClientMsgType::CHAT_REQUEST;

        quint16 thisUserID;
    };

    //------------------------SERVER HEADERS-------------------------//

    struct PacketReplyHeader
    {
        const quint16 headerSize = sizeof(PacketReplyHeader);
        const quint16 packetSize = sizeof(PacketReplyHeader);
        const quint8 msgType = ServerMsgType::MSG_SERVER_ACK;

        // const quint8 placeHolder = 0; // 字节对齐
        // unsigned char md5Hash[16];

        quint16 packetSeq = 0;
    };

    struct LoginReplyHeader
    {
        const quint16 headerSize = sizeof(LoginReplyHeader);
        quint16 packetSize;
        const quint8 msgType = ServerMsgType::LOGIN_REPLY;

        quint16 loginUserID;
        quint8 status;

        quint16 friendCount;
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

    static bool isPacketReplyMsg(QByteArray& bytes)
    {
        HeaderBase headerBase = *(HeaderBase*)bytes.data();
        return (headerBase.msgType == ServerMsgType::MSG_SERVER_ACK &&
                headerBase.packetSize == sizeof(PacketReplyHeader) &&
                headerBase.headerSize == sizeof(PacketReplyHeader));
    }

    static bool isLoginReplyMsg(QByteArray& bytes)
    {
        HeaderBase headerBase = *(HeaderBase*)bytes.data();
        return (headerBase.msgType == ServerMsgType::LOGIN_REPLY &&
                headerBase.headerSize == sizeof(LoginReplyHeader));
    }

    static bool isLogoutReplyMsg(QByteArray& bytes)
    {
        HeaderBase headerBase = *(HeaderBase*)bytes.data();
        return (headerBase.msgType == ServerMsgType::LOGOUT_REPLY &&
                headerBase.headerSize == sizeof(LogoutReplyHeader) &&
                headerBase.packetSize == sizeof(LogoutReplyHeader));
    }

    static bool isChatRequestReplyMsg(QByteArray& bytes)
    {
        HeaderBase headerBase = *(HeaderBase*)bytes.data();
        return (headerBase.msgType == ServerMsgType::CHAT_REQUEST_REPLY &&
                headerBase.headerSize == sizeof(ChatRequestReplyHeader));
    }
};

#endif // CHAT_PACKET_UDP_H
