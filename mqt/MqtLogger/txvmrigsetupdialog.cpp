#include <QSettings>
#include <QMessageBox>
#include "regsettings.h"
#include "txvmrigsetupdialog.h"
#include "ui_txvmrigsetupdialog.h"
#include "serialCommonData.h"


TxVmRigSetupDialog::TxVmRigSetupDialog(TxKeyerCapabilities voiceCap_, int maxNumButtons_, int nb, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TxVmRigSetupDialog),
    voiceCap(voiceCap_),
    numButtons(nb),
    maxNumButtons(maxNumButtons_)

{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("TxVmSetupDialog/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);


    initSetup();
}

TxVmRigSetupDialog::~TxVmRigSetupDialog()
{
    delete ui;
}
void TxVmRigSetupDialog::doCloseEvent()
{
    RegSettings settings;
    settings.getSettings().setValue("TxVmSetupDialog/geometry", saveGeometry());
}
void TxVmRigSetupDialog::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void TxVmRigSetupDialog::accept()
{
    doCloseEvent();
    QDialog::accept();
}


void TxVmRigSetupDialog::initSetup()
{

    ui->numButtons->setRange(MININUM_BUTTONS, maxNumButtons);
    ui->numButtons->setValue(numButtons);

    if (voiceCap.getSupportSerial())
    {
        ui->comportSel->setVisible(true);
        ui->comportLbl->setVisible(true);
    }
    else
    {
        ui->comportSel->setVisible(false);
        ui->comportLbl->setVisible(false);
    }




    connect(ui->numButtons, QOverload<int>::of(&QSpinBox::valueChanged), this, &TxVmRigSetupDialog::onNumButtonsValueChanged);


}




void TxVmRigSetupDialog::setSetupRadioGroupBoxTitle(QString selectedRadioName)
{
    ui->setupRadioGroupBox->setTitle(selectedRadioName);
}

void TxVmRigSetupDialog::setPttEomGroupBoxVisible(bool visible)
{
    ui->EomGroupBox->setVisible(visible);
}


void TxVmRigSetupDialog::setEomRadioButtons(TxKeyerCommon::KeyerEomTypes eomType)
{
    if (eomType == TxKeyerCommon::KeyerEomTypes::CAT)
    {
        ui->catEomRb->setChecked(true);
        ui->timerEomRb->setChecked(false);
    }
    else if (eomType == TxKeyerCommon::KeyerEomTypes::Timer)
    {
        ui->catEomRb->setChecked(false);
        ui->timerEomRb->setChecked(true);
    }
    else
    {
        ui->catEomRb->setChecked(false);
        ui->timerEomRb->setChecked(false);
    }
}


TxKeyerCommon::KeyerEomTypes TxVmRigSetupDialog::getSelectedEomType()
{
    TxKeyerCommon::KeyerEomTypes selectedType = TxKeyerCommon::KeyerEomTypes::Eom_None;
    if (ui->catEomRb->isChecked())
    {
        selectedType = TxKeyerCommon::KeyerEomTypes::CAT;
    }
    else if (ui->timerEomRb->isChecked())
    {
        selectedType = TxKeyerCommon::KeyerEomTypes::Timer;
    }

    return selectedType;
}

void TxVmRigSetupDialog::setSwitchToCwVisible(bool visible)
{
    ui->switchToCw->setVisible(visible);
}




void TxVmRigSetupDialog::setSwitchToCwChecked(bool checked)
{
    ui->switchToCw->setChecked(checked);
}


bool TxVmRigSetupDialog::getSetCwModeAndRestoreState()
{
    return ui->switchToCw->isChecked();
}

void TxVmRigSetupDialog::setButtonWidgetsVisible(bool visible)
{
    ui->maxAvailButtonsTitle->setVisible(visible);
    ui->numButtons->setVisible(visible);
    ui->nunButtonsLbl->setVisible(visible);

}

void TxVmRigSetupDialog::onNumButtonsValueChanged(int num)
{
    numButtons = num;
}


void TxVmRigSetupDialog::setMaxNumOfButtons(int maxNumButtons)
{

    ui->numButtons->setRange(MININUM_BUTTONS, maxNumButtons);
}




void TxVmRigSetupDialog::setMaxNumOfButtonsLabel(int maxNumButtons)
{
    ui->maxAvailButtonsLbl->setText(QString::number(maxNumButtons));
}
