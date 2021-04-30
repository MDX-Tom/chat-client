#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "chat_client.h"

#include <QMainWindow>
#include <QTimerEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handle_socketReceivedData();
    void on_btnSubmitUser();
    void on_btnLogout();
    void on_btnFriends();
    void on_btnSendMsg();
    void on_btnDelCurrentTab();

private:
    Ui::MainWindow *ui;
    ChatClient* chatClient;

    virtual void timerEvent(QTimerEvent* event);

    void RefreshBtnFriendsView();
    void AddTabChatView(QString& tabTitle, QString& friendUserID);
    void DelCurrentTabChatView();
};

#endif // MAIN_WINDOW_H
