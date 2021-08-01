#include <QSettings>

#include "txVmExternalButtonDialog.h"
#include "ui_txVmExternalButtonDialog.h"

static bool inhibitCallbacks = false;
static TxVmExternalButtonDialog *txvmbd = nullptr;

TxVmExternalButtonDialog::TxVmExternalButtonDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TxVmExternalButtonDialog)
{
    ui->setupUi(this);
}

TxVmExternalButtonDialog::~TxVmExternalButtonDialog()
{
    delete ui;
}

void TxVmExternalButtonDialog::doCloseEvent()
{
    QSettings settings;
    settings.setValue("TxVmInternalButtonDialog/geometry", saveGeometry());
    inhibitCallbacks = true;
    txvmbd = nullptr;
}

void TxVmExternalButtonDialog::setVolumeMults()
{

}

bool TxVmExternalButtonDialog::validateDur(QString durName, QString dur, int &dur_)
{

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

void TxVmExternalButtonDialog::doSetVU(unsigned int rmsvol, unsigned int peakvol, unsigned int samples)
{

}

void TxVmExternalButtonDialog::setVmData(VoiceKeyerParams *vmData)
{

}

void TxVmExternalButtonDialog::on_okButtonCicked()
{

}

void TxVmExternalButtonDialog::on_cancelbuttonClicked()
{

}

void TxVmExternalButtonDialog::onVmRepeatPauseDurEditingFinished()
{

}

void TxVmExternalButtonDialog::onVmMessageDurEditingFinished()
{

}

void TxVmExternalButtonDialog::on_replayButton_clicked()
{

}

void TxVmExternalButtonDialog::on_recordButton_clicked()
{

}

void TxVmExternalButtonDialog::on_stopButton_clicked()
{

}

void TxVmExternalButtonDialog::on_recordLevel_valueChanged(double arg1)
{

}

void TxVmExternalButtonDialog::on_recordSlider_valueChanged(int position)
{

}
