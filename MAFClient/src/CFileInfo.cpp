#include "CFileInfo.h"

CFileInfo::CFileInfo(QString *path, QString *name, qint64 *size, int id)
{
    filePath = path;
    fileName = name;
    fileSize = size;
    sendId = id;
}

CFileInfo::~CFileInfo()
{
    delete filePath;
    delete fileName;
    delete fileSize;
}

void CFileInfo::setID(int id)
{
    sendId = id;
}
