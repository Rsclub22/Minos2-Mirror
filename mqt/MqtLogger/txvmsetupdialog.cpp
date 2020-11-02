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

    connect(ui->numButtons, SIGNAL(valueChanged(int)), this, SLOT(onNumButtonsValueChanged(int)));



}


void TxVmSetupDialog::onNumButtonsValueChanged(int num)
{
    vmCommonParams->setNumButtons(num);
}


void TxVmSetupDialog::setVmCommonParamsData(VoiceKeyerCommonParams *vmCommonParams_)
{

    vmCommonParams = vmCommonParams_;
    ui->numButtons->setValue(vmCommonParams->getNumButtons());
}
