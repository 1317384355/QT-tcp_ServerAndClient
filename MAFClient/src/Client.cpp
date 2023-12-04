#include "Client.h"

Client::Client(QWidget *parent)
    : QMainWindow(parent), ui(new Ui_Client)
{
    ui->setupUi(this);
    setWindowTitle("客户端");

    // 按钮初始化
    ui->textEdit->setEnabled(false);
    ui->btnClose->setEnabled(false);
    ui->btnSendMsg->setEnabled(false);

    // 状态栏设置标签, 展示连接状态
    labelStatus = new QLabel(this);
    labelIP = new QLabel(this);
    ui->statusbar->addWidget(labelStatus);      // 左对齐
    ui->statusbar->addPermanentWidget(labelIP); // 右对齐

    // 文件信息表格初始化
    ui->tableWidget->verticalHeader()->hide();                            // 隐藏列头
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);  // 不可编辑
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows); // 选中一行
    ui->tableWidget->setColumnWidth(0, 200);                              // 列宽设为200
    ui->tableWidget->setColumnWidth(1, 200);
    ui->tableWidget->setColumnWidth(2, 200);
    ui->tableWidget->setColumnWidth(3, 200);

    addr = ui->lineEdit->text();              // 连接地址初始化为 192.168.123.214
    rFolder = QDir::currentPath();            // 接收文件夹 初始化为 当前程序地址
    ui->lineEditRecvFolder->setText(rFolder); // 并展示

    msgSocket = new ClientMsg();
    msgThread = new QThread;
    msgSocket->moveToThread(msgThread); // socket移入子线程
    msgThread->start();                 // 线程开始运行

    connect(this, &Client::startConnect, msgSocket, &ClientMsg::on_startConnect);
    connect(msgSocket, &ClientMsg::connected, this, [this]() { // 连接成功
        labelStatus->setText("连接成功");
        labelIP->setText(addr); // 连接目标地址

        ui->btnClose->setEnabled(true);
        ui->btnSendMsg->setEnabled(true);
    });
    connect(this, &Client::sendMsg, msgSocket, &ClientMsg::on_sendMsg);
    connect(msgSocket, &ClientMsg::recvMsg, this, &Client::on_recvMsg);
    connect(msgSocket, &ClientMsg::recvFileApply, this, &Client::showRecv);
    // 断开操作, 按钮主动断开 和 断开后续操作
    connect(ui->btnClose, &QPushButton::clicked, msgSocket, &ClientMsg::on_endConnect);
    connect(msgSocket, &ClientMsg::disconnected, this, [this]() { //
        ui->btnStart->setEnabled(true);
        ui->lineEdit->setEnabled(true);
        ui->btnClose->setEnabled(false);
        ui->btnSendMsg->setEnabled(false);

        labelIP->setText("");
        ui->textEdit->clear();
        labelStatus->setText("连接已断开");
    });
}

Client::~Client()
{
    // 关闭线程后析构
    msgThread->quit();
    msgThread->wait();
    msgThread->deleteLater();

    // 关闭socket连接,然后析构
    emit ui->btnClose->clicked();
    msgSocket->deleteLater();
    for (auto ptr : info_list)
        delete ptr;
    // for()
    delete ui;
}

void Client::on_recvMsg(QString msg)
{
    ui->textEdit->append("服务端：" + msg);
}

void Client::showError(int error)
{
    QMessageBox::warning(this, "waring", QString("ERROR:%1").arg(error), QMessageBox::Ok);
}

void Client::addTableRow(QString fileName, qint64 fileSize, QString Attribute)
{
    // 为选择文件在表格中展示相应信息
    int curRow = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(curRow);

    QTableWidgetItem *item = new QTableWidgetItem(fileName);
    item->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget->setItem(curRow, 0, item);

    int count = 0;
    float size = fileSize;
    while (size > 1024.0 && count < 3)
    {
        count++;
        size /= 1024.0;
    }
    item = new QTableWidgetItem(QString::number(size, 'f', 1) + (count < 1 ? "B" : (count < 2 ? "KB" : (count < 3 ? "MB" : "GB"))));
    item->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget->setItem(curRow, 1, item);

    item = new QTableWidgetItem(Attribute);
    item->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget->setItem(curRow, 2, item);

    item = new QTableWidgetItem("待" + Attribute);
    item->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget->setItem(curRow, 3, item);
}

void Client::showRecv(QString fileName, qint64 fileSize, qint16 sendID)
{
    CFileInfo *fInfor = new CFileInfo(new QString(rFolder), new QString(fileName), new qint64(fileSize), sendID);
    info_list.push_back(fInfor);

    CRecvFile *recvSocket = nullptr;
    file_list.push_back(recvSocket);

    addTableRow(fileName, fileSize, "接收");
}

void Client::on_btnStart_clicked()
{
    labelStatus->setText("尝试连接中...");
    ui->btnStart->setEnabled(false);
    ui->lineEdit->setEnabled(false);
    addr = ui->lineEdit->text();
    emit Client::startConnect(addr, MSG_PORT);
}

