#include "txkeyercwrigcontrolform.h"
#include "ui_txkeyercwrigcontrolform.h"

TxKeyerCwRigControlForm::TxKeyerCwRigControlForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TxKeyerCwRigControlForm)
{
    ui->setupUi(this);
}

TxKeyerCwRigControlForm::~TxKeyerCwRigControlForm()
{
    delete ui;
}
