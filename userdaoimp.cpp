#include "userdaoimp.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QSqlDatabase>
#include <QFile>
#include <QPixmap>

//鏁版嵁搴撳悕瀛
#define dbName "liveradio"
//琛ㄥ悕
#define tbName "userinfo"

UserDaoImp::UserDaoImp()
{

}

//娣诲姞鐢ㄦ埛
bool UserDaoImp::addUser(user_t user)
{
    //缃戠粶缂栫爜杞寲涓簎incode鐮
    QString qname = QString(user.username);
    QString qpasswd = QString(user.password);
    //寤虹珛鏁版嵁搴撹繛鎺
    DBHelper *helper = DBHelper::getInstance();
    helper->createConn(dbName);

    qsrand(time(0));
    QString str = "D:\\desk\\liveRadio_server\\liveRadio_server\\image\\user\\user";
    int i = qrand()%10 + 1;
    str += QString::number(i) + ".jpg";
    //鎵цsql璇彞
    QSqlQuery query(helper->getDb());
    query.prepare("insert into userinfo (name,password,portrait) values(:name, :password, :portrait);");
    query.bindValue(":password",qpasswd);
    query.bindValue(":name", qname);
    query.bindValue(":portrait", str);
    if (!query.exec()) {          //sql璇彞鍑洪敊,杈撳嚭鍑洪敊淇℃伅
        const QSqlError &error = query.lastError();
        qDebug() << error.text();
        helper->destroyConn();
        return false;
    }
    helper->destroyConn();
    return true;
}

//鏌ヨ鐢ㄦ埛
user_t UserDaoImp::findUser(const QString &name)
{
    user_t user;
    memset(&user, 0, sizeof(user));
    DBHelper *helper = DBHelper::getInstance();
    helper->createConn(dbName);
    QSqlQuery query(helper->getDb());
    query.prepare("select name,password,online,money,vip,portrait from userinfo;");
    query.exec();    //姣斿鐢ㄦ埛鐨勫鍚
    while(query.next()){
        QString username = query.value(0).toString();
        QString userpasswd = query.value(1).toString();
        int flag = query.value(2).toInt();
        double money = query.value(3).toDouble();
        QString vip = query.value(4).toString();
        QString portrait = query.value(5).toString();
        if (name == username) {
            strncpy(user.username, username.toUtf8().data(), 20);      //璐︽埛淇℃伅鎷疯礉
            strncpy(user.password, userpasswd.toUtf8().data(), 20);
            strncpy(user.vip, vip.toUtf8().data(), 10);
            user.money = money;
            user.flag = flag;
                 //澶村儚鏁版嵁
            strcpy(user.portrait, portrait.toUtf8().data());
            //qDebug() << "头像路径 " << portrait;
            helper->destroyConn();
            return user;
        }
    }
    strcpy(user.data, "ok");    //涓嶅瓨鍦ㄨ繑鍥炵殑鏁版嵁娈甸潪绌
    helper->destroyConn();      //鍌瘉閾炬帴
    return user;
}

/**
 *@brief modify online status
 *@param
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
bool UserDaoImp::updateUser(user_t user)
{
    DBHelper *helper = DBHelper::getInstance();
    helper->createConn(dbName);
    QSqlQuery query(helper->getDb());

    QString name = QString(user.username);
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

/**
 *@brief modify money
 *@param
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
void UserDaoImp::topUpUser(user_t user)
{
    DBHelper *helper = DBHelper::getInstance();
    helper->createConn(dbName);
    QSqlQuery query(helper->getDb());

    QString name = QString(user.username);
    double money = user.money;
    QString sql = QString("update %1 set money=%2 where name='%3';").arg(tbName).arg(money).arg(name);
    qDebug() << sql;
    if (!query.exec(sql)) {//????
        QString error = query.lastError().text();
        qDebug() << "query.exec(sql) " << error;
    }
    helper->destroyConn();
}

/**
 *@brief modify portrait
 *@param
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
void UserDaoImp::updatePortrait(user_t user)
{
    DBHelper *helper = DBHelper::getInstance();
    helper->createConn(dbName);
    QSqlQuery query(helper->getDb());

    QString name = QString(user.username);
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
