#include "ServerFile.h"

ServerFile::ServerFile(QList<SFileInfo *> *list, QObject *parent) : QObject(parent)
{
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, &ServerFile::new_client);

    threadPool.setMaxThreadCount(10);
}

ServerFile::~ServerFile()
{
}

void ServerFile::startConnect(unsigned short port)
{
    m_server->listen(QHostAddress::Any, port);
}

void ServerFile::endConnect()
{
    while (list_socket.count())
    {
        emit list_socket.first()->disconnected();
    }
    m_server->disconnect();
    m_server->close();
}

void ServerFile::setFolderName(QString dir)
{
    folderName = dir;
}

void ServerFile::addSendInfo(SFileInfo *info)
{
    sendId[info->sendId] = info;
}

void ServerFile::new_client()
{
    QTcpSocket *socket = m_server->nextPendingConnection();
    list_socket.append(socket); // 添加当前链接

    connect(socket, &QTcpSocket::disconnected, this, [=]() { // 断开连接后的操作
        socket->close();
        list_socket.removeOne(socket); // list移除断开的socket
    });

    if (true == socket->waitForReadyRead(5000))
    { // 5s内收到消息,则分析数据
        // 分析数据
        QString data = QString(socket->readAll());
        msgSymbel head = msgSymbel(data.section("##", 0, 0).toInt());

        if (head == FILE_SEND_APPLY)
        { // 文件发送申请
            QString *filePath = new QString(folderName);
            QString *fileName = new QString(data.section("##", 1, 1));
            qint64 fileSize = data.section("##", 2, 2).toInt();

            SFileInfo *info = new SFileInfo(filePath, fileName, fileSize);
            SRecvFile *rFile = new SRecvFile(socket, info, this);
            emit ServerFile::recvFile(info, rFile);

            QThread::msleep(50);
            threadPool.start(rFile);
            // connect(rFile, &SRecvFile::recvFinish, socket, &QTcpSocket::disconnected);
        }
        else if (head == FILE_RECV_RESPONSE)
        { // 文件接收回应
            qint16 id = data.section("##", 1, 1).toInt();
            SFileInfo *info = sendId[id];

            SSendFile *sFile = new SSendFile(socket, info, this);
            emit ServerFile::sendFile(info, sFile);

            QThread::msleep(50);
            threadPool.start(sFile);
        }
    }
    else
        emit socket->disconnected();
}