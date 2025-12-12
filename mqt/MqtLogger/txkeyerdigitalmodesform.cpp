#include "txkeyerdigitalmodesform.h"
#include "ui_txkeyerdigitalmodesform.h"

TxKeyerDigitalModesForm::TxKeyerDigitalModesForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TxKeyerDigitalModesForm)
{
    ui->setupUi(this);
}

TxKeyerDigitalModesForm::~TxKeyerDigitalModesForm()
{
    delete ui;
}
