#include "send_file_dialog.h"
#include "ui_send_file_dialog.h"

#include <QFileDialog>

SendFileDialog::SendFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SendFileDialog)
{
    ui->setupUi(this);

    this->selectedFile = "";

    this->ui->label->setText("");

    this->ui->progressBar->setRange(0, 100);
    this->ui->progressBar->reset();

    this->file = QFileInfo();
}

SendFileDialog::~SendFileDialog()
{
    delete ui;
}

void SendFileDialog::SetProgress(int progress)
{
    this->ui->progressBar->setValue(progress);
    qApp->processEvents();
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
    if (fileDialog->exec())
    {
        fileNames = fileDialog->selectedFiles();
    }

    if (fileNames.length() == 0)
    {
        return;
    }

    this->selectedFile = fileNames.at(0);
    this->file.setFile(selectedFile);

    double megaBytes = this->file.size() / 1e6;
    this->ui->label->setText(this->file.fileName() + " " + QString::number(megaBytes, 'g', 2) + "MB");

    this->ui->progressBar->reset();
}

void SendFileDialog::on_btnSend()
{
    if (this->selectedFile == "")
    {
        return;
    }

    emit this->sendFile();
}
