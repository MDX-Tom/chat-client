#include "chat_client.h"
#include "ui_chat_client.h"

#include "chat_user.h"
#include "socket_udp.h"
#include "chat_packet_udp.h"

#include <QFile>
#include <QTimer>
#include <QDateTime>
#include <QCryptographicHash>
#include <QMessageBox>

#include <QMainWindow>

ChatClient::ChatClient(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , sendFileDialog(new SendFileDialog)
{
    this->ui->setupUi(this);

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

    // 初始化
    this->socketUDP = new SocketUDP(); 
    QObject::connect(this->socketUDP, SIGNAL(on_receivedData()),
                     this, SLOT(UDPReceiveHandler()));
    this->user = new ChatUser(00000, "pswd_00000", "name_00000", new QVector<quint16>());
    this->chatDialogs = new QMap<quint16, QListWidget*>();

    // 定时请求新消息
    startTimer(this->fetchNewContentIntervalMs); // ms
}


ChatClient::~ChatClient()
{
    delete this->ui;

    delete this->chatDialogs;
    delete this->user;
    delete this->socketUDP;
}


/// 定时向服务器获取新消息
void ChatClient::timerEvent(QTimerEvent *event)
{
    if (!this->LoggedIn())
    {
        // 未登录，不请求
        return;
    }

    this->SendChatRequest();
}


//-----------------------------------发送函数----------------------------------


/// 向服务器发送LOGIN_REQUEST请求
bool ChatClient::Login(quint16 id, QString password)
{
    if (this->user->LoggedIn())
    {
        // 已经登录
        throw QString("已经登录！登录操作无效！");
        return false;
    }

    // 写入Header信息
    ChatPacketUDP::LoginRequestHeader header;
    header.thisUserID = id;
    QByteArray passwordBytes = password.toLatin1();
    quint16 sizeBefore = passwordBytes.size();
    passwordBytes.resize(sizeof(header.password));
    memcpy(header.password, passwordBytes.data(), sizeof(header.password));
    memset(header.password + sizeBefore, 0, sizeof(header.password) - sizeBefore);

    // 打包Header
    QByteArray bytesPacket;
    bytesPacket.resize(header.packetSize);
    memcpy(bytesPacket.data(), &header, sizeof(header));

    // 通过Socket发送请求Packet
    return this->socketUDP->SendPackedBytes(bytesPacket);
}


/// 向服务器发送LOGOUT_REQUEST请求
bool ChatClient::Logout()
{
    if (!this->user->LoggedIn())
    {
        // 还没有登录
        throw QString("还未登录！退出操作无效！");
        return false;
    }

    // 写入Header信息
    ChatPacketUDP::LogoutRequestHeader header;
    header.thisUserID = this->user->getID();

    // 打包Header
    QByteArray bytesPacket;
    bytesPacket.resize(header.packetSize);
    memcpy(bytesPacket.data(), &header, sizeof(header));

    // 通过Socket发送请求Packet
    return this->socketUDP->SendPackedBytes(bytesPacket);
}


/// 向服务器（对象用户）发送CHAT_CONTENT(TEXT)
bool ChatClient::SendText(QString Text, quint16 targetUserID)
{
    if (!this->user->LoggedIn())
    {
        // 尚未登录，不发送
        throw QString("尚未登录！发送消息无效！");
        return false;
    }

    // 将QString转化为QBytearray（UTF-8）
    QByteArray bytesText = Text.toLocal8Bit();

    // 准备Header
    ChatPacketUDP::TextMsgHeader header;
    header.packetSize = sizeof(header) + bytesText.size();
    if (header.packetSize > this->socketUDP->MaxPacketSize())
    {
        // 是否要分包处理超长信息？
        throw QString("消息过长...");
        return false;
    }
    header.fromUserID = this->user->getID();
    header.targetUserID = targetUserID;

    // 打包Header
    QByteArray bytesToSend;
    bytesToSend.resize(header.packetSize);
    memcpy(bytesToSend.data(), &header, sizeof(header));
    memcpy(bytesToSend.data() + sizeof(header), bytesText.data(), bytesText.size());

    // 通过Socket发送请求Packet
    return this->socketUDP->SendPackedBytes(bytesToSend);
}


/// 向服务器（对象用户）发送文件
bool ChatClient::SendFile
                        (QString &fileNameWithPath,
                         QHostAddress targetAddr,
                         quint16 targetPort,
                         quint16 targetUserID = 0)
{
    qDebug().noquote() << "Selected file: " << fileNameWithPath << Qt::endl;

    // 打开文件
    QFile file(fileNameWithPath);
    if (!file.open(QIODevice::ReadOnly))
    {
        throw "打开文件失败...";
        file.close();
        return false;
    }

    // 计算分包个数
    qint64 bytesTotal = file.size(); // 文件总字节数
    if (bytesTotal >= INT32_MAX)
    {
        throw "文件过大，最大只支持4GB...";
        file.close();
        return false;
    }
    qint16 bytesCountPerPacket = this->socketUDP->MaxPacketSize() - sizeof(ChatPacketUDP::FileMsgHeader); // 单包包含的文件字节数
    qint16 packetCountTotal = bytesTotal / bytesCountPerPacket + 1; // 分包数量

    // 分包发送
    for (int i = 0; i < packetCountTotal; i++)
    {
        // 处理包头
        ChatPacketUDP::FileMsgHeader header;
        header.packetSize = sizeof(header) + bytesCountPerPacket; // == this->maxPayloadSize
        header.fromUserID = this->user->getID();
        header.targetUserID = targetUserID;
        header.packetCountTotal = packetCountTotal;
        header.packetCountCurrent = i + 1;

        // 写入Header和Payload到QByteArray
        QByteArray bytesPacket;
        QByteArray bytesFile = file.read(bytesCountPerPacket);
        header.packetSize = sizeof(header) + bytesFile.size();
        bytesPacket.resize(header.packetSize);
        memcpy(bytesPacket.data(), &header, sizeof(header)); // sizeof(header) == header.headerSize
        memcpy(bytesFile.data() + sizeof(header), &header, bytesFile.size()); // bytesFile.size() == bytesCountPerPacket

        // 更新进度指示信号
        float progress = 1.0 * i / packetCountTotal;
        this->RefreshSendFileProgress(progress);

        quint8 retrySeq = 0;
        while (retrySeq++)
        {
            // 重发次数超过MAX
            if (retrySeq > this->retryCountMax)
            {
                throw QString("网络错误：连接超时。重传次数已达：") + QString(this->retryCountMax);
                return false;
            }

            this->socketUDP->SendPackedBytes(bytesPacket, targetAddr, targetPort);

            // 发送前MD5Hash
            QByteArray sentHash = QCryptographicHash::hash(bytesPacket, QCryptographicHash::Md5);

            // 等待回包
            bool toggleTimeout = false;
            bool ackValid = false;
            QTimer::singleShot(this->waitForReplyMs, [toggleTimeout] () mutable { toggleTimeout = true; });
            while (!toggleTimeout)
            {
                // 校验收到的包
                if (this->receivedACKHash == sentHash)
                {
                    ackValid = true;
                    break;
                }
            }

            // 判断是否需要重传
            if (ackValid)
            {
                break;
            }
        }

    }

    file.close();

    return true;
}
bool ChatClient::SendFile
                        (QString &fileNameWithPath,
                         quint16 targetUserID = 0)
{
    QHostAddress a = this->socketUDP->ServerAddr();
    quint16 p = this->socketUDP->ServerPort();
    return SendFile(fileNameWithPath, a, p, targetUserID);
}


/// 向服务器发送CHAT_REQUEST请求
bool ChatClient::SendChatRequest()
{
    if (!this->LoggedIn())
    {
        // 尚未登录，不发送
        throw QString("尚未登录！获取新消息指令无效！");
        return false;
    }

    // 准备Header
    ChatPacketUDP::ChatRequestHeader header;
    header.thisUserID = this->user->getID();

    // 打包Header
    QByteArray bytesToSend;
    bytesToSend.resize(header.packetSize);
    memcpy(bytesToSend.data(), &header, sizeof(header));

    // 通过Socket发送请求Packet
    return this->socketUDP->SendPackedBytes(bytesToSend);
}


//-----------------------------------接收函数----------------------------------


/// 槽函数，处理收到的UDP包
void ChatClient::UDPReceiveHandler()
{
    QHostAddress addr = this->socketUDP->GetReceivedAddr();
    quint16 port = this->socketUDP->GetReceivedPort();
    QByteArray dataBytes = this->socketUDP->GetReceivedData();

    //
    if (ChatPacketUDP::isPacketReplyMsg(dataBytes))
    {
        ChatPacketUDP::PacketReplyHeader ackHeader = *(ChatPacketUDP::PacketReplyHeader*)dataBytes.data();

        this->receivedACKHash.resize(16);
        memcpy(this->receivedACKHash.data(), ackHeader.md5Hash, 16);
    }

    //
    else if (ChatPacketUDP::isChatRequestReplyMsg(dataBytes))
    {
        ChatPacketUDP::ChatRequestReplyHeader header = *(ChatPacketUDP::ChatRequestReplyHeader*)dataBytes.data();

        if (header.pendingMsgTotalCount == 0)
        {
            qDebug() << "没有新消息！" << Qt::endl;
            return;
        }

        quint16 dealedPayloadSize = 0;
        for (int i = 0; i < header.pendingMsgTotalCount; i++)
        {
            char* currentPosition = dataBytes.data() + header.headerSize + dealedPayloadSize;
            ChatPacketUDP::ChatContentServerHeader currentContentHeader =
                    *(ChatPacketUDP::ChatContentServerHeader*)(currentPosition);

            if (currentContentHeader.contentType != ChatPacketUDP::Status::SUCCESS ||
                currentContentHeader.msgType != ChatPacketUDP::ServerMsgType::CHAT_CONTENT_SERVER)
            {
                qDebug() << "数据类型错误！" << Qt::endl;
                continue;
            }

            // 解包当前的TextMessage
            quint16 textSize = currentContentHeader.currentPacketSize - currentContentHeader.headerSize;
            QByteArray textBytes;
            textBytes.resize(textSize);
            memcpy(textBytes.data(), currentPosition + currentContentHeader.headerSize, textSize);
            QString textString(textBytes);

            // 用户还未打开和这个用户的聊天窗口，需要帮助用户打开它
            if (this->chatDialogs->find(currentContentHeader.fromUserID) == chatDialogs->end())
            {
                QString title = QString::number(currentContentHeader.fromUserID);
                this->AddTabChatView(title, currentContentHeader.fromUserID);
            }

            // 自动移到有新信息的Tab，并更新信息
            QDateTime timeCurrent = QDateTime::currentDateTime();
            QString time = timeCurrent.toString("yyyy-MM-dd hh-mm-ss"); // 收到消息的实际时间

            QString textToDisplay = "(" + time + " "+ QString::number(currentContentHeader.fromUserID) + ") - : " + textString;
            QListWidget* chatList = this->chatDialogs->value(currentContentHeader.fromUserID);
            QListWidgetItem* itemToAdd = new QListWidgetItem(textToDisplay);
            itemToAdd->setTextAlignment(Qt::AlignLeft);

            chatList->addItem(itemToAdd);

            this->ui->tabChatView->setCurrentIndex(chatList->property("tabIndex").toInt());

            qDebug() << "从" << QString::number(currentContentHeader.fromUserID) << "收到聊天信息: " << textString << Qt::endl;
            break;

            // 更新当前读取到的信息
            dealedPayloadSize += currentContentHeader.currentPacketSize;
        }

    }

    else if (ChatPacketUDP::isLoginReplyMsg(dataBytes))
    {
        ChatPacketUDP::LoginReplyHeader header = *(ChatPacketUDP::LoginReplyHeader*)dataBytes.data();

        if (this->user->LoggedIn())
        {
            qDebug() << "已经登录！忽略服务器的LOGIN_REPLY信息！" << Qt::endl;
            return;
        }

        if (header.status != ChatPacketUDP::Status::SUCCESS)
        {
            qDebug().noquote() << "登录失败！" << Qt::endl;
            QMessageBox::critical(nullptr, "登录错误", "登入用户失败，请检查用户ID和密码是否正确。");
            return;
        }

        // 录入获取到的用户信息
        quint16 thisUserID = header.loginUserID;
        QString password = this->ui->inputPassword->document()->toPlainText();
        QString nickName = QString::number(thisUserID); // to be completed...
        QVector<quint16>* friends = new QVector<quint16>();
        for (int i = 0; i < header.friendCount; i++)
        {
            quint16 friendID = *(quint16*)(dataBytes.data() + header.headerSize + i * sizeof(quint16));
            friends->append(friendID);
        }

        delete this->user;
        this->user = new ChatUser(thisUserID, password, nickName, friends);

        // 确认已经登入成功后，修改“登录状态”文字
        QPalette pe;
        pe.setColor(QPalette::WindowText, Qt::green);
        this->ui->labelLoginStatus->setPalette(pe);
        this->ui->labelLoginStatus->setText("· 状态：已登录");

        // 设置“登录”按钮隐藏，“退出登录”按钮显示
        this->ui->btnLogout->show();
        this->ui->btnSubmitUser->hide();

        this->SetLoginStatus(true);

        // 更新好友列表显示
        this->RefreshBtnFriendsView();

        qDebug() << "登录成功！" << Qt::endl;
    }

    else if (ChatPacketUDP::isLogoutReplyMsg(dataBytes))
    {
        ChatPacketUDP::LogoutReplyHeader header = *(ChatPacketUDP::LogoutReplyHeader*)dataBytes.data();

        if (header.status != ChatPacketUDP::Status::SUCCESS)
        {
            qDebug().noquote() << "登出失败！" << Qt::endl;
            QMessageBox::information(nullptr, "退出登录错误", "登出用户失败，请检查网络连接。");
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

        this->SetLoginStatus(false);
        this->RefreshBtnFriendsView();

        qDebug() << "登出成功！" << Qt::endl;
    }

    else
    {
        // 尝试解析
        if (dataBytes.size() >= sizeof(ChatPacketUDP::HeaderBase))
        {
            ChatPacketUDP::HeaderBase header = *(ChatPacketUDP::HeaderBase*)dataBytes.data();
            qDebug().noquote() << "服务器发来无法解析的信息: " << header.msgType << Qt::endl;
            QMessageBox::information(nullptr, "出错", "服务器发来无法解析的信息: " + QString::number(header.msgType));
        }


    } // END_IF
}


//-----------------------------------UI函数----------------------------------

/// 刷新文件发送进度条
void ChatClient::RefreshSendFileProgress(float progress)
{

}


/// 刷新“好友”列表显示
void ChatClient::RefreshBtnFriendsView()
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

    if (!this->LoggedIn())
    {
        return;
    }

    // 添加一个弹簧
    // QSpacerItem* verticalSpacer = new QSpacerItem(20, 40);
    // verticalLayoutFriends->addItem(verticalSpacer);

    if (this->user->getFriends()->count() >= 7)
    {
        QSize s = scrollAreaFriendsWidget->size();
        scrollAreaFriendsWidget->setSizeIncrement(s.width(), s.height() + (this->user->getFriends()->count() - 7) * 30);
    }

    // 将好友列表添加进去
    for (int i = this->user->getFriends()->count() - 1; i >= 0; i--)
    {
        quint16 friendID = this->user->getFriends()->at(i);
        QPushButton* btnFriend = new QPushButton();
        btnFriend->setText("朋友：" + QString::number(friendID));
        btnFriend->setProperty("friendID", friendID); // 绑定这个按钮和好友ID
        btnFriend->setProperty("friendIDIndex", i); // 绑定这个按钮和好友序号

        // 设置信号和槽函数
        btnFriend->connect(btnFriend, SIGNAL(clicked()), this, SLOT(on_btnFriends()));

        // 添加至上部
        verticalLayoutFriends->insertWidget(1, btnFriend);
    }
}


/// 用好友刷新Tab显示
void ChatClient::AddTabChatView(QString& tabTitle, quint16 friendID)
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
    this->chatDialogs->insert(friendID, listNew);
    listNew->setParent(tabNew);

    tabView->addTab(tabNew, tabTitle);
}

