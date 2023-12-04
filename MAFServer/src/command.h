#pragma once
#include "SFileInfo.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QRunnable>
#include <QThread>
#include <QFile>
#include <QTime>

#define MSG_PORT 8000
#define SEND_FILE_PORT 8001

// 每次发送 8*1024*sizeof(char) 大小的数据
#define SEND_SIZE 8192
#define DATA_SIZE 65536

enum errorOnFile
{
    TCP_CONNECT_ERROR = 100, // socket 打开失败
    FILE_OPEN_ERROR,         // file 打开失败
    SEND_ID_NULL,            // 未设置 sendId
    HEAD_SEND_ERROR,         // fileHead 发送失败
    FILE_SEND_ERROR,         // 文件发送失败
    FILE_RECV_ERROR,         // 文件接收失败
};

enum msgSymbel
{
    MASSAGE = 0,        // 聊天消息
    FILE_SEND_APPLY,    // 文件发送申请
    FILE_RECV_RESPONSE, // 文件接收回应
};

/*  消息格式 用SEND_ID来告诉服务器该发送哪个文件
PORT = 8000;
MASSAGE    + ## + msg
FILE_APPLY + ## + fileName + ## + fileSize + ## + SEND_ID
PORT = 8001;
FILE_APPLY + ## + fileName + ## + fileSize
FILE_RECV  + ## + SEND_ID
*/