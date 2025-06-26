#include <QSettings>
#include <QMessageBox>

#include "regsettings.h"
#include "MTrace.h"
#include "RPCPubSub.h"
#include "SendRPCDM.h"
#include "tlogcontainer.h"
#include "KeyerJson.h"
#include "txVmExternalButtonDialog.h"
#include "ui_txVmExternalButtonDialog.h"

static TxVmExternalButtonDialog *txvmbd = nullptr;

TxVmExternalButtonDialog::TxVmExternalButtonDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TxVmExternalButtonDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("TxVmExternalButtonDialog/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);
    // record
    recordFrame = new SliderSpinner(this, tr("\nRecord"), Qt::Vertical, -10, +10, 0);
    ui->levelsFrame->layout()->addWidget(recordFrame);
    connect(recordFrame, &SliderSpinner::valueChanged, this, &TxVmExternalButtonDialog::volsChanged);

    // replay
    replayFrame = new SliderSpinner(this, tr("\nReplay"), Qt::Vertical, -10, +10, 0);
    ui->levelsFrame->layout()->addWidget(replayFrame);
    connect(replayFrame, &SliderSpinner::valueChanged, this, &TxVmExternalButtonDialog::volsChanged);

    // passthrough
    passthroughFrame = new SliderSpinner(this, tr("Pass\nThrough"), Qt::Vertical, -10, +10, 0);
    ui->levelsFrame->layout()->addWidget(passthroughFrame);
    connect(passthroughFrame, &SliderSpinner::valueChanged, this, &TxVmExternalButtonDialog::volsChanged);

    windowFrame = new SliderSpinner(this, tr("Window (ms)"), Qt::Horizontal, 1, +100, 1);
    ui->compFrame->layout()->addWidget(windowFrame);
    connect(windowFrame, &SliderSpinner::valueChanged, this, &TxVmExternalButtonDialog::compressionChanged);

    thresholdFrame = new SliderSpinner(this, tr("Threshold (db below max)"), Qt::Horizontal, -40, 0, 0);
    ui->compFrame->layout()->addWidget(thresholdFrame);
    connect(thresholdFrame, &SliderSpinner::valueChanged, this, &TxVmExternalButtonDialog::compressionChanged);

    ratioFrame = new SliderSpinner(this, tr("Compression Ratio"), Qt::Horizontal, 0, +50, 0);
    ui->compFrame->layout()->addWidget(ratioFrame);
    connect(ratioFrame, &SliderSpinner::valueChanged, this, &TxVmExternalButtonDialog::compressionChanged);

    attackFrame = new SliderSpinner(this, tr("Attack (ms)"), Qt::Horizontal, 1, 100, 0);
    ui->compFrame->layout()->addWidget(attackFrame);
    connect(attackFrame, &SliderSpinner::valueChanged, this, &TxVmExternalButtonDialog::compressionChanged);

    releaseFrame = new SliderSpinner(this, tr("Release (ms)"), Qt::Horizontal, 1, 100, 0);
    ui->compFrame->layout()->addWidget(releaseFrame);
    connect(releaseFrame, &SliderSpinner::valueChanged, this, &TxVmExternalButtonDialog::compressionChanged);

    makeUpGainFrame = new SliderSpinner(this, tr("Makeup Gain (db)"), Qt::Horizontal, 0, +20, 0);
    ui->compFrame->layout()->addWidget(makeUpGainFrame);
    connect(makeUpGainFrame, &SliderSpinner::valueChanged, this, &TxVmExternalButtonDialog::compressionChanged);

    trace("TxVmExternalButtonDialog started");

    connect(LogContainer->sendDM, &TSendDM::keyerConfig, this, &TxVmExternalButtonDialog::onKeyerConfig);
    LogContainer->sendDM->publishKeyerMS(true);   // force resubscribe so we get keyer configs
}

TxVmExternalButtonDialog::~TxVmExternalButtonDialog()
{
    trace("TxVmExternalButtonDialog finished");
    LogContainer->sendDM->publishKeyerMS(false);
    delete ui;
}

void TxVmExternalButtonDialog::doCloseEvent()
{
    RegSettings settings;
    settings.getSettings().setValue("TxVmExternalButtonDialog/geometry", saveGeometry());
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
    vmData->setKeyerName(name);
    vmData->setKeyerRepeatPauseDur(repeatPauseDur_);
//    vmData->setVmDuration(messageDur_);
    vmData->setKeyerRepeatFlag(ui->repeatChkBox->isChecked());
    accept();
}


void TxVmExternalButtonDialog::on_buttonBox_rejected()
{
    reject();
}


