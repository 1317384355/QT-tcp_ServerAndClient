#include "ServerFile.h"

ServerFile::ServerFile(QObject *parent) : QObject(parent),
                                          m_server(nullptr)
{
    threadPool.setMaxThreadCount(10);
}

ServerFile::~ServerFile()
{
}

void ServerFile::on_startConnect()
{
    if (m_server == nullptr)
    {
        m_server = new QTcpServer(this);
        connect(m_server, &QTcpServer::newConnection, this, &ServerFile::new_client);
    }
    m_server->listen(QHostAddress::Any, SEND_FILE_PORT);
}

void ServerFile::on_endConnect()
{
    threadPool.waitForDone();
    m_server->disconnect();
    m_server->close();
}

void ServerFile::setFolderName(QString dir)
{
    folderName = dir;
}

void ServerFile::addSendInfo(SFileInfo *info)
{ // 加快检索速度
    map_sendId[info->sendId] = (info);
}

void ServerFile::new_client()
{
    QTcpSocket *socket = m_server->nextPendingConnection();

    if (true == socket->waitForReadyRead(500))
    { // 5s内收到消息,则分析数据
        // 分析数据
        QString data = QString(socket->readAll());
        msgSymbel head = msgSymbel(data.section("##", 0, 0).toInt());

        if (head == FILE_SEND_APPLY)
        { // 收到文件发送申请
            QString filePath(folderName);
            QString fileName = data.section("##", 1, 1);
            qint64 fileSize = data.section("##", 2, 2).toInt();

            SFileInfo *info = new SFileInfo(filePath, fileName, fileSize);
            SRecvFile *rFile = new SRecvFile(socket, info, this);
            connect(rFile, &SRecvFile::waitDelete, this, [socket]() { //
                socket->close();
                socket->deleteLater();
            });
            emit ServerFile::recvFile(info, rFile);

            // 休眠10ms等待ui界面加载
            threadPool.start(rFile);

            // connect(rFile, &SRecvFile::recvFinish, socket, &QTcpSocket::disconnected);
        }
        else if (head == FILE_RECV_RESPONSE)
        { // 文件接收回应
            qint16 id = data.section("##", 1, 1).toInt();
            SFileInfo *info = map_sendId[id];
            map_sendId.erase(id); // 删除id,防止重复发送

            SSendFile *sFile = new SSendFile(socket, info, this);
            emit ServerFile::sendFile(info, sFile);

            threadPool.start(sFile); // 启动线程
        }
    }
    else
        emit socket->close(); // 关闭socket
}