#include "clientsocket.h"
#include "socketmanager.h"
#include "roommanager.h"

QMutex ClientSocket::mutex1;
QMutex ClientSocket::mutex2;

ClientSocket::ClientSocket(QTcpSocket *socket,QObject *parent)
    : QObject(parent),socket(socket)
{
    memset(&myinfo, 0, sizeof (myinfo));
    QObject::connect(socket,SIGNAL(readyRead()),this,SLOT(onReadyRead()));  //处理客户端请求的数据包
}

ClientSocket::~ClientSocket()
{

}

QTcpSocket* ClientSocket::getSocket()const
{
    return socket;
}

user_t ClientSocket::getUser() const
{
    return myinfo;
}

//接收客户端的协议包
void ClientSocket::onReadyRead()
{
    user_t user;
    memset(&user, 0, sizeof(user));
    while (socket->read((char*)&user,sizeof(user)) > 0) {
        switch (user.type) {
        case REG:
            serverReg(user);            //注册
            break;
        case LOGIN:
            serverLogin(user);          //登录
            break;
        case ROOMNAME:
            serverRoomName(user);       //新主播开播
            break;
        case ROOMLIST:
            serverRoomList(user);       //用户登录成功
            break;
        case JOINROOM:
            serverJoinRoom(user);       //听众加入直播间
            break;
        case QUITROOM:
            serverQuitRoom(user);       //听众离开直播间
            break;
        case EXIT:
            serverExit(user);           //用户下线
            break;
        case QUIT:
            serverQuit(user);           //关闭直播间
            break;
        case TEXT:
            serverChatText(user);       //文字聊天
            break;
        case BS:
            serverBs(user);
            break;
        default:
            break;
        }
    }
}

void ClientSocket::serverReg(user_t user)
{
    UserDao *ud = new UserDaoImp();
    QString pname = QString::fromLocal8Bit(user.username);
    mutex1.lock();
    user_t myuser = ud->findUser(pname);
    //mutex1.unlock();
    if (strncmp(myuser.data, "ok", 2) != 0) {
        strcpy(user.data, "用户名已存在");
    } else {
        //mutex2.lock();  //同一时刻只能一个用户注册
        if (ud->addUser(user)) {
            strcpy(user.data, "注册成功");
        } else {
            strcpy(user.data, "注册失败");
        }
        //mutex2.unlock();
    }
    mutex1.unlock();
    emit sigWrite(socket, user, sizeof(user));
    emit sigMes(pname + " reg");
    delete ud;
}

void ClientSocket::serverLogin(user_t user)
{
    UserDao *ud = new UserDaoImp();
    QString pname = QString::fromLocal8Bit(user.username);
    QString str;
    mutex1.lock();
    user_t myuser = ud->findUser(pname);
    mutex1.unlock();
    if (strncmp(myuser.data, "ok", 2) == 0) {
        strcpy(user.data, "用户不存在");
        str = "用户不存在";
    } else {
        if (strcmp(myuser.password, user.password) == 0) {
            if (myuser.flag == UP) {
                strcpy(user.data, "该用户已经在线");
                str = "用户已在线";
            } else {
                strcpy(user.data, "登录成功");
                str = " success";
                ud->updateUser(myinfo);
                myinfo = user;
            }
        } else {
            strcpy(user.data, "密码错误");
        }
    }
    qDebug() << __FILE__ << __FUNCTION__ << __LINE__;
    qDebug() << "name:" << myinfo.username << "password:" << myinfo.password << "flag:" << myinfo.flag << "rname:" << myinfo.roomName;
    emit sigWrite(socket, user,sizeof(user));
    emit sigMes(pname + " try login, " + str);
    delete ud;
}

