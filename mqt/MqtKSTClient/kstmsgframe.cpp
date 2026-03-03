#include "kstmsgframe.h"
#include "ui_kstmsgframe.h"

KSTMsgFrame::KSTMsgFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::KSTMsgFrame)
{
    ui->setupUi(this);
}

KSTMsgFrame::~KSTMsgFrame()
{
    delete ui;
}
