#include "kstactivechatsframe.h"
#include "ui_kstactivechatsframe.h"

KSTActiveChatsFrame::KSTActiveChatsFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::KSTActiveChatsFrame)
{
    ui->setupUi(this);
}

KSTActiveChatsFrame::~KSTActiveChatsFrame()
{
    delete ui;
}