void ClientSocket::serverRoomName(user_t user)
{
    QString roomname = QString::fromLocal8Bit(user.roomName);
    RoomManager *rm = RoomManager::getInstance();
    rm->insertRoom(roomname);
    //更新所处的房间
    strcpy(myinfo.roomName,roomname.toLocal8Bit());
    qDebug() << __FILE__ << __FUNCTION__ << __LINE__;
    qDebug() << "name:" << myinfo.username << "password:" << myinfo.password << "flag:" << myinfo.flag << "rname:" << myinfo.roomName;

    emit sigMes(roomname + " roomname");

    SocketManager *sm = SocketManager::getInstance();
    QVector<ClientSocket*> sockets = sm->getAllSocket();
    strcpy(user.data, roomname.toLocal8Bit().data());   //房间信息拷贝到数据段
    foreach (ClientSocket* it,sockets) {
        emit sigWrite(it->getSocket(), user, sizeof (user_t));//给每个客户端发信息，更新房间列表
    }
    //emit sigWrite(socket, user, sizeof (user));
}

//用户下线从容器里清除信息
void ClientSocket::serverExit(user_t user)
{
    QString pname = QString::fromLocal8Bit(user.username);
    QString proom = QString::fromLocal8Bit(user.roomName);
    SocketManager *sm = SocketManager::getInstance();
    sm->eraseSocket(this);

    UserDao *ud = new UserDaoImp;
    user.flag = DOWN;
    ud->updateUser(user);
    delete ud;

    emit sigMes(pname + " exit");
}

//
void ClientSocket::serverRoomList(user_t user)
{
    //房间列表的信息拷贝到user的数据段
    RoomManager *rm = RoomManager::getInstance();
    QVector<QString> rooms = rm->getAllRoom();

    strcpy(user.data,QString("start").toLocal8Bit().data());//开始发送房间
    emit sigWrite(socket,user,sizeof (user));

    for (auto &it : rooms) {
        strcpy(user.data,it.toLocal8Bit().data());
        emit sigWrite(socket, user, sizeof (user_t));//每个房间发送一次
    }

    strcpy(user.data,QString("end").toLocal8Bit().data());//发送结束
    emit sigWrite(socket,user,sizeof (user));

    emit sigMes(QString::fromLocal8Bit(user.username) + " request roomlist");
}

void ClientSocket::serverJoinRoom(user_t user)
{
    QString name = QString::fromLocal8Bit(user.username);
    QString rname = QString::fromLocal8Bit(user.roomName);
    strcpy(myinfo.roomName, rname.toLocal8Bit().data());

    SocketManager *sm = SocketManager::getInstance();//此房间的每个用户都会收到信息，更新他们的房间用户列表
    QVector<ClientSocket*> sockets = sm->getAllSocket();
    for (auto &it : sockets) {
        if (it->getUser().roomName == rname) {
            QTcpSocket *everysocket = it->getSocket();
            if (it->getUser().username == name) {
                continue;
            }
            emit sigWrite(everysocket, user, sizeof (user));

            user_t temp = it->getUser();
            temp.type = JOINROOM;
            emit sigWrite(socket, temp, sizeof(user_t));//把之前进入房间的用户的信息发给新上线的用户
            emit sigMes("给 " + QString(it->getUser().username) + " 发送了房间成员增加消息");
        }
    }

    strcpy(myinfo.roomName,user.roomName);   //房间名
    qDebug() << __FILE__ << __FUNCTION__ << __LINE__;
    qDebug() << "name:" << myinfo.username << "password:" << myinfo.password << "flag:" << myinfo.flag << "rname:" << myinfo.roomName;
    //setUser(user,2);
    emit sigMes(name + " 加入了 " + rname  + " 房间");
}

