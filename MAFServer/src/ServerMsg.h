#pragma once
#include "command.h"
#include "SFileInfo.h"
#include <QTcpServer>

class ServerMsg : public QObject
{
    Q_OBJECT
signals:
    void recvMsg(QString ip, QString date);
    void listAddNewSocket(QString ip);
    void listRemoveSocket(int index);

public:
    ServerMsg(QObject *parent = nullptr);
    ~ServerMsg();

public slots:
    void on_startConnect();
    void on_endConnect();
    void sendMsg(int index, QString msg);
    void sendFile(int index, SFileInfo *info);

private:
    QTcpServer *m_server;
    QList<QTcpSocket *> list_socket;

private slots:
    void new_client();
};
