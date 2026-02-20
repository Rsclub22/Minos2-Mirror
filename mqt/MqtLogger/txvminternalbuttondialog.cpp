#include <QMessageBox>
#include <QSettings>
#include "regsettings.h"
#include "sbdriver.h"
#include "WaveShowDialog.h"

#include "txvminternalbuttondialog.h"
#include "ui_txvminternalbuttondialog.h"
#include "txkeyerCommonConstants.h"

using namespace TxKeyerCommon;


static bool txvmIntInhibitCallbacks = false;

static TxVmInternalButtonDialog *txvmbd = nullptr;
void TxVmInternalButtonDialog::doSetVU(vudata v)
{
    if (!txvmIntInhibitCallbacks)
        ui->levelMeter->levelChanged( v.peak / 32768.0, v.rms / 32768.0, v.blocks );
}

TxVmInternalButtonDialog::TxVmInternalButtonDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TxVmInternalButtonDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("TxVmInternalButtonDialog/geometry").toByteArray();
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
    RegSettings settings;
    settings.getSettings().setValue("TxVmInternalButtonDialog/geometry", saveGeometry());
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


void TxVmInternalButtonDialog::setVmData(TxKeyerParams* vmData_)
{
    txvmbd = this;
    vmData = vmData_;
    ui->txVmTypeLbl->setText(vmData->getType());
    ui->txVmNameEdit->setText(vmData->getbuttonName());
    ui->txVmRepeatChkBox->setChecked(vmData->getKeyerRepeatFlag());
    ui->txVmRepeatPauseDur->setText(QString::number(vmData->getKeyerRepeatPauseDur()));
    ui->txVmMessageDur->setText(QString::number(vmData->getKeyerDuration()));

    connect(SoundSystemDriver::getSbDriver(), &SoundSystemDriver::setVU, this, &TxVmInternalButtonDialog::doSetVU);

    inVolChange = true;

    QString fileName = TX_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + "Internal" + ".ini";
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
    vmData->setButtonName(name);
    vmData->setKeyerRepeatPauseDur(repeatPauseDur_);
    vmData->setKeyerDuration(messageDur_);
    vmData->setKeyerRepeatFlag(ui->txVmRepeatChkBox->isChecked());
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
    // *********************************vmData->getVkBase()->sendMsgNum(vmData->getKeyerButtonNum());
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
    //************************************vmData->getVkBase()->stopMsg(vmData);
    //******************************ui->txVmMessageDur->setText(QString::number(vmData->getKeyerDuration()));
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
        QString fileName = TX_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + "Internal" + ".ini";
        QSettings settings(fileName, QSettings::IniFormat);
        settings.setValue("RecordLevel", position);
    }
    setVolumeMults();
}



void TxVmInternalButtonDialog::on_showButton_clicked()
{
    int fno = vmData->getKeyerButtonNum();
    WaveShowDialog wsd(this, fno);
    wsd.exec();
}

