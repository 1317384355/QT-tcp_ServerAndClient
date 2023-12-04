#include "SFileInfo.h"
SFileInfo::SFileInfo(QString path, QString file, qint64 size, qint16 id)
{
    filePath = path;
    fileName = file;
    fileSize = size;
    sendId = id;
}

SFileInfo::~SFileInfo()
{
}