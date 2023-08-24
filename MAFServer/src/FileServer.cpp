#include "FileServer.h"

FileServer::FileServer(QObject *parent) : QObject(parent)
{
    m_server = new QTcpServer();
}

FileServer::~FileServer()
{
    m_server->deleteLater();
}

void FileServer::startConnect(unsigned short port)
{
    m_server->listen(QHostAddress::Any, port);

    // 放这里是因为如果放在构造函数里，关闭服务器再开启服务器后，将无法正常发出信号
    connect(m_server, &QTcpServer::newConnection, this, &FileServer::new_client);
}

void FileServer::endConnect()
{
    m_tcp->disconnected();
    m_server->disconnect();
    m_server->close();
}

void FileServer::setFolderName(QString &dir)
{
    folderName = dir;
}

// void FileServer::SendFile()
// {
// }
void FileServer::new_client()
{
    m_tcp = m_server->nextPendingConnection();

    emit listAddNewSocket(m_tcp->peerAddress().toString());

    // 接收数据
    connect(m_tcp, &QTcpSocket::readyRead, this, &FileServer::recvFile);

    // 断开连接后的操作
    connect(m_tcp, &QTcpSocket::disconnected, this, [=]()
            {
                m_tcp->close();
                emit FileServer::listRemoveSocket(0); });
}

void FileServer::recvFile()
{
    // 取出接收的内容
    QByteArray buf = m_tcp->readAll();
    if (true == isStart)
    {
        // 接收头部
        isStart = false;
        // 解析头部
        filename = QString(buf).section("##", 0, 0);
        filesize = QString(buf).section("##", 1, 1).toInt();
        recvfilesize = 0;
        // 打开文件
        file.setFileName(filename);
        QDir::setCurrent(folderName);
        bool isOk = file.open(QIODevice::WriteOnly);
        if (false == isOk)
        {
            return;
        }
    }
    else
    {
        qint64 len = file.write(buf);
        if (len > 0)
        {
            recvfilesize += len; // 累计接收大小
            int percent = (static_cast<int>(recvfilesize) * 100) / filesize;
            // 发出更新进度条的信号
            emit curPercent(percent);
        }

        if (recvfilesize == filesize)
        {
            file.close();
            isStart = true;
        }
    }
}