#include "main_window.h"
#include "ui_main_window.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QDebug>
#include <QMessageBox>
#include <QSpacerItem>


using ChatMessage::ChatContentType;
using ChatMessage::ClientMsgType;
using ChatMessage::ServerMsgType;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 创建一个Chat客户端主控实例
    this->chatClient = new ChatClient();

    QObject::connect(this->chatClient->socketUDP, SIGNAL(on_receivedData()),
            this, SLOT(handle_socketReceivedData()));

    // 设定Login状态指示文字的状态
    QPalette pe;
    pe.setColor(QPalette::WindowText, Qt::red);
    this->ui->labelLoginStatus->setPalette(pe);

    // 隐藏“退出登录”按钮
    this->ui->btnLogout->hide();

    // 显示“登录”按钮
    this->ui->btnSubmitUser->show();

    // 删除样例Widgets
    this->ui->inputUserID->clear();
    this->ui->inputPassword->clear();
    this->ui->inputTextMsg->clear();

    this->ui->verticalLayoutFriends->removeWidget(this->ui->pushButtonFriendExample);
    delete this->ui->pushButtonFriendExample;

    while (this->ui->tabChatView->count())
    {
        QWidget* tab = this->ui->tabChatView->currentWidget();
        this->ui->tabChatView->removeTab(0);
        delete tab;
    }

    // 定时请求新消息
    startTimer(this->chatClient->fetchNewContentIntervalMs); // ms
}


MainWindow::~MainWindow()
{
    delete ui;
}


/// 定时向服务器获取新消息
void MainWindow::timerEvent(QTimerEvent *event)
{
    if (!this->chatClient->LoggedIn())
    {
        // 未登录，不请求
        return;
    }

    this->chatClient->SendChatRequest();
}


/// 刷新“好友”列表显示
void MainWindow::RefreshBtnFriendsView()
{
    QWidget* scrollAreaFriendsWidget = this->ui->scrollAreaFriends_Widget;
    QVBoxLayout* verticalLayoutFriends = this->ui->verticalLayoutFriends;

    // 删除所有VerticalLayout中的PushButton

    while (QWidget* btnInLayout = verticalLayoutFriends->itemAt(1)->widget())
    {
        if (!btnInLayout->inherits("QPushButton"))
        {
            break;
        }
        verticalLayoutFriends->removeWidget(btnInLayout);
        delete btnInLayout;
    }

    if (!this->chatClient->LoggedIn())
    {
        return;
    }

    // 添加一个弹簧
    // QSpacerItem* verticalSpacer = new QSpacerItem(20, 40);
    // verticalLayoutFriends->addItem(verticalSpacer);

    if (this->chatClient->user->getFriends()->count() >= 7)
    {
        QSize s = scrollAreaFriendsWidget->size();
        scrollAreaFriendsWidget->setSizeIncrement(s.width(), s.height() + (this->chatClient->user->getFriends()->count() - 7) * 30);
    }

    // 将好友列表添加进去
    for (int i = this->chatClient->user->getFriends()->count() - 1; i >= 0; i--)
    {
        QString friendID = this->chatClient->user->getFriends()->at(i);
        QPushButton* btnFriend = new QPushButton();
        btnFriend->setText("朋友：" + friendID);
        btnFriend->setProperty("friendID", friendID); // 绑定这个按钮和好友ID
        btnFriend->setProperty("friendIDIndex", i); // 绑定这个按钮和好友序号

        // 设置信号和槽函数
        btnFriend->connect(btnFriend, SIGNAL(clicked()), this, SLOT(on_btnFriends()));

        // 添加至上部
        verticalLayoutFriends->insertWidget(1, btnFriend);
    }
}


