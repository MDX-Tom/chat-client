#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include "chat_user.h"
#include "socket_udp.h"
#include "ui_chat_client.h"
#include "send_file_dialog.h"

#include <QMainWindow>
#include <QTimerEvent>
#include <QListWidget>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class ChatClient : public QMainWindow
{
    Q_OBJECT

private:
    // 文件预读取
    bool preloadFile = false;

    // 强制传完
    bool forceSendFileFinish = true;

    // 超时重传等待时间
    double waitForReplyMs = 80; // 可以添加自适应算法
    // 超时重传次数
    const quint8 retryCountMax = 20;
    // 发多少个包等待ACK
    quint16 waitForReplyCount = 165; // 可以添加自适应算法，< 65535

    // 收到的ACK哈希值
    // QByteArray receivedACKHash;

    // TEXT发包Seqence
    quint16 sendPacketSeq = 0;
    // TEXT收到包的Sequence
    quint16 ackPacketSeq = 65535;

    // 存储的文件Packet,以文件分片号[i]为索引
    QMap<quint32, QByteArray> filePacketBytes;
    // 滑动窗口内是否收到了ack包,以文件分片号[i]为索引
    QMap<quint32, bool> ackValid;

    Ui::MainWindow *ui;
    SendFileDialog* sendFileDialog;

    void RefreshBtnFriendsView();
    void AddTabChatView(QString& tabTitle, quint16 friendUserID);
    void DelCurrentTabChatView();

signals:
    void sendFileProgress(int);

public slots:
    void UDPReceiveHandler();

private slots:
    void on_btnSubmitUser();
    void on_btnLogout();
    void on_btnFriends();

    void on_btnSendMsg();
    void btnSendMsg(); // 新线程发送

    void on_btnSendFile();
    void sendFile(); // 经FileDialog激活后，开新线程发送

    void on_btnDelCurrentTab();

    void on_btnExit();

public:
    ChatClient(QWidget *parent = nullptr);
    ~ChatClient();

    const short fetchNewContentIntervalMs = 1000;

    virtual void timerEvent(QTimerEvent* event);

    ChatUser* user;
    SocketUDP* socketUDP;

    QMap<quint16, QListWidget*>* chatDialogs;

    //------------------------基本操作------------------------------

    bool LoggedIn() { return this->user->LoggedIn(); }
    void SetLoginStatus(bool status) { this->user->SetLoginStatus(status); }

    bool Login(quint16 id, QString password);
    bool Logout();

    bool SendText(QString textContent, quint16 targetUserID);

    bool SendFile(QString& fileNameWithPath, QHostAddress targetAddr, quint16 targetPort, quint16 targetUserID);
    bool SendFile(QString& fileNameWithPath, quint16 targetUserID);

    bool SendChatRequest();

    //-------------------------UI 操作-----------------------------
};

#endif // CHAT_CLIENT_H
