#ifndef SEND_FILE_DIALOG_H
#define SEND_FILE_DIALOG_H

#include <QDialog>
#include <QFileInfo>

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

    QString selectedFile;

public slots:
    void SetProgress(int progress);

private:
    Ui::SendFileDialog *ui;
    QFileInfo file;

private slots:
    void on_btnSelect();
    void on_btnSend();

signals:
    void sendFile();
};

#endif // SEND_FILE_DIALOG_H
