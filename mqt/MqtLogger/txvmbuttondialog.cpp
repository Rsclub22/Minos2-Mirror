#include "txvmbuttondialog.h"
#include "ui_txvmbuttondialog.h"

TxVmButtonDialog::TxVmButtonDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TxVmButtonDialog)
{
    ui->setupUi(this);
}

TxVmButtonDialog::~TxVmButtonDialog()
{
    delete ui;
}
