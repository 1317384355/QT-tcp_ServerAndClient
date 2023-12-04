#include "SSendFile.h"

SSendFile::SSendFile(QTcpSocket *socket, SFileInfo *info, QObject *parent) : QObject(parent)
{
    m_tcp = socket;
    // 读取文件信息
    filePath = info->filePath;
    fileName = info->fileName;
    fileSize = info->fileSize;
}

SSendFile::~SSendFile()
{
}

void SSendFile::run()
{
    emit sendStart(QString("%1##%2").arg(filePath).arg(fileSize));

    QFile file(filePath);
    if (false == file.open(QIODevice::ReadOnly))
    {
        emit sendError(FILE_OPEN_ERROR);
        return;
    }

    dataSize = 0;  // 已接收数据长度初始化
    sendTimes = 0; // 接收次数初始化

    // 开始发送文件, 循环保证文件 发送结束/错误 前不会run函数不会结束
    for (qint64 len; dataSize < fileSize; sendTimes++)
    {
        // 每次发送大小
        char buf[SEND_SIZE] = {0};
        // 记录每次读取数据长度
        len = file.read(buf, SEND_SIZE);
        // 发送数据
        m_tcp->write(buf, len);
        // 记录每次发送数据长度
        dataSize += len;
        // 减少向ui界面发送信号的次数,并增加sleep避免数据发送过快接收方无法及时解析
        if (sendTimes % 5 == 0)
        {
            int percent = dataSize * 100 / fileSize; // 计算百分比
            emit curPercent(percent);                // 向ui界面发送百分比
            QThread::usleep(10);                     // 线程休眠10us
        }

        // 清空缓冲区,避免多次堆积一次发送
        m_tcp->flush();
    }

    m_tcp->waitForBytesWritten();
    QThread::usleep(20);
    file.close();
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