void ChatClient::DelCurrentTabChatView()
{
    QTabWidget* tabView = this->ui->tabChatView;

    QWidget* tab = this->ui->tabChatView->currentWidget();
    quint16 friendID = tab->property("friendID").toUInt();
    tabView->removeTab(this->ui->tabChatView->currentIndex());
    delete tab;

    // 还要删除存储的聊天信息
    this->chatDialogs->remove(friendID);
}


/// “好友”按钮的信号槽函数
void ChatClient::on_btnFriends()
{
    QPushButton* btn = dynamic_cast<QPushButton*>(sender());

    if (btn == Q_NULLPTR)
    {
        qDebug() << "按钮不存在" <<Qt::endl;
        return;
    }

    quint8 friendIDIndex = btn->property("friendIDIndex").toInt();
    quint16 friendID = this->user->getFriends()->at(friendIDIndex);
    if (friendID != btn->property("friendID").toInt())
    {
        qDebug() << "好友列表顺序错误！！！" << Qt::endl;
        return;
    }

    if (this->chatDialogs->find(friendID) != this->chatDialogs->end())
    {
        QMessageBox::information(this, "已经打开了对话框", "好友:" + QString::number(friendID) + " 的聊天框已打开，请勿重复操作！");
        qDebug() << "好友对话已经打开！" << Qt::endl;
        return;
    }

    QString tabTitle(friendID);
    AddTabChatView(tabTitle, friendID);
}


