#include "kstplanesframe.h"
#include "ui_kstplanesframe.h"

KSTPlanesFrame::KSTPlanesFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::KSTPlanesFrame)
{
    ui->setupUi(this);
}

KSTPlanesFrame::~KSTPlanesFrame()
{
    delete ui;
}
