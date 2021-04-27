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
    voiceMemCap.setSupportRepeatMsg(true);
    voiceMemCap.setSetupButton(false);


    (*vmKeyersList)[keyerName] = voiceMemCap;

}


void RigControlVoiceMemoryKeyer::setPttOnOff(bool onOff)
{
    Q_UNUSED(onOff)
}


void RigControlVoiceMemoryKeyer::voiceKeyerInit(int numButtons)
{
    Q_UNUSED(numButtons)
}
void RigControlVoiceMemoryKeyer::sendMsgNum(int buttonNum)
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    tslf->sendRigTxVoiceMessage(QString::number(buttonNum +1));  // add for Icom message Number
}
void RigControlVoiceMemoryKeyer::stopMsg()
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    tslf->sendRigTxVoiceMessage(STOPCODE);
}




bool RigControlVoiceMemoryKeyer::readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams)
{
    QString fileName = VOICE_KEYER_PATH + VOICE_KEYER_BASE_FILE_NAME + vmParams.getType() + ".ini";
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("button" + QString::number(buttonNum));

    vmParams.setType(config.value("type", "").toString());
    vmParams.setVmName(config.value("name", "").toString());
    vmParams.setVmRepeatFlag(config.value("repeatFlag", false).toBool());
    vmParams.setVmDuration(config.value("messageDuration", 0).toInt());
    vmParams.setVmRepeatPauseDur(config.value("repeatPauseDuration", 0).toInt());
    vmParams.setvmButtonNum(config.value("buttonNum", -1).toInt());
    config.endGroup();

    return true;
}

void RigControlVoiceMemoryKeyer::saveVmButtonParams(const VoiceKeyerParams &vmParams_ )
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

int RigControlVoiceMemoryKeyer::setup(VoiceKeyerFactory *voiceKeyerFactory, VoiceKeyerCommonParams &vmCommonParams)
{
    VoiceKeyerCapabilities voiceCap = voiceKeyerFactory->supportedVoiceKeyers()->value("rigControl");
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    TxVmRigSetupDialog txVmSetupDialog(voiceCap, tslf->txVmButtonsFrame);
    txVmSetupDialog.setWindowTitle(tr("Rig Control Voice Memory Setup"));

    txVmSetupDialog.setVmCommonParamsData(&vmCommonParams);

    return txVmSetupDialog.exec();

}

int RigControlVoiceMemoryKeyer::editButton(VoiceKeyerParams *vmData, QString title)
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    TxVmRigButtonDialog vmButtonDialog(tslf->txVmButtonsFrame);

    vmButtonDialog.setWindowTitle(title);
    vmButtonDialog.setVmData(vmData);
    int ret = vmButtonDialog.exec();
    return ret;

}



