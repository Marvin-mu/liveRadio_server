#include "socketmanager.h"

SocketManager* SocketManager::instance = NULL;
QMutex SocketManager::mutex;

SocketManager::SocketManager()
{

}

SocketManager* SocketManager::getInstance()
{
    if (instance == 0) {
        mutex.lock();
        if (instance == 0) {
            instance = new SocketManager();
        }
        mutex.unlock();
    }
    return instance;
}

void SocketManager::insertSocket(ClientSocket *cs)
{
    sockets.push_back(cs);
}

void SocketManager::eraseSocket(ClientSocket *cs)
{
    for (auto &it : sockets) {
        if (it == cs) {
            sockets.erase(&it);
            break;
        }
    }
}

QVector<ClientSocket*> SocketManager::getAllSocket()const
{
    return sockets;
}
/*
void SocketManager::updateUser(user_t user)
{
    QString name = QString::fromLocal8Bit(user.username);
    for (auto &it : sockets) {
        if (it->getUser().username == name) {
            it->setUser(user);
            break;
        }
    }
}
*/