void TxVmExternalButtonDialog::setVmData(TxKeyerParams *vmData_)
{
    txvmbd = this;
    vmData = vmData_;
    ui->txVmTypeLbl->setText(vmData->getType());
    ui->txVmNameEdit->setText(vmData->getKeyerName());
    ui->repeatChkBox->setChecked(vmData->getKeyerRepeatFlag());
    ui->repeatPauseDur->setText(QString::number(vmData->getKeyerRepeatPauseDur()));
}

void TxVmExternalButtonDialog::on_replayButton_clicked()
{
    trace("TxVmExternalButtonDialog::on_replayButton_clicked()");
    emit LogContainer->sendKeyerPlay( vmData->getKeyerButtonNum() );
}

void TxVmExternalButtonDialog::on_recordButton_clicked()
{
    emit LogContainer->sendKeyerRecord( vmData->getKeyerButtonNum() );
}

void TxVmExternalButtonDialog::on_stopButton_clicked()
{
    trace("TxVmExternalButtonDialog::on_stopButton_clicked");
    emit LogContainer->sendKeyerStop();
}

void TxVmExternalButtonDialog::pubSliders()
{
    int v0 = recordFrame->getIntValue();
    int v1 = replayFrame->getIntValue();
    int v2 = passthroughFrame->getIntValue();

    getCompSliders();
    QString sliders = QString("%1;%2;%3;%4;%5;%6;%7;%8;%9;%10;%11")
            .arg(v0).arg(v1).arg(v2)
            .arg(compParams.window)
            .arg(compParams.threshold)
            .arg(compParams.ratio)
            .arg(compParams.attack)
            .arg(compParams.release)
            .arg(compParams.makeUpGain)
            .arg(compParams.doFilter)
            .arg(compParams.doCompression);
            ;

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

        int buttonNumber = vmData->getKeyerButtonNum();
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
            ui->levelMeter->levelChanged( peakvol / 32768.0, rmsvol / 32768.0, samples );
        }
        else
            if (key == rpcConstants::keyerSliders)
            {
                QStringList vals = val.split(";");
                inVolChangeCount++;

                trace(QString("onKeyerConfig keyerSliders %1;%2;%3;%4").arg(vals[0], vals[1], vals[2], vals[3]));
                recordFrame->setIntValue(vals[0].toInt());
                replayFrame->setIntValue(vals[1].toInt());
                passthroughFrame->setIntValue(vals[2].toInt());

                compParams.window = vals[3].toDouble();
                compParams.threshold = vals[4].toDouble();
                compParams.ratio = vals[5].toDouble();
                compParams.attack = vals[6].toDouble();
                compParams.release = vals[7].toDouble();
                compParams.makeUpGain = vals[8].toDouble();

                compParams.doFilter = (vals[9].toInt() > 0);
                compParams.doCompression = (vals[10].toInt() > 0);

                setCompSliders();

                inVolChangeCount--;
            }
}


void TxVmExternalButtonDialog::getCompSliders()
{
    compParams.window = windowFrame->getValue();       // milliseconds
    compParams.threshold = thresholdFrame->getValue();

    double rrange = ratioFrame->maximum() - ratioFrame->minimum() + 1;
    compParams.ratio = 1 - ratioFrame->getValue()/rrange;

    compParams.attack = attackFrame->getValue();     // ms
    compParams.release = releaseFrame->getValue(); // ms
    compParams.makeUpGain = makeUpGainFrame->getValue();

    compParams.doFilter = ui->doFilter->isChecked();
    compParams.doCompression = ui->doCompression->isChecked();
}

void TxVmExternalButtonDialog::setCompSliders()
{
    windowFrame->setValue(compParams.window);       // milliseconds
    thresholdFrame->setValue(compParams.threshold);

    double rrange = ratioFrame->maximum() - ratioFrame->minimum() + 1;
    ratioFrame->setValue(rrange * (1 - compParams.ratio));

    attackFrame->setValue(compParams.attack);     // ms
    releaseFrame->setValue(compParams.release); // ms
    makeUpGainFrame->setValue(compParams.makeUpGain);

    ui->doFilter->setChecked(compParams.doFilter);
    ui->doCompression->setChecked(compParams.doCompression);
}


void TxVmExternalButtonDialog::volsChanged()
{
    if (inVolChangeCount <= 0)
    {
        pubSliders();
    }
}
void TxVmExternalButtonDialog::compressionChanged()
{
    if (inVolChangeCount <= 0)
    {
        getCompSliders();
        pubSliders();
    }
}



void TxVmExternalButtonDialog::on_doFilter_stateChanged(int /*arg1*/)
{
    if (inVolChangeCount <= 0)
    {
        getCompSliders();
        pubSliders();
    }
}


void TxVmExternalButtonDialog::on_doCompression_stateChanged(int /*arg1*/)
{
    if (inVolChangeCount <= 0)
    {
        getCompSliders();
        pubSliders();
    }
}

