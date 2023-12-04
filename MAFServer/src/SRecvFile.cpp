#include "SRecvFile.h"

SRecvFile::SRecvFile(QTcpSocket *socket, SFileInfo *info, QObject *parent) : QObject(parent)
{
    m_tcp = socket;

    filePath = info->filePath;
    fileName = info->fileName;
    fileSize = info->fileSize;
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

    // 接收文件
    emit recvStart(fileName + " recv start"); // 发送信号 文件开始接收
    dataSize = 0;                             // 已接收数据长度初始化
    recvTimes = 0;                            // 接收次数初始化

    while (dataSize < fileSize)
    {
        if (m_tcp->bytesAvailable() > DATA_SIZE || dataSize + m_tcp->bytesAvailable() >= fileSize)
        {
            // 从socket中读取数据
            QByteArray buf = m_tcp->read(DATA_SIZE);
            // 数据写入文件
            qint64 len = file.write(buf);
            if (len > 0)
            { // 接收计算百分比
                dataSize += len;
                if (recvTimes % 5 == 0)
                {
                    int percent = dataSize * 100 / fileSize;
                    emit curPercent(percent); // 向ui界面发送百分比
                }
                recvTimes++;
            }
        }
    }
    // 关闭文件
    file.waitForBytesWritten(500); // 等待文件写入完毕
    file.close();                  // 接收结束后, 关闭文件

    // 关闭socket
    m_tcp->disconnectFromHost();
    m_tcp->close();

    // 接收完毕
    if (dataSize == fileSize)
    {
        emit curPercent(100);                         // 最后一次发送不一定能到达100
        emit recvFinish(fileName + " recv finished"); // 发送信号 文件接收完全
    }
    else
    {
        emit recvError(FILE_RECV_ERROR); // 发送信号 文件接收错误
    }
    emit waitDelete();
}