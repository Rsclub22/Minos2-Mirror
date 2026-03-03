#include "ksttomeframe.h"
#include "ui_ksttomeframe.h"

KSTTomeFrame::KSTTomeFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::KSTTomeFrame)
{
    ui->setupUi(this);
}

KSTTomeFrame::~KSTTomeFrame()
{
    delete ui;
}
