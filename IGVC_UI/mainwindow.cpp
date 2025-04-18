#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setStyleSheet("background-color: #ffffff;");
    setWindowTitle("IGVC UI");
    qDebug()<<"Started";
    execbox = new ExecBox(this, ui);

}

MainWindow::~MainWindow()
{
    delete ui;
}
