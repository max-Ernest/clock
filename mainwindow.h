#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QCloseEvent>
#include <QSystemTrayIcon> //托盘使用的头文件
#include "reminderdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT


private:
    QSystemTrayIcon *SysIcon;
    QAction *min; //最小化
    QAction *max; //最大化
    QAction *restor; //恢复
    QAction *quit; //退出
    QMenu *menu;


public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void startReminder();
    void showReminder();
    void stopTime();
    void updateCountdown();

    void closeDialog();

    void closeEvent(QCloseEvent *event);
    void on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason);

private:
    Ui::MainWindow *ui;
    QTimer *timer;      // 定时器对象
    QTimer *oneSec;
    QTimer *closeTime;
    int interval;       // 提醒间隔，单位：秒
    int countdownSeconds; // 保存总秒数
    ReminderDialog* dialog;
};
#endif // MAINWINDOW_H