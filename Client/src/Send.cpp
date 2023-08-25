#include "Send.h"

Send::Send(QObject *parent) : QObject(parent)
{
    m_msgSocket = new QTcpSocket(this);
    msgThread = new QThread(this);
    m_msgSocket->moveToThread(msgThread);

    m_fileSocket = new QTcpSocket(this);
    fileThread = new QThread(this);
    m_fileSocket->moveToThread(fileThread);

    timer = new QTimer(this);
}

Send::~Send()
{
}

void Send::startConnect(QHostAddress ip, unsigned short msg_port, unsigned short file_port)
{
    msgThread->start();
    fileThread->start();

    m_msgSocket->connectToHost(ip, msg_port);
    // 这个不放在waitForConnected函数前会导致 连接成功 后不会发出connected信号
    connect(m_msgSocket, &QTcpSocket::connected, this, [=]() { //
        m_fileSocket->connectToHost(ip, file_port);
        connect(m_fileSocket, &QTcpSocket::connected, this, &Send::connected);
        if (m_fileSocket->waitForConnected(2000) == false) // 阻塞线程，直到连接完成或超时。
        {                                                  // 5s未连接，则断开
            this->endConnect();
        }
    });

    if (m_msgSocket->waitForConnected(2000) == false) // 阻塞线程，直到连接完成或超时。
    {                                                 // 5s未连接，则断开
        this->endConnect();
    }

    connect(m_msgSocket, &QTcpSocket::readyRead, this, [=]()
            { emit Send::recvMsg(m_msgSocket->readAll()); });

    connect(m_fileSocket, &QTcpSocket::disconnected, this, &Send::endConnect);
    connect(m_msgSocket, &QTcpSocket::disconnected, this, &Send::endConnect);

    connect(timer, &QTimer::timeout, [=]() { //
        // 关闭定时器
        timer->stop();
        // 发送文件数据
        sendFileData();
    });
}

void Send::sendMsg(QString msg)
{
    m_msgSocket->write(msg.toUtf8().data());
}

// 发送文件
void Send::sendFile(QString path)
{
    file.setFileName(path);
    // 获取文件信息
    QFileInfo info(path);
    filesize = info.size();     // 获取文件大小
    filename = info.fileName(); // 获取文件名
    // 只读方式打开文件
    bool isOk = file.open(QIODevice::ReadOnly);
    if (true == isOk)
    {
        // 创建文件头信息
        QString head = QString("%1##%2").arg(filename).arg(filesize);
        // 发送头部信息
        qint64 len = m_fileSocket->write(head.toUtf8());
        if (len > 0)
        {
            // 防止tcp黏包问题,延时20ms
            timer->start(20);
        }
        else
        {
            // 发送失败
            file.close();
        }
    }
}

void Send::sendFileData()
{
    qint64 len = 0;
    // 读多少发多少
    do
    {
        // 每次发送的大小
        char buf[4 * 1024] = {0};
        // 记录每行数据
        len = file.read(buf, sizeof(buf));
        // 计算百分比，发给主线程
        sendfilesize += len;
        int percent = (static_cast<int>(sendfilesize) * 100) / filesize;
        // 发出更新进度条的信号
        emit curPercent(percent);
        // 发送数据
        m_fileSocket->write(buf, len);
    } while (len > 0);

    if (sendfilesize == filesize)
    {
        file.close();
        filename.clear();
        filesize = 0;
    }
}

void Send::endConnect()
{
    m_msgSocket->close();
    msgThread->quit();
    msgThread->wait();

    m_fileSocket->close();
    fileThread->quit();
    fileThread->wait();

    emit Send::disconnected();
}
