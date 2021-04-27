#include <QSettings>
#include "txvmrigsetupdialog.h"
#include "ui_txvmrigsetupdialog.h"

TxVmRigSetupDialog::TxVmRigSetupDialog(VoiceKeyerCapabilities voiceCap_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TxVmRigSetupDialog),
    voiceCap(voiceCap_)

{
    ui->setupUi(this);

    QSettings settings;
    QByteArray geometry = settings.value("TxVmSetupDialog/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);


    initSetup();
}

TxVmRigSetupDialog::~TxVmRigSetupDialog()
{
    delete ui;
}
void TxVmRigSetupDialog::doCloseEvent()
{
    QSettings settings;
    settings.setValue("TxVmSetupDialog/geometry", saveGeometry());
}
void TxVmRigSetupDialog::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void TxVmRigSetupDialog::accept()
{
    doCloseEvent();
    QDialog::accept();
}


void TxVmRigSetupDialog::initSetup()
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

    connect(ui->numButtons, QOverload<int>::of(&QSpinBox::valueChanged), this, &TxVmRigSetupDialog::onNumButtonsValueChanged);



}


void TxVmRigSetupDialog::onNumButtonsValueChanged(int num)
{
    vmCommonParams->setNumButtons(num);
}


void TxVmRigSetupDialog::setVmCommonParamsData(VoiceKeyerCommonParams *vmCommonParams_)
{

    vmCommonParams = vmCommonParams_;
    ui->numButtons->setValue(vmCommonParams->getNumButtons());
}
