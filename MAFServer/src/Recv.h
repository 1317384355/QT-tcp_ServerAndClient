#pragma once
#include <QThread>
#include <QTcpSocket>
#include <QHostAddress>
#include <QFile>

class Recv : public QThread
{
    Q_OBJECT

signals:
    void recvOver();
    void disconnected();

public:
    explicit Recv(QTcpSocket *tcp, QObject *parent = nullptr);
    ~Recv();

    QString getAddress();

public slots:

protected:
    void run() override;

private:
    QTcpSocket *m_tcp;
};
