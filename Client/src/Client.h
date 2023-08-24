#pragma once
#include "ui_Client.h"
#include "Send.h"
#include <QMainWindow>
#include <QThread>
#include <QFileDialog>
#include <QRadioButton>
#include <QProgressBar>

class Client : public QMainWindow
{
    Q_OBJECT

signals:
    void startConnect(QString ip, unsigned short port);
    void sendMsg(QString msg);
    void sendFile(QString path);

public:
    Client(QWidget *parent = nullptr);
    ~Client();

private:
    Ui_Client *ui;
    QFlags<Qt::WindowType> flags;

    QLabel *labelStatus;
    QLabel *labelIP;

    QString addr;            // 储存将要连接的地址
    unsigned short port = 0; // 储存将要连接的端口

    QThread *newThread;
    Send *worker;

private slots:
    void recvMsg(QByteArray date);
    void recvFile(QByteArray date);

    void curPercent(int percent);

    void on_btnStart_clicked();
    void on_btnSend_clicked();

    void on_btnSelectFile_clicked();
    void on_btnDeleteFile_clicked();
    void on_btnSendFile_clicked();
    void on_btnShow_clicked(bool checked);
};