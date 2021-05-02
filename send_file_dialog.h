#ifndef SEND_FILE_DIALOG_H
#define SEND_FILE_DIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class SendFileDialog;
}
QT_END_NAMESPACE

class SendFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SendFileDialog(QWidget *parent = nullptr);
    ~SendFileDialog();

    QString targetUserID;
    QString selectedFile;
    Ui::SendFileDialog *ui;

private slots:
    void on_btnSelect();
    void on_btnSend();
};

#endif // SEND_FILE_DIALOG_H
