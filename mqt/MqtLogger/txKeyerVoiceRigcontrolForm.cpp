#include "txKeyerVoiceRigcontrolForm.h"
#include "ui_txKeyerVoiceRigcontrolForm.h"

TxVoiceRigControlForm::TxVoiceRigControlForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TxVoiceRigControlForm)
{
    ui->setupUi(this);
}

TxVoiceRigControlForm::~TxVoiceRigControlForm()
{
    delete ui;
}
