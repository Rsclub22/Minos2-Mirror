/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      CW Message Keyer
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2023
//
//
//
//
/////////////////////////////////////////////////////////////////////////////


#include <QSettings>
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "voicekeyerfactory.h"
#include "txvmrigsetupdialog.h"
#include "txvmrigbuttondialog.h"
#include "rigcontrolcwmessagekeyer.h"

const char * STOPCW = "\xFF";

using namespace voiceKeyerCommon;

RigControlCwMessageKeyer::RigControlCwMessageKeyer(QObject *parent) : VoiceKeyerBase(parent)
{

}


RigControlCwMessageKeyer::~RigControlCwMessageKeyer()
{

}


void RigControlCwMessageKeyer::registerVoiceKeyer(VoiceKeyerFactory::VmKeyers* vmKeyersList)
{
    QString keyerName = "cwRigControl";

    VoiceKeyerCapabilities voiceMemCap;

    voiceMemCap.setVmIdNum(VoiceKeyerId::CW_RigControl);
    voiceMemCap.setKeyerType(keyerTypes[VoiceKeyerId::CW_RigControl]);
    voiceMemCap.setKeyerName(keyerName);
    voiceMemCap.setNumVoiceKeys(8);
    voiceMemCap.setsupportSerial(false);
    voiceMemCap.setUseCatPTTForEom(true);
    voiceMemCap.setEnableCwMode(true);
    voiceMemCap.setSupportRepeatMsg(true);
    voiceMemCap.setHasPip(false);
    voiceMemCap.setHasTxStatus(true);
    voiceMemCap.setSetupButton(true);
    voiceMemCap.setHasAvailStatus(true);
    voiceMemCap.setHasMessageRepeat(true);



    (*vmKeyersList)[keyerName] = voiceMemCap;

}

void RigControlCwMessageKeyer::setPttOnOff(bool onOff)
{
    Q_UNUSED(onOff)
}

bool RigControlCwMessageKeyer::getUsePttForEomFlag()
{
    return usePttForEom;
}

bool RigControlCwMessageKeyer::getSetCwModeAndRestoreFlag()
{
    return setCwModeAndRestoreCurrentMode;
}

void RigControlCwMessageKeyer::voiceKeyerInit(int &numButtons)
{
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::CW_RigControl] + ".ini";
    QSettings config(fileName, QSettings::IniFormat);
    numButtons = config.value("Common/NumButtons", VOICEKEYER_MAX_NUMBUTTONS).toInt();
    usePttForEom = config.value("Common/UseCatPttForEom", true).toBool();
    setCwModeAndRestoreCurrentMode = config.value("Common/SwitchToCwMode", true).toBool();
}



void RigControlCwMessageKeyer::sendCwMsg(const QString message)
{
    if (!message.isEmpty())
    {
        TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
        tslf->sendRigTxCwMessage(message);
    }

}



void RigControlCwMessageKeyer::stopCwMsg()
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    tslf->sendRigTxCwMessage(STOPCW);
}


bool RigControlCwMessageKeyer::readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams)
{
    QString fileName = VOICE_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + vmParams.getType() + ".ini";
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("button" + QString::number(buttonNum));

    vmParams.setType(config.value("type", "").toString());
    vmParams.setVmName(config.value("name", "").toString());
    vmParams.setVmCwMessage(config.value("cwMessageText", "").toString());
    vmParams.setVmRepeatFlag(config.value("repeatFlag", false).toBool());
    vmParams.setVmDuration(config.value("messageDuration", 0).toInt());
    vmParams.setVmRepeatPauseDur(config.value("repeatPauseDuration", 0).toInt());
    vmParams.setvmButtonNum(config.value("buttonNum", buttonNum).toInt());
    config.endGroup();

    return true;
}


void RigControlCwMessageKeyer::saveVmButtonParams(const VoiceKeyerParams &vmParams_ )
{
    VoiceKeyerParams vmParams = vmParams_;

    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::RigControl] + ".ini";
    QSettings readConfig(fileName, QSettings::IniFormat);

    bool saveByRadioName = readConfig.value("Common/SaveButtonByRadioName", false).toBool();


    fileName = VOICE_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + vmParams.getType() + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    if (saveByRadioName && !vmParams.getSelRadioName().isEmpty())
    {
        config.beginGroup(vmParams.getSelRadioName() + "_" + "button" + QString::number(vmParams.getvmButtonNum()));
    }
    else
    {
        config.beginGroup("button" + QString::number(vmParams.getvmButtonNum()));
    }


    config.setValue("type", vmParams.getType());
    config.setValue("name", vmParams.getVmName());
    config.setValue("cwMessageText", vmParams.getVmCwMessage());
    config.setValue("repeatFlag", vmParams.getVmRepeatFlag());
    config.setValue("messageDuration", vmParams.getVmDuration());
    config.setValue("repeatPauseDuration", vmParams.getVmRepeatPauseDur());
    config.setValue("buttonNum", vmParams.getvmButtonNum());
    config.endGroup();

}

<<<<<<< HEAD
int RigControlCwMessageKeyer::setup(VoiceKeyerFactory *voiceKeyerFactory, int &numButtons, QString /*selectedRadioName*/)
=======
int RigControlCwMessageKeyer::setup(VoiceKeyerFactory *voiceKeyerFactory, int &maxNumButtons, int &numButtons, QString selectedRadioName)
>>>>>>> 8fkh_newDev
{


    VoiceKeyerCapabilities voiceCap = voiceKeyerFactory->supportedVoiceKeyers()->value("cwRigControl");
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    TxVmRigSetupDialog txVmSetupDialog(voiceCap, maxNumButtons, numButtons, tslf->txVmButtonsFrame);
    txVmSetupDialog.setWindowTitle(tr("Rig Control CW Memory Setup"));

    cwMemType = hamlibData::CW_MEMORY_TYPES::NONE;

    int ret = txVmSetupDialog.exec();

    if (ret == QDialog::Accepted)
    {
        numButtons = txVmSetupDialog.getNumButtons();
        QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::CW_RigControl] + ".ini";
        QSettings config(fileName, QSettings::IniFormat);
        config.setValue("Common/NumButtons", numButtons);
        config.setValue("Common/UseCatPttForEom", txVmSetupDialog.getCatPttForEomState() );
        usePttForEom = txVmSetupDialog.getCatPttForEomState();
        config.setValue("Common/SwitchToCwMode", txVmSetupDialog.getSetCwModeAndRestoreState());
        setCwModeAndRestoreCurrentMode = txVmSetupDialog.getSetCwModeAndRestoreState();


    }
    return ret;


}


void RigControlCwMessageKeyer::setCwMemType(int _cwMemType)
{
    cwMemType = _cwMemType;
}


int RigControlCwMessageKeyer::editButton(VoiceKeyerParams *vmData, QString title)
{

    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    TxVmRigButtonDialog vmButtonDialog(tslf->txVmButtonsFrame);

    vmButtonDialog.setWindowTitle(title);
    vmButtonDialog.setVmData(vmData);
    vmButtonDialog.setVmTypeLabelcwMemType(cwMemType);

    vmButtonDialog.setSerialMessageTextBoxVisible(false);

    if (cwMemType == hamlibData::CW_MEMORY_TYPES::YAESU_MEM_RECALL)
    {
        vmButtonDialog.setCwMessageTextBoxVisible(false);
    }
    else
    {
       vmButtonDialog.setCwMessageTextBoxVisible(true);
    }

    vmButtonDialog.setDialogForCatPttEom(usePttForEom);

    int ret = vmButtonDialog.exec();
    return ret;

}


