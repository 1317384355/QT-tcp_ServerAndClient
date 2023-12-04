#pragma once
#include "ui_Client.h"
#include "CFileInfo.h"
#include "ClientMsg.h"
#include "CSendFile.h"
#include "CRecvFile.h"
#include <QMainWindow>
#include <QFileDialog>
#include <QProgressBar>
#include <QList>
#include <QMessageBox>

class Client : public QMainWindow
{
    Q_OBJECT

signals:
    void startConnect(QString ip, unsigned short port);
    void sendMsg(QString msg);

public:
    Client(QWidget *parent = nullptr);
    ~Client();

private:
    Ui_Client *ui;

    QLabel *labelStatus;
    QLabel *labelIP;

    QString addr, // 储存 连接地址
        rFolder;  // 存放 接收文件

    ClientMsg *msgSocket;         // 接收处理消息 socket
    QThread *msgThread;           // 消息socket 工作线程
    QList<CFileInfo *> info_list; // 维护文件信息 链表
    QList<void *> file_list;      // 维护收发线程 链表
    QList<CRecvFile *> wait_list; // 接收文件对象可复用, 设置留存 3 个

    void addTableRow(QString fileName, qint64 fileSize, QString Attribute);

private slots:
    void on_recvMsg(QString msg);
    void showRecv(QString fileName, qint64 fileSize, qint16 sendID);
    void showError(int error);

    void on_btnStart_clicked();
    void on_btnSendMsg_clicked();

    void on_btnSelectFile_clicked();
    void on_btnDeleteFile_clicked();
    void on_btnSendFile_clicked();
    void on_btnRecvFile_clicked();
    void on_btnSelectRecvFolder_clicked();
    void on_tableWidget_cellPressed(int row);
};