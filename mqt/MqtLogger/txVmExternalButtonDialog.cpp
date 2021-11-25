#include <QSettings>
#include "SendRPCDM.h"
#include "tlogcontainer.h"
#include "KeyerJson.h"
#include "txVmExternalButtonDialog.h"
#include "ui_txVmExternalButtonDialog.h"

static bool inhibitCallbacks = false;
static TxVmExternalButtonDialog *txvmbd = nullptr;

TxVmExternalButtonDialog::TxVmExternalButtonDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TxVmExternalButtonDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(LogContainer->sendDM, &TSendDM::keyerConfig, this, &TxVmExternalButtonDialog::onKeyerConfig);
    LogContainer->sendDM->publishKeyerMS(true);
}

TxVmExternalButtonDialog::~TxVmExternalButtonDialog()
{
    LogContainer->sendDM->publishKeyerMS(false);
    delete ui;
}

void TxVmExternalButtonDialog::doCloseEvent()
{
    QSettings settings;
    settings.setValue("TxVmInternalButtonDialog/geometry", saveGeometry());
    inhibitCallbacks = true;
    txvmbd = nullptr;
}

bool TxVmExternalButtonDialog::validateDur(QString durName, QString dur, int &dur_)
{
    bool ok;
    int d = dur.trimmed().toInt(&ok);
    if (ok && (d >= REPEAT_DUR_MIN && d <= REPEAT_DUR_MAX))
    {
        dur_ = d;
        return true;
    }

    QMessageBox msgBox;
    msgBox.setText(tr("%1 Duration ").arg(durName) + dur + tr(" - out of range"));
    msgBox.setInformativeText(tr("Please set value between 0 and 180 seconds"));
    msgBox.exec();
    return false;
}
void TxVmExternalButtonDialog::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void TxVmExternalButtonDialog::accept()
{
    doCloseEvent();
    QDialog::accept();
}

void TxVmExternalButtonDialog::on_buttonBox_accepted()
{
    int repeatPauseDur_ = 0;
    if (!validateDur(tr("Repeat Pause"), ui->repeatPauseDur->text(), repeatPauseDur_))
    {
        return;
    }

//    int messageDur_ = 0;
//    if (!validateDur(tr("Message"), ui->txVmMessageDur->text(), messageDur_))
//    {
//        return;
//    }

    QString name = ui->txVmNameEdit->text();
    vmData->setVmName(name);
    vmData->setVmRepeatPauseDur(repeatPauseDur_);
//    vmData->setVmDuration(messageDur_);
    vmData->setVmRepeatFlag(ui->repeatChkBox->isChecked());
    accept();
}


void TxVmExternalButtonDialog::on_buttonBox_rejected()
{
    reject();
}


void TxVmExternalButtonDialog::setVmData(VoiceKeyerParams *vmData_)
{
    txvmbd = this;
    vmData = vmData_;
    ui->txVmTypeLbl->setText(vmData->getType());
    ui->txVmNameEdit->setText(vmData->getVmName());
    ui->repeatChkBox->setChecked(vmData->getVmRepeatFlag());
    ui->repeatPauseDur->setText(QString::number(vmData->getVmRepeatPauseDur()));
    ui->txVmMessageDur->setText(QString::number(vmData->getVmDuration()));

}

void TxVmExternalButtonDialog::onVmRepeatPauseDurEditingFinished()
{

}

void TxVmExternalButtonDialog::onVmMessageDurEditingFinished()
{

}

void TxVmExternalButtonDialog::on_replayButton_clicked()
{
    emit LogContainer->sendKeyerPlay( vmData->getvmButtonNum() );
}

void TxVmExternalButtonDialog::on_recordButton_clicked()
{
    emit LogContainer->sendKeyerRecord( vmData->getvmButtonNum() );
}

void TxVmExternalButtonDialog::on_stopButton_clicked()
{
    emit LogContainer->sendKeyerStop();
}

void TxVmExternalButtonDialog::on_recordLevel_valueChanged(double arg1)
{
    if (!inVolChange)
    {
        inVolChange = true;
        ui->recordSlider->setValue(static_cast<int>(arg1 * 10));
        inVolChange = false;
    }
}

void TxVmExternalButtonDialog::on_recordSlider_valueChanged(int /*position*/)
{
    if (!inVolChange)
    {
        pubSliders();

        inVolChange = true;
        int v = ui->recordSlider->value();
        ui->recordLevel->setValue(v/10.0);
        inVolChange = false;
    }
}

void TxVmExternalButtonDialog::on_replayValue_valueChanged(double arg1)
{
    if (!inVolChange)
    {
        inVolChange = true;
        ui->replaySlider->setValue(static_cast<int>(arg1 * 10));
        inVolChange = false;
    }
}


void TxVmExternalButtonDialog::on_replaySlider_valueChanged(int /*value*/)
{
    if (!inVolChange)
    {
        pubSliders();

        inVolChange = true;
        int v = ui->replaySlider->value();
        ui->replayValue->setValue(v/10.0);
        inVolChange = false;
    }
}


void TxVmExternalButtonDialog::on_passThroughValue_valueChanged(double arg1)
{
    if (!inVolChange)
    {
        inVolChange = true;
        ui->passThroughSlider->setValue(static_cast<int>(arg1 * 10));
        inVolChange = false;
    }
}


void TxVmExternalButtonDialog::on_passThroughSlider_valueChanged(int /*value*/)
{
    if (!inVolChange)
    {
        pubSliders();

        inVolChange = true;
        int v = ui->passThroughSlider->value();
        ui->passThroughValue->setValue(v/10.0);
        inVolChange = false;
    }
}
void TxVmExternalButtonDialog::pubSliders()
{
    int v0 = ui->recordSlider->value();
    int v1 = ui->replaySlider->value();
    int v2 = ui->passThroughSlider->value();
    QString sliders = QString("%1;%2;%3").arg(v0).arg(v1).arg(v2);
    RPCPubSub::publish(rpcConstants::KeyerConfigCategory, rpcConstants::keyerSliders, sliders, psPublished);
}
void TxVmExternalButtonDialog::onKeyerConfig(QString key, QString val)
{
    if (key == rpcConstants::keyerConfig)
    {
        // JSON
        KeyerJson kj;
        kj.parseConfig(val);
        // and now use it!

        int buttonNumber = vmData->getvmButtonNum();
        KeyerKeyJson &kkj = kj.kjj[buttonNumber];
        ui->repeatChkBox->setChecked(kkj.autoRepeat);
        ui->txVmNameEdit->setText(kkj.CQName);
        ui->repeatPauseDur->setText(QString::number(kkj.autoRepeatDelay));

    }
    else
        if (key == rpcConstants::keyerMeter)
        {
            //    vu = QString("%1;%2;%3").arg(rmsLevel, peakLevel).arg(numSamples);
            QStringList vals = val.split(";");
            int rmsvol = vals[0].toDouble();
            int peakvol = vals[1].toDouble();
            int samples = vals[1].toInt();
            ui->levelMeter->levelChanged( rmsvol / 32768.0, peakvol / 32768.0, samples );
        }
        else
            if (key == rpcConstants::keyerSliders)
            {
                QStringList vals = val.split(";");
                //    sliders = QString("%1;%2;%3").arg(rec, replay, passthrough);
                ui->recordSlider->setValue(vals[0].toDouble());
                ui->replaySlider->setValue(vals[1].toDouble());
                ui->passThroughSlider->setValue(vals[2].toDouble());
            }
}
