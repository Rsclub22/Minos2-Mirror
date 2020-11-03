#include "rigcontrolvoicekeyer.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"

RigControlVoiceKeyer::RigControlVoiceKeyer(QObject *parent) : VoiceKeyerBase(parent)
{

}


RigControlVoiceKeyer::~RigControlVoiceKeyer()
{

}


void RigControlVoiceKeyer::registerVoiceKeyer(VoiceKeyerFactory::VmKeyers* vmKeyersList)
{
    QString keyerName = "rigControl";

    VoiceKeyerCapabilities voiceMemCap;

    voiceMemCap.setVmIdNum(VoiceKeyerId::RigControl);
    voiceMemCap.setKeyerType(keyerTypes[VoiceKeyerId::RigControl]);
    voiceMemCap.setKeyerName(keyerName);
    voiceMemCap.setNumVoiceKeys(8);
    voiceMemCap.setSupportRepeatMsg(true);
    voiceMemCap.setSetupButton(false);


    (*vmKeyersList)[keyerName] = voiceMemCap;

}


void RigControlVoiceKeyer::voiceKeyerInit(int numButtons)
{
    Q_UNUSED(numButtons)
}
void RigControlVoiceKeyer::sendMsgNum(int msgNum)
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    tslf->sendRigTxVoiceMessage(QString::number(msgNum));
}
void RigControlVoiceKeyer::stopMsg()
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    tslf->sendRigTxVoiceMessage(STOPCODE);
}

int RigControlVoiceKeyer::getKeyerState(int &state)
{
    Q_UNUSED(state)

    return 0;
}


bool RigControlVoiceKeyer::readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams)
{
    QString fileName = VOICE_KEYER_PATH + VOICE_KEYER_BASE_FILE_NAME + vmParams.getType() + ".ini";
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

    QString fileName = VOICE_KEYER_PATH + VOICE_KEYER_BASE_FILE_NAME + vmParams.getType() + ".ini";
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("button" + QString::number(vmParams.getvmButtonNum()));

    config.setValue("type", vmParams.getType());
    config.setValue("name", vmParams.getVmName());
    config.setValue("repeatFlag", vmParams.getVmRepeatFlag());
    config.setValue("repeatDuration", vmParams.getVmRepeatDur());
    config.setValue("buttonNum", vmParams.getvmButtonNum());
    config.endGroup();

}
