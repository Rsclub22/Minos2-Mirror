#include "kstsendmeepframe.h"
#include "ui_kstsendmeepframe.h"

KSTSendMeepFrame::KSTSendMeepFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::KSTSendMeepFrame)
{
    ui->setupUi(this);
}

KSTSendMeepFrame::~KSTSendMeepFrame()
{
    delete ui;
}
