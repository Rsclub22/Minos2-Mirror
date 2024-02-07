#include <QSettings>
#include "regsettings.h"
#include "txvmrigsetupdialog.h"
#include "ui_txvmrigsetupdialog.h"

TxVmRigSetupDialog::TxVmRigSetupDialog(VoiceKeyerCapabilities voiceCap_, int nb, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TxVmRigSetupDialog),
    voiceCap(voiceCap_),
    numButtons(nb)

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

    ui->numButtons->setRange(MININUM_BUTTONS, MAXNUM_BUTTONS);
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

/*
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::RigControl] + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    if (voiceCap.getUseCatPTTForEom())
    {
        ui->pttEOMChkBox->setVisible(true);

        ui->pttEOMChkBox->setChecked(config.value("Common/UseCatPttForEom", false).toBool());

    }
    else
    {
        ui->pttEOMChkBox->setVisible(false);
    }


    if (voiceCap.getEnableCwMode())
    {
        ui->switchToCw->setVisible(true);
        ui->switchToCw->setChecked(config.value("Common/SwitchToCwMode", true).toBool());
    }
    else
    {
        ui->switchToCw->setVisible(false);
    }


    // save buttons by radio file name checkbox


    ui->saveByRadioNameChkBox->setChecked(config.value("Common/SaveButtonByRadioName", false).toBool());
*/
    connect(ui->numButtons, QOverload<int>::of(&QSpinBox::valueChanged), this, &TxVmRigSetupDialog::onNumButtonsValueChanged);



}


void TxVmRigSetupDialog::setSelectedRadioNameLabel(QString selectedRadioName)
{
    ui->selectedRadioNameLbl->setText(selectedRadioName);
}

void TxVmRigSetupDialog::setPttEOMChkBoxVisible(bool visible)
{
    ui->pttEOMChkBox->setVisible(visible);
}


void TxVmRigSetupDialog::setPttEOMChkBoxChecked(bool checked)
{
    ui->pttEOMChkBox->setChecked(checked);
}

void TxVmRigSetupDialog::setSwitchToCwVisible(bool visible)
{
    ui->switchToCw->setVisible(visible);
}


void TxVmRigSetupDialog::setSwitchToCwChecked(bool checked)
{
    ui->switchToCw->setChecked(checked);
}

void TxVmRigSetupDialog::setSaveByRadioNameChkBoxChecked(bool checked)
{
    ui->saveByRadioNameChkBox->setChecked(checked);
}

bool TxVmRigSetupDialog::getSaveButtonsByRadioNameState()
{
    return ui->saveByRadioNameChkBox->isChecked();
}

bool TxVmRigSetupDialog::getCatPttForEomState()
{
    return ui->pttEOMChkBox->isChecked();
}

bool TxVmRigSetupDialog::getSetCwModeAndRestoreState()
{
    return ui->switchToCw->isChecked();
}

void TxVmRigSetupDialog::onNumButtonsValueChanged(int num)
{
    numButtons = num;
}
