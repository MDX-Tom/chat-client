#include "send_file_dialog.h"
#include "ui_send_file_dialog.h"

#include <QFileDialog>

SendFileDialog::SendFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SendFileDialog)
{
    ui->setupUi(this);

    this->selectedFile = "";
}

SendFileDialog::SendFileDialog(ChatClient* chatClient, QString targetUserID, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SendFileDialog)
{
    ui->setupUi(this);

    this->chatClient = chatClient;
    this->targetUserID = targetUserID;

    this->selectedFile = "";
}

SendFileDialog::~SendFileDialog()
{
    delete ui;
}

void SendFileDialog::on_btnSelect()
{
    //定义文件对话框类
    QFileDialog *fileDialog = new QFileDialog(this);

    //定义文件对话框标题
    fileDialog->setWindowTitle(QStringLiteral("请选择文件..."));

    //设置默认文件路径
    fileDialog->setDirectory("~/");

    //设置文件过滤器
    fileDialog->setNameFilter(tr("File(*.*)"));

    //设置可以选择多个文件,默认为只能选择一个文件QFileDialog::ExistingFiles
    fileDialog->setFileMode(QFileDialog::ExistingFiles);

    //设置视图模式
    fileDialog->setViewMode(QFileDialog::Detail);

    //所有选择的文件的路径
    QStringList fileNames;
    if (fileDialog->exec()) {
        fileNames = fileDialog->selectedFiles();
    }

    this->selectedFile = fileNames.at(0);
}

void SendFileDialog::on_btnSend()
{
    if (this->selectedFile == "")
    {
        return;
    }

    this->chatClient->SendChatContent(
                this->targetUserID, ChatPacketUDP::ChatContentType::FILE, this->selectedFile);
}
