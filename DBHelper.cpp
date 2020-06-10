#include "DBHelper.h"
#include <QSqlError>
#include <QDebug>
#include <QTime>

DBHelper* DBHelper::instance = NULL;
QMutex DBHelper::mutex;

QSqlDatabase DBHelper::getDb()const
{
    return db;
}

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
    if (db.isValid()) {
        if (db.isOpen()) {
            db.close();
        }
        QString name;
        {
            name = QSqlDatabase::database().connectionName();
        }
        db.removeDatabase(name);
    }
}

DBHelper::~DBHelper()
{
    if (NULL != instance) {
        delete instance;
    }
    instance = NULL;
}

//构造函数,mysql数据库
DBHelper::DBHelper()
{
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        db = QSqlDatabase::database("qt_sql_deafult_connection");
    } else {
        QString name = QTime::currentTime().toString("hhmmsszzz");
        db = QSqlDatabase::addDatabase("QMYSQL",name);
    }
    if (!db.isValid()) {
        qDebug() << "db is not valid";
        return;
    }
}
