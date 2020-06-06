#include "userdaoimp.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QSqlDatabase>

//数据库名字
#define dbName "liveradio"
//表名
#define tbName "userinfo"

UserDaoImp::UserDaoImp()
{

}

//添加用户
bool UserDaoImp::addUser(user_t user)
{
    //网络编码转化为nincode码
    QString qname = QString::fromLocal8Bit(user.username);
    QString qpasswd = QString::fromLocal8Bit(user.password);
    //建立数据库连接
    DBHelper *helper = DBHelper::getInstance();
    helper->createConn(dbName);
    //执行sql语句
    QSqlQuery query;
    QString sql = QString("insert into %1 (name,password) values").arg(tbName);
    sql += "('" + qname + "','" + qpasswd + "');";
    if (!query.exec(sql)) {          //sql语句出错,输出出错信息
        const QSqlError &error = query.lastError();
        qDebug() << error.text();
        helper->destroyConn();
        return false;
    }
    helper->destroyConn();
    return true;
}

//查询用户
user_t UserDaoImp::findUser(const QString &name)
{
    user_t user;
    memset(&user, 0, sizeof(user));
    DBHelper *helper = DBHelper::getInstance();
    helper->createConn(dbName);
    QSqlQuery query;
    QString sql = QString("select %1,%2,%3 from %4;").arg("name").arg("password").arg("online").arg(tbName);
    query.exec(sql);    //比对用户的姓名
    while(query.next()){
        QString username = query.value(0).toString();
        QString userpasswd = query.value(1).toString();
        int flag = query.value(2).toInt();
        qDebug() << name << " " << username << " " << userpasswd << " " << flag;
        if (name == username) {
            char *pname = username.toLocal8Bit().data();
            strncpy(user.username, pname, 20);    //账户信息拷贝
            strncpy(user.password, userpasswd.toLocal8Bit().data(), 20);
            user.flag = flag;
            helper->destroyConn();
            return user;
        }
    }
    helper->destroyConn();    //催毁链接
    strcpy(user.data, "ok");       //不存在返回的数据段非空
    return user;
}
