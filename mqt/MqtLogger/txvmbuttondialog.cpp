#include "txvmbuttondialog.h"
#include "ui_txvmbuttondialog.h"


TxVmButtonDialog::TxVmButtonDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TxVmButtonDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);



    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(on_okButton_clicked()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(on_cancelbutton_clicked()));
    connect(ui->txVmRepeatDur, SIGNAL(editingFinished()), this, SLOT(onVmRepeatDurEditingFinished()));
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
    ui->txVmRepeatDur->setText(QString::number(vmData->getVmRepeatDur()));

}

void TxVmButtonDialog::onVmRepeatDurEditingFinished()
{
    int dur_ = 0;
    validateRepeatDur(ui->txVmRepeatDur->text(), dur_);
}

bool TxVmButtonDialog::validateRepeatDur(QString dur, int& dur_)
{
    bool ok;
    int d = dur.trimmed().toInt(&ok);
    if (ok && (d >= REPEAT_DUR_MIN && d <= REPEAT_DUR_MAX))
    {
        dur_ = d;
        return true;
    }

    QMessageBox msgBox;
    msgBox.setText(tr("Repeat Duration ") + dur + tr(" - out of range"));
    msgBox.setInformativeText(tr("Please set value between 0 and 180 seconds"));
    msgBox.exec();
    return false;

}

void TxVmButtonDialog::on_okButton_clicked()
{
    int dur_ = 0;
    if (!validateRepeatDur(ui->txVmRepeatDur->text(), dur_))
    {
        return;
    }

    QString name = ui->txVmNameEdit->text();
    vmData->setVmName(name);
    vmData->setVmRepeatDur(dur_);
    vmData->setVmRepeatFlag(ui->txVmRepeatChkBox->isChecked());
    accept();

}


void TxVmButtonDialog::on_cancelbutton_clicked()
{
    reject();
}

