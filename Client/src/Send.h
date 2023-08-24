#pragma once
#include <QTcpSocket>
#include <QHostAddress>
#include <QFile>
#include <QFileInfo>
#include <QTimer>

class Send : public QObject
{
    Q_OBJECT

signals:
    void connected();
    void disconnected();
    void readyRead(QByteArray date);
    void curPercent(int percent);

public:
    explicit Send(bool isFile = false, QObject *parent = nullptr);
    ~Send();

public slots:
    void startConnect(QString ip, unsigned short port);
    void sendMsg(QString msg);
    void sendFile(QString path);
    void sendFileData();
    // void resvFile();
    void endConnect();

private:
    QTcpSocket *m_tcp;
    QFile file;          // 文件对象
    QString filename;    // 文件名字
    qint16 filesize;     // 文件大小
    qint16 sendfilesize; // 已发送大小

    QTimer *timer;
    bool isFileSocket = false;
};