//有用户离开房间
void ClientSocket::serverQuitRoom(user_t user)
{
    QString name = QString::fromLocal8Bit(user.username);
    QString rname = QString::fromLocal8Bit(user.roomName);

    strcpy(myinfo.roomName,"");   //房间名置为空
    SocketManager *sm = SocketManager::getInstance();//此房间的每个用户都会收到信息，更新他们的房间用户列表
    QVector<ClientSocket*> sockets = sm->getAllSocket();
    for (auto &it : sockets) {
        if (it->getUser().roomName == rname) {
            QTcpSocket *everysocket = it->getSocket();
            emit sigWrite(everysocket, user, sizeof (user));
            emit sigMes("给 " + QString(it->getUser().username) + " 发送了房间成员退出消息");
        }
    }
    qDebug() << __FILE__ << __FUNCTION__ << __LINE__;
    qDebug() << "name:" << myinfo.username << "password:" << myinfo.password << "flag:" << myinfo.flag << "rname:" << myinfo.roomName;
    //setUser(user,2);
    emit sigMes(name + " 离开了 " + rname  + " 房间");
}

//主播关播
void ClientSocket::serverQuit(user_t user)
{
    SocketManager *sm = SocketManager::getInstance();
    QString roomname = QString::fromLocal8Bit(user.roomName);
    QVector<ClientSocket*> sockets = sm->getAllSocket();
    for (auto &it : sockets) {
        if (it->getUser().roomName == roomname && it->getUser().username != roomname) {//在此直播间的用户,除开主播
            strcpy(user.data,QString("主播已关闭直播间,谢谢配合").toLocal8Bit().data());
            emit sigWrite(it->getSocket(), user, sizeof (user_t));
            emit sigMes(QString(it->getUser().username) + " 将会收到关播通知");
        }

        RoomManager *rm = RoomManager::getInstance();
        rm->eraseRoom(roomname);    //清除房间信息

        //通知每个客户端刷新房间列表
        user_t temp = it->getUser();
        temp.type = ROOMLIST;
        QVector<QString> rooms = rm->getAllRoom();
        strcpy(temp.data,QString("start").toLocal8Bit().data());//开始发送房间
        emit sigWrite(it->getSocket(),temp,sizeof (user));

        foreach (QString it1, rooms) {
            strcpy(temp.data,it1.toLocal8Bit().data());
            emit sigWrite(it->getSocket(), temp, sizeof (user_t));//每个房间发送一次
        }
        strcpy(temp.data,QString("end").toLocal8Bit().data());//发送结束
        emit sigWrite(it->getSocket(),temp,sizeof (user));
        emit sigMes(QString::fromLocal8Bit(it->getUser().username) + " request roomlist");
    }
    emit sigMes(roomname + " quit");

    strcpy(myinfo.roomName,"");   //房间名置为空
    qDebug() << __FILE__ << __FUNCTION__ << __LINE__;
    qDebug() << "name:" << myinfo.username << "password:" << myinfo.password << "flag:" << myinfo.flag << "rname:" << myinfo.roomName;
}

//实现文字聊天
void ClientSocket::serverChatText(user_t user)
{
    QString roomname = QString::fromLocal8Bit(user.roomName);
    qDebug() << __FUNCTION__ << __LINE__ ;
    qDebug() << user.data;
    SocketManager *sm = SocketManager::getInstance();
    QVector<ClientSocket*> sockets = sm->getAllSocket();
    for (auto &it : sockets) {
       if (it->getUser().roomName == roomname) {
            QTcpSocket *everySocket = it->getSocket();
            emit sigWrite(everySocket, user, sizeof(user_t));
            emit sigMes(QString(it->getUser().username) + " 将会收到公屏新内容");
        }
    }
}

//转发弹幕
void ClientSocket::serverBs(user_t user)
{
    QString text = QString::fromLocal8Bit(user.data);
    QString roomname = QString::fromLocal8Bit(user.roomName);
    SocketManager *sm = SocketManager::getInstance();
    QVector<ClientSocket*> sockets = sm->getAllSocket();
    for (auto &it : sockets) {
       if (it->getUser().roomName == roomname) {
            QTcpSocket *everySocket = it->getSocket();
            emit sigWrite(everySocket, user, sizeof(user_t));
            sigMes(QString(it->getUser().username) + " 会收到 bs " + text);
        }
    }
}
