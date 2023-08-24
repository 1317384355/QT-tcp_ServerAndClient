#include "Server.h"
#if 1
Server::Server(QWidget *parent)
    : QMainWindow(parent), ui(new Ui_Server)
{
    ui->setupUi(this);

    setWindowTitle("服务器");           // 设置窗口名字
    flags = windowFlags();              // 保存初始窗口状态
    ui->textEdit->setEnabled(false);    // 消息显示窗口设为不可编辑
    ui->btnSendMsg->setEnabled(false);  // 发送消息初始不可用
    ui->btnSendFile->setEnabled(false); // 发送文件初始不可用

    // 状态栏显示
    labelStatus = new QLabel(this);
    ui->statusbar->addWidget(labelStatus);

    m_msgServer = new MsgServer(this);
    msgThread = new QThread();
    m_msgServer->moveToThread(msgThread);

    m_fileServer = new FileServer(this);
    recvFolder = QDir::currentPath(); // 保存文件路径初始化为当前运行路径
    m_fileServer->setFolderName(recvFolder);
    fileThread = new QThread();
    m_fileServer->moveToThread(msgThread);
}

Server::~Server()
{
    delete ui;
}

void Server::forbidClose()
{
    // 懒得重写关闭按钮功能，就偷懒直接 禁用 窗口关闭按钮
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint);
    this->show();
}

void Server::recvMsg(QString ip, QByteArray date)
{
    ui->textEdit->append(ip.right(ip.size() - 7) + ":");
    ui->textEdit->append(date + "\n");
}

void Server::listMsgAddItem(QString ip)
{
    if (ui->listWidgetMsg->count() == 0)
    {
        ui->btnSendMsg->setEnabled(true);
        labelStatus->setText("连接数：" + QString::number(ui->listWidgetMsg->count() + ui->listWidgetFile->count()));
    }
    ui->listWidgetMsg->addItem(ip.right(ip.size() - 7));
}

void Server::listMsgRemoveItem(int index)
{
    ui->listWidgetMsg->takeItem(index);
    if (ui->listWidgetMsg->count() == 0)
    {
        ui->btnSendMsg->setEnabled(false);
        if (ui->listWidgetFile->count() == 0)
            labelStatus->setText("等待连接中...");
    }
}

void Server::listFileAddItem(QString ip)
{
    if (ui->listWidgetFile->count() == 0)
    {
        ui->btnSendFile->setEnabled(true);
        labelStatus->setText("连接数：" + QString::number(ui->listWidgetMsg->count() + ui->listWidgetFile->count()));
    }
    ui->listWidgetFile->addItem(ip.right(ip.size() - 7));
}

void Server::listFileRemoveItem(int index)
{
    ui->listWidgetFile->takeItem(index);
    if (ui->listWidgetFile->count() == 0)
    {
        ui->btnSendMsg->setEnabled(false);
        if (ui->listWidgetMsg->count() == 0)
            labelStatus->setText("等待连接中...");
    }
}

void Server::on_btnStart_clicked()
{
    labelStatus->setText("等待连接中...");

    m_msgServer->startConnect(MSGPORT);   // 开启消息监听端口
    m_fileServer->startConnect(FILEPORT); // 开启文件监听端口

    ui->btnStart->setEnabled(false); // 禁止重复开启
    forbidClose();
    connect(m_msgServer, &MsgServer::listAddNewSocket, this, &Server::listMsgAddItem);
    connect(m_msgServer, &MsgServer::listRemoveSocket, this, &Server::listMsgRemoveItem);
    connect(m_msgServer, &MsgServer::readyRead, this, &Server::recvMsg);

    connect(m_fileServer, &FileServer::listAddNewSocket, this, &Server::listFileAddItem);
    connect(m_fileServer, &FileServer::listRemoveSocket, this, &Server::listFileRemoveItem);
    connect(m_fileServer, &FileServer::curPercent, ui->barRecv, &QProgressBar::setValue);

    msgThread->start();
    msgThread->start();
}

void Server::on_btnSendMsg_clicked()
{ // 向所有的客户端发送当前输入文本框的信息
    QString msg = ui->plainTextEdit->toPlainText();
    m_msgServer->SendMsg(msg);
    ui->textEdit->append("server:\n" + msg + "\n");
    ui->plainTextEdit->clear();
}

void Server::on_btnSelectFile_clicked()
{
    ui->lineSend->setText(QFileDialog::getOpenFileName());
}

void Server::on_btnSendFile_clicked()
{
}

void Server::on_btnChooseRecvFolder_clicked()
{
    recvFolder = QFileDialog::getExistingDirectory();
    m_fileServer->setFolderName(recvFolder);
    ui->lineRecv->setText(recvFolder);
}

