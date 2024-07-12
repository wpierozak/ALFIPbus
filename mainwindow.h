#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QDebug>
#include"ipbusswt.h"
#include"SWTelectronics.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public slots:
    void send();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void debugTransaction(const Transaction &transaction);
private:
    Ui::MainWindow *ui;
    IPbusSWT_Packet ipbus_swt;
    SWTelectronics comm;

};
#endif // MAINWINDOW_H