/// "关闭当前对话框"按钮信号槽函数
void ChatClient::on_btnDelCurrentTab()
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

    // quint16 friendID = this->ui->tabChatView->currentWidget()->property("friendID").toInt();
    DelCurrentTabChatView();
}


/// “登录”按钮的信号槽函数
void ChatClient::on_btnSubmitUser()
{
    QTextDocument* docID = this->ui->inputUserID->document();
    quint16 inputID = docID->toPlainText().toInt();
    // todo: 检查输入

    QTextDocument* docPassword = this->ui->inputPassword->document();
    QString inputPassword = docPassword->toPlainText();

    qDebug() << "ID: " << inputID << " ; Password: " << inputPassword << Qt::endl;

    try
    {
        this->Login(inputID, inputPassword);
    }
    catch (QString errorString)
    {
        qDebug() << "ERROR: " << errorString << Qt::endl;
        QMessageBox::critical(this, "网络错误", "网络错误：" + errorString);
        return;
    }
}


/// “退出登录”按钮信号槽函数
void ChatClient::on_btnLogout()
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
        this->Logout();
    }
    catch (QString errorString)
    {
        qDebug() << "ERROR: " << errorString << Qt::endl;
        QMessageBox::critical(this, "网络错误",  "网络错误：" + errorString);
        return;
    }

    // 删除所有聊天tab和暂存记录
    this->chatDialogs->clear();
    while (this->ui->tabChatView->count())
    {
        QWidget* tab = this->ui->tabChatView->currentWidget();
        this->ui->tabChatView->removeTab(0);
        delete tab;
    }
}


/// "发送消息"按钮信号槽函数
void ChatClient::on_btnSendMsg()
{
    QTextDocument* docTextMsg = this->ui->inputTextMsg->document();
    QString inputTextMsg = docTextMsg->toPlainText();

    quint16 targetUserID = this->ui->tabChatView->currentWidget()->property("friendID").toInt();

    try
    {
        this->SendText(inputTextMsg, targetUserID);
    }
    catch (QString errorString)
    {
        qDebug() << "ERROR: " << errorString << Qt::endl;
        QMessageBox::critical(this, "发送失败：网络错误",  "发送失败：" + errorString);
        return;
    }

    // 更新UI
    QDateTime timeCurrent = QDateTime::currentDateTime();
    QString time = timeCurrent.toString("yyyy-MM-dd hh-mm-ss");

    QString textToDisplay = inputTextMsg + " - (我 @ " + time + ")";
    QListWidget* chatList = this->chatDialogs->value(targetUserID);
    QListWidgetItem* itemToAdd = new QListWidgetItem(textToDisplay);
    itemToAdd->setTextAlignment(Qt::AlignRight);

    chatList->addItem(itemToAdd);
}


///
void ChatClient::on_btnSendFile()
{

}

