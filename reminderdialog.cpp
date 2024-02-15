#include "reminderdialog.h"
#include <QLabel>
#include <QVBoxLayout>

ReminderDialog::ReminderDialog(const QString &reminderText, QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Reminder");
    setFixedSize(400, 200); // 设置对话框的大小

    QLabel *label = new QLabel(this);
    label->setTextFormat(Qt::RichText);
    label->setAlignment(Qt::AlignCenter);
    label->setText(reminderText);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(label);
    setLayout(layout);
}
