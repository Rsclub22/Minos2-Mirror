#include "base_pch.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"

#include "txvminternalbuttondialog.h"
#include "txvmsetupdialog.h"

#include "sbdriver.h"
#include "keyerlog.h"


#include "InternalVoiceMemoryKeyer.h"

InternalVoiceMemoryKeyer::InternalVoiceMemoryKeyer(QObject *parent) : VoiceKeyerBase(parent)
{

}

InternalVoiceMemoryKeyer::~InternalVoiceMemoryKeyer()
{
    SoundSystemDriver::sbdvp_unload();
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
    Q_UNUSED(numButtons)
    sblog = true;

    QString errmess;
    if ( !SoundSystemDriver::getSbDriver() ->sbdvp_init( errmess, 48000, 0, 0, 0 ,0 ) )
    {
       trace( "sbdvp_init failed! " + errmess );
    }
    SoundSystemDriver::getSbDriver()->setVolumeMults(0, 0, 0);  // for now, set everything to 0db

    //connect( SoundSystemDriver::getSbDriver(), &SoundSystemDriver::recpbFinished, this, )

    for (int i = 0; i < numButtons; i++)
    {
        int msgLen = SoundSystemDriver::getSbDriver() ->getMessageLen(i);

        QString inifileName = VOICE_KEYER_PATH + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::Internal] + ".ini";
        QSettings config(inifileName, QSettings::IniFormat);
        config.beginGroup("button" + QString::number(i));

        config.setValue("messageDuration", msgLen);

        config.endGroup();
    }
}

void InternalVoiceMemoryKeyer::sendMsgNum(int msgNum)
{
    // play message - we need a PTT/NoPtt switch
    QString fileName;
    fileName = QString("CQF%1.WAV").arg(msgNum);

    if ( !SoundSystemDriver::getSbDriver() ->play_file( fileName, true/*xmit*/, 0/*clipRecord*/ ))
    {

    }

}

void InternalVoiceMemoryKeyer::stopMsg()
{
    // stop recording/playing message
    SoundSystemDriver::getSbDriver() ->stopDMA();

}


void InternalVoiceMemoryKeyer::doRecording(VoiceKeyerParams * vkParam)
{
    // execute recording from params dialog
    // button number and filename are in vkParam

    // Can we work it to use space bar as PTT for recording?
    QString fileName = QString("CQF%1.WAV").arg(vkParam->getvmButtonNum());
    SoundSystemDriver::getSbDriver() ->record_file( fileName );

    int msgLen = SoundSystemDriver::getSbDriver() ->getMessageLen(vkParam->getvmButtonNum());
    vkParam->setVmDuration(msgLen);

    QString inifileName = VOICE_KEYER_PATH + VOICE_KEYER_BASE_FILE_NAME + vkParam->getType() + ".ini";
    QSettings config(inifileName, QSettings::IniFormat);
    config.beginGroup("button" + QString::number(vkParam->getvmButtonNum()));

    config.setValue("messageDuration", vkParam->getVmDuration());

    config.endGroup();
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
    Q_UNUSED(onOff)
}

int InternalVoiceMemoryKeyer::setup(VoiceKeyerFactory *voiceKeyerFactory, VoiceKeyerCommonParams &vmCommonParams)
{
    VoiceKeyerCapabilities voiceCap = voiceKeyerFactory->supportedVoiceKeyers()->value("internal");
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    TxVmSetupDialog txVmSetupDialog(voiceCap, tslf->txVmButtonsFrame);
    txVmSetupDialog.setWindowTitle(tr("Internal Voice Memory Setup"));

    txVmSetupDialog.setVmCommonParamsData(&vmCommonParams);

    return txVmSetupDialog.exec();
}

int InternalVoiceMemoryKeyer::editButton(VoiceKeyerParams *vmData, QString title)
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    TxVmInternalButtonDialog vmButtonDialog(tslf->txVmButtonsFrame);
    int buttonNumber = vmData->getvmButtonNum();

    vmButtonDialog.setWindowTitle(tr("Voice Memory %1 - Edit").arg(buttonNumber + 1));
    vmButtonDialog.setVmData(vmData);
    int ret = vmButtonDialog.exec();
    return ret;

}
