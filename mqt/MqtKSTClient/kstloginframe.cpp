#include "kstloginframe.h"
#include "ui_kstloginframe.h"

KSTLoginFrame::KSTLoginFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::KSTLoginFrame)
{
    ui->setupUi(this);
}

KSTLoginFrame::~KSTLoginFrame()
{
    delete ui;
}
