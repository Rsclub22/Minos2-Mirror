#include "base_pch.h"
#include "InternalVoiceMemoryKeyer.h"

InternalVoiceMemoryKeyer::InternalVoiceMemoryKeyer(QObject *parent) : VoiceKeyerBase(parent)
{

}

InternalVoiceMemoryKeyer::~InternalVoiceMemoryKeyer()
{

}
void InternalVoiceMemoryKeyer::registerVoiceKeyer(VoiceKeyerFactory::VmKeyers* vmKeyersList)
{
    QString keyerName = "Internal";

    VoiceKeyerCapabilities voiceMemCap;

    voiceMemCap.setVmIdNum(VoiceKeyerId::Internal);
    voiceMemCap.setKeyerType(keyerTypes[VoiceKeyerId::Internal]);
    voiceMemCap.setKeyerName(keyerName);
    voiceMemCap.setNumVoiceKeys(8);
    voiceMemCap.setSupportRepeatMsg(true);
    voiceMemCap.setSetupButton(false);


    (*vmKeyersList)[keyerName] = voiceMemCap;

}
void InternalVoiceMemoryKeyer::voiceKeyerInit(int numButtons)
{

}

void InternalVoiceMemoryKeyer::sendMsgNum(int msgNum)
{
    // play message - we need a PTT/NoPtt switch
}

void InternalVoiceMemoryKeyer::stopMsg()
{
    // stop playing message
}

void InternalVoiceMemoryKeyer::doRecording(VoiceKeyerParams * vkParam)
{
    // execute recording from params dialog
    // button number and filename are in vkParam
}

bool InternalVoiceMemoryKeyer::readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams)
{
    QString fileName = VOICE_KEYER_PATH + VOICE_KEYER_BASE_FILE_NAME + vmParams.getType() + ".ini";
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("button" + QString::number(buttonNum));

    vmParams.setType(config.value("type", "").toString());
    vmParams.setVmName(config.value("name", "").toString());
    vmParams.setVmRepeatFlag(config.value("repeatFlag", false).toBool());
    vmParams.setVmDuration(config.value("messageDuration", 0).toInt());
    vmParams.setVmRepeatPauseDur(config.value("repeatPauseDuration", 0).toInt());
    vmParams.setvmButtonNum(config.value("buttonNum", buttonNum).toInt());
    config.endGroup();

    return true;
}

void InternalVoiceMemoryKeyer::saveVmButtonParams(const VoiceKeyerParams &vmParams_)
{
    VoiceKeyerParams vmParams = vmParams_;

    QString fileName = VOICE_KEYER_PATH + VOICE_KEYER_BASE_FILE_NAME + vmParams.getType() + ".ini";
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("button" + QString::number(vmParams.getvmButtonNum()));

    config.setValue("type", vmParams.getType());
    config.setValue("name", vmParams.getVmName());
    config.setValue("repeatFlag", vmParams.getVmRepeatFlag());
    config.setValue("messageDuration", vmParams.getVmDuration());
    config.setValue("repeatPauseDuration", vmParams.getVmRepeatPauseDur());
    config.setValue("buttonNum", vmParams.getvmButtonNum());
    config.endGroup();
}

void InternalVoiceMemoryKeyer::setPttOnOff(bool onOff)
{

}
