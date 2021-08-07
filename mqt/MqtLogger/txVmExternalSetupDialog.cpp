#include <QSettings>

#include "txVmExternalSetupDialog.h"
#include "ui_txVmExternalSetupDialog.h"

TxVmExternalSetupDialog::TxVmExternalSetupDialog(VoiceKeyerCapabilities voiceCap_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TxVmExternalSetupDialog),
    voiceCap(voiceCap_)

{
    ui->setupUi(this);

    QSettings settings;
    QByteArray geometry = settings.value("txVmWxternalSetupDialog/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);


    initSetup();

}

TxVmExternalSetupDialog::~TxVmExternalSetupDialog()
{
    delete ui;
}
void TxVmExternalSetupDialog::doCloseEvent()
{
    QSettings settings;
    settings.setValue("txVmExternalSetupDialog/geometry", saveGeometry());
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
    vmCommonParams = vmCommonParams_;
}

void TxVmExternalSetupDialog::initSetup()
{
    // get info from remote keyer
}

void TxVmExternalSetupDialog::on_okButton_clicked()
{
    accept();
}


void TxVmExternalSetupDialog::on_cancelButton_clicked()
{
    reject();
}