/// 用好友刷新Tab显示
void MainWindow::AddTabChatView(QString& tabTitle, QString& friendID)
{
    QTabWidget* tabView = this->ui->tabChatView;

    QWidget* tabNew = new QWidget();
    QSize s = tabView->size();
    tabNew->setGeometry(0, 0, s.width() - 24, s.height() - 50); // 水平两端各2px，垂直上端空出30px
    tabNew->setProperty("friendID", friendID);

    QListWidget* listNew = new QListWidget();
    s = tabNew->size();
    listNew->setGeometry(10, 10, s.width() - 20, s.height() - 20);
    listNew->setProperty("tabIndex", tabView->count());
    this->chatClient->chatDialogs->insert(friendID, listNew);
    listNew->setParent(tabNew);

    tabView->addTab(tabNew, tabTitle);
}

void MainWindow::DelCurrentTabChatView()
{
    QTabWidget* tabView = this->ui->tabChatView;

    QWidget* tab = this->ui->tabChatView->currentWidget();
    QString friendID = tab->property("friendID").toString();
    tabView->removeTab(this->ui->tabChatView->currentIndex());
    delete tab;

    // 还要删除存储的聊天信息
    this->chatClient->chatDialogs->remove(friendID);
}


/// “好友”按钮的信号槽函数
void MainWindow::on_btnFriends()
{
    QPushButton* btn = dynamic_cast<QPushButton*>(sender());

    if (btn == Q_NULLPTR)
    {
        qDebug() << "按钮不存在" <<Qt::endl;
        return;
    }

    int friendIDIndex = btn->property("friendIDIndex").toInt();
    QString friendID = this->chatClient->user->getFriends()->at(friendIDIndex);
    if (friendID != btn->property("friendID").toString())
    {
        qDebug() << "好友列表顺序错误！！！" << Qt::endl;
        return;
    }

    if (this->chatClient->chatDialogs->find(friendID) != this->chatClient->chatDialogs->end())
    {
        QMessageBox::information(this, "已经打开了对话框", "好友:" + friendID + " 的聊天框已打开，请勿重复操作！");
        qDebug() << "好友对话已经打开！" << Qt::endl;
        return;
    }

    AddTabChatView(friendID, friendID);
}


/// "关闭当前对话框"按钮信号槽函数
void MainWindow::on_btnDelCurrentTab()
{
    if (this->ui->tabChatView->count() == 0)
    {
        qDebug() << "没有标签页可供删除了！" << Qt::endl;
        return;
    }

    QMessageBox::StandardButton result =
            QMessageBox::information(this, "确认提醒", "提醒：关闭当前标签页将删除和该用户的所有聊天记录！", QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::No)
    {
        // 用户反悔
        return;
    }

    QString friendID = this->ui->tabChatView->currentWidget()->property("friendID").toString();
    DelCurrentTabChatView();
}


/// “登录”按钮的信号槽函数
void MainWindow::on_btnSubmitUser()
{
    QTextDocument* docID = this->ui->inputUserID->document();
    QString inputID = docID->toPlainText();

    QTextDocument* docPassword = this->ui->inputPassword->document();
    QString inputPassword = docPassword->toPlainText();

    qDebug() << "ID: " << inputID << " ; Password: " << inputPassword << Qt::endl;

    try
    {
        this->chatClient->Login(inputID, inputPassword);
    }
    catch (QString errorString)
    {
        qDebug() << "ERROR: " << errorString << Qt::endl;
        QMessageBox::critical(this, "网络错误", "网络错误：" + errorString);
        return;
    }
}


/// “退出登录”按钮信号槽函数
void MainWindow::on_btnLogout()
{
    QMessageBox::StandardButton result =
            QMessageBox::information(this, "确认提醒", "提醒：退出登录将删除所有聊天记录！", QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::No)
    {
        // 用户反悔
        return;
    }

    try
    {
        this->chatClient->Logout();
    }
    catch (QString errorString)
    {
        qDebug() << "ERROR: " << errorString << Qt::endl;
        QMessageBox::critical(this, "网络错误",  "网络错误：" + errorString);
        return;
    }

    // 删除所有聊天tab和暂存记录
    this->chatClient->chatDialogs->clear();
    while (this->ui->tabChatView->count())
    {
        QWidget* tab = this->ui->tabChatView->currentWidget();
        this->ui->tabChatView->removeTab(0);
        delete tab;
    }
}


