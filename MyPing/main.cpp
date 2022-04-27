#include "mainwindow.h"
#include <QApplication>
#include <winsock.h>

int main(int argc, char *argv[])
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa); //初始化Windows Socket

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
