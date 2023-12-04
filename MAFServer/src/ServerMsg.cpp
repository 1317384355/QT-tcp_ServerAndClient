#include "ServerMsg.h"

ServerMsg::ServerMsg(QObject *parent) : QObject(parent),
                                        m_server(nullptr)
{
}

ServerMsg::~ServerMsg()
{
}

void ServerMsg::on_startConnect()
{
    if (m_server == nullptr)
    {
        m_server = new QTcpServer(this);
        connect(m_server, &QTcpServer::newConnection, this, &ServerMsg::new_client);
    }
    m_server->listen(QHostAddress::Any, MSG_PORT);
}

void ServerMsg::on_endConnect()
{
    for (auto socket : list_socket)
    {
        emit socket->disconnected();
    }
    m_server->close();
}

void ServerMsg::sendMsg(int index, QString msg)
{
    list_socket.at(index)->write((QString("%1##%2").arg(MASSAGE).arg(msg)).toUtf8());
    list_socket.at(index)->flush();
}

void ServerMsg::sendFile(int index, SFileInfo *info)
{ // 此处仅为发送文件申请
    list_socket[index]->write((QString("%1##%2##%3##%4").arg(FILE_SEND_APPLY).arg(info->fileName).arg(info->fileSize).arg(info->sendId)).toUtf8());
    list_socket.at(index)->flush();
}

void ServerMsg::new_client()
{
    QTcpSocket *socket = m_server->nextPendingConnection();
    list_socket.append(socket); // 添加当前链接

    emit ServerMsg::listAddNewSocket(socket->peerAddress().toString());

    // 接收数据
    connect(socket, &QTcpSocket::readyRead, this, [=]() { //
        // 分析数据
        QString data = QString(socket->readAll());
        msgSymbel head = msgSymbel(data.section("##", 0, 0).toInt());
        QString body = data.section("##", 1, 1);

        emit ServerMsg::recvMsg(socket->peerAddress().toString(), body);
    });

    // 断开连接后的操作
    connect(socket, &QTcpSocket::disconnected, this, [=]() { //
        socket->close();
        emit ServerMsg::listRemoveSocket(list_socket.indexOf(socket));
        list_socket.removeOne(socket); // list移除断开的socket
        socket->deleteLater();
    });
}