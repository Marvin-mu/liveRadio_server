#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QTcpServer>
#include "user.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief The MainWindow class
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNewConnection();                                     //处理新客户端连接
    void onSigWrite(QTcpSocket *socket, user_t user, int len);  //处理写数据信号
    void onSigMes(QString);                                     //处理现实信号

private:
    Ui::MainWindow *ui;
    QTcpServer *server;
};
#endif // MAINWINDOW_H
