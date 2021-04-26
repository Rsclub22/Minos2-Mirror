#include <QMessageBox>
#include <QSettings>

#include "txvminternalbuttondialog.h"
#include "voicekeyerfactory.h"
#include "ui_txvminternalbuttondialog.h"


TxVmInternalButtonDialog::TxVmInternalButtonDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TxVmInternalButtonDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &TxVmInternalButtonDialog::on_okButtonCicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &TxVmInternalButtonDialog::on_cancelbuttonClicked);
    connect(ui->txVmRepeatPauseDur , &QLineEdit::editingFinished, this, &TxVmInternalButtonDialog::onVmRepeatPauseDurEditingFinished);
    connect(ui->txVmMessageDur , &QLineEdit::editingFinished, this, &TxVmInternalButtonDialog::onVmMessageDurEditingFinished);


}

TxVmInternalButtonDialog::~TxVmInternalButtonDialog()
{
    delete ui;
}


void TxVmInternalButtonDialog::setVmData(VoiceKeyerParams* vmData_)
{
    vmData = vmData_;
    ui->txVmTypeLbl->setText(vmData->getType());
    ui->txVmNameEdit->setText(vmData->getVmName());
    ui->txVmRepeatChkBox->setChecked(vmData->getVmRepeatFlag());
    ui->txVmRepeatPauseDur->setText(QString::number(vmData->getVmRepeatPauseDur()));
    ui->txVmMessageDur->setText(QString::number(vmData->getVmDuration()));

    ui->recordingFrame->setVisible(vmData->getVkBase()->hasRecord());
    ui->levelsFrame->setVisible(vmData->getVkBase()->hasRecord());

//    if (vmData->getVkBase()->hasRecord())
//    {
//        ui->inChannelCB->addItems(rass.inputDevices);
//        ui->outChannelCB->addItems(rass.outputDevices);

//        QSettings settings(filename, QSettings::IniFormat);

//        QString indev = settings.value(indevKey, "").toString();
//        QString outdev = settings.value(outdevKey, "").toString();

//        ui->inChannelCB->setCurrentText(indev);
//        ui->outChannelCB->setCurrentText(outdev);

//        connect(ui->inChannelCB, &QComboBox::currentTextChanged, this, &TxVmInternalButtonDialog::inChannelCB_currentTextChanged);
//        connect(ui->outChannelCB, &QComboBox::currentTextChanged, this, &TxVmInternalButtonDialog::outChannelCB_currentTextChanged);

//        rass.setVUCallBack( &::volcallback );

//        inVolChange = true;

//        int recordLevel = settings.value("RecordLevel", 0).toInt();

//        ui->recordSlider->setValue(recordLevel);

//        inVolChange = false;

//        bool mono = settings.value("Mono", false).toBool();
//        ui->recordMono->setChecked(mono);

//    }

}

void TxVmInternalButtonDialog::onVmRepeatPauseDurEditingFinished()
{
    int dur_ = 0;
    validateDur(tr("Repeat Pause"), ui->txVmRepeatPauseDur->text(), dur_);
}

void TxVmInternalButtonDialog::onVmMessageDurEditingFinished()
{
    int dur_ = 0;
    validateDur(tr("Message"), ui->txVmMessageDur->text(), dur_);
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
    msgBox.setInformativeText(tr("Please set value between 0 and 180 seconds"));
    msgBox.exec();
    return false;

}

void TxVmInternalButtonDialog::on_okButtonCicked()
{
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
}

void TxVmInternalButtonDialog::on_stopButton_clicked()
{
    // stop record/replay
    vmData->getVkBase()->stopMsg();
}
void TxVmInternalButtonDialog::inChannelCB_currentTextChanged(const QString &arg1)
{
//    QString fileName = VOICE_KEYER_PATH + VOICE_KEYER_BASE_FILE_NAME + vmParams.getType() + ".ini";
//    QSettings config(fileName, QSettings::IniFormat);
//    config.beginGroup("button" + QString::number(buttonNum));

//    settings.setValue(indevKey, arg1);

//    trace("About to re-initialise audio");
//    rass.closedown();
//    rass.initialise(ui->inChannelCB->currentText(), ui->outChannelCB->currentText());
}

void TxVmInternalButtonDialog::outChannelCB_currentTextChanged(const QString &arg1)
{
//    if (!closing)
//    {
//        QString filename = "./Configuration/RigRecorder.ini";
//        QSettings settings(filename, QSettings::IniFormat);
//        settings.setValue(outdevKey, arg1);
//        trace("About to re-initialise audio");
//        rass.closedown();
//        rass.initialise(ui->inChannelCB->currentText(), ui->outChannelCB->currentText());
//    }
}
void TxVmInternalButtonDialog::setVolumeMults()
{
//    int record = ui->recordSlider->value();
//    rass.setRecordLevel(record);

//    inVolChange = true;

//    ui->recordLevel->setValue(record/10.0);

//    inVolChange = false;
}

void TxVmInternalButtonDialog::on_recordLevel_valueChanged(double arg1)
{
//    if (!inVolChange)
//    {
//        ui->recordSlider->setValue(static_cast<int>(arg1 * 10));
//    }
}

void TxVmInternalButtonDialog::on_recordSlider_valueChanged(int position)
{
//    if (!inVolChange)
//    {
//        QString filename = "./Configuration/RigRecorder.ini";
//        QSettings settings(filename, QSettings::IniFormat);
//        settings.setValue("RecordLevel", position);
//    }
//    setVolumeMults();
}

void TxVmInternalButtonDialog::on_recordMono_stateChanged(int /*arg1*/)
{
//    bool mono = ui->recordMono->isChecked();
//    rass.setMono(mono);
//    QString filename = "./Configuration/RigRecorder.ini";
//    QSettings settings(filename, QSettings::IniFormat);
//    settings.setValue("Mono", mono);
}
