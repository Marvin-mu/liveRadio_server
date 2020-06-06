#ifndef USERDAO_H
#define USERDAO_H

/*
 * data access object 数据库访问对象 --> 面向对象的数据库接口类
 * 抽象类
 * */
#include "user.h"
#include <QString>

class UserDao
{
public:
    UserDao();
    virtual ~UserDao();
    //纯虚函数,方便继承后修改函数
    virtual bool addUser(user_t user) = 0;
    virtual user_t findUser(const QString &name) = 0;
};

#endif // USERDAO_H
