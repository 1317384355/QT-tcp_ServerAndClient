#include "Client.h"

Client::Client(QWidget *parent)
    : QMainWindow(parent), ui(new Ui_Client)
{
    ui->setupUi(this);

    setWindowTitle("客户端");
    flags = windowFlags();
    ui->textEdit->setEnabled(false);
    ui->btnClose->setEnabled(false);
    ui->btnSend->setEnabled(false);

    labelStatus = new QLabel(this);
    labelIP = new QLabel(this);
    ui->statusbar->addWidget(labelStatus);
    ui->statusbar->addPermanentWidget(labelIP);

    ui->tableWidget->verticalHeader()->hide();
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setColumnWidth(0, 50);
    ui->tableWidget->setColumnWidth(1, 200);
    ui->tableWidget->setColumnWidth(2, 200);

    worker = new Send();
    newThread = new QThread();
    worker->moveToThread(newThread);

    // 连接
    connect(this, &Client::startConnect, worker, &Send::startConnect);
    connect(worker, &Send::connected, this, [=]()
            {
                labelStatus->setText("连接成功");
                labelIP->setText(addr + ":" + QString::number(port));
    
                ui->btnClose->setEnabled(true);
                ui->btnSend->setEnabled(true); 
                setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint);
                this->show(); });

    connect(worker, &Send::readyRead, this, &Client::recvMsg);
    connect(this, &Client::sendMsg, worker, &Send::sendMsg);
    connect(this, &Client::sendFile, worker, &Send::sendFile);
    connect(worker, &Send::curPercent, this, &Client::curPercent);

    // 断开连接后的操作
    connect(ui->btnClose, &QPushButton::clicked, worker, &Send::endConnect);
    connect(worker, &Send::disconnected, this, [=]()
            {
                newThread->quit();
                newThread->wait();

                ui->btnStart->setEnabled(true);
                ui->btnClose->setEnabled(false);
                ui->btnSend->setEnabled(false);
                setWindowFlags(flags);
                this->show();
                
                labelIP->setText("");
                ui->textEdit->clear();
                labelStatus->setText("连接已断开"); });
}

Client::~Client()
{
    worker->deleteLater();
    newThread->quit();
    newThread->wait();
    newThread->deleteLater();
    delete ui;
}

void Client::recvMsg(QByteArray date)
{
    ui->textEdit->append("服务端：" + date);
}

void Client::recvFile(QByteArray date)
{
}

void Client::curPercent(int percent)
{
    int curRow = ui->tableWidget->rowCount() - 1;
    QProgressBar *bar = (QProgressBar *)ui->tableWidget->cellWidget(curRow, 2);
    bar->setValue(percent);
}

void Client::on_btnStart_clicked()
{
    labelStatus->setText("尝试连接中...");

    addr = ui->lineEditAddr->text();
    port = ui->lineEditPort->text().toShort();
    ui->btnStart->setEnabled(false);

    newThread->start();

    emit Client::startConnect(addr, port);
}

void Client::on_btnSend_clicked()
{
    QString msg = ui->plainTextEdit->toPlainText();
    if (msg.isEmpty())
    {
        return;
    }

    ui->textEdit->append("客户端：" + msg);
    ui->plainTextEdit->clear();

    emit Client::sendMsg(msg);
}

void Client::on_btnSelectFile_clicked()
{
    if (!ui->tableWidget->isVisible())
    {
        on_btnShow_clicked(true);
    }

    QString file = QFileDialog::getOpenFileName(this, "选择一个文件", "", "*.*");
    if (file.isEmpty() == true)
    {
        return;
    }

    int curRow = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(curRow);
    ui->tableWidget->setCellWidget(curRow, 0, new QRadioButton());
    ui->tableWidget->setItem(curRow, 1, new QTableWidgetItem(file));
    ui->tableWidget->setItem(curRow, 2, new QTableWidgetItem("待发送"));

    ui->btnSendFile->setEnabled(true);
}

void Client::on_btnDeleteFile_clicked()
{
    if (ui->tableWidget->rowCount() == 0)
    {
        ui->btnSendFile->setEnabled(false);
    }
}

void Client::on_btnSendFile_clicked()
{
    int curRow = ui->tableWidget->rowCount() - 1;

    emit sendFile(ui->tableWidget->item(curRow, 1)->text());

    QProgressBar *bar = new QProgressBar();
    bar->setRange(0, 100);
    bar->setValue(0);
    bar->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->tableWidget->setCellWidget(curRow, 2, bar);
}

void Client::on_btnShow_clicked(bool checked)
{
    ui->tableWidget->setVisible(checked);
}