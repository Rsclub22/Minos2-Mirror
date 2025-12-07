#include "txkeyernoneform.h"
#include "ui_txkeyernoneform.h"

TxKeyerNoneForm::TxKeyerNoneForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TxKeyerNoneForm)
{
    ui->setupUi(this);
}

TxKeyerNoneForm::~TxKeyerNoneForm()
{
    delete ui;
}
