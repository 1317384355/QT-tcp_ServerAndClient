#pragma once
#include "command.h"
#include "CFileInfo.h"

class CSendFile : public QThread
{
    Q_OBJECT

signals:
    void sendStart(QString text);
    void sendError(int error);
    void curPercent(int percent);
    void sendFinish(QString text);

public:
    explicit CSendFile(QString ip, CFileInfo *info, QObject *parent = nullptr);
    ~CSendFile();

    void setIp(QString ip);

protected:
    void run() override;

private:
    QString addr;
    QString filePath; // 待发送文件路径 + 文件名
    QString fileName; // 待发送文件名
    qint64 fileSize;  // 待发送数据大小
    qint64 dataSize;  // 已发送数据大小

    int sendTimes; // 发送次数
};
