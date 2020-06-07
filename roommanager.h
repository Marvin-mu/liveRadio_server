#ifndef ROOMMANAGER_H
#define ROOMMANAGER_H

#include<QString>
#include<QMutex>
#include<QVector>
using namespace std;

class RoomManager
{
public:
    static RoomManager* getInstance();      //创建单例对象
    void insertRoom(const QString& room);   //添加房间
    void eraseRoom(const QString& room);    //删除房间
    QVector<QString> getAllRoom()const;      //获取所有房间
    bool findRoom(const QString& room);     //查找房间
private:
    RoomManager();
    static RoomManager* instance;
    static QMutex mutex;                    //互斥锁
    QVector<QString> rooms;
};

#endif // ROOMMANAGER_H
