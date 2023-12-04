#include "Server.h"

Server::Server(QWidget *parent)
    : QMainWindow(parent), ui(new Ui_Server)
{
    ui->setupUi(this);

    setWindowTitle("服务器");           // 设置窗口名字
    ui->textEdit->setEnabled(false);    // 消息显示窗口设为不可编辑
    ui->btnSendMsg->setEnabled(false);  // 发送消息初始不可用
    ui->btnSendFile->setEnabled(false); // 发送文件初始不可用
    ui->btnClose->setEnabled(false);    // 关闭服务器初始不可用

    recvFolder = QDir::currentPath();

    // 状态栏显示
    labelStatus = new QLabel(this);
    ui->statusbar->addWidget(labelStatus);

    ui->tableWidget->verticalHeader()->hide();                            // 隐藏列头
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);  // 不可编辑
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows); // 行选中
    ui->tableWidget->setColumnWidth(0, 200);                              // 列宽设为200
    ui->tableWidget->setColumnWidth(1, 200);
    ui->tableWidget->setColumnWidth(2, 200);
    ui->tableWidget->setColumnWidth(3, 200);

    // 消息处理服务器
    m_msgServer = new ServerMsg;
    msgThread = new QThread;
    m_msgServer->moveToThread(msgThread);
    msgThread->start();
    connect(this, &Server::startConnect, m_msgServer, &ServerMsg::on_startConnect);
    connect(this, &Server::sendMsg, m_msgServer, &ServerMsg::sendMsg);
    connect(this, &Server::sendFile, m_msgServer, &ServerMsg::sendFile);
    connect(this, &Server::endConnect, m_msgServer, &ServerMsg::on_endConnect);
    connect(m_msgServer, &ServerMsg::recvMsg, this, &Server::recvMsg);
    connect(m_msgServer, &ServerMsg::listAddNewSocket, this, &Server::listMsgAddItem);
    connect(m_msgServer, &ServerMsg::listRemoveSocket, this, &Server::listMsgRemoveItem);

    // 文件处理服务器
    m_fileServer = new ServerFile();
    fileThread = new QThread;
    m_fileServer->moveToThread(fileThread);
    fileThread->start();
    connect(this, &Server::startConnect, m_fileServer, &ServerFile::on_startConnect);
    connect(m_fileServer, &ServerFile::sendFile, this, &Server::startSend);
    connect(m_fileServer, &ServerFile::recvFile, this, &Server::startRecv);
    connect(this, &Server::endConnect, m_fileServer, &ServerFile::on_endConnect);
    connect(this, &Server::addSendInfo, m_fileServer, &ServerFile::addSendInfo);
    connect(this, &Server::changeFoler, m_fileServer, &ServerFile::setFolderName);
}

Server::~Server()
{
    msgThread->quit();
    msgThread->wait();
    msgThread->deleteLater();
    m_msgServer->deleteLater(); // 必须先析构线程再析构服务器, 否则会访问越界

    emit endConnect();
    fileThread->quit();
    fileThread->wait();
    fileThread->deleteLater();
    m_fileServer->deleteLater();

    delete ui;
}

// 表格新加行 展示文件信息
int Server::addTableRow(SFileInfo *info)
{
    int curRow = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(curRow);

    // 展示文件名
    QTableWidgetItem *item = new QTableWidgetItem(QString(info->fileName));
    item->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget->setItem(curRow, 0, item);

    // 计算大小并显示,依次为B/KB/MB/GB
    int count = 0;
    double size = info->fileSize;
    while (size > 1024.0 && count < 3)
    {
        count++;
        size /= 1024.0;
    }
    item = new QTableWidgetItem(QString::number(size, 'f', 1) + (count < 1 ? "B" : (count < 2 ? "KB" : (count < 3 ? "MB" : "GB"))));
    item->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget->setItem(curRow, 1, item);

    // 收发属性展示, 仅为指示当前行文件为接收或发送
    item = new QTableWidgetItem((info->sendId >= 0 ? "发送" : "接收"));
    item->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget->setItem(curRow, 2, item);

    if (info->sendId >= 0)
    { // 仅有待发送文件写入第四列
        item = new QTableWidgetItem("待发送");
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(curRow, 3, item);
    }
    // 多线程处理时,可能因客户端发送文件而新加行,返回新建行确保新建行和目标行一致
    return curRow;
}

// 显示传输过程中错误
void Server::showError(int error)
{
    QMessageBox::warning(this, "waring", QString("ERROR:%1").arg(error), QMessageBox::Ok);
}

// 接收返回消息
void Server::recvMsg(QString ip, QString msg)
{
    ui->textEdit->append(ip.right(ip.size() - 7) + ":");
    ui->textEdit->append(msg);
}

// 开始发送文件, 并添加显示发送进度
void Server::startSend(SFileInfo *info, SSendFile *sFile)
{
    ui->btnDeleteFile->setEnabled(false);

    int curRow = list_info.indexOf(info);
    info->sendId = 0;
    ui->tableWidget->takeItem(curRow, 3);
    // 添加进度条
    QProgressBar *bar = new QProgressBar(this);
    bar->setRange(0, 100);
    bar->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->tableWidget->setCellWidget(curRow, 3, bar);

    ui->btnDeleteFile->setEnabled(true);

    connect(sFile, &SSendFile::sendStart, ui->textEdit, &QTextEdit::append);
    connect(sFile, &SSendFile::sendError, this, &Server::showError);
    connect(sFile, &SSendFile::curPercent, bar, &QProgressBar::setValue);
    connect(sFile, &SSendFile::sendFinish, ui->textEdit, &QTextEdit::append);
}

