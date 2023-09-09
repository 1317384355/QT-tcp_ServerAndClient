#pragma once
#include "command.h"
#include "SFileInfo.h"
#include "SSendFile.h"
#include "SRecvFile.h"
#include <QThreadPool>

class ServerFile : public QObject
{
    Q_OBJECT

signals:
    void recvOver();
    void disconnected();
    void recvFile(SFileInfo *info, SRecvFile *rFile);
    void sendFile(SFileInfo *info, SSendFile *SFile);
    void curSendPercent(int percent);
    void curRecvPercent(int percent);

public:
    ServerFile(QList<SFileInfo *> *list, QObject *parent = nullptr);
    ~ServerFile();

public slots:
    void startConnect(unsigned short port);
    void endConnect();
    void setFolderName(QString dir);
    void addSendInfo(SFileInfo *info);

private:
    QTcpServer *m_server;
    QList<QTcpSocket *> list_socket;
    SFileInfo *sendId[1024];

    QThreadPool threadPool;
    QString folderName; // 文件名字，接收文件夹路径
private slots:
    void new_client();
};
