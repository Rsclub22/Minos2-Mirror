#include "planesframe.h"
#include "ui_planesframe.h"

planesFrame::planesFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::planesFrame)
{
    ui->setupUi(this);
}

planesFrame::~planesFrame()
{
    delete ui;
}
