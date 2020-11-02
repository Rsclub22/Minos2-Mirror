#include "txvmsetupdialog.h"
#include "ui_txvmsetupdialog.h"

TxVmSetupDialog::TxVmSetupDialog(VoiceKeyerCapabilities voiceCap_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TxVmSetupDialog),
    voiceCap(voiceCap_)

{
    ui->setupUi(this);

    initSetup();
}

TxVmSetupDialog::~TxVmSetupDialog()
{
    delete ui;
}

void TxVmSetupDialog::initSetup()
{

    ui->numButtons->setRange(MININUM_BUTTONS, MAXNUM_BUTTONS);
    if (voiceCap.getSupportSerial())
    {
        ui->comportSel->setVisible(true);
        ui->comportLbl->setVisible(true);
    }
    else
    {
        ui->comportSel->setVisible(false);
        ui->comportLbl->setVisible(false);
    }



}





void TxVmSetupDialog::setVmCommonParamsData(VoiceKeyerCommonParams *vmCommonParams)
{


    ui->numButtons->setValue(vmCommonParams->getNumButtons());
}
