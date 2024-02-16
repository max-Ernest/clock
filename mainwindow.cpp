#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "reminderdialog.h"
#include <QTimer>
#include <QDebug>
#include <QMessageBox>
#include <QIcon>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QIcon icon(":/src/clock.ico");

    menu = new QMenu(this);
    SysIcon = new QSystemTrayIcon(this);
    SysIcon->setIcon(icon);
    SysIcon->setToolTip("倒计时");
    min = new QAction("窗口最小化",this);
    connect(min,&QAction::triggered,this,&MainWindow::hide);
//    connect(min,SIGNAL(trigger()),this,&MainWindow::hide);
//    max = new QAction("窗口最大化",this);
//    connect(max,&QAction::triggered,this,&MainWindow::showMaximized);
    restor = new QAction("恢复原来的样子",this);
    connect(restor,&QAction::triggered,this,&MainWindow::showNormal);
    quit = new QAction("退出",this);
    // connect(quit,&QAction::triggered,this,&MainWindow::close);
    connect(quit, &QAction::triggered, qApp, &QApplication::quit);
    connect(SysIcon,&QSystemTrayIcon::activated,this,&MainWindow::on_activatedSysTrayIcon);

    menu->addAction(min);
//    menu->addAction(max);
    menu->addAction(restor);
    menu->addSeparator(); //分割
    menu->addAction(quit);
    SysIcon->setContextMenu(menu);
    SysIcon->show();
    close();


    this->setWindowIcon(icon);
    ui->min->setValue(30);
    ui->sec->setValue(0);
    ui->min->setRange(0, 59);
    ui->sec->setRange(0, 60);



    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::showReminder);

    interval = 0;
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::startReminder);

    connect(ui->stopButton, &QPushButton::clicked, this, &MainWindow::stopTime);

    oneSec = new QTimer(this);
    connect(oneSec, &QTimer::timeout, this, &MainWindow::updateCountdown);
    startReminder();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startReminder() {
    int min = ui->min->value();
    int sec = ui->sec->value();

    interval = 60 * min + sec;

    if (interval == 0) {
        // 创建一个信息框
        QMessageBox msgBox;

        // 设置信息框的文本内容
        msgBox.setText("间隔时间不能为 0!!!");
        msgBox.exec();
        return;
    }
    countdownSeconds = interval; // 保存总秒数
    updateCountdown(); // 更新倒计时显示
    // 启动定时器
    timer->start(interval * 1000);
    oneSec->start(1000);
}

void MainWindow::showReminder() {

    // 获取主窗口当前的状态
    Qt::WindowStates mainWindowState = this->windowState();

    // 如果主窗口当前是最小化状态或者在系统托盘中
    if (mainWindowState & (Qt::WindowMinimized | Qt::WindowFullScreen | Qt::WindowMaximized | Qt::WindowNoState)) {
        // 将主窗口显示出来
        this->show();
    }


    QString reminderText = "<html><body><p style='font-size:16pt; color:#336699;'>望远时间到！</p></body></html>";

    dialog = new ReminderDialog(reminderText, this);
//    dialog.exec();      // 它会阻塞程序的执行，直到对话框关闭才会继续执行下面的代码。
    dialog->show();

    closeTime = new QTimer(this);
//    connect(closeTime, &QTimer::timeout, dialog, &QDialog::close);
    connect(closeTime, &QTimer::timeout, [this]() {
        closeTime->stop(); // 停止计时器
        if (dialog) {
            dialog->close(); // 关闭对话框
            delete dialog; // 释放对话框对象的内存
            dialog = nullptr; // 重置指针
        }
    });

    connect(closeTime, &QTimer::timeout, this, &MainWindow::startReminder);
    connect(dialog, &QDialog::finished, this, &MainWindow::startReminder);


    closeTime->start(120 * 1000);

    timer->stop();
    oneSec->stop();
}

void MainWindow::stopTime() {
    timer->stop();
    oneSec->stop();
    QString reminderText = "<html><body><p style='font-size:16pt; color:#336699;'>倒计时中断！</p></body></html>";

    ReminderDialog dialog(reminderText, this);
    dialog.exec();
}

void MainWindow::updateCountdown() {
    int minutes = countdownSeconds / 60;
    int seconds = countdownSeconds % 60;
    QString countdownText = QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
    ui->countdownLabel->setText(countdownText);
    // 更新倒计时剩余时间
    countdownSeconds--;
    if (countdownSeconds < 0) {
        // 如果倒计时已结束，停止定时器
        oneSec->stop();
    }
}


void MainWindow::closeEvent(QCloseEvent *event) {
    if(SysIcon->isVisible())
    {
        this->hide();
//        SysIcon->showMessage("倒计时小工具","倒计时将显示在此处");
        event->ignore();
    }
    else {
        event->accept();
    }
}



void MainWindow::on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {

    case QSystemTrayIcon::Trigger:
        break;
    case QSystemTrayIcon::DoubleClick:
        this->show();
        break;
    default:
        break;

    }
}


void MainWindow::closeDialog() {
    closeTime->stop();  // 停止定时器
    closeTime->deleteLater();  // 删除定时器对象
    closeTime = nullptr;  // 将指针置为空
}
