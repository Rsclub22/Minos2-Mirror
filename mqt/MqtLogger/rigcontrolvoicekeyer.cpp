#include "rigcontrolvoicekeyer.h"

RigControlVoiceKeyer::RigControlVoiceKeyer(QObject *parent) : VoiceKeyerBase(parent)
{

}


RigControlVoiceKeyer::~RigControlVoiceKeyer()
{

}


void RigControlVoiceKeyer::registerVoiceKeyer(VoiceKeyerFactory::VmKeyers* vmKeyersList)
{
    QString keyerType = "rigControl";

    VoiceKeyerCapabilities voiceMemCap;

    voiceMemCap.setVmIdNum(VoiceKeyerId::RigControl);
    voiceMemCap.setComPort("");
    voiceMemCap.setComSpeed("");
    voiceMemCap.setKeyerName("rigControl");
    voiceMemCap.setNumVoiceKeys(8);
    voiceMemCap.setSupportRepeatMsg(true);
    voiceMemCap.setSupportSerial(false);


    (*vmKeyersList)[keyerType] = voiceMemCap;

}


void RigControlVoiceKeyer::voiceKeyerInit(int numButtons)
{

}
void RigControlVoiceKeyer::sendMsgNum(int msgNum)
{
    emit sendRigVoiceKeyerMessage(QString::number(msgNum));
}
void RigControlVoiceKeyer::stopMsg()
{

}

int RigControlVoiceKeyer::getKeyerState(int &state)
{

}


bool RigControlVoiceKeyer::readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams)
{
    QString fileName = VOICE_MEMORY_PATH + VOICE_MEMORY_BASE_FILE_NAME + vmParams.getType() + ".ini";
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("button" + QString::number(buttonNum));

    vmParams.setType(config.value("type", "None").toString());
    vmParams.setVmName(config.value("name", "").toString());
    vmParams.setVmRepeatFlag(config.value("repeatFlag", false).toBool());
    vmParams.setVmRepeatDur(config.value("repeatDuration", 0).toInt());
    vmParams.setvmButtonNum(config.value("buttonNum", -1).toInt());
    config.endGroup();

    return true;
}

void RigControlVoiceKeyer::saveVmButtonParams(const VoiceKeyerParams &vmParams_ )
{
    VoiceKeyerParams vmParams = vmParams_;

    QString fileName = VOICE_MEMORY_PATH + VOICE_MEMORY_BASE_FILE_NAME + vmParams.getType() + ".ini";
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("button" + QString::number(vmParams.getvmButtonNum()));

    config.setValue("type", vmParams.getType());
    config.setValue("name", vmParams.getVmName());
    config.setValue("repeatFlag", vmParams.getVmRepeatFlag());
    config.setValue("repeatDuration", vmParams.getVmRepeatDur());
    config.setValue("buttonNum", vmParams.getvmButtonNum());
    config.endGroup();

}
