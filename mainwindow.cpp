#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    ipbus("172.0.0.1", 44)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
