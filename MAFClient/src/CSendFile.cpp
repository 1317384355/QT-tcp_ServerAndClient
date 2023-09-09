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

void CSendFile::run()
{
    emit sendStart(fileName + " send start");

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

    this->msleep(100);
    dataSize = 0;
    // 创建头信息
    QString head = QString("%1##%2##%3").arg(FILE_APPLY).arg(fileName).arg(fileSize);
    // 发送头信息, > 0 表示发送成功
    if (m_tcp.write(head.toUtf8()) > 0)
    {
        m_tcp.flush();
        this->msleep(100);
    }
    else
    {
        m_tcp.close();
        file.close();
        emit sendError(HEAD_SEND_ERROR);
        return;
    }

    // 开始发送文件, 循环保证文件 发送结束/错误 前不会run函数不会结束
    qint64 len = 0;
    do
    {
        // 每次发送大小
        char buf[4 * 1024] = {0};
        // 记录每次读取数据长度
        len = file.read(buf, sizeof(buf));
        // 计算百分比
        dataSize += len;
        int percent = (static_cast<int>(dataSize) * 100) / fileSize;
        // 更新进度
        emit curPercent(percent);
        // 发送数据
        m_tcp.write(buf, len);
        m_tcp.flush();

    } while (len > 0); // len < 0 表示读取 结束/错误

    // 文件发送结束验证
    if (dataSize == fileSize)
    {
        m_tcp.flush();
        emit sendFinish(fileName + " send finished");
    }
    else
    {
        emit sendError(FILE_SEND_ERROR);
    }
    file.close();
    m_tcp.disconnectFromHost();
}
