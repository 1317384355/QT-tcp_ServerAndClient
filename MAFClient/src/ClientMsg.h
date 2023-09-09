#pragma once
#include "command.h"

class ClientMsg : public QObject
{
    Q_OBJECT

signals:
    void connected();
    void disconnected();
    void recvMsg(QString msg);
    void recvFileApply(QString fileName, qint64 fileSize, qint16 sendID);

public:
    explicit ClientMsg(QObject *parent = nullptr);
    ~ClientMsg();

public slots:
    void endConnect(); // 断开连接并发出信号
    void startConnect(QString ip, unsigned short port);
    void sendMsg(QString msg);

private:
    QTcpSocket *m_tcp;

private slots:
    void readData();
};
