#include <QSettings>
#include "sbdriver.h"
#include "MTrace.h"

#include "txVmInternalSetupDialog.h"
#include "ui_txVmInternalSetupDialog.h"

const QString indevKey("InDevice");
const QString outdevKey("OutDevice");

txVmInternalSetupDialog::txVmInternalSetupDialog(VoiceKeyerCapabilities voiceCap_, int nb, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::txVmInternalSetupDialog),
    voiceCap(voiceCap_),
    numButtons(nb)

{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QSettings settings;
    QByteArray geometry = settings.value("txVmInternalSetupDialog/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);


    initSetup();
}

txVmInternalSetupDialog::~txVmInternalSetupDialog()
{
    delete ui;
}
void txVmInternalSetupDialog::doCloseEvent()
{
    QSettings settings;
    settings.setValue("txVmInternalSetupDialog/geometry", saveGeometry());
}
void txVmInternalSetupDialog::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void txVmInternalSetupDialog::accept()
{
    doCloseEvent();
    QDialog::accept();
}


void txVmInternalSetupDialog::initSetup()
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

    connect(ui->numButtons, QOverload<int>::of(&QSpinBox::valueChanged), this, &txVmInternalSetupDialog::onNumButtonsValueChanged);

    ui->inChannelCB->addItems(SoundSystemDriver::getSbDriver()->getInputDevices());
    ui->outChannelCB->addItems(SoundSystemDriver::getSbDriver()->getOutputDevices());

    QString fileName = VOICE_KEYER_PATH + VOICE_KEYER_BASE_FILE_NAME + "Internal" + ".ini";
    QSettings settings(fileName, QSettings::IniFormat);

    QString indev = settings.value(indevKey, "").toString();
    QString outdev = settings.value(outdevKey, "").toString();

    ui->inChannelCB->setCurrentText(indev);
    ui->outChannelCB->setCurrentText(outdev);

    connect(ui->inChannelCB, &QComboBox::currentTextChanged, this, &txVmInternalSetupDialog::inChannelCB_currentTextChanged);
    connect(ui->outChannelCB, &QComboBox::currentTextChanged, this, &txVmInternalSetupDialog::outChannelCB_currentTextChanged);


}


void txVmInternalSetupDialog::onNumButtonsValueChanged(int num)
{
    numButtons = num;
}


void txVmInternalSetupDialog::inChannelCB_currentTextChanged(const QString &arg1)
{
    QString fileName = VOICE_KEYER_PATH + VOICE_KEYER_BASE_FILE_NAME + "Internal" + ".ini";
    QSettings settings(fileName, QSettings::IniFormat);

    settings.setValue(indevKey, arg1);

    trace("About to re-initialise audio");
    SoundSystemDriver::getSbDriver()->closedown();
    SoundSystemDriver::getSbDriver()->initialise(ui->inChannelCB->currentText(), ui->outChannelCB->currentText(), "", "");
}

void txVmInternalSetupDialog::outChannelCB_currentTextChanged(const QString &arg1)
{
    QString fileName = VOICE_KEYER_PATH + VOICE_KEYER_BASE_FILE_NAME + "Internal" + ".ini";
    QSettings settings(fileName, QSettings::IniFormat);
    settings.setValue(outdevKey, arg1);
    trace("About to re-initialise audio");
    SoundSystemDriver::getSbDriver()->closedown();
    SoundSystemDriver::getSbDriver()->initialise(ui->inChannelCB->currentText(), ui->outChannelCB->currentText(), "", "");
}
