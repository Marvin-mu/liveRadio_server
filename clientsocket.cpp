#include "clientsocket.h"
#include "socketmanager.h"
#include "roommanager.h"

QMutex ClientSocket::mutex1;
QMutex ClientSocket::mutex2;

ClientSocket::ClientSocket(QTcpSocket *socket,QObject *parent)
    : QObject(parent),socket(socket)
{
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
            serverJoinRoom(user);       //听众
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
    delete ud;
}

void ClientSocket::serverLogin(user_t user)
{
    UserDao *ud = new UserDaoImp();
    QString pname = QString::fromLocal8Bit(user.username);
    mutex1.lock();
    user_t myuser = ud->findUser(pname);
    //mutex1.unlock();
    if (strncmp(myuser.data, "ok", 2) == 0) {
        strcpy(user.data, "用户不存在");
    } else {
        if (strcmp(myuser.password, user.password) == 0) {
            if (myuser.flag == UP) {
                strcpy(user.data, "该用户已经在线");
            } else {
                strcpy(user.data, "登录成功");
            }
        } else {
            strcpy(user.data, "密码错误");
        }
    }
    mutex1.unlock();
    emit sigWrite(socket, user,sizeof(user));
    delete ud;
}

void ClientSocket::serverRoomName(user_t user)
{
    QString roomname = QString::fromLocal8Bit(user.roomName);
    RoomManager *rm = RoomManager::getInstance();
    rm->insertRoom(roomname);
    qDebug() << __FILE__ << __FUNCTION__ << __LINE__;
    emit sigMes(roomname + " liveing");
    emit sigWrite(socket, user, sizeof (user));
}

void ClientSocket::serverExit(user_t user)
{
    QString pname = QString::fromLocal8Bit(user.username);
    QString proom = QString::fromLocal8Bit(user.roomName);
    SocketManager *sm = SocketManager::getInstance();
    sm->eraseSocket(this);
    emit sigMes(pname + " exit");
    qDebug() << __FILE__ << __FUNCTION__ << __LINE__;
}

void ClientSocket::serverRoomList(user_t user)
{
    //房间列表的信息拷贝到user的数据段
    QString str = "roomlist*";
    RoomManager *rm = RoomManager::getInstance();
    QVector<QString> rooms = rm->getAllRoom();
    for (auto &it : rooms) {
        str += it;
        str += "*";//房间数据用*号隔开
    }
    strcpy(user.data, str.toUtf8().data());
    emit sigWrite(socket, user, sizeof (user_t));
    emit sigMes("request roomlist");
    qDebug() << __FILE__ << __FUNCTION__ << __LINE__;
}

void ClientSocket::serverJoinRoom(user_t user)
{
    QString name = QString::fromLocal8Bit(user.username);
    QString rname = QString::fromLocal8Bit(user.roomName);
    strcpy(myinfo.roomName, rname.toUtf8().data());
    emit sigMes(name + " 加入了 " + rname  + " 房间");
}

void ClientSocket::serverQuit(user_t user)
{
    SocketManager *sm = SocketManager::getInstance();
    QString roomname = QString::fromLocal8Bit(user.roomName);
    QVector<ClientSocket*> sockets = sm->getAllSocket();
    for (auto &it : sockets) {
        if (it->getUser().roomName == roomname) {//在此直播间的用户
            strcpy(user.data,QString("主播已关闭直播间").toUtf8().data());
            emit sigWrite(it->getSocket(), user, sizeof (user_t));
        }
    }
    emit sigMes(roomname + "quit");
}

//实现文字聊天
void ClientSocket::serverChatText(user_t user)
{
    QString roomname = QString::fromLocal8Bit(user.roomName);
    qDebug() << __FUNCTION__ << __LINE__ << user.data;
    //RoomManager* rm = RoomManager::getInstance();
    //QVector<QString> rooms = rm->getAllRoomList();
    //for (auto it = rooms.begin(); it != rooms.end(); ++it) {
      //  QString rname = *it;
       //if (rname == roomname) {
         //   QTcpSocket *everySocket = socket;
           // emit sigWrite(everySocket, user, sizeof(user_t));
        //}
    //}
}
