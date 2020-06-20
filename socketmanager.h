#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include "clientsocket.h"
#include <QMutex>
#include <QVector>
#include "user.h"

/**
 * @brief The SocketManager class
 */
class SocketManager
{
public:
    static SocketManager* getInstance();
    void insertSocket(ClientSocket *cs);         //插入套接字(用户上线)
    void eraseSocket(ClientSocket *cs);          //用户下线，清理套接字
    QVector<ClientSocket*> getAllSocket()const;  //获取所有在线用户的套接字

private:
    SocketManager();
    static SocketManager* instance;     //对象
    static QMutex mutex;                //互斥锁
    QVector<ClientSocket*> sockets;     //存放用户的套接字
};

#endif // SOCKETMANAGER_H
