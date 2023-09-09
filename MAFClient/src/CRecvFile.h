#pragma once
#include "command.h"
#include "CFileInfo.h"
#include <QTimer>

class CRecvFile : public QThread
{
    Q_OBJECT

signals:
    void recvError(int error);
    void recvStart(QString text);
    void curPercent(int percent);
    void recvFinish(QString text);

public:
    explicit CRecvFile(QObject *parent = nullptr);
    ~CRecvFile();

    void setInfo(QString ip, CFileInfo *info, QString folder);

protected:
    void run() override;

private:
    QTcpSocket *m_tcp; // 接收用socket

    int sendId;       // 文件ID,用于和服务器验证
    QFile *file;      // 文件
    QString filePath; // 文件储存路径
    QString fileName; // 待接收文件路径
    qint64 fileSize;  // 待接收数据大小
    qint64 dataSize;  // 已接收数据大小
};