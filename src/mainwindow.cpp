#include "mainwindow.h"
#include "./ui_mainwindow.h"


// Mainwindow UI where all UI elements are placed.
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setStyleSheet("background-color: #ffffff;");
    qDebug()<<"Started";
    //execbox (short for execution box) where every logic is initialized
    execbox = new ExecBox(this, ui);

}

MainWindow::~MainWindow()
{
    delete ui;
}