/// "发送消息"按钮信号槽函数
void MainWindow::on_btnSendMsg()
{
    QTextDocument* docTextMsg = this->ui->inputTextMsg->document();
    QString inputTextMsg = docTextMsg->toPlainText();

    QString targetUserID = this->ui->tabChatView->currentWidget()->property("friendID").toString();

    try
    {
        this->chatClient->SendChatContent(targetUserID, int(ChatContentType::TEXT), inputTextMsg); // TO BE REPLACED
    }
    catch (QString errorString)
    {
        qDebug() << "ERROR: " << errorString << Qt::endl;
        QMessageBox::critical(this, "发送失败：网络错误",  "发送失败，网络错误：" + errorString);
        return;
    }

    // 自动移到有新信息的Tab，并更新信息
    QDateTime timeCurrent = QDateTime::currentDateTime();
    QString time = timeCurrent.toString("yyyy-MM-dd hh-mm-ss");

    QString textToDisplay = inputTextMsg + " - (我 @ " + time + ")";
    QListWidget* chatList = this->chatClient->chatDialogs->value(targetUserID);
    QListWidgetItem* itemToAdd = new QListWidgetItem(textToDisplay);
    itemToAdd->setTextAlignment(Qt::AlignRight);

    chatList->addItem(itemToAdd);
}


///
void MainWindow::chatContentHandler(QJsonObject& jsonObject)
{
    bool hasNewContent = jsonObject.value("hasNewContent").toBool();
    if (!hasNewContent)
    {
        qDebug() << "没有新消息！" << endl;
        return;
    }

    // 极其复杂的转换过程
    QString contentArrayString = jsonObject.value("contentArray").toString();
    contentArrayString.replace('\'', '"');
    QJsonDocument contentArrayDoc;
    QVariant content = contentArrayDoc.fromJson(contentArrayString.toLocal8Bit()).toVariant();

    QVariantList contentList = content.toList();
    for (int i = 0; i < contentList.count(); i++)
    {
        QVariantMap contentMap = contentList.at(i).toMap();
        QString fromUserID = contentMap.value("fromUserID").toString();
        int contentType = contentMap.value("contentType").toInt();

        switch (contentType)
        {
        case ChatContentType::TEXT:
        {
            // TEXT
            QString contentText = contentMap.value("content").toString();

            QMap<QString, QListWidget*>* chatDialogs = this->chatClient->chatDialogs;
            if (chatDialogs->find(fromUserID) == chatDialogs->end())
            {
                // 用户还未打开和这个用户的聊天窗口
                // 帮助用户打开它
                AddTabChatView(fromUserID, fromUserID);
            }

            // 自动移到有新信息的Tab，并更新信息
            QDateTime timeCurrent = QDateTime::currentDateTime();
            QString time = timeCurrent.toString("yyyy-MM-dd hh-mm-ss"); // 收到消息的实际时间

            QString textToDisplay = "(" + time + " "+ fromUserID + ") - : " + contentText;
            QListWidget* chatList = chatDialogs->value(fromUserID);
            QListWidgetItem* itemToAdd = new QListWidgetItem(textToDisplay);
            itemToAdd->setTextAlignment(Qt::AlignLeft);

            chatList->addItem(itemToAdd);

            this->ui->tabChatView->setCurrentIndex(chatList->property("tabIndex").toInt());

            qDebug() << "从" << fromUserID << "收到聊天信息: " << contentText << Qt::endl;
            break;
        }

        default:
        {
            qDebug().noquote() << "不受支持的contentType: " << contentType << Qt::endl;
            QMessageBox::information(this, "不支持的消息类型", "收到不支持的消息类型！");
            break;
        }
        } // END_SWITCH
    }

    return;
}


