#include <QMessageBox>
#include <QSettings>
#include "sbdriver.h"
#include "WaveShowDialog.h"
#include "voicekeyerfactory.h"

#include "txvminternalbuttondialog.h"
#include "ui_txvminternalbuttondialog.h"

static bool txvmIntInhibitCallbacks = false;

static TxVmInternalButtonDialog *txvmbd = nullptr;
void TxVmInternalButtonDialog::doSetVU(unsigned int peakvol, unsigned int rmsvol , unsigned int samples, qint64 delay, int buffered)
{
    if (!txvmIntInhibitCallbacks)
        ui->levelMeter->levelChanged( peakvol / 32768.0, rmsvol / 32768.0, samples );
}

TxVmInternalButtonDialog::TxVmInternalButtonDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TxVmInternalButtonDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QSettings settings;
    QByteArray geometry = settings.value("TxVmInternalButtonDialog/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);


    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &TxVmInternalButtonDialog::on_okButtonCicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &TxVmInternalButtonDialog::on_cancelbuttonClicked);
    connect(ui->txVmRepeatPauseDur , &QLineEdit::editingFinished, this, &TxVmInternalButtonDialog::onVmRepeatPauseDurEditingFinished);

    txvmIntInhibitCallbacks = false;
}

TxVmInternalButtonDialog::~TxVmInternalButtonDialog()
{
    txvmbd = nullptr;
    txvmIntInhibitCallbacks = true;

    delete ui;
}

void TxVmInternalButtonDialog::doCloseEvent()
{
    QSettings settings;
    settings.setValue("TxVmInternalButtonDialog/geometry", saveGeometry());
    txvmIntInhibitCallbacks = true;
    txvmbd = nullptr;
}
void TxVmInternalButtonDialog::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void TxVmInternalButtonDialog::accept()
{
    doCloseEvent();
    QDialog::accept();
}


void TxVmInternalButtonDialog::setVmData(VoiceKeyerParams* vmData_)
{
    txvmbd = this;
    vmData = vmData_;
    ui->txVmTypeLbl->setText(vmData->getType());
    ui->txVmNameEdit->setText(vmData->getVmName());
    ui->txVmRepeatChkBox->setChecked(vmData->getVmRepeatFlag());
    ui->txVmRepeatPauseDur->setText(QString::number(vmData->getVmRepeatPauseDur()));
    ui->txVmMessageDur->setText(QString::number(vmData->getVmDuration()));

    connect(SoundSystemDriver::getSbDriver(), &SoundSystemDriver::setVU, this, &TxVmInternalButtonDialog::doSetVU);

    inVolChange = true;

    QString fileName = VOICE_KEYER_PATH + VOICE_KEYER_BASE_FILE_NAME + "Internal" + ".ini";
    QSettings settings(fileName, QSettings::IniFormat);

    int recordLevel = settings.value("RecordLevel", 0).toInt();

    ui->recordSlider->setValue(recordLevel);

    inVolChange = false;

}

void TxVmInternalButtonDialog::onVmRepeatPauseDurEditingFinished()
{
    int dur_ = 0;
    validateDur(tr("Repeat Pause"), ui->txVmRepeatPauseDur->text(), dur_);
}

bool TxVmInternalButtonDialog::validateDur(QString durName, QString dur, int& dur_)
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

void TxVmInternalButtonDialog::on_okButtonCicked()
{
    on_stopButton_clicked();
    int repeatPauseDur_ = 0;
    if (!validateDur(tr("Repeat Pause"), ui->txVmRepeatPauseDur->text(), repeatPauseDur_))
    {
        return;
    }

    int messageDur_ = 0;
    if (!validateDur(tr("Message"), ui->txVmMessageDur->text(), messageDur_))
    {
        return;
    }

    QString name = ui->txVmNameEdit->text();
    vmData->setVmName(name);
    vmData->setVmRepeatPauseDur(repeatPauseDur_);
    vmData->setVmDuration(messageDur_);
    vmData->setVmRepeatFlag(ui->txVmRepeatChkBox->isChecked());
    accept();

}


void TxVmInternalButtonDialog::on_cancelbuttonClicked()
{
    on_stopButton_clicked();
    reject();
}


void TxVmInternalButtonDialog::on_replayButton_clicked()
{
    // This is a test replay - we shouldn't be PTTing if we can avoid it
    vmData->getVkBase()->sendMsgNum(vmData->getvmButtonNum());
}

void TxVmInternalButtonDialog::on_recordButton_clicked()
{
    // make a recording for this button
    vmData->getVkBase()->doRecording(vmData);

    ui->buttonBox->button(QDialogButtonBox::StandardButton::Cancel)->setEnabled(false);
    ui->recordButton->setEnabled(false);
}

void TxVmInternalButtonDialog::on_stopButton_clicked()
{
    // stop record/replay
    vmData->getVkBase()->stopMsg(vmData);
    ui->txVmMessageDur->setText(QString::number(vmData->getVmDuration()));
    ui->recordButton->setEnabled(true);
}
void TxVmInternalButtonDialog::setVolumeMults()
{
    int record = ui->recordSlider->value();
    SoundSystemDriver::getSbDriver()->setVolumeMults(record, 0, 0, CompressorParams(), false, false);  // for now, set everything to 0db

    inVolChange = true;

    ui->recordLevel->setValue(record/10.0);

    inVolChange = false;
}

void TxVmInternalButtonDialog::on_recordLevel_valueChanged(double arg1)
{
    if (!inVolChange)
    {
        ui->recordSlider->setValue(static_cast<int>(arg1 * 10));
    }
}

void TxVmInternalButtonDialog::on_recordSlider_valueChanged(int position)
{
    if (!inVolChange)
    {
        QString fileName = VOICE_KEYER_PATH + VOICE_KEYER_BASE_FILE_NAME + "Internal" + ".ini";
        QSettings settings(fileName, QSettings::IniFormat);
        settings.setValue("RecordLevel", position);
    }
    setVolumeMults();
}



void TxVmInternalButtonDialog::on_showButton_clicked()
{
    int fno = vmData->getvmButtonNum();
    WaveShowDialog wsd(this, fno);
    wsd.exec();
}

