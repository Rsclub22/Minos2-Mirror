#include "kstbuttonsframe.h"
#include "ui_kstbuttonsframe.h"

KSTButtonsFrame::KSTButtonsFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::KSTButtonsFrame)
{
    ui->setupUi(this);
}

KSTButtonsFrame::~KSTButtonsFrame()
{
    delete ui;
}
