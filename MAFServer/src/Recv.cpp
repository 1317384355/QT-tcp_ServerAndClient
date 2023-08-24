#include "Recv.h"

Recv::Recv(QTcpSocket *tcp, QObject *parent) : QThread(parent)
{
    m_tcp = tcp;
    connect(this, &Recv::disconnected, m_tcp, &QTcpSocket::disconnected);
}

Recv::~Recv()
{
    m_tcp->deleteLater();
}

QString Recv::getAddress()
{
    return m_tcp->localAddress().toString();
}

void Recv::run()
{
    QFile *file = new QFile("D:\\recv.txt");
    file->open(QFile::WriteOnly);

    connect(m_tcp, &QTcpSocket::readyRead, this, [=]()
            {
                static int count = 0;
                static int dateSize = 0;
                if (count = 0)
                {
                    m_tcp->read((char *)&dateSize, 4);
                }
                QByteArray data = m_tcp->readAll();
                count+= data.size();
                file->write(data);

                if(count == dateSize)
                {
                    m_tcp->close();
                    file->close();
                    file->deleteLater();
                    emit recvOver();
                } });

    // 进入事件循环
    exec();
}