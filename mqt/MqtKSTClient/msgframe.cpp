#include "msgframe.h"
#include "ui_msgframe.h"

msgFrame::msgFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::msgFrame)
{
    ui->setupUi(this);
}

msgFrame::~msgFrame()
{
    delete ui;
}
