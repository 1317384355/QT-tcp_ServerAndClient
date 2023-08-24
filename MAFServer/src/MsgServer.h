#pragma once
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>

class MsgServer : public QObject
{
    Q_OBJECT
signals:
    void readyRead(QString ip, QByteArray date);
    void listAddNewSocket(QString ip);
    void listRemoveSocket(int index);

public:
    MsgServer(QObject *parent = nullptr);
    ~MsgServer();

    void startConnect(unsigned short port);
    void endConnect();
    void SendMsg(QString msg);

public slots:

private:
    QTcpServer *m_server;
    QList<QTcpSocket *> list_socket;

private slots:
    void new_client();
};
