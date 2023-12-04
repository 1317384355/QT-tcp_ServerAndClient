#pragma once
#include "ui_Server.h"
#include "ServerMsg.h"
#include "ServerFile.h"
#include <QMainWindow>
#include <QFileDialog>
#include <QLabel> // 状态栏用
#include <QProgressBar>
#include <QMessageBox>

class Server : public QMainWindow
{
    Q_OBJECT
signals:
    void startConnect();
    void sendMsg(int index, QString msg);      // 发送消息
    void sendFile(int index, SFileInfo *info); // 发送文件申请
    void changeFoler(QString dir);             // 修改接收文件名
    void addSendInfo(SFileInfo *info);
    void endConnect();

public:
    Server(QWidget *parent = nullptr);
    ~Server();

private:
    Ui_Server *ui;
    QLabel *labelStatus; // 显示当前连接状态

    ServerMsg *m_msgServer;
    ServerFile *m_fileServer;
    QString recvFolder;
    QThread *msgThread, *fileThread;

    QList<SFileInfo *> list_info;
    qint16 sendId = 1;

    int addTableRow(SFileInfo *info);

private slots:
    void showError(int error);
    void recvMsg(QString ip, QString msg);
    void startSend(SFileInfo *info, SSendFile *sFile);
    void startRecv(SFileInfo *info, SRecvFile *rFile);
    void listMsgAddItem(QString ip);
    void listMsgRemoveItem(int index);

    void on_btnStart_clicked();               // 开启服务器
    void on_btnSendMsg_clicked();             // 向某个客户端发送消息
    void on_btnSelectFile_clicked();          // 选择文件（待发送
    void on_btnDeleteFile_clicked();          // 删除选中文件
    void on_btnSendFile_clicked();            // 发送文件
    void on_btnSelectRecvFolder_clicked();    // 选择接收文件夹路径
    void on_tableWidget_cellPressed(int row); // 禁止发送表格中接收文件
    void on_btnClose_clicked();               // 关闭服务器
};