#include "Send.h"

Send::Send(bool isFile, QObject *parent) : QObject(parent)
{
    m_tcp = new QTcpSocket(this);
    isFileSocket = isFile;
}

Send::~Send()
{
    m_tcp->deleteLater();
}

void Send::startConnect(QString ip, unsigned short port)
{
    // 如果连接不到就会卡在这里，暂时不会解决
    m_tcp->connectToHost(QHostAddress(ip), port);
    connect(m_tcp, &QTcpSocket::connected, this, &Send::connected);
    if (m_tcp->waitForConnected(5000) == false)
    { // 5s未连接，则断开
        this->endConnect();
    }

    connect(m_tcp, &QTcpSocket::readyRead, this, [=]()
            { emit Send::readyRead(m_tcp->readAll()); });
    connect(m_tcp, &QTcpSocket::disconnected, this, &Send::endConnect);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [=]()
            {
        //关闭定时器
        timer->stop();
        //发送文件数据
        sendFileData(); });
}

void Send::sendMsg(QString msg)
{
    m_tcp->write(msg.toUtf8().data());
}

// void Send::sendFile(QString path)
// {
//     QFile file(path);
//     QFileInfo info(path);
//     int fileSize = info.size();

//     if (file.open(QFile::ReadOnly) == false)
//     {
//         return;
//     }

//     while (file.atEnd() == false)
//     {
//         static int num = 0;
//         if (num == 0)
//         {
//             m_tcp->write((char *)&fileSize, 4);
//         }

//         QByteArray line = file.readLine();
//         num += line.size();
//         int percent = (num * 100 / fileSize);
//         emit curPercent(percent);
//         m_tcp->write(line);
//     }
// }

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
        qint64 len = m_tcp->write(head.toUtf8());
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
    else
    {
        return;
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
        m_tcp->write(buf, len);
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
    m_tcp->close();
    emit Send::disconnected();
}
