#include "txvmbuttondialog.h"
#include "ui_txvmbuttondialog.h"


TxVmButtonDialog::TxVmButtonDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TxVmButtonDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(on_okButton_clicked()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(on_cancelbutton_clicked()));
}

TxVmButtonDialog::~TxVmButtonDialog()
{
    delete ui;
}


void TxVmButtonDialog::setVmData(VoiceKeyerParams* vmData_)
{
    vmData = vmData_;
    ui->txVmNameEdit->setText(vmData->getVmName());
    ui->txVmRepeatChkBox->setChecked(vmData->getVmRepeatFlag());
    ui->txVmRepeatDurSpinBx->setValue(vmData->getVmRepeatDur());

}

void TxVmButtonDialog::on_okButton_clicked()
{

    QString name = ui->txVmNameEdit->text();
    vmData->setVmName(name);
    vmData->setVmRepeatDur(ui->txVmRepeatDurSpinBx->value());
    vmData->setVmRepeatFlag(ui->txVmRepeatChkBox->isChecked());
    accept();

}


void TxVmButtonDialog::on_cancelbutton_clicked()
{
    reject();
}

