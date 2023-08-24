#include "MsgServer.h"

MsgServer::MsgServer(QObject *parent) : QObject(parent)
{
    m_server = new QTcpServer();
}

MsgServer::~MsgServer()
{
    m_server->deleteLater();
}

void MsgServer::startConnect(unsigned short port)
{
    m_server->listen(QHostAddress::Any, port);

    // 放这里是因为如果放在构造函数里，关闭服务器再开启服务器后，将无法正常发出信号
    connect(m_server, &QTcpServer::newConnection, this, &MsgServer::new_client);
}

void MsgServer::SendMsg(QString msg)
{
    for (int i = list_socket.count() - 1; i >= 0; i--)
    {
        if (list_socket[i])
            list_socket[i]->write(msg.toUtf8().data());
    }
}
void MsgServer::endConnect()
{
    while (list_socket.count())
    {
        emit list_socket.first()->disconnected();
    }
    m_server->disconnect();
    m_server->close();
}

void MsgServer::new_client()
{
    QTcpSocket *socket = m_server->nextPendingConnection();
    list_socket.append(socket); // 添加当前链接

    emit listAddNewSocket(socket->peerAddress().toString());

    // 接收数据
    connect(socket, &QTcpSocket::readyRead, this, [=]()
            { emit MsgServer::readyRead(socket->peerAddress().toString(), socket->readAll()); });

    // 断开连接后的操作
    connect(socket, &QTcpSocket::disconnected, this, [=]()
            {
                socket->close();
                emit MsgServer::listRemoveSocket(list_socket.indexOf(socket));

                list_socket.removeOne(socket); // list移除断开的socket
                socket->deleteLater(); });
}