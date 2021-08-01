#include <QSettings>

#include "txVmExternalSetupDialog.h"
#include "ui_txVmExternalSetupDialog.h"

TxVmExternalSetupDialog::TxVmExternalSetupDialog(VoiceKeyerCapabilities voiceCap_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TxVmExternalSetupDialog)
{
    ui->setupUi(this);
}

TxVmExternalSetupDialog::~TxVmExternalSetupDialog()
{
    delete ui;
}
void TxVmExternalSetupDialog::doCloseEvent()
{
    QSettings settings;
    settings.setValue("txVmInternalSetupDialog/geometry", saveGeometry());
}
void TxVmExternalSetupDialog::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void TxVmExternalSetupDialog::accept()
{
    doCloseEvent();
    QDialog::accept();
}
void TxVmExternalSetupDialog::setVmCommonParamsData(VoiceKeyerCommonParams *vmCommonParams_)
{

}
