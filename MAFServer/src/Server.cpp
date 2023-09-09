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
    connect(this, &Server::sendMsg, m_msgServer, &ServerMsg::sendMsg);
    connect(this, &Server::sendFile, m_msgServer, &ServerMsg::sendFile);
    connect(m_msgServer, &ServerMsg::recvMsg, this, &Server::recvMsg);
    connect(m_msgServer, &ServerMsg::listAddNewSocket, this, &Server::listMsgAddItem);
    connect(m_msgServer, &ServerMsg::listRemoveSocket, this, &Server::listMsgRemoveItem);

    // 文件处理服务器
    m_fileServer = new ServerFile(&list_info);
    fileThread = new QThread;
    m_fileServer->moveToThread(fileThread);
    fileThread->start();
    connect(m_fileServer, &ServerFile::sendFile, this, &Server::startSend);
    connect(m_fileServer, &ServerFile::recvFile, this, &Server::startRecv);
    connect(this, &Server::addSendInfo, m_fileServer, &ServerFile::addSendInfo);
    connect(this, &Server::changeFoler, m_fileServer, &ServerFile::setFolderName);
}

Server::~Server()
{
    m_msgServer->deleteLater();
    msgThread->quit();
    msgThread->wait();
    msgThread->deleteLater();

    m_fileServer->deleteLater();
    fileThread->quit();
    fileThread->wait();
    fileThread->deleteLater();

    for (auto info_ptr : list_info)
    {
        delete info_ptr;
    }
    delete ui;
}

int Server::addTableRow(SFileInfo *info)
{ // 为选择文件在表格中展示相应信息
    int curRow = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(curRow);

    QTableWidgetItem *item = new QTableWidgetItem(QString(*info->fileName));
    item->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget->setItem(curRow, 0, item);

    int count = 0;
    float size = info->fileSize;
    while (size > 1024.0 && count < 3)
    {
        count++;
        size /= 1024.0;
    }
    item = new QTableWidgetItem(QString::number(size, 'f', 1) + (count < 1 ? "B" : (count < 2 ? "KB" : (count < 3 ? "MB" : "GB"))));
    item->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget->setItem(curRow, 1, item);

    item = new QTableWidgetItem((info->sendId >= 0 ? "发送" : "接收"));
    item->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget->setItem(curRow, 2, item);

    if (info->sendId >= 0)
    {
        item = new QTableWidgetItem("待发送");
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(curRow, 3, item);
    }
    return curRow;
}

void Server::showError(int error)
{
    QMessageBox::warning(this, "waring", QString("ERROR:%1").arg(error), QMessageBox::Ok);
}

void Server::recvMsg(QString ip, QString msg)
{
    ui->textEdit->append(ip.right(ip.size() - 7) + ":");
    ui->textEdit->append(msg);
}

void Server::startSend(SFileInfo *info, SSendFile *sFile)
{
    ui->btnDeleteFile->setEnabled(false);

    int curRow = list_info.indexOf(info);
    ui->tableWidget->takeItem(curRow, 3);
    // 添加进度条
    QProgressBar *bar = new QProgressBar(this);
    bar->setRange(0, 100);
    bar->setValue(0);
    bar->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->tableWidget->setCellWidget(curRow, 3, bar);

    ui->btnDeleteFile->setEnabled(true);

    connect(sFile, &SSendFile::sendStart, ui->textEdit, &QTextEdit::append);
    connect(sFile, &SSendFile::sendError, this, &Server::showError);
    connect(sFile, &SSendFile::curPercent, bar, &QProgressBar::setValue);
    connect(sFile, &SSendFile::sendFinish, ui->textEdit, &QTextEdit::append);
}

void Server::startRecv(SFileInfo *info, SRecvFile *rFile)
{
    ui->btnDeleteFile->setEnabled(false);

    int curRow = addTableRow(info); // 对于接收文件夹,最后一列不会添加项

    // 添加进度条
    QProgressBar *bar = new QProgressBar(this);
    bar->setRange(0, 100);
    bar->setValue(0);
    bar->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->tableWidget->setCellWidget(curRow, 3, bar);

    ui->btnDeleteFile->setEnabled(true);

    connect(rFile, &SRecvFile::recvStart, ui->textEdit, &QTextEdit::append);
    connect(rFile, &SRecvFile::recvError, this, &Server::showError);
    connect(rFile, &SRecvFile::curPercent, bar, &QProgressBar::setValue);
    connect(rFile, &SRecvFile::recvFinish, ui->textEdit, &QTextEdit::append);
}

void Server::listMsgAddItem(QString ip)
{
    if (ui->listWidgetMsg->count() == 0)
    {
        ui->btnSendMsg->setEnabled(true);
        labelStatus->setText("连接数：" + QString::number(ui->listWidgetMsg->count()));
    }
    ui->listWidgetMsg->addItem(ip.right(ip.size() - 7));
}

void Server::listMsgRemoveItem(int index)
{
    delete ui->listWidgetMsg->takeItem(index);
    if (ui->listWidgetMsg->count() == 0)
    {
        ui->btnSendMsg->setEnabled(false);
        labelStatus->setText("等待连接中...");
    }
}

void Server::on_btnStart_clicked()
{
    labelStatus->setText("等待连接中...");

    m_msgServer->startConnect(MSG_PORT);        // 开启消息监听端口
    m_fileServer->startConnect(SEND_FILE_PORT); // 开启文件监听端口

    ui->btnStart->setEnabled(false); // 禁止重复开启
    ui->btnClose->setEnabled(true);
}

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

void Server::on_btnSelectFile_clicked()
{
    // 选择发送文件
    QString *file = new QString(QFileDialog::getOpenFileName(this, "选择一个文件", "", "*.*"));
    if (file->isEmpty() == true)
        return;

    QFileInfo info(*file);
    SFileInfo *fileInfo = new SFileInfo(file, new QString(info.fileName()), info.size(), sendId++);
    list_info.push_back(fileInfo);
    addTableRow(fileInfo);
}

void Server::on_btnSendFile_clicked()
{
    int row = ui->listWidgetMsg->currentRow();
    if (row < 0)
        return;

    int curRow = ui->tableWidget->currentRow();
    if (row < 0)
        return;

    SFileInfo *info = list_info.at(curRow);
    if (info->sendId == -1)
        info->sendId = sendId++;
    // 此处不是发送文件,而是发送文件申请
    emit Server::sendFile(row, info);
    emit Server::addSendInfo(info);
}

void Server::on_btnSelectRecvFolder_clicked()
{
    recvFolder = QFileDialog::getExistingDirectory();
    emit this->changeFoler(recvFolder);
    ui->lineRecv->setText(recvFolder);
}

void Server::on_tableWidget_cellPressed(int row)
{
    if (row >= 0 && list_info.at(row)->sendId >= 0)
    {
        ui->btnSendFile->setEnabled(true);
    }
    else
    {
        ui->btnSendFile->setEnabled(false);
    }
}

void Server::on_btnClose_clicked()
{
    m_msgServer->endConnect();
    m_fileServer->endConnect();

    ui->btnStart->setEnabled(true);  // 重新启用 开始服务器 按钮
    ui->btnClose->setEnabled(false); // 同时禁用 关闭服务器 按钮
    labelStatus->setText("服务器已关闭");
}
