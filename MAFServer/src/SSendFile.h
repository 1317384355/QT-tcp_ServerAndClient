#pragma once
#include "command.h"
#include "SFileInfo.h"
#include <QRunnable>

class SSendFile : public QObject, public QRunnable
{
    Q_OBJECT

signals:
    void sendStart(QString text);
    void sendError(int error);
    void curPercent(int percent);
    void sendFinish(QString text);

public:
    explicit SSendFile(QTcpSocket *socket, SFileInfo *info, QObject *parent = nullptr);
    ~SSendFile();

protected:
    void run() override;

private:
    QTcpSocket *m_tcp;

    QString filePath; // 待发送文件路径 + 文件名
    QString fileName; // 待发送文件名
    qint64 fileSize;  // 待发送数据大小
    qint64 dataSize;  // 已发送数据大小
};
