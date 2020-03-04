#include "n1mmbroadcastconfig.h"
#include "ui_n1mmbroadcastconfig.h"

N1MMBroadcastConfig::N1MMBroadcastConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::N1MMBroadcastConfig)
{
    ui->setupUi(this);
}

N1MMBroadcastConfig::~N1MMBroadcastConfig()
{
    delete ui;
}
