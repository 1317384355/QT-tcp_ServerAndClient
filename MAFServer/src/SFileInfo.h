#pragma once
#include <QString>

class SFileInfo
{
public:
    QString *filePath; // 发送文件是完整路径, 接收文件仅有文件夹
    QString *fileName;
    qint64 fileSize;
    qint16 sendId;
    SFileInfo(QString *path, QString *file, qint64 size, qint16 id = -1);
    ~SFileInfo();
};
