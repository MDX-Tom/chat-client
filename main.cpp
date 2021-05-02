#include "chat_client.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ChatClient MainWindow;

    MainWindow.show();
    return app.exec();
}
