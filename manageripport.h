#ifndef MANAGERIPPORT_H
#define MANAGERIPPORT_H

#include <QVector>
#include <QMutex>
#include "DBHelper.h"

class ManagerIpPort
{
public:
    static ManagerIpPort *getInsatnce();
    void create(QString);//�·��䴴��
    QString get(QString name);//��ȡĳ������
    void remove(QString name);//��������

private:
    ManagerIpPort();
    static ManagerIpPort* instance;
    static QMutex mutex;
};

#endif // MANAGERIPPORT_H
