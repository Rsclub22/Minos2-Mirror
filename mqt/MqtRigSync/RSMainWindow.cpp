#include "RSMainWindow.h"
#include "ui_RSMainWindow.h"

RSMainWindow::RSMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::RSMainWindow)
{
    ui->setupUi(this);
}

RSMainWindow::~RSMainWindow()
{
    delete ui;
}