void Client::on_btnSendMsg_clicked()
{
    // 从文本框读取待发送文字
    QString msg = ui->plainTextEdit->toPlainText();
    if (msg.isEmpty())
        return;

    // 消息框展示发送消息, 并清空文本框
    ui->textEdit->append("客户端：" + msg);
    ui->plainTextEdit->clear();

    // 发出信号,
    emit Client::sendMsg(msg);
}

void Client::on_btnSelectFile_clicked()
{
    // 选择发送文件
    QString *file = new QString(QFileDialog::getOpenFileName(this, "选择一个文件", "", "*.*"));
    if (file->isEmpty() == true)
        return;

    // 读取文件信息
    QFileInfo info(*file);
    // 读取信息 存至 info_list 备用
    CFileInfo *fileInfo = new CFileInfo(file, new QString(info.fileName()), new qint64(info.size()));
    info_list.push_back(fileInfo);
    // 保证列表数量 和 widget展示数量 相同, 置为 nullptr 表示该文件不曾被发送
    CSendFile *sendSocket = nullptr;
    file_list.push_back(sendSocket);

    addTableRow(info.fileName(), info.size(), "发送");
}

void Client::on_btnDeleteFile_clicked()
{
    int curRow = ui->tableWidget->currentRow();
    if (curRow < 0) // 当前无选中 行, 返回
        return;

    // 移除选中行, 删除链表中对应内容
    ui->tableWidget->removeRow(curRow);
    if (info_list.at(curRow)->sendId == -1) // id > 0 才是合法的接收线程
    {
        delete (CSendFile *)file_list.takeAt(curRow);
    }
    else
    {
        CRecvFile *recvFile = (CRecvFile *)file_list.takeAt(curRow);
        if (wait_list.count() < 4)
            wait_list.push_back(recvFile);
        else
            delete recvFile;
    }
    delete info_list.takeAt(curRow);

    // 表格中无文件时,将发送文件按钮设为 不可用
    if (ui->tableWidget->rowCount() == 0)
    {
        ui->btnSendFile->setEnabled(false);
    }
}

void Client::on_btnSendFile_clicked()
{
    int curRow = ui->tableWidget->currentRow();
    if (curRow < 0) // 当前无选中 行, 返回
        return;

    CSendFile *sendSocket = (CSendFile *)file_list.at(curRow);
    if (sendSocket == nullptr)
    {
        // 添加进度条表示发送进度
        QProgressBar *bar = new QProgressBar(this);
        bar->setRange(0, 100);
        bar->setValue(0);
        bar->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        ui->tableWidget->setCellWidget(curRow, 3, bar);

        // 初始化实例 并连接信号
        sendSocket = new CSendFile(addr, info_list.at(curRow), this);
        connect(sendSocket, &CSendFile::sendStart, ui->textEdit, &QTextEdit::append);
        connect(sendSocket, &CSendFile::sendError, this, &Client::showError);
        connect(sendSocket, &CSendFile::curPercent, bar, &QProgressBar::setValue);
        connect(sendSocket, &CSendFile::sendFinish, ui->textEdit, &QTextEdit::append);
    }
    emit sendSocket->curPercent(0);
    sendSocket->setIp(this->addr);
    sendSocket->start();
}

void Client::on_btnRecvFile_clicked()
{
    int curRow = ui->tableWidget->currentRow();
    if (curRow < 0) // 当前无选中 行, 返回
        return;

    CRecvFile *recvSocket = (CRecvFile *)file_list.at(curRow);
    if (recvSocket != nullptr)
    { // 禁止对一条文件信息二次接收
        return;
    }

    QProgressBar *bar = new QProgressBar(this);
    bar->setRange(0, 100);
    bar->setValue(0);
    bar->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->tableWidget->setCellWidget(curRow, 3, bar);

    // 初始化实例 并连接信号
    if (wait_list.isEmpty() == false) // 不为空, 即存在可复用的 CRecvFile 对象
        recvSocket = wait_list.takeFirst();
    else
        recvSocket = new CRecvFile(this);
    connect(recvSocket, &CRecvFile::recvStart, ui->textEdit, &QTextEdit::append);
    connect(recvSocket, &CRecvFile::recvError, this, &Client::showError);
    connect(recvSocket, &CRecvFile::curPercent, bar, &QProgressBar::setValue);
    connect(recvSocket, &CRecvFile::recvFinish, ui->textEdit, &QTextEdit::append);

    // 重新设置接收信息, 并开始执行
    recvSocket->setInfo(addr, info_list.at(curRow), this->rFolder);
    recvSocket->start();
}

void Client::on_btnSelectRecvFolder_clicked()
{
    QString folderName = QFileDialog::getExistingDirectory();
    if (folderName.isEmpty())
        return;
    rFolder = folderName;
    ui->lineEditRecvFolder->setText(rFolder);
}

void Client::on_tableWidget_cellPressed(int row)
{
    if (row < 0)
        return;

    if (info_list.at(row)->sendId <= 0)
    {
        ui->btnSendFile->setEnabled(true);
        ui->btnRecvFile->setEnabled(false);
    }
    else
    {
        ui->btnSendFile->setEnabled(false);
        ui->btnRecvFile->setEnabled(true);
    }
}