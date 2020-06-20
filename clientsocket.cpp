#include "clientsocket.h"
#include "socketmanager.h"
#include "roommanager.h"
#include "manageripport.h"

QMutex ClientSocket::mutex1;
QMutex ClientSocket::mutex2;

/**
 *@brief deal cs data
 *@param socket
 *@param parent
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
ClientSocket::ClientSocket(QTcpSocket *socket,QObject *parent)
    : QObject(parent),socket(socket)
{
    memset(&myinfo, 0, sizeof (myinfo));
    QObject::connect(socket,SIGNAL(readyRead()),this,SLOT(onReadyRead()));  //处理客户端请求的数据包
}

/**
 *@brief destructor
 *@param
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
ClientSocket::~ClientSocket()
{

}

/**
 *@brief get socket
 *@param
 *@return socket
 *@author marvin
 *@data 2020-06-19
 **/
QTcpSocket* ClientSocket::getSocket()const
{
    return socket;
}

/**
 *@brief get user
 *@param
 *@return user_t
 *@author marvin
 *@data 2020-06-19
 **/
user_t ClientSocket::getUser() const
{
    return myinfo;
}

/**
 *@brief The slot function will be executed if there is a siganl(readyRead)
 *@param
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
void ClientSocket::onReadyRead()
{
    user_t user;
    memset(&user, 0, sizeof(user));
    while (socket->read((char*)&user, sizeof(user)) > 0) {
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
            serverBs(user);             //弹幕消息
            break;
        case IP:
            serverIp(user);             //分配ip消息
            break;
        case TOP_UP:
            serverTopUp(user);          //充值数据包
            break;
        default:
            break;
        }
    }
}

/**
 *@brief processing registration requests
 *@param user
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
void ClientSocket::serverReg(user_t user)
{
    UserDao *ud = new UserDaoImp;
    QString pname = QString(user.username);
    mutex1.lock();
    user_t myuser = ud->findUser(pname);
    mutex1.unlock();
    QString str;
    if (strncmp(myuser.data, "ok", 2) != 0) {
        str = "用户名已存在";
        strcpy(user.data, "already exist");
    } else {
        //同一时刻只能一个用户注册
        mutex2.lock();
        if (ud->addUser(user)) {
            str = "注册成功";
            strcpy(user.data, "success");
        } else {
            str = "注册失败";
            strcpy(user.data, "failed");
        }
        mutex2.unlock();
    }
    emit sigWrite(socket, user, sizeof(user));
    emit sigMes(pname + " reg," + str);
    delete ud;
}

/**
 *@brief processing login requests
 *@param
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
void ClientSocket::serverLogin(user_t user)
{
    UserDao *ud = new UserDaoImp();
    QString pname = QString(user.username);
    QString str;
    mutex1.lock();
    user_t myuser = ud->findUser(pname);
    mutex1.unlock();
    if (strncmp(myuser.data, "ok", 2) == 0) {
        strcpy(user.data, "there in no");
        str = "用户不存在";
    } else {
        if (strcmp(myuser.password, user.password) == 0) {
            if (myuser.flag == UP) {
                strcpy(user.data, "online");
                str = "用户已在线";
            } else {
                strcpy(user.vip, myuser.vip);
                user.money = myuser.money;
                strcpy(user.data, "success");
                strcpy(user.portrait, myuser.portrait);//拷贝头像数据
                str = " success";
                mutex2.lock();
                ud->updateUser(user);
                mutex2.unlock();
                myinfo = user;
                strcpy(myinfo.username, pname.toUtf8().data());
            }
        } else {
            strcpy(user.data, "password error");
        }
    }
    qDebug() << __FILE__ << __FUNCTION__ << __LINE__;
    qDebug() << "name:" << myinfo.username
             << "flag:" << myinfo.flag << "rname:" << myinfo.roomName
             << "money:" << myinfo.money;
    emit sigWrite(socket, user, sizeof(user));
    emit sigMes(pname + " try login, " + str);
    delete ud;
}

/**
 *@brief This function will be executed there is a new room.
 *@param user
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
void ClientSocket::serverRoomName(user_t user)
{
    QString roomname = QString(user.username);//用户名为房间名
    RoomManager *rm = RoomManager::getInstance();
    rm->insertRoom(roomname);   //新增房间
    ManagerIpPort *mi = ManagerIpPort::getInsatnce();//创建组播ip
    mi->create(roomname);
    //更新所处的房间
    strcpy(myinfo.roomName, roomname.toUtf8().data());
    qDebug() << __FILE__ << __FUNCTION__ << __LINE__
    << "name:" << myinfo.username << "password:" << myinfo.password << "flag:" << myinfo.flag << "rname:" << myinfo.roomName;
    emit sigMes(roomname + " roomname");
    SocketManager *sm = SocketManager::getInstance();
    QVector<ClientSocket*> sockets = sm->getAllSocket();
    strcpy(user.data, roomname.toUtf8().data());   //房间信息拷贝到数据段
    UserDao *ud = new UserDaoImp;
    foreach (ClientSocket* it,sockets) {
        mutex2.lock();
        user_t myuser = ud->findUser(it->getUser().username);   //查找每个用户的头像,作为房间头像
        mutex2.unlock();
        strcpy(user.portrait, myuser.portrait);
        emit sigWrite(it->getSocket(), user, sizeof (user_t));//给每个客户端发信息，更新房间列表
    }
}

/**
 *@brief This function will be executed if there is a user exit.
 *@param
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
void ClientSocket::serverExit(user_t user)
{
    QString pname = QString(user.username);
    QString proom = QString(user.roomName);
    SocketManager *sm = SocketManager::getInstance();
    sm->eraseSocket(this);

    UserDao *ud = new UserDaoImp;
    user.flag = DOWN;
    mutex2.lock();
    ud->updateUser(user);//更新数据库的在线状态
    mutex2.unlock();
    delete ud;
    emit sigMes(pname + " exit");
}

/**
 *@brief processing roomlist requests
 *@param
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
void ClientSocket::serverRoomList(user_t user)
{
    //房间列表的信息拷贝到user的数据段
    RoomManager *rm = RoomManager::getInstance();
    QVector<QString> rooms = rm->getAllRoom();
    strcpy(user.data,QString("start").toUtf8().data());//开始发送房间
    emit sigWrite(socket,user,sizeof (user));
    UserDao *ud = new UserDaoImp;
    for (auto &it : rooms) {
        mutex2.lock();
        user_t myuser = ud->findUser(it);   //查找每个用户的头像
        mutex2.unlock();
        strcpy(user.portrait, myuser.portrait);
        strcpy(user.data,it.toUtf8().data());
        emit sigWrite(socket, user, sizeof (user_t));//把每个房间信息发送一次给当前请求的用户
    }
    strcpy(user.data,QString("end").toUtf8().data());//发送结束
    emit sigWrite(socket,user,sizeof (user));
    delete ud;
    emit sigMes(QString(user.username) + " request roomlist");
}

/**
 *@brief someone enter a room
 *@param user
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
void ClientSocket::serverJoinRoom(user_t user)
{
    QString name = QString(user.username);
    QString rname = QString(user.roomName);
    strcpy(myinfo.roomName, rname.toUtf8().data());
    UserDao *ud = new UserDaoImp;

    SocketManager *sm = SocketManager::getInstance();//此房间的每个用户都会收到信息，更新他们的房间用户列表
    QVector<ClientSocket*> sockets = sm->getAllSocket();
    for (auto &it : sockets) {
        if (QString(it->getUser().roomName) == rname) {//如果在这个房间
            QTcpSocket *everysocket = it->getSocket();
            if (QString(it->getUser().username) == name) {
                continue;
            }
            mutex2.lock();
            user_t myuser1 = ud->findUser(name);//查找当前用户的头像
            mutex2.unlock();
            strcpy(user.portrait, myuser1.portrait);
            emit sigWrite(everysocket, user, sizeof (user));//告诉此房间内的每一个用户

            user_t temp = it->getUser();
            mutex2.lock();
            user_t myuser = ud->findUser(temp.username);
            mutex2.unlock();
            strcpy(temp.portrait, myuser.portrait);     //其他用户的拷贝头像信息
            //
            temp.type = JOINROOM;
            emit sigWrite(socket, temp, sizeof(user_t));    //把之前进入房间的用户的信息发给进入房间的用户
            emit sigMes("给 " + QString(it->getUser().username) + " 发送了房间成员增加消息");
        }
    }

    strcpy(myinfo.roomName,user.roomName);   //房间名
    qDebug() << __FILE__ << __FUNCTION__ << __LINE__
    << "name:" << myinfo.username << "password:" << myinfo.password << "flag:" << myinfo.flag << "rname:" << myinfo.roomName;
    emit sigMes(name + " 加入了 " + rname  + " 房间");
    delete ud;
}

/**
 *@brief leave somebody's room
 *@param user
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
void ClientSocket::serverQuitRoom(user_t user)
{
    QString name = QString(user.username);
    QString rname = QString(user.roomName);

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
    qDebug() << __FILE__ << __FUNCTION__ << __LINE__
    << "name:" << myinfo.username << "password:" << myinfo.password << "flag:" << myinfo.flag << "rname:" << myinfo.roomName;

    emit sigMes(name + " 离开了 " + rname  + " 房间");
}

/**
 *@brief The host closes the room
 *@param user
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
void ClientSocket::serverQuit(user_t user)
{
    SocketManager *sm = SocketManager::getInstance();
    QString roomname = QString(user.roomName);
    QVector<ClientSocket*> sockets = sm->getAllSocket();
    for (auto &it : sockets) {
        if (it->getUser().roomName == roomname && it->getUser().username != roomname) {//在此直播间的用户,除开主播
            strcpy(user.data,QString("主播已关闭直播间,谢谢配合").toUtf8().data());
            emit sigWrite(it->getSocket(), user, sizeof (user_t));
            emit sigMes(QString(it->getUser().username) + " 将会收到关播通知");
        }
        RoomManager *rm = RoomManager::getInstance();
        rm->eraseRoom(roomname);    //清除房间信息
        //通知每个客户端刷新房间列表
        user_t temp = it->getUser();
        temp.type = ROOMLIST;
        QVector<QString> rooms = rm->getAllRoom();
        strcpy(temp.data,QString("start").toUtf8().data());//开始发送房间
        emit sigWrite(it->getSocket(), temp, sizeof (user));

        foreach (QString it1, rooms) {
            strcpy(temp.data, it1.toUtf8().data());
            emit sigWrite(it->getSocket(), temp, sizeof (user_t));//每个房间发送一次
        }
        strcpy(temp.data,QString("end").toUtf8().data());//发送结束
        emit sigWrite(it->getSocket(),temp,sizeof (user));
        emit sigMes(QString(it->getUser().username) + " request roomlist");
    }
    emit sigMes(roomname + " quit");

    UserDao *ud = new UserDaoImp;
    mutex2.lock();
    ud->updateUser(user);
    mutex2.unlock();
    delete ud;

    ManagerIpPort *mi = ManagerIpPort::getInsatnce();//销毁组播ip
    mi->remove(roomname);

    strcpy(myinfo.roomName,"");   //房间名置为空
    qDebug() << __FILE__ << __FUNCTION__ << __LINE__
    << "name:" << myinfo.username << "password:" << myinfo.password << "flag:" << myinfo.flag << "rname:" << myinfo.roomName;
}

/**
 *@brief processing chat
 *@param user
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
void ClientSocket::serverChatText(user_t user)
{
    QString roomname = QString(user.roomName);
    qDebug() << __FUNCTION__ << __LINE__
             << user.data;
    SocketManager *sm = SocketManager::getInstance();
    QVector<ClientSocket*> sockets = sm->getAllSocket();
    for (auto &it : sockets) {
       if (QString(it->getUser().roomName) == roomname) {
            QTcpSocket *everySocket = it->getSocket();
            emit sigWrite(everySocket, user, sizeof(user_t));
            emit sigMes(QString(it->getUser().username) + " 将会收到公屏新内容");
        }
    }
}

/**
 *@brief processing bs
 *@param user
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
void ClientSocket::serverBs(user_t user)
{
    QString text = QString(user.data);
    QString roomname = QString(user.roomName);
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

/**
 *@brief assign ip
 *@param user
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
void ClientSocket::serverIp(user_t user)
{
    QString roomname = QString(user.roomName);
    ManagerIpPort *mi = ManagerIpPort::getInsatnce();//获取组播ip
    QString ip = "ip/";
    ip += mi->get(roomname);
    qDebug() << __FUNCTION__ << " " << QString(user.username) << " " << ip;
    strcpy(user.data, ip.toLocal8Bit().data());
    emit sigWrite(socket, user, sizeof (user));
}

/**
 *@brief processing credits
 *@param user
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
void ClientSocket::serverTopUp(user_t user)
{
    UserDao *ud = new UserDaoImp;
    QString data = user.data;
    mutex1.lock();
    if (data == "host") {//主播收益
        user_t myuser = ud->findUser(QString(user.username));
        myuser.money += user.money;
        ud->topUpUser(myuser);
        qDebug() << __LINE__;
    } else {//普通用户充值
        ud->topUpUser(user);
        qDebug() << __LINE__;
    }
    mutex1.unlock();
    emit sigMes(QString(user.username) + "帐户积分 " + QString::number(user.money));
}



