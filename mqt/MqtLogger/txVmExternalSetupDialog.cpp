#include <QSettings>

#include "SendRPCDM.h"
#include "tlogcontainer.h"
#include "KeyerJson.h"
#include "txVmExternalSetupDialog.h"
#include "ui_txVmExternalSetupDialog.h"

TxVmExternalSetupDialog::TxVmExternalSetupDialog(VoiceKeyerCapabilities voiceCap_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TxVmExternalSetupDialog),
    voiceCap(voiceCap_)

{
    ui->setupUi(this);

    connect(LogContainer->sendDM, &TSendDM::keyerConfig, this, &TxVmExternalSetupDialog::onKeyerConfig);

    RPCPubSub::publish(rpcConstants::KeyerConfigCategory, rpcConstants::keyerSendConfig, "sliders:config:meters", psPublished);

    QSettings settings;
    QByteArray geometry = settings.value("txVmWxternalSetupDialog/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    initSetup();

}

TxVmExternalSetupDialog::~TxVmExternalSetupDialog()
{
    RPCPubSub::publish(rpcConstants::KeyerConfigCategory, rpcConstants::keyerSendConfig, "", psRevoked);

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

void TxVmExternalSetupDialog::onKeyerConfig(QString key, QString val)
{
    if (key == rpcConstants::keyerConfig)
    {
        // JSON
        KeyerJson kj;
        kj.parseConfig(val);
        // and now use it!
    }
    else
        if (key == rpcConstants::keyerMeter)
        {
            //    vu = QString("%1;%2;%3").arg(rmsLevel, peakLevel).arg(numSamples);

        }
        else
            if (key == rpcConstants::keyerSliders)
            {
                //    sliders = QString("%1;%2;%3").arg(rec, replay, passthrough);

            }
}

