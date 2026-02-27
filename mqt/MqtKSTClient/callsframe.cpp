#include "callsframe.h"
#include "ui_callsframe.h"

callsFrame::callsFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::callsFrame)
{
    ui->setupUi(this);
}

callsFrame::~callsFrame()
{
    delete ui;
}
