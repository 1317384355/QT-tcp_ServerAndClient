#pragma once
#include <QFile>

class CFileInfo
{
public:
    int sendId;        // 文件发送ID, id == -1表示该线程为发送线程
    QString *filePath; // 文件路径+文件名
    QString *fileName; // 文件名
    qint64 *fileSize;  // 文件大小
    CFileInfo(QString *path, QString *name, qint64 *size, int id = -1);
    ~CFileInfo();
    void setID(int id);
};