///
void MainWindow::loginReplyHandler(QJsonObject& jsonObject)
{
    QString status = jsonObject.value("status").toString();
    if (status != "success")
    {
        qDebug().noquote() << "登录失败！" << Qt::endl;
        QMessageBox::critical(this, "登录错误", "登入用户失败，请检查用户ID和密码是否正确。");
        return;
    }

    // 录入获取到的用户信息
    QString thisUserID = jsonObject.value("thisUserID").toString();
    QString password = jsonObject.value("password").toString();
    QString nickName = jsonObject.value("nickName").toString();

    QJsonArray friendsArray = jsonObject.value("friends").toArray();
    QVector<QString>* friends = new QVector<QString>();
    for (int i = 0; i < friendsArray.count(); i++)
    {
        friends->append(friendsArray.at(i).toString());
    }

    delete this->chatClient->user;
    this->chatClient->user = new ChatUser(thisUserID, password, nickName, friends);

    // 确认已经登入成功后，修改“登录状态”文字
    QPalette pe;
    pe.setColor(QPalette::WindowText, Qt::green);
    this->ui->labelLoginStatus->setPalette(pe);
    this->ui->labelLoginStatus->setText("· 状态：已登录");

    // 设置“登录”按钮隐藏，“退出登录”按钮显示
    this->ui->btnLogout->show();
    this->ui->btnSubmitUser->hide();

    this->chatClient->SetLoginStatus(true);

    // 更新好友列表显示
    RefreshBtnFriendsView();

    qDebug() << "登录成功！" << Qt::endl;
}


///
void MainWindow::logoutReplyHandler(QJsonObject& jsonObject)
{
    QString status = jsonObject.value("status").toString();
    if (status != "success")
    {
        qDebug().noquote() << "登出失败！" << Qt::endl;
        QMessageBox::information(this, "退出登录错误", "登出用户失败，请检查网络连接。");
        return;
    }

    // 确认已经登出成功后，修改“登录状态”文字
    QPalette pe;
    pe.setColor(QPalette::WindowText, Qt::red);
    this->ui->labelLoginStatus->setPalette(pe);
    this->ui->labelLoginStatus->setText("· 状态：未登录");

    // 设置“退出登录”按钮隐藏，“登录”按钮显示
    this->ui->btnLogout->hide();
    this->ui->btnSubmitUser->show();

    this->chatClient->SetLoginStatus(false);
    RefreshBtnFriendsView();

    qDebug() << "登出成功！" << Qt::endl;
}


/// SocketTCP收到信息后将信息自动读入，然后触发此槽函数
void MainWindow::handle_socketReceivedData()
{
    // 解析服务器发来的Json串
    QString jsonString = this->chatClient->socketTCP->GetReceivedData();
    if (jsonString.isEmpty())
    {
        qDebug().noquote() << "收到空字符串信息！" << jsonString << Qt::endl;
        return;
    }
    qDebug().noquote() << "收到数据：" << jsonString << Qt::endl;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toLocal8Bit().data());
    if (jsonDoc.isNull())
    {
        qDebug().noquote() << "Json解析错误！数据：" << jsonString << Qt::endl;
        return;
    }

    QJsonObject jsonObject = jsonDoc.object();

    // 检查有无msgType指示
    if (jsonObject.value("msgType") == QJsonValue::Undefined)
    {
        qDebug().noquote() << "Json错误,没有发现msgType属性！数据：" << jsonString << Qt::endl;
        return;
    }

    int msgType = jsonObject.value("msgType").toInt();
    switch (msgType)
    {

    case ServerMsgType::CHAT_REQUEST_REPLY:
    {
        // CHAT_REQUEST_REPLY
        this->chatContentHandler(jsonObject);
        break;
    }

    case ServerMsgType::LOGIN_REPLY:
    {
        // LOGIN_REPLY
        this->loginReplyHandler(jsonObject);
        break;
    }

    case ServerMsgType::LOGOUT_REPLY:
    {
        // LOGOUT_REPLY
        this->logoutReplyHandler(jsonObject);
        break;
    }

    default:
    {
        qDebug().noquote() << "服务器发来无法解析的msgType: " << msgType << Qt::endl;
        QMessageBox::information(this, "网络出错", "服务器发来无法解析的msgType: " + QString(msgType));
        break;
    }
    } // END_SWITCH

    return;
}
