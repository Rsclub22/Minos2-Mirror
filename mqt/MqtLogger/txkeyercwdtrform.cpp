#include "txkeyercwdtrform.h"
#include "ui_txkeyercwdtrform.h"

TxKeyerCwDtrForm::TxKeyerCwDtrForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TxKeyerCwDtrForm)
{
    ui->setupUi(this);
}

TxKeyerCwDtrForm::~TxKeyerCwDtrForm()
{
    delete ui;
}
