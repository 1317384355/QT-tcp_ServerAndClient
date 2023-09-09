#include "SSendFile.h"

SSendFile::SSendFile(QTcpSocket *socket, SFileInfo *info, QObject *parent) : QObject(parent)
{
    m_tcp = socket;
    // 读取文件信息
    filePath = *info->filePath;
    fileName = *info->fileName;
    fileSize = info->fileSize;
}

SSendFile::~SSendFile()
{
}

void SSendFile::run()
{
    emit sendStart(fileName + " send start");

    QFile file(filePath);
    if (false == file.open(QIODevice::ReadOnly))
    {
        emit sendError(FILE_OPEN_ERROR);
        return;
    }

    dataSize = 0;
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
        m_tcp->write(buf, len);
        // 每5%进度刷新一次
        if (percent % 5 == 0)
            m_tcp->flush();

    } while (len > 0); // len < 0 表示读取 结束/错误

    // 文件发送结束验证
    if (dataSize == fileSize)
    {
        m_tcp->flush();
        emit sendFinish(fileName + " send finished");
    }
    else
    {
        emit sendError(FILE_SEND_ERROR);
    }
    file.close();
}
