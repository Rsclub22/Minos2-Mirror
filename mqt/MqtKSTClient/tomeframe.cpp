#include "tomeframe.h"
#include "ui_tomeframe.h"

tomeFrame::tomeFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::tomeFrame)
{
    ui->setupUi(this);
}

tomeFrame::~tomeFrame()
{
    delete ui;
}
