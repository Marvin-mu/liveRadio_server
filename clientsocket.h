#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <QTcpSocket>
#include "user.h"
#include <QMutex>
#include "userdaoimp.h"
#include <QDebug>
#include <QVector>

class ClientSocket : public QObject
{
    Q_OBJECT
public:
    explicit ClientSocket(QTcpSocket *socket, QObject *parent = 0);
    ~ClientSocket();

    void serverReg(user_t user);       //注册
    void serverLogin(user_t user);     //登录
    void serverExit(user_t user);      //退出
    void serverRoomName(user_t user);  //客户端请求开房
    void serverRoomList(user_t user);  //客户端请求房间列表
    void serverJoinRoom(user_t);        //房间新听众
    void serverQuitRoom(user_t);
    void serverQuit(user_t user);       //关闭直播间请求
    void serverChatText(user_t user);  //客户端请求文字聊天
    void serverBs(user_t user);         //处理弹幕
    void serverIp(user_t);//
    void serverTopUp(user_t);

    QTcpSocket* getSocket()const;
    user_t getUser()const;

private:
    QTcpSocket *socket;
    user_t myinfo;
    static QMutex mutex1;               //保证线程安全的互斥锁
    static QMutex mutex2;

signals:
    void sigWrite(QTcpSocket *socket, user_t user, int len);
    void sigMes(QString);              //线程信息通讯

private slots:
    void onReadyRead();
};

#endif // CLIENTSOCKET_H
