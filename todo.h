#ifndef TODO_H
#define TODO_H
#include <QString>

class ToDo {
private:
    int id;
    QString name;   // 顺序
    int priority;      // 次序
    bool finish;
public:
    ToDo(int i, QString n, int p) : id(i), name(n), priority(p), finish(false) {}
    ToDo(int i, QString n, int p, bool f) : id(i), name(n), priority(p), finish(f) {}
    void finished(int state);
    void unfinish();
    bool getFinish();
    QString getName();
    void setName(QString s);
    int getPriority();
    int getId();


};




#endif // TODO_H
