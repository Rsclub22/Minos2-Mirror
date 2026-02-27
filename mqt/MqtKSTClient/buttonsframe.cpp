#include "buttonsframe.h"
#include "ui_buttonsframe.h"

buttonsFrame::buttonsFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::buttonsFrame)
{
    ui->setupUi(this);
}

buttonsFrame::~buttonsFrame()
{
    delete ui;
}
