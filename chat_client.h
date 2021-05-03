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
    // 超时重传等待时间
    const int waitForReplyMs = 1000; // 可以添加自适应算法
    // 超时重传次数
    const quint8 retryCountMax = 3;
    // 收到的ACK哈希值
    QByteArray receivedACKHash;

    Ui::MainWindow *ui;
    SendFileDialog* sendFileDialog;

    void RefreshBtnFriendsView();
    void AddTabChatView(QString& tabTitle, quint16 friendUserID);
    void DelCurrentTabChatView();

public slots:
    void UDPReceiveHandler();

private slots:
    void on_btnSubmitUser();
    void on_btnLogout();
    void on_btnFriends();
    void on_btnSendMsg();
    void on_btnSendFile();
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

    void RefreshSendFileProgress(float); // 指示文件发送进度
};

#endif // CHAT_CLIENT_H
