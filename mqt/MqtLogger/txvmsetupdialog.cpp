#include <QSettings>
#include "txvmsetupdialog.h"
#include "ui_txvmsetupdialog.h"

TxVmSetupDialog::TxVmSetupDialog(VoiceKeyerCapabilities voiceCap_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TxVmSetupDialog),
    voiceCap(voiceCap_)

{
    ui->setupUi(this);

    QSettings settings;
    QByteArray geometry = settings.value("TxVmSetupDialog/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);


    initSetup();
}

TxVmSetupDialog::~TxVmSetupDialog()
{
    delete ui;
}
void TxVmSetupDialog::doCloseEvent()
{
    QSettings settings;
    settings.setValue("TxVmSetupDialog/geometry", saveGeometry());
}
void TxVmSetupDialog::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void TxVmSetupDialog::accept()
{
    doCloseEvent();
    QDialog::accept();
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

    connect(ui->numButtons, QOverload<int>::of(&QSpinBox::valueChanged), this, &TxVmSetupDialog::onNumButtonsValueChanged);



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
