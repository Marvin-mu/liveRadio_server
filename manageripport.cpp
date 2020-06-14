#include "manageripport.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QSqlDatabase>
#include <QString>

//数据库名字
//#define dbName "liveradio"
//表名
//#define tbName "roomlist"

ManagerIpPort* ManagerIpPort::instance = nullptr;
QMutex ManagerIpPort::mutex;

ManagerIpPort::ManagerIpPort()
{

}

ManagerIpPort* ManagerIpPort::getInsatnce()
{
    if (NULL == instance) {
        mutex.lock();
        if (NULL == instance) {
            instance = new ManagerIpPort();
        }
        mutex.unlock();
    }
    return instance;
}

void ManagerIpPort::create(QString name)
{
    DBHelper *helper = DBHelper::getInstance();
    QString ip = "224.0.0.";
    static int i = 1;
    i += 3;
    ip += QString::number(i);
    static int port = 8888;
    port += 3;
    helper->createConn("liveradio");
    QSqlQuery query(helper->getDb());
    QString sql = QString("insert into %1 (name,ip,port) values(\'%2\',\'%3\',%4);").arg("roomlist").arg(name).arg(ip).arg(port);
    if (!query.exec(sql)) {          //sql语句出错,输出出错信息
        const QSqlError &error = query.lastError();
        qDebug() << error.text();
    }
    helper->destroyConn();
}

QString ManagerIpPort::get(QString name)
{
    DBHelper *helper = DBHelper::getInstance();
    helper->createConn("liveradio");
    QSqlQuery query(helper->getDb());
    QString sql = QString("select %1,%2,%3 from %4 where name='%5\';").arg("name").arg("ip").arg("port").arg("roomlist").arg(name);
    query.exec(sql);
    QString str = "";
    if (query.next()) {
        QString roomname = query.value(0).toString();
        QString ip = query.value(1).toString();
        QString port = query.value(2).toString();
        qDebug() << roomname << " " << ip << " " << port;  //用于调试
        str += ip;
        str += "/";
        str += port;
    }
    helper->destroyConn();
    return  str;
}

void ManagerIpPort::remove(QString name)
{
    DBHelper *helper = DBHelper::getInstance();
    helper->createConn("liveradio");
    QSqlQuery query(helper->getDb());

    QString sql = QString("delete from %1 where name='%2';").arg("roomlist").arg(name);
    qDebug() << sql;
    if (!query.exec(sql)) {//
        QString error = query.lastError().text();
        qDebug() << "query.exec(sql) " << error;
    }
    helper->destroyConn();
}
