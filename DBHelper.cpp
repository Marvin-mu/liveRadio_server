#include "DBHelper.h"
#include <QSqlError>
#include <QDebug>
#include <QTime>

DBHelper* DBHelper::instance = NULL;
QMutex DBHelper::mutex;

DBHelper* DBHelper::getInstance()
{
    if (instance == NULL){
        mutex.lock();
        if (NULL == instance) {
            instance = new DBHelper();
        }
        mutex.unlock();
    }
    return instance;
}

//建立数据库链接
void DBHelper::createConn(const QString dbName)
{
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        db = QSqlDatabase::database("qt_sql_deafult_connection");
    } else {
        db = QSqlDatabase::addDatabase("QMYSQL");
    }
    db.setHostName("localhost");
    db.setUserName("marvin");
    db.setPassword("HATE970219??");
    db.setDatabaseName(dbName);
    db.open();
    if (!db.isOpen()) {
        qDebug() << "isOpen # " << db.lastError().text();
    }
}

//摧毁数据库链接
void DBHelper::destroyConn()
{
    db.close();
    QString name;
    {
        name = QSqlDatabase::database().connectionName();
    }
    db.removeDatabase(name);
}

DBHelper::~DBHelper()
{
    instance = NULL;
    delete instance;
}

//构造函数,mysql数据库
DBHelper::DBHelper()
{
    db = QSqlDatabase::addDatabase("QMYSQL",QTime::currentTime().toString());
}
