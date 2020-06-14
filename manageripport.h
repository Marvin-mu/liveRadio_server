#ifndef MANAGERIPPORT_H
#define MANAGERIPPORT_H

#include <QVector>
#include <QMutex>
#include "DBHelper.h"

class ManagerIpPort
{
public:
    static ManagerIpPort *getInsatnce();
    void create(QString);//新房间创建
    QString get(QString name);//获取某个房间
    void remove(QString name);//房间销毁

private:
    ManagerIpPort();
    static ManagerIpPort* instance;
    static QMutex mutex;
};

#endif // MANAGERIPPORT_H
