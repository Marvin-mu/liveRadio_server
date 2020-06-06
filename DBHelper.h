#ifndef DBHELPER_H
#define DBHELPER_H

#include <QtSql/QSqlDatabase>   //数据库头文件
#include <QMutex>
/*
 * 采用单例模式,只能有一个对象使用此数据库
 * */

class DBHelper
{
public:
    static DBHelper* getInstance();
    void createConn(const QString dbName);      //建立链接
    void destroyConn();     //摧毁链接
    ~DBHelper();

private:
    QSqlDatabase db;    //数据库对象
    static DBHelper* instance;
    static QMutex mutex;
    DBHelper();
};

#endif // DBHELPER_H