void Server::on_btnClose_clicked()
{
    disconnect(m_msgServer, &MsgServer::listAddNewSocket, this, &Server::listMsgAddItem);
    disconnect(m_msgServer, &MsgServer::listRemoveSocket, this, &Server::listMsgRemoveItem);
    disconnect(m_msgServer, &MsgServer::readyRead, this, &Server::recvMsg);

    m_msgServer->endConnect();

    ui->btnStart->setEnabled(true); // 重新启用 开始服务器 按钮
    labelStatus->setText("服务器已关闭");
    setWindowFlags(flags); // 重新启用窗口 关闭 按钮
    this->show();          // 不添加show，窗口就会消失，不懂
}

// void Server::new_client()
// {
//     Recv *subRecv = new Recv(m_server->nextPendingConnection());
//     subRecv->start();

//     ui->textEdit->append(subRecv->getAddress() + " 已连接\n");
//     // ui->btnSend->setEnabled(true); // 有连接才 启用 发送按钮
//     if (list_recv.isEmpty())
//     {
//         // 懒得重写关闭按钮功能，就偷懒直接 禁用 窗口关闭按钮
//         setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint);
//         this->show();
//     }
//     list_recv.append(subRecv);
//     labelStatus->setText("连接设备：" + QString::number(list_recv.count()));

//     connect(subRecv, &Recv::recvOver, this, [=]()
//             {
//                 subRecv->exit();
//                 subRecv->wait(); });

//     // // 断开连接后的操作
//     // connect(socket, &QTcpSocket::disconnected, this, [=]()
//     //         {
//     //             socket->close();
//     //             list_socket.removeOne(socket); // 移除断开的socket
//     //             if (list_socket.isEmpty())     // 如果当前没有客户端连接，禁用发送
//     //             {
//     //                 ui->btnSend->setEnabled(false);
//     //             }

//     //             ui->textEdit->append(socket->peerAddress().toString() + " 已断开\n");
//     //             labelStatus->setText("连接设备：" + QString::number(list_socket.count())); });
// }
#else
Server::Server(QWidget *parent)
    : QMainWindow(parent), ui(new Ui_Server)
{
    ui->setupUi(this);

    setWindowTitle("服务器");        // 设置窗口名字
    flags = windowFlags();           // 保存初始窗口状态
    ui->textEdit->setEnabled(false); // 消息显示窗口设为不可编辑

    labelStatus = new QLabel(this);
    ui->statusbar->addWidget(labelStatus);

    m_server = new QTcpServer(this);
}

Server::~Server()
{
    delete ui;
}

void Server::on_btnStart_clicked()
{
    labelStatus->setText("等待连接中...");
    unsigned short port = ui->lineEdit->text().toShort(); // 读取当前输入端口
    m_server->listen(QHostAddress::Any, port);            // 开启监听端口
    ui->btnStart->setEnabled(false);                      // 禁止重复开启

    // 放这里是因为如果放在构造函数里，关闭服务器再开启服务器后，将无法正常发出信号
    connect(m_server, SIGNAL(newConnection()), this, SLOT(new_client()));
}

void Server::on_btnSend_clicked()
{ // 向所有的客户端发送当前输入文本框的信息
    QString msg = ui->plainTextEdit->toPlainText();
    for (int i = list_socket.count() - 1; i >= 0; i--)
    {
        if (list_socket[i])
            list_socket[i]->write(msg.toUtf8().data());
    }
    ui->textEdit->append("server:\n" + msg + "\n");
    ui->plainTextEdit->clear();
}

void Server::on_btnClose_clicked()
{
    // 先发出信号将所有的连接 断开
    while (list_socket.count())
    {
        emit list_socket.first()->disconnected();
    }

    // 关闭服务器监听
    m_server->disconnect();
    m_server->close();

    ui->btnStart->setEnabled(true); // 重新启用 开始服务器 按钮
    labelStatus->setText("服务器已关闭");
    setWindowFlags(flags); // 重新启用窗口 关闭 按钮
    this->show();          // 不添加show，窗口就会消失，不懂
}

void Server::new_client()
{
    QTcpSocket *socket = m_server->nextPendingConnection();
    ui->textEdit->append(socket->localAddress().toString() + " 已连接\n");
    ui->btnSend->setEnabled(true); // 有连接才 启用 发送按钮
    if (list_socket.isEmpty())
    {
        // 懒得重写关闭按钮功能，就偷懒直接 禁用 窗口关闭按钮
        setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint);
        this->show();
    }
    list_socket.append(socket); // 添加当前链接
    labelStatus->setText("连接设备：" + QString::number(list_socket.count()));

    // 接收数据
    connect(socket, &QTcpSocket::readyRead, this, [=]()
            {
                QByteArray date = socket->readAll();
                ui->textEdit->append(socket->peerAddress().toString() + "\n" + date + "\n"); });

    // 断开连接后的操作
    connect(socket, &QTcpSocket::disconnected, this, [=]()
            { 
                socket->close();
                list_socket.removeOne(socket); // 移除断开的socket
                if (list_socket.isEmpty())     // 如果当前没有客户端连接，禁用发送
                {
                    ui->btnSend->setEnabled(false);
                }

                ui->textEdit->append(socket->peerAddress().toString() + " 已断开\n");
                labelStatus->setText("连接设备：" + QString::number(list_socket.count())); });
}
#endif