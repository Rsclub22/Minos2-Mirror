#include "sendmeepframe.h"
#include "ui_sendmeepframe.h"

sendMeepFrame::sendMeepFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::sendMeepFrame)
{
    ui->setupUi(this);
}

sendMeepFrame::~sendMeepFrame()
{
    delete ui;
}
