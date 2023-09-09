#include "SRecvFile.h"

SRecvFile::SRecvFile(QTcpSocket *socket, SFileInfo *info, QObject *parent) : QObject(parent)
{
    m_tcp = socket;

    filePath = *info->filePath;
    fileName = *info->fileName;
    fileSize = info->fileSize;
    dataSize = 0;
}

SRecvFile::~SRecvFile()
{
}

void SRecvFile::run()
{
    // 打开文件
    QFile file(filePath + "/" + fileName);
    if (false == file.open(QIODevice::WriteOnly))
    {
        emit recvError(FILE_OPEN_ERROR);
        return;
    }

    emit recvStart(fileName + " recv start"); // 发送信号 文件开始接收
    // 接收文件
    dataSize = 0; // 已接收数据长度初始化
    while (true == m_tcp->waitForReadyRead(5000))
    { // 超时(5s)无信息接收结束循环
        // 从socket中读取数据
        QByteArray buf = m_tcp->readAll();
        // 数据写入文件
        qint64 len = file.write(buf);
        if (len > 0)
        { // 接收计算百分比
            dataSize += len;
            int percent = (static_cast<int>(dataSize) * 100) / fileSize;
            emit curPercent(percent); // 向ui界面发送百分比
        }
        if (dataSize >= fileSize)
            break;
    }

    if (dataSize == fileSize)
        emit recvFinish(fileName + " recv finished"); // 发送信号 文件接收完全
    else
        emit recvError(FILE_RECV_ERROR);

    file.close();       // 接收结束后, 关闭文件
    file.deleteLater(); // 析构
    this->deleteLater();
}