#include "todo.h"
#include <QDebug>


void ToDo::finished(int state)
{
    if (state) {
        this->finish = true;
        qDebug() << true << endl;
    } else {
        this->finish = false;
        qDebug() << false << endl;
    }
}

void ToDo::unfinish()
{
    this->finish = false;
}

bool ToDo::getFinish()
{
    return finish;
}

QString ToDo::getName()
{
    return name;
}

void ToDo::setName(QString s)
{
    this->name = s;
}


int ToDo::getPriority()
{
    return priority;
}

int ToDo::getId()
{
    return id;
}


