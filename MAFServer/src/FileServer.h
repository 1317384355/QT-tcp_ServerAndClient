#pragma once
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QFile>
#include <QFileDialog>

class FileServer : public QObject
{
    Q_OBJECT

signals:
    void recvOver();
    void disconnected();
    void readyRead(QByteArray date);
    void listAddNewSocket(QString ip);
    void listRemoveSocket(int index);
    void curPercent(int percent);

public:
    FileServer(QObject *parent = nullptr);
    ~FileServer();

    void startConnect(unsigned short port);
    void endConnect();
    void setFolderName(QString &dir);
    // void SendFile();

public slots:

private:
    QTcpServer *m_server;
    QTcpSocket *m_tcp;

    QFile file;
    QString folderName;
    QString filename; // 文件名称
    qint16 filesize;  // 文件大小

    qint16 recvfilesize; // 已接收大小
    bool isStart;        // 接收头部标记

private slots:
    void new_client();
    void recvFile();
};
