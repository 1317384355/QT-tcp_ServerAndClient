#pragma once
#include "command.h"

class SRecvFile : public QObject, public QRunnable
{
    Q_OBJECT

signals:
    void recvError(int error);
    void recvStart(QString text);
    void curPercent(int percent);
    void recvFinish(QString text);
    void waitDelete();

public:
    explicit SRecvFile(QTcpSocket *socket, SFileInfo *info, QObject *parent = nullptr);
    ~SRecvFile();

protected:
    void run() override;

private:
    QTcpSocket *m_tcp; // 接收用socket

    QString filePath; // 文件储存路径
    QString fileName; // 待接收文件路径
    qint64 fileSize;  // 待接收数据大小
    qint64 dataSize;  // 已接收数据大小

    int recvTimes; // 接收次数,用于减少发送百分比信号的次数
};