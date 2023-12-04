#include "CSendFile.h"

CSendFile::CSendFile(QString ip, CFileInfo *info, QObject *parent) : QThread(parent)
{
    // 读取文件信息
    addr = ip;
    filePath = *info->filePath;
    fileName = *info->fileName;
    fileSize = *info->fileSize;
}

CSendFile::~CSendFile()
{
}

void CSendFile::setIp(QString ip)
{
    addr = ip;
}

void CSendFile::run()
{
    QTcpSocket m_tcp;
    m_tcp.connectToHost(addr, FILE_PORT);
    if (false == m_tcp.waitForConnected(500))
    {
        emit sendError(TCP_CONNECT_ERROR);
        return;
    }

    QFile file(filePath);
    if (false == file.open(QIODevice::ReadOnly))
    {
        m_tcp.close();
        emit sendError(FILE_OPEN_ERROR);
        return;
    }

    // 开始发送
    this->msleep(25);
    emit sendStart(fileName + " send start");
    dataSize = 0;
    sendTimes = 0;
    // 创建头信息
    QString head = QString("%1##%2##%3").arg(FILE_APPLY).arg(fileName).arg(fileSize);
    // 发送头信息, > 0 表示发送成功
    if (m_tcp.write(head.toUtf8()) > 0)
    {
        m_tcp.flush();
        this->msleep(50);
    }
    else
    {
        m_tcp.close();
        file.close();
        emit sendError(HEAD_SEND_ERROR);
        return;
    }

    // 开始发送文件, 循环保证文件 发送结束/错误 前不会run函数不会结束
    for (qint64 len; dataSize < fileSize; sendTimes++)
    {
        // 每次发送大小
        char buf[SEND_SIZE] = {0};
        // 记录每次读取数据长度
        len = file.read(buf, SEND_SIZE);
        // 发送数据
        m_tcp.write(buf, len);
        // 记录每次发送数据长度
        dataSize += len;
        // 减少向ui界面发送信号的次数,并增加sleep避免数据发送过快接收方无法及时解析
        if (sendTimes % 5 == 0)
        {
            int percent = dataSize * 100 / fileSize; // 计算百分比
            emit curPercent(percent);                // 向ui界面发送百分比
            this->usleep(10);                        // 防止发送过快
        }
        // 更新进度
        m_tcp.flush();
    }
    // 结束,释放资源
    m_tcp.waitForBytesWritten();
    this->usleep(20);
    file.close();
    m_tcp.waitForDisconnected();
    // 文件发送结束验证
    if (dataSize == fileSize)
    {
        emit curPercent(100); // 最后一次发送不一定能到达100
        emit sendFinish(fileName + " send finished");
    }
    else
    {
        emit sendError(FILE_SEND_ERROR);
    }
}
