#include "ClientMsg.h"

ClientMsg::ClientMsg(QObject *parent) : QObject(parent)
{
    m_tcp = new QTcpSocket(this); // 接收消息
    connect(m_tcp, &QTcpSocket::readyRead, this, &ClientMsg::readData);
    // 若连接发出断开信号, 则调用断开函数
    connect(m_tcp, &QTcpSocket::disconnected, this, &ClientMsg::endConnect);
}

ClientMsg::~ClientMsg()
{
    m_tcp->disconnect();
}

void ClientMsg::startConnect(QString ip, unsigned short port)
{
    m_tcp->connectToHost(ip, port);
    if (true == m_tcp->waitForConnected(2000))
    { // 阻塞线程,直到连接 成功/失败
        emit ClientMsg::connected();
    }
    else
    { // 失败,调用关闭程序
        this->endConnect();
    }
}

void ClientMsg::sendMsg(QString msg)
{
    m_tcp->write((QString::number(MASSAGE) + "##" + msg).toUtf8());
}

void ClientMsg::readData()
{
    // 分析数据
    QString data = QString(m_tcp->readAll());
    msgSymbel head = msgSymbel(data.section("##", 0, 0).toInt());
    if (head == MASSAGE)
    {
        emit ClientMsg::recvMsg(data.section("##", 1, 1));
    }
    else if (head == FILE_APPLY)
    {
        QString fileName = data.section("##", 1, 1);
        qint64 fileSize = data.section("##", 2, 2).toInt();
        int sendId = data.section("##", 3, 3).toInt();
        emit ClientMsg::recvFileApply(fileName, fileSize, sendId);
    }
}

void ClientMsg::endConnect()
{
    m_tcp->disconnectFromHost();    // 关闭 socket
    emit ClientMsg::disconnected(); // 向主程序出已关闭信号
}