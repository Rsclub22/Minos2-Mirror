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

    ui->compressionSlider->setMaximum(COMPRESSION_LIMIT);

    connect(LogContainer->sendDM, &TSendDM::keyerConfig, this, &TxVmExternalButtonDialog::onKeyerConfig);
    LogContainer->sendDM->publishKeyerMS(true);   // force resubscribe so we get keyer configs
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
    msgBox.setInformativeText(tr("Please set value between %1 and %2 seconds").arg(REPEAT_DUR_MIN).arg(REPEAT_DUR_MAX));
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
}

void TxVmExternalButtonDialog::on_replayButton_clicked()
{
    trace("TxVmExternalButtonDialog::on_replayButton_clicked()");
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

void TxVmExternalButtonDialog::on_recordValue_valueChanged(double arg1)
{
    if (inVolChangeCount <= 0)
    {
        inVolChangeCount = 1;
        ui->recordSlider->setValue(static_cast<int>(arg1 * 10));
        pubSliders();
        inVolChangeCount--;
    }
}

void TxVmExternalButtonDialog::on_recordSlider_valueChanged(int /*position*/)
{
    if (inVolChangeCount <= 0)
    {
        pubSliders();

        inVolChangeCount = 1;
        int v = ui->recordSlider->value();
        ui->recordValue->setValue(v/10.0);
        inVolChangeCount--;
    }
}

void TxVmExternalButtonDialog::on_replayValue_valueChanged(double arg1)
{
    if (inVolChangeCount <= 0)
    {
        inVolChangeCount = 1;
        ui->replaySlider->setValue(static_cast<int>(arg1 * 10));
        pubSliders();
        inVolChangeCount--;
    }
}


void TxVmExternalButtonDialog::on_replaySlider_valueChanged(int /*value*/)
{
    if (inVolChangeCount <= 0)
    {
        pubSliders();

        inVolChangeCount = 1;
        int v = ui->replaySlider->value();
        ui->replayValue->setValue(v/10.0);
        inVolChangeCount--;
    }
}


void TxVmExternalButtonDialog::on_passThroughValue_valueChanged(double arg1)
{
    if (inVolChangeCount <= 0)
    {
        inVolChangeCount = 1;
        ui->passThroughSlider->setValue(static_cast<int>(arg1 * 10));
        pubSliders();
        inVolChangeCount--;
    }
}


void TxVmExternalButtonDialog::on_passThroughSlider_valueChanged(int /*value*/)
{
    if (inVolChangeCount <= 0)
    {
        pubSliders();

        inVolChangeCount = 1;
        int v = ui->passThroughSlider->value();
        ui->passThroughValue->setValue(v/10.0);
        inVolChangeCount--;
    }
}
void TxVmExternalButtonDialog::on_compressionValue_valueChanged(int arg1)
{
    if (inVolChangeCount <= 0)
    {
        inVolChangeCount = 1;
        ui->compressionSlider->setValue(arg1);
        pubSliders();
        inVolChangeCount--;
    }
}
void TxVmExternalButtonDialog::on_compressionSlider_valueChanged(int /*value*/)
{
    pubSliders();
    if (inVolChangeCount <= 0)
    {
        inVolChangeCount = 1;
        int v = ui->compressionSlider->value();
        ui->compressionValue->setValue(v);
        inVolChangeCount--;
    }
}

void TxVmExternalButtonDialog::pubSliders()
{
    int v0 = ui->recordSlider->value();
    int v1 = ui->replaySlider->value();
    int v2 = ui->passThroughSlider->value();
    int c = ui->compressionSlider->value();
    QString sliders = QString("%1;%2;%3;%4").arg(v0).arg(v1).arg(v2).arg(c);
    RPCPubSub::publish(rpcConstants::KeyerConfigCategory, rpcConstants::keyerSliders, sliders, psPublished);
}
void TxVmExternalButtonDialog::onKeyerConfig(QString key, QString val)
{
    if (key == rpcConstants::keyerConfig)
    {
        // JSON
        KeyerJson kj;
        kj.parseConfig(val, false);
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
                inVolChangeCount++;

                trace(QString("onKeyerConfig keyerSliders %1;%2;%3;%4").arg(vals[0], vals[1], vals[2], vals[3]));
                ui->recordSlider->setValue(vals[0].toDouble());
                ui->recordValue->setValue(vals[0].toDouble()/10.0);
                ui->replaySlider->setValue(vals[1].toDouble());
                ui->replayValue->setValue(vals[1].toDouble()/10.0);
                ui->passThroughSlider->setValue(vals[2].toDouble());
                ui->passThroughValue->setValue(vals[2].toDouble()/10.0);

                ui->compressionSlider->setValue(vals[3].toInt());
                ui->compressionValue->setValue(vals[3].toInt());

                inVolChangeCount--;
            }
}


