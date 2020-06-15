#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "clientsocket.h"
#include "socketmanager.h"
#include <QTcpSocket>
#include <QHostAddress>
#include <QMetaType>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("草莓红播服务器v3.0");
    this->setWindowIcon(QIcon(":/image/icon.jpg"));
    //创建套接字管理对象并连接
    server = new QTcpServer(this);

    //关联来自客户端的信号槽
    connect(server,SIGNAL(newConnection()),this,SLOT(onNewConnection()));

    //启动监听,8888端口
    if (this->server->listen(QHostAddress::Any,8888)) {
        ui->textBrowser->append("监听成功");
    } else {
        ui->textBrowser->append("监听失败");
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onNewConnection()
{
    QTcpSocket *client = server->nextPendingConnection();   //client存放了客户端信息
    ui->textBrowser->append(client->localAddress().toString() + " 上线");
    ClientSocket *cs = new ClientSocket(client);    //创建处理客户端的对象

    QThread *th = new QThread(this);    //创建一个线程
    connect(client, &QTcpSocket::disconnected, cs,
            [=](void)->void{
            //th->wait();
            th->terminate();
            delete th;
            }
        );
    connect(this, &MainWindow::destroyed, th, [=](void)->void{
        th->terminate();
        delete th;
        this->close();
        });//关闭服务器先处理子线程

    qRegisterMetaType<user_t>("user_t");    //完成结构体注册
    connect(cs, SIGNAL(sigMes(QString)), this, SLOT(onSigMes(QString)));
    connect(cs, SIGNAL(sigWrite(QTcpSocket*, user_t, int)), this, SLOT(onSigWrite(QTcpSocket*, user_t, int)));//向客户端发送数据包
    cs->moveToThread(th);   //移动到子线程运行任务
    th->start();    //启动线程

    //管理客户端套接字的操作
    SocketManager *sockets = SocketManager::getInstance();
    sockets->insertSocket(cs);  //新客户端上线，每个客户端有一个线程，被存在容器里
}

void MainWindow::onSigWrite(QTcpSocket *socket, user_t user, int len)
{
    socket->write((char*)&user, len);
}

void MainWindow::onSigMes(QString mes)
{
    ui->textBrowser->append(mes);
}

