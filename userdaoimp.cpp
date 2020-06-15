#include "userdaoimp.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QSqlDatabase>
#include <QFile>
#include <QPixmap>

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
    //网络编码转化为uincode码
    QString qname = QString::fromLocal8Bit(user.username);
    QString qpasswd = QString::fromLocal8Bit(user.password);
    //建立数据库连接
    DBHelper *helper = DBHelper::getInstance();
    helper->createConn(dbName);

    QString str = "D:\\desk\\liveRadio_server\\liveRadio_server\\image\\user\\user";
    int i = qrand()%10 + 1;
    str += QString::number(i) + ".jpg";
    //执行sql语句
    QSqlQuery query(helper->getDb());
    query.prepare("insert into userinfo (name,password,portrait) values(:name, :password, :portrait);");
    query.bindValue(":password",qpasswd);
    query.bindValue(":name", qname);
    query.bindValue(":portrait", str);
    if (!query.exec()) {          //sql语句出错,输出出错信息
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
    QSqlQuery query(helper->getDb());
    query.prepare("select name,password,online,money,vip,portrait from userinfo;");
    query.exec();    //比对用户的姓名
    while(query.next()){
        QString username = query.value(0).toString();
        QString userpasswd = query.value(1).toString();
        int flag = query.value(2).toInt();
        double money = query.value(3).toDouble();
        QString vip = query.value(4).toString();
        QString portrait = query.value(5).toString();
        if (name == username) {
            strncpy(user.username, username.toLocal8Bit().data(), 20);      //账户信息拷贝
            strncpy(user.password, userpasswd.toLocal8Bit().data(), 20);
            strncpy(user.vip, vip.toLocal8Bit().data(), 10);
            user.money = money;
            user.flag = flag;
            QFile *file = new QFile(portrait);
            file->open(QIODevice::ReadOnly);
            QByteArray byte;
            byte = file->readAll();
            QByteArray byte64 = byte.toBase64();
                 //头像数据
            strncpy(user.portrait, byte64.data(), byte64.size());
            qDebug() << portrait << "头像大小 " << byte64.size() << strlen(byte64.data());
            helper->destroyConn();
            return user;
        }
    }
    strcpy(user.data, "ok");    //不存在返回的数据段非空
    helper->destroyConn();      //催毁链接
    return user;
}

bool UserDaoImp::updateUser(user_t user)
{
    DBHelper *helper = DBHelper::getInstance();
    helper->createConn(dbName);
    QSqlQuery query(helper->getDb());

    QString name = QString::fromLocal8Bit(user.username);
    int flag = user.flag;
    QString sql = QString("update %1 set online=%2 where name='%3';").arg(tbName).arg(flag).arg(name);
    qDebug() << sql;
    if (!query.exec(sql)) {//????
        QString error = query.lastError().text();
        qDebug() << "query.exec(sql) " << error;
        helper->destroyConn();
        return false;
    }
    helper->destroyConn();
    return true;
}

void UserDaoImp::topUpUser(user_t user)
{
    DBHelper *helper = DBHelper::getInstance();
    helper->createConn(dbName);
    QSqlQuery query(helper->getDb());

    QString name = QString::fromLocal8Bit(user.username);
    double money = QString(user.data).toDouble();
    QString sql = QString("update %1 set money=%2 where name='%3';").arg(tbName).arg(money).arg(name);
    qDebug() << sql;
    if (!query.exec(sql)) {//????
        QString error = query.lastError().text();
        qDebug() << "query.exec(sql) " << error;
    }
    helper->destroyConn();
}

void UserDaoImp::updatePortrait(user_t user)
{
    DBHelper *helper = DBHelper::getInstance();
    helper->createConn(dbName);
    QSqlQuery query(helper->getDb());

    QString name = QString::fromLocal8Bit(user.username);
    QByteArray portrait = user.portrait;
    query.prepare("update userinfo set portrait=:portrait where name=:name;");
    query.bindValue(":portrait",portrait);
    query.bindValue(":name",name);
    if (!query.exec()) {//
        QString error = query.lastError().text();
        qDebug() << "query.exec(sql) " << error;
    }
    helper->destroyConn();
}









