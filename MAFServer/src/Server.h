#pragma once
#define MSGPORT 8000
#define FILEPORT 8001

#include "ui_Server.h"
#include "Recv.h"
#include "MsgServer.h"
#include "FileServer.h"
#include <QMainWindow>
#include <QThread>
#include <QLabel> // 状态栏用

class Server : public QMainWindow
{
    Q_OBJECT

public:
    Server(QWidget *parent = nullptr);
    ~Server();

private:
    Ui_Server *ui;
    QFlags<Qt::WindowType> flags; // 保存开始时窗口状态
    QLabel *labelStatus;          // 显示当前连接状态

    MsgServer *m_msgServer;
    FileServer *m_fileServer;
    QString recvFolder;
    QThread *msgThread, *fileThread;

    // QList<Recv *> list_recv;
    // QList<QTcpSocket *> list_socket; // 用链表保存当前连接的客户端

    inline void forbidClose();
private slots:
    // void new_client(); // 出现新链接会调用的函数
    void recvMsg(QString ip, QByteArray date);
    void listMsgAddItem(QString ip);
    void listMsgRemoveItem(int index);

    // void recvFile
    void listFileAddItem(QString ip);
    void listFileRemoveItem(int index);

    void on_btnStart_clicked();            // 开启服务器
    void on_btnSendMsg_clicked();          // 向全体客户端发送消息
    void on_btnSelectFile_clicked();       // 选择文件路径（待发送
    void on_btnSendFile_clicked();         // 发送文件
    void on_btnChooseRecvFolder_clicked(); // 选择接收文件夹路径
    void on_btnClose_clicked();            // 关闭服务器
};