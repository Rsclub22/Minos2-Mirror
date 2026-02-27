#include "loginframe.h"
#include "ui_loginframe.h"

loginFrame::loginFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::loginFrame)
{
    ui->setupUi(this);
}

loginFrame::~loginFrame()
{
    delete ui;
}
