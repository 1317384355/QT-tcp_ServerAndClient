#pragma once
#include <QTcpSocket>
#include <QHostAddress>
#include <QThread>
#include <QFile>
#include <QFileInfo>
#include <QTimer>

class Send : public QObject
{
    Q_OBJECT

signals:
    void connected();
    void disconnected();
    void recvFile(QByteArray date);
    void recvMsg(QByteArray date);
    // void
    void curPercent(int percent);

public:
    explicit Send(QObject *parent = nullptr);
    ~Send();

    void startConnect(QHostAddress ip, unsigned short msg_port, unsigned short file_port);
public slots:
    void sendMsg(QString msg);
    void sendFile(QString path);
    void sendFileData();
    // void resvFile();
    void endConnect();

private:
    QTcpSocket *m_msgSocket, *m_fileSocket;
    QThread *msgThread, *fileThread;
    QFile file;          // 文件对象
    QString filename;    // 文件名字
    qint16 filesize;     // 文件大小
    qint16 sendfilesize; // 已发送大小

    QTimer *timer;
};
