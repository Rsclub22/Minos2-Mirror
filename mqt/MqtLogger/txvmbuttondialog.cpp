#include "txvmbuttondialog.h"
#include "ui_txvmbuttondialog.h"
#include <QMessageBox>


TxVmButtonDialog::TxVmButtonDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TxVmButtonDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);



    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &TxVmButtonDialog::on_okButton_clicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &TxVmButtonDialog::on_cancelbutton_clicked);
    connect(ui->txVmRepeatPauseDur , &QLineEdit::editingFinished, this, &TxVmButtonDialog::onVmRepeatPauseDurEditingFinished);
    connect(ui->txVmMessageDur , &QLineEdit::editingFinished, this, &TxVmButtonDialog::onVmMessageDurEditingFinished);
}

TxVmButtonDialog::~TxVmButtonDialog()
{
    delete ui;
}


void TxVmButtonDialog::setVmData(VoiceKeyerParams* vmData_)
{
    vmData = vmData_;
    ui->txVmTypeLbl->setText(vmData->getType());
    ui->txVmNameEdit->setText(vmData->getVmName());
    ui->txVmRepeatChkBox->setChecked(vmData->getVmRepeatFlag());
    ui->txVmRepeatPauseDur->setText(QString::number(vmData->getVmRepeatPauseDur()));
    ui->txVmMessageDur->setText(QString::number(vmData->getVmDuration()));

    ui->recordingFrame->setVisible(vmData->getVkBase()->hasRecord());

}

void TxVmButtonDialog::onVmRepeatPauseDurEditingFinished()
{
    int dur_ = 0;
    validateDur(tr("Repeat Pause"), ui->txVmRepeatPauseDur->text(), dur_);
}

void TxVmButtonDialog::onVmMessageDurEditingFinished()
{
    int dur_ = 0;
    validateDur(tr("Message"), ui->txVmMessageDur->text(), dur_);
}

bool TxVmButtonDialog::validateDur(QString durName, QString dur, int& dur_)
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

void TxVmButtonDialog::on_okButton_clicked()
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


void TxVmButtonDialog::on_cancelbutton_clicked()
{
    reject();
}


void TxVmButtonDialog::on_replayButton_clicked()
{
    // This is a test replay - we shouldn't be PTTing if we can avoid it
    vmData->getVkBase()->sendMsgNum(vmData->getvmButtonNum());
}

void TxVmButtonDialog::on_recordButton_clicked()
{
    // make a recording for this button
    vmData->getVkBase()->doRecording(vmData);
}

void TxVmButtonDialog::on_stopButton_clicked()
{
    // stop record/replay
    vmData->getVkBase()->stopMsg();
}
