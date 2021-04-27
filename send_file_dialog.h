#ifndef SEND_FILE_DIALOG_H
#define SEND_FILE_DIALOG_H

#include <QDialog>

#include "chat_client.h"

namespace Ui {
class SendFileDialog;
}

class SendFileDialog : public QDialog
{
    Q_OBJECT

public:
    SendFileDialog(ChatClient* chatClient, QString targetUserID, QWidget *parent = nullptr);
    ~SendFileDialog();

private:
    explicit SendFileDialog(QWidget *parent = nullptr);

    Ui::SendFileDialog *ui;
    ChatClient* chatClient;

    QString targetUserID;
    QString selectedFile;

private slots:
    void on_btnSelect();
    void on_btnSend();
};

#endif // SEND_FILE_DIALOG_H
