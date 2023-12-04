#pragma once
#include "SSendFile.h"
#include "SRecvFile.h"
#include <QThreadPool>
#include <unordered_map>

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
    ServerFile(QObject *parent = nullptr);
    ~ServerFile();

public slots:
    void on_startConnect();
    void on_endConnect();
    void setFolderName(QString dir);
    void addSendInfo(SFileInfo *info);

private:
    QTcpServer *m_server;
    std::unordered_map<qint16, SFileInfo *> map_sendId;

    QThreadPool threadPool;
    QString folderName; // 文件名字，接收文件夹路径
private slots:
    void new_client();
};
