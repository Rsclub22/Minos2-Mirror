#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "voicekeyerfactory.h"
#include "txvmrigsetupdialog.h"
#include "txvmrigbuttondialog.h"
#include "rigcontrolvoicememorykeyer.h"



RigControlVoiceMemoryKeyer::RigControlVoiceMemoryKeyer(QObject *parent) : VoiceKeyerBase(parent)
{

}


RigControlVoiceMemoryKeyer::~RigControlVoiceMemoryKeyer()
{

}


void RigControlVoiceMemoryKeyer::registerVoiceKeyer(VoiceKeyerFactory::VmKeyers* vmKeyersList)
{
    QString keyerName = "rigControl";

    VoiceKeyerCapabilities voiceMemCap;

    voiceMemCap.setVmIdNum(VoiceKeyerId::RigControl);
    voiceMemCap.setKeyerType(keyerTypes[VoiceKeyerId::RigControl]);
    voiceMemCap.setKeyerName(keyerName);
    voiceMemCap.setNumVoiceKeys(8);
    voiceMemCap.setsupportSerial(false);
    voiceMemCap.setUseCatPTTForEom(true);
    voiceMemCap.setEnableCwMode(false);
    voiceMemCap.setSupportRepeatMsg(true);
    voiceMemCap.setHasPip(false);
    voiceMemCap.setHasTxStatus(true);
    voiceMemCap.setSetupButton(true);
    voiceMemCap.setHasAvailStatus(true);
    voiceMemCap.setHasMessageRepeat(true);


    (*vmKeyersList)[keyerName] = voiceMemCap;

}


void RigControlVoiceMemoryKeyer::setPttOnOff(bool onOff)
{
    Q_UNUSED(onOff)
}

bool RigControlVoiceMemoryKeyer::getUsePttForEomFlag()
{
    return usePttForEom;
}


void RigControlVoiceMemoryKeyer::voiceKeyerInit(int &numButtons)
{
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::RigControl] + ".ini";
    QSettings config(fileName, QSettings::IniFormat);
    numButtons = config.value("Common/NumButtons", VOICEKEYER_MAX_NUMBUTTONS).toInt();
    usePttForEom = config.value("Common/UseCatPttForEom", true).toBool();
}

void RigControlVoiceMemoryKeyer::sendMsgNum(int buttonNum)
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    tslf->sendRigTxVoiceMessage(QString::number(buttonNum +1));  // add for Icom message Number

}

void RigControlVoiceMemoryKeyer::stopMsg(VoiceKeyerParams *vkParams)
{
    Q_UNUSED(vkParams)
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    tslf->sendRigStopTxVoiceMessage("Stop Voice Msg");
}



bool RigControlVoiceMemoryKeyer::readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams)
{

    bool saveByRadioName = readSaveButtonByRadioNameIni();


    QString fileName = VOICE_KEYER_PATH + VOICE_KEYER_BASE_FILE_NAME + vmParams.getType() + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    if (saveByRadioName && !vmParams.getSelRadioName().isEmpty())
    {
        config.beginGroup(vmParams.getSelRadioName().replace('/', '_'));
    }
    else
    {
        config.beginGroup("AllRadios");
    }

    QString newKey = "button" + QString::number(buttonNum);


    vmParams.setType(config.value(newKey + "/type", "").toString());
    vmParams.setVmName(config.value(newKey + "/name", "").toString());
    vmParams.setVmRepeatFlag(config.value(newKey + "/repeatFlag", false).toBool());
    vmParams.setVmDuration(config.value(newKey + "/messageDuration", 0).toInt());
    vmParams.setVmRepeatPauseDur(config.value(newKey + "/repeatPauseDuration", 0).toInt());
    vmParams.setvmButtonNum(config.value(newKey + "/buttonNum", buttonNum).toInt());
    config.endGroup();

    return true;
}

void RigControlVoiceMemoryKeyer::saveVmButtonParams(const VoiceKeyerParams &vmParams_ )
{
    VoiceKeyerParams vmParams = vmParams_;

    bool saveByRadioName = readSaveButtonByRadioNameIni();

    QString fileName = VOICE_KEYER_PATH + VOICE_KEYER_BASE_FILE_NAME + vmParams.getType() + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    if (saveByRadioName  && !vmParams.getSelRadioName().isEmpty())
    {
       config.beginGroup(vmParams.getSelRadioName().replace('/', '_'));
    }
    else
    {
       config.beginGroup("AllRadios");
    }

    QString newKey = "button" + QString::number(vmParams.getvmButtonNum());
    config.setValue(newKey + "/type", vmParams.getType());
    config.setValue(newKey + "/name", vmParams.getVmName());
    config.setValue(newKey + "/repeatFlag", vmParams.getVmRepeatFlag());
    config.setValue(newKey + "/messageDuration", vmParams.getVmDuration());
    config.setValue(newKey + "/repeatPauseDuration", vmParams.getVmRepeatPauseDur());
    config.setValue(newKey + "/buttonNum", vmParams.getvmButtonNum());
    config.endGroup();

    config.setValue("version", 2);

}

int RigControlVoiceMemoryKeyer::setup(VoiceKeyerFactory *voiceKeyerFactory, int &numButtons)
{
    VoiceKeyerCapabilities voiceCap = voiceKeyerFactory->supportedVoiceKeyers()->value("rigControl");
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    TxVmRigSetupDialog txVmSetupDialog(voiceCap, numButtons, tslf->txVmButtonsFrame);
    txVmSetupDialog.setWindowTitle(tr("Rig Control Voice Memory Setup"));

    int ret = txVmSetupDialog.exec();

    if (ret == QDialog::Accepted)
    {
        numButtons = txVmSetupDialog.getNumButtons();
        QString fileName = VOICEKEYER_COMMON_PARAMS_PATH + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::RigControl] + ".ini";
        QSettings config(fileName, QSettings::IniFormat);
        config.setValue("Common/NumButtons", numButtons);
        config.setValue("Common/UseCatPttForEom", txVmSetupDialog.getCatPttForEomState() );
        config.setValue("Common/SaveButtonByRadioName", txVmSetupDialog.getSaveButtonsByRadioNameState());
        usePttForEom = txVmSetupDialog.getCatPttForEomState();

    }
    return ret;
}



int RigControlVoiceMemoryKeyer::editButton(VoiceKeyerParams *vmData, QString title)
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    TxVmRigButtonDialog vmButtonDialog(tslf->txVmButtonsFrame);

    if (readSaveButtonByRadioNameIni())
    {
        vmButtonDialog.setRadioNameLbl(vmData->getSelRadioName());
    }
    else
    {
        vmButtonDialog.setRadioNameLbl("All Radios");
    }

    vmButtonDialog.setWindowTitle(title);
    vmButtonDialog.setVmData(vmData);
    vmButtonDialog.setCwMessageTextBoxVisible(false);
    vmButtonDialog.setDialogForCatPttEom(usePttForEom);
    vmButtonDialog.setSerialMessageTextBoxVisible(false);

    int ret = vmButtonDialog.exec();
    return ret;

}


bool RigControlVoiceMemoryKeyer::readSaveButtonByRadioNameIni()
{
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::RigControl] + ".ini";
    QSettings readConfig(fileName, QSettings::IniFormat);

    return readConfig.value("Common/SaveButtonByRadioName", false).toBool();

}



