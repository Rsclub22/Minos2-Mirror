#include "activechatsframe.h"
#include "ui_activechatsframe.h"

activeChatsFrame::activeChatsFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::activeChatsFrame)
{
    ui->setupUi(this);
}

activeChatsFrame::~activeChatsFrame()
{
    delete ui;
}
