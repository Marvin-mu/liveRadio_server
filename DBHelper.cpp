#include "DBHelper.h"
#include <QSqlError>
#include <QDebug>
#include <QTime>

DBHelper* DBHelper::instance = NULL;
QMutex DBHelper::mutex;

/**
 *@brief
 *@param
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
QSqlDatabase DBHelper::getDb()const
{
    return db;
}

/**
 *@brief
 *@param
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
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

/**
 *@brief create connect
 *@param dbName
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
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

/**
 *@brief destroy
 *@param
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
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

/**
 *@brief destructor
 *@param
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
DBHelper::~DBHelper()
{
    if (NULL != instance) {
        delete instance;
    }
    instance = NULL;
}

/**
 *@brief structre
 *@param
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
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
