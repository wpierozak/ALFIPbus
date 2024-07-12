#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QPushButton>
#include<QDebug>
#include<iostream>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->send_ip, &QPushButton::clicked, this, &MainWindow::send);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::send()
{
    std::string prefix = "300000000000";
    std::string input =  ui->input->text().toStdString();
    input = prefix+ input;

    int end = 19;
    quint8 bytes[10];
    int idx = 0;
    for(int i = end; i >= 0; i-=2)
    {
        bytes[idx] = string_to_byte(input[i-1], input[i]);
        idx++;
    }

    ipbus_swt.add_transaction(bytes);

    Transaction& q = ipbus_swt.packet.transactionsList.back();
    debugTransaction(q);
    ipbus_swt.translate_response(ipbus_swt.packet.transactionsList.length()-1);

    swt_to_byte(ipbus_swt.swt_res.back(), bytes);

    QString response;
    for(int i = 9; i >= 0; i--)
    {
        response.push_back(hexToChar(bytes[i] >> 4));
        response.push_back(hexToChar(bytes[i]& 0x0F));
    }

    ui->lineEdit_2->clear();
    ui->lineEdit_2->insert(response);
}

void MainWindow::debugTransaction(const Transaction& transaction)
{
    ui->debug->clear();

    ui->debug->insertPlainText("Request Header:\n");
    ui->debug->insertPlainText("\tInfoCode:\t\t" + transaction.requestHeader->infoCodeString() + '\n');
    ui->debug->insertPlainText("\tTypeID:\t\t" + QString::number(transaction.requestHeader->TypeID)+ '\n');
    ui->debug->insertPlainText("\tWords:\t\t" + QString::number(transaction.requestHeader->Words)+ '\n');
    ui->debug->insertPlainText("\tTransactionID:\t" + QString::number(transaction.requestHeader->TransactionID)+ '\n') ;
    ui->debug->insertPlainText("\tProtocolVersion:\t" + QString::number(transaction.requestHeader->ProtocolVersion)+ '\n');
    ui->debug->insertPlainText("\tAddress:\t\t0x" + QString::number(*transaction.address,16).toUpper()+ '\n');
    ui->debug->insertPlainText("\tData:\t\t0x" + QString::number(*transaction.data,16).toUpper()+ '\n');

    // ui->debug->insertPlainText("Response Header:\n" );
    // ui->debug->insertPlainText("\tInfoCode: " + transaction.responseHeader->infoCodeString()+ '\n');
    // ui->debug->insertPlainText("\tTypeID: " + QString::number(transaction.responseHeader->TypeID)+ '\n');
    // ui->debug->insertPlainText("\tWords: " + QString::number(transaction.responseHeader->Words)+ '\n');
    // ui->debug->insertPlainText("\tTransactionID: " + QString::number(transaction.responseHeader->TransactionID)+ '\n') ;
    // ui->debug->insertPlainText("\tProtocolVersion: " + QString::number(transaction.responseHeader->ProtocolVersion)+ '\n');

}
