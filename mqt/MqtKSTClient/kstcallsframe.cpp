#include "kstcallsframe.h"
#include "ui_kstcallsframe.h"

KSTCallsFrame::KSTCallsFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::KSTCallsFrame)
{
    ui->setupUi(this);
}

KSTCallsFrame::~KSTCallsFrame()
{
    delete ui;
}
