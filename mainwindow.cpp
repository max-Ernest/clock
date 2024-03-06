#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "reminderdialog.h"
#include "todo.h"
#include <QTimer>
#include <QDebug>
#include <QMessageBox>
#include <QIcon>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QInputDialog>
#include <QRandomGenerator>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

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
    restor = new QAction("恢复原来的样子",this);
    connect(restor,&QAction::triggered,this,&MainWindow::showNormal);
    quit = new QAction("退出",this);
    connect(quit, &QAction::triggered, this, [this]() {
        exit();
        QApplication::quit();
    });
    connect(SysIcon,&QSystemTrayIcon::activated,this,&MainWindow::on_activatedSysTrayIcon);

    menu->addAction(min);
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



    connect(ui->addButton, &QPushButton::clicked, this, &MainWindow::addToDo);
    connect(ui->delButton, &QPushButton::clicked, this, &MainWindow::delToDo);
    openDB();
    start();
    playout = new QGridLayout();
    load();
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
    dialog->setWindowFlags(dialog->windowFlags() | Qt::WindowStaysOnTopHint);
    dialog->show();

    closeTime = new QTimer(this);
//    connect(closeTime, &QTimer::timeout, dialog, &QDialog::close);
    connect(closeTime, &QTimer::timeout, [this]() {
        closeTime->stop();                      // 停止计时器
        if (dialog) {
            dialog->close();                    // 关闭对话框
            delete dialog;                      // 释放对话框对象的内存
            dialog = nullptr;                   // 重置指针
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

void MainWindow::addToDo()
{
    bool bOk = false;
    QString name = QInputDialog::getMultiLineText(this,
                                                  "待办名",
                                                  "请输入待办名字",
                                                  "",
                                                  &bOk
                                                  );
    int p = QInputDialog::getInt(this,
                                 "待办优先级",
                                 "请输入优先级",
                                 1,				//默认值
                                 1,				//最小值
                                 10,			//最大值
                                 1,				//步进
                                 &bOk);

    ToDo *temp = new ToDo(QRandomGenerator::global()->bounded(1000, 9999), name, p);
    ToDoList.append(temp);
    load();
}

void MainWindow::delToDo()
{
    qDebug() << "删除" << endl;
    bool bOk = false;
    QString name = QInputDialog::getMultiLineText(this,
                                                  "待办名",
                                                  "请输入待办名字",
                                                  "",
                                                  &bOk
                                                  );
    int index = -1;
    for (int i = 0; i < ToDoList.size(); i++) {
        if (ToDoList[i]->getName() == name) {
            index = i;
            break;
        }
    }
    if (index != -1) {
        ToDoList.erase(ToDoList.begin() + index);
    }
    load();

}

void MainWindow::load()
{
    while (playout->count()) {
        QLayoutItem* item = playout->takeAt(0);
        delete item->widget();
        delete item;
    }
    // 自定义排序，按照待办优先级
    qSort(ToDoList.begin(), ToDoList.end(), [](ToDo *a, ToDo *b) {
       return a->getPriority() < b->getPriority();
    });

    qDebug() << "-------------加载-----------------"  << endl;
    for (auto v : ToDoList) {
        qDebug() << v->getName() << "," << v->getFinish() << endl;
        QCheckBox *cBox = new QCheckBox(v->getName());
        cBox->setMinimumSize(QSize(60, 30));
        connect(cBox, &QCheckBox::stateChanged, [=](int state) {
             v->finished(state);
        });
        cBox->setChecked(v->getFinish());
        playout->addWidget((cBox));
    }
    ui->scrollArea->widget()->setLayout(playout);
}

void MainWindow::openDB()
{
    //打开数据库
    DB = QSqlDatabase::addDatabase("QSQLITE");
    DB.setDatabaseName("./StudentDB.db");//打开数据库
    if (DB.open())
    {
        qDebug() << "Database opened successfully！";
        QSqlQuery createTableQuery;
        createTableQuery.prepare("CREATE TABLE IF NOT EXISTS todo ("
                                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                "name TEXT NOT NULL,"
                                "priority INTEGER,"
                                "finish BOOLEAN"
                                ");");
        if (!createTableQuery.exec())
        {
            qDebug() << "创建表失败：" << createTableQuery.lastError().text();
        }
        else
        {
            qDebug() << "表'todo'创建成功！";
        }
    }
    else
    {
        qDebug() << "无法打开数据库：" << endl;
    }
}

void MainWindow::start()
{
    queryString = "SELECT id, name, priority, finish FROM todo";
    QSqlQuery query(queryString);
    if (!query.exec()) {
        qDebug()<<"Failed to execute query: "<< query.lastError().text();
        return;
    }
    qDebug() << "-------------读取-----------------"  << endl;
    while (query.next()) {
        int i = query.value(0).toInt();
        QString n = query.value(1).toString();
        int p = query.value(2).toInt();
        bool f = query.value(3).toBool();
        qDebug() << n << "," << f << endl;
        ToDoList.append(new ToDo(i, n, p, f));
    }
    QDate currentDate = QDate::currentDate();
    QString dateString = currentDate.toString("yyyy-MM-dd");
    qDebug() << dateString << endl;
    if (!ToDoList.empty()) {
        if (ToDoList[0]->getName() != dateString) {
            ToDoList[0]->setName(dateString);
            setFinish();
        }
    } else {
        ToDoList.append(new ToDo(1111, dateString, 0));
    }
}

void MainWindow::exit()
{
    QSqlQuery clearQuery("DELETE FROM todo");
    if (!clearQuery.exec())
    {
        qDebug() << "清空 todo 表失败："<< clearQuery.lastError().text();
    } else {
        qDebug() << "todo 表已清空";
    }

    queryString = "INSERT INTO todo (id, name, priority, finish) VALUES (?, ?, ?, ?)";
    QSqlQuery insertQuery;
    qDebug() << "-------------退出-----------------"  << endl;
    insertQuery.prepare(queryString);
    for (auto v : ToDoList) {
        qDebug() << v->getName() << "," << v->getFinish() << endl;
        insertQuery.bindValue(0, v->getId());
        insertQuery.bindValue(1, v->getName());
        insertQuery.bindValue(2, v->getPriority());
        insertQuery.bindValue(3, v->getFinish());
        if (!insertQuery.exec()) {
            qDebug() << "插入数据失败" << insertQuery.lastError().text();
        }
    }
}

void MainWindow::setFinish()
{
    for (auto v : ToDoList) {
        v->unfinish();
    }
}


void MainWindow::closeDialog() {
    closeTime->stop();                  // 停止定时器
    closeTime->deleteLater();           // 删除定时器对象
    closeTime = nullptr;                // 将指针置为空
}