// 开始接收文件,并添加接收进度
void Server::startRecv(SFileInfo *info, SRecvFile *rFile)
{
    ui->btnDeleteFile->setEnabled(false);

    int curRow = addTableRow(info); // 对于接收文件夹,最后一列留空

    // 添加进度条
    QProgressBar *bar = new QProgressBar(this);
    bar->setRange(0, 100);
    bar->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->tableWidget->setCellWidget(curRow, 3, bar);

    ui->btnDeleteFile->setEnabled(true);

    connect(rFile, &SRecvFile::recvStart, ui->textEdit, &QTextEdit::append);
    connect(rFile, &SRecvFile::recvError, this, &Server::showError);
    connect(rFile, &SRecvFile::curPercent, bar, &QProgressBar::setValue);
    connect(rFile, &SRecvFile::recvFinish, ui->textEdit, &QTextEdit::append);
}

// 对应ServerMsg 中 list_socket, 展示当前连接用户及其ip地址
void Server::listMsgAddItem(QString ip)
{
    if (ui->listWidgetMsg->count() == 0)
    {
        ui->btnSendMsg->setEnabled(true);
        labelStatus->setText("连接数：" + QString::number(ui->listWidgetMsg->count()));
    }
    ui->listWidgetMsg->addItem(ip.right(ip.size() - 7));
}

// 对应 ServerMsg 中 list_socket, 移除已断开连接用户
void Server::listMsgRemoveItem(int index)
{
    delete ui->listWidgetMsg->takeItem(index);
    if (ui->listWidgetMsg->count() == 0)
    {
        ui->btnSendMsg->setEnabled(false);
        labelStatus->setText("等待连接中...");
    }
}

// 开启服务器
void Server::on_btnStart_clicked()
{
    labelStatus->setText("等待连接中...");

    emit startConnect(); // 开启文件监听端口

    ui->btnStart->setEnabled(false); // 禁止重复开启
    ui->btnClose->setEnabled(true);
}

// 向某个客户端发送消息
void Server::on_btnSendMsg_clicked()
{
    QString msg = ui->plainTextEdit->toPlainText();
    if (msg.isEmpty()) // 发送窗口为空直接返回
        return;

    // 向选中用户发送消息
    int curRow = ui->listWidgetMsg->currentRow();
    if (curRow < 0)
        return;

    emit Server::sendMsg(curRow, msg);
    ui->textEdit->append(QString("server To %1:\n%2\n").arg(curRow).arg(msg));

    ui->plainTextEdit->clear();
}

// 选择文件（待发送
void Server::on_btnSelectFile_clicked()
{
    // 选择发送文件
    QString file = QFileDialog::getOpenFileName(this, "选择一个文件", "", "*.*");
    if (file.isEmpty() == true)
        return;
    QFileInfo info(file);
    SFileInfo *fileInfo = new SFileInfo(file, info.fileName(), info.size(), sendId++);
    list_info.push_back(fileInfo);
    addTableRow(fileInfo);
}

void Server::on_btnDeleteFile_clicked()
{
    int curRow = ui->tableWidget->currentRow();
    if (curRow < 0)
        return;

    // 删除选中行及其对应文件信息
    ui->tableWidget->removeRow(curRow);
    list_info.removeAt(curRow);
}

// 发送文件申请
void Server::on_btnSendFile_clicked()
{
    int row = ui->listWidgetMsg->currentRow();
    if (row < 0)
        return;

    int curRow = ui->tableWidget->currentRow();
    if (row < 0)
        return;

    SFileInfo *info = list_info.at(curRow);
    if (info->sendId == 0) // 一个文件第二次发送必须重新获得id
        info->sendId = sendId++;
    // 此处不是发送文件,而是发送文件申请
    emit Server::sendFile(row, info);
    emit Server::addSendInfo(info);
}

// 选择接收文件夹路径
void Server::on_btnSelectRecvFolder_clicked()
{
    recvFolder = QFileDialog::getExistingDirectory();
    emit this->changeFoler(recvFolder);
    ui->lineRecv->setText(recvFolder);
}

// 表格选择事件, 当前行为接收文件时或已发送过文件时, 关闭发送选项
void Server::on_tableWidget_cellPressed(int row)
{
    if (row >= 0 && list_info.at(row)->sendId > 0)
    {
        ui->btnSendFile->setEnabled(true);
    }
    else
    {
        ui->btnSendFile->setEnabled(false);
    }
}

// 关闭服务器
void Server::on_btnClose_clicked()
{
    emit Server::endConnect();

    ui->btnStart->setEnabled(true);  // 重新启用 开始服务器 按钮
    ui->btnClose->setEnabled(false); // 同时禁用 关闭服务器 按钮
    labelStatus->setText("服务器已关闭");
}
