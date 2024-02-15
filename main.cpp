

#include "mainwindow.h"

#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false); //最后一个窗口关闭时，禁止退出应用程序

    MainWindow w;
    w.show();
    qDebug() << "启动";
    return a.exec();
}



