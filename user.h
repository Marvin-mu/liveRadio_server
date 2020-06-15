#ifndef USER_H
#define USER_H

enum Flag{
    DOWN,       //离线,0
    UP,         //在线，1
    MASTER      //开播，2
};

enum type{
    REG=1,            //注册
    LOGIN,          //登录
    EXIT,           //用户下线
    ROOMLIST,       //在线直播间
    ROOMNAME,       //创建直播间
    JOINROOM,       //观看直播间
    QUITROOM,       //退出直播间
    QUIT,           //关闭房间
    TEXT,           //文字聊天
    BS,             //弹幕
    IP,             //组播ip
    TOP_UP          //充值
};

typedef struct
{
    int type;               //类型
    int flag;               //状态标志位
    char username[20];      //用户名
    char password[20];      //密码
    char portrait[30000];   //头像
    double money;           //余额
    char vip[10];           //vip
    char roomName[20];      //聊天室名称
    char data[1024];        //数据

}user_t;

#endif // USER_H
