#include "socketmanager.h"

SocketManager* SocketManager::instance = NULL;
QMutex SocketManager::mutex;

/**
 * @brief SocketManager::SocketManager
 */
SocketManager::SocketManager()
{

}

/**
 * @brief SocketManager::getInstance
 * @return
 */
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

/**
 * @brief SocketManager::insertSocket
 * @param cs
 */
void SocketManager::insertSocket(ClientSocket *cs)
{
    sockets.push_back(cs);
}

/**
 * @brief SocketManager::eraseSocket
 * @param cs
 */
void SocketManager::eraseSocket(ClientSocket *cs)
{
    for (auto &it : sockets) {
        if (it == cs) {
            sockets.erase(&it);
            break;
        }
    }
}

/**
 *@brief
 *@param
 *@return
 *@author marvin
 *@data 2020-06-19
 **/
QVector<ClientSocket*> SocketManager::getAllSocket()const
{
    return sockets;
}
