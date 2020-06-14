#ifndef USERDAOIMP_H
#define USERDAOIMP_H

#include "userdao.h"
#include "DBHelper.h"

//公有继承数据库接口类,并实现mysql数据库的操作
class UserDaoImp:public UserDao
{
public:
    UserDaoImp();
    virtual bool addUser(user_t user);              //增加用户,放入数据库
    virtual user_t findUser(const QString &name);   //在数据库中查找是否存在此用户
    virtual bool updateUser(user_t user);           //更新在线状态
    virtual void topUpUser(user_t);                 //充值帐户积分
    virtual void updatePortrait(user_t);            //修改头像
};

#endif // USERDAOIMP_H
