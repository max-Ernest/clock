#ifndef REMINDERDIALOG_H
#define REMINDERDIALOG_H

#include <QDialog>

class ReminderDialog : public QDialog {
public:
    ReminderDialog(const QString &reminderText, QWidget *parent = nullptr);
};

#endif // REMINDERDIALOG_H


