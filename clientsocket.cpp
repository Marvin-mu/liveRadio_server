#include "clientsocket.h"
#include "socketmanager.h"

QMutex ClientSocket::mutex1;
QMutex ClientSocket::mutex2;

ClientSocket::ClientSocket(QTcpSocket *socket,QObject *parent)
    : QObject(parent),socket(socket)
{
    QObject::connect(socket,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
}

ClientSocket::~ClientSocket()
{

}

QTcpSocket* ClientSocket::getSocket()const
{
    return socket;
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
            serverRoomList(user);       //新主播开播
            break;

        case EXIT:
            serverExit(user);           //用户下线
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
}

void ClientSocket::serverExit(user_t user)
{
    //OnlineDao *od = new OnlineDaoImp();
    QString pname = QString::fromLocal8Bit(user.username);
    QString proom = QString::fromLocal8Bit(user.roomName);
    mutex2.lock();
    //od->delOnline(pname);
    mutex2.unlock();
    if (user.flag == MASTER) {//房主下线,通知房间其他人离开
//        RoomUserManager *rum = RoomUserManager::getInstance();
//        QVector<Chat> rooms = rum->getAllChat();
//        for (auto it = rooms.begin(); it != rooms.end(); ++it) {
//            Chat mychat = *it;
//            if (mychat.getRoomName() == pname) {
//                QTcpSocket *everySocket = mychat.getSocket();
//                emit sigWrite(everySocket, user, sizeof(user_t));
//            }
//        }
        //房主下播,从在线房间列表移除
        //rm = RoomManager::getInstance();
        //rm->eraseRoom(proom);
    }
}

void ClientSocket::serverRoomList(user_t user)
{
    //房间列表的信息拷贝到user的数据段
    QString str = "roomlist";
    strcpy(user.data,str.toUtf8().data());
    emit sigWrite(socket, user, sizeof (user_t));
}

//实现文字聊天
void ClientSocket::serverChatText(user_t user)
{
    QString roomname = QString::fromLocal8Bit(user.roomName);
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
