#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "SendRPCDM.h"
#include "txvminternalbuttondialog.h"
#include "txVmInternalSetupDialog.h"
#include "voicekeyerCommonConstants.h"

#include "sbdriver.h"
#include "keyerlog.h"
#include "MTrace.h"

#include "InternalVoiceMemoryKeyer.h"

using namespace voiceKeyerCommon;

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

    voiceMemCap.setVmIdNum(VoiceKeyerId::InternalVoiceKeyer);
    voiceMemCap.setKeyerType(keyerTypes[VoiceKeyerId::InternalVoiceKeyer]);
    voiceMemCap.setKeyerName(keyerName);
    voiceMemCap.setsupportSerial(false);
    voiceMemCap.setUseCatPTTForEom(false);
    voiceMemCap.setEnableCwMode(false);
    voiceMemCap.setNumVoiceKeys(8);
    voiceMemCap.setSupportRepeatMsg(true);
    voiceMemCap.setHasPip(false);
    voiceMemCap.setHasTxStatus(true);
    voiceMemCap.setSetupButton(true);
    voiceMemCap.setHasAvailStatus(false);
    voiceMemCap.setHasMessageRepeat(true);



    (*vmKeyersList)[keyerName] = voiceMemCap;

}
void InternalVoiceMemoryKeyer::voiceKeyerInit(int &numButtons)
{
    sblog = true;

    QString fileName = VOICE_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + "Internal" + ".ini";
    QSettings settings(fileName, QSettings::IniFormat);

    QString indev = settings.value(indevKey, "").toString();
    QString outdev = settings.value(outdevKey, "").toString();
    numButtons = settings.value("Common/NumButtons", VOICEKEYER_MAX_NUMBUTTONS).toInt();

    QString errmess;
    if ( !SoundSystemDriver::getSbDriver() ->sbdvp_init( indev, outdev, "", "", errmess, 48000, 0, 0, 0 ) )
    {
       trace( "sbdvp_init failed! " + errmess );
    }
    SoundSystemDriver::getSbDriver()->setVolumeMults(0, 0, 0, CompressorParams(), false, false);  // for now, set everything to 0db

    for (int i = 0; i < numButtons; i++)
    {
        int msgLen = SoundSystemDriver::getSbDriver() ->getMessageLen(i);

       QString inifileName = VOICE_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::InternalVoiceKeyer] + ".ini";
        QSettings config(inifileName, QSettings::IniFormat);
        config.beginGroup("button" + QString::number(i));

        config.setValue("messageDuration", msgLen);

        config.endGroup();
    }
    connect(SoundSystemDriver::getSbDriver(), &SoundSystemDriver::ptt, this, &InternalVoiceMemoryKeyer::onDoPTT);
}

void InternalVoiceMemoryKeyer::sendMsgNum(int msgNum)
{
    // play message - we need a PTT/NoPtt switch
    QString fileName;
    fileName = QString("CQF%1.WAV").arg(msgNum + 1);

    if ( !SoundSystemDriver::getSbDriver() ->play_file( fileName, true/*xmit*/, 0/*clipRecord*/ ))
    {

    }

}

void InternalVoiceMemoryKeyer::stopMsg(VoiceKeyerParams * vkParam)
{
    // stop recording/playing message
    SoundSystemDriver::getSbDriver() ->stoprec();
    SoundSystemDriver::getSbDriver() ->stopDMA();

    if (vkParam)
    {
        int msgLen = SoundSystemDriver::getSbDriver() ->getMessageLen(vkParam->getvmButtonNum());
        vkParam->setVmDuration(msgLen);
        QString inifileName = VOICE_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + vkParam->getType() + ".ini";
        QSettings config(inifileName, QSettings::IniFormat);
        config.beginGroup("button" + QString::number(vkParam->getvmButtonNum()));

        config.setValue("messageDuration", vkParam->getVmDuration());

        config.endGroup();
    }
}


void InternalVoiceMemoryKeyer::doRecording(VoiceKeyerParams * vkParam)
{
    // execute recording from params dialog
    // button number and filename are in vkParam

    // Can we work it to use space bar as PTT for recording?
    QString fileName = QString("CQF%1.WAV").arg(vkParam->getvmButtonNum() + 1);
    SoundSystemDriver::getSbDriver() ->record_file( fileName );

}



bool InternalVoiceMemoryKeyer::readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams)
{
    QString fileName = VOICE_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + vmParams.getType() + ".ini";
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

    QString fileName = VOICE_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + vmParams.getType() + ".ini";
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
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    if (tslf)
    {
        trace(QString("InternalVoiceMemory setPttOnOff = %1").arg((onOff ? "On" : "Off")));
        LogContainer->sendDM->sendRigControlPttOnOff(tslf, onOff);
    }
}
void InternalVoiceMemoryKeyer::onDoPTT(bool onOff)
{

    setPttOnOff(onOff);
    emit internalVoiceMemoryKeyerPlayState(onOff);


}



int InternalVoiceMemoryKeyer::setup(VoiceKeyerFactory *voiceKeyerFactory, int &maxNumButtons, int &numButtons, QString selectedRadio)
{
    Q_UNUSED(selectedRadio)
    Q_UNUSED(maxNumButtons)

    VoiceKeyerCapabilities voiceCap = voiceKeyerFactory->supportedVoiceKeyers()->value("internal");
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    txVmInternalSetupDialog txvmSetup(voiceCap, maxNumButtons, numButtons, tslf->txVmButtonsFrame);
    txvmSetup.setWindowTitle(tr("Internal Voice Memory Setup"));

    int ret = txvmSetup.exec();

    if (ret == QDialog::Accepted)
    {
        numButtons = txvmSetup.getNumButtons();
        QString fileName = VOICE_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::RigControl] + ".ini";
        QSettings config(fileName, QSettings::IniFormat);
        config.setValue("Common/NumButtons", numButtons);
    }
    return ret;
}

int InternalVoiceMemoryKeyer::editButton(VoiceKeyerParams *vmData, QString title)
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    TxVmInternalButtonDialog vmButtonDialog(tslf->txVmButtonsFrame);

    vmButtonDialog.setWindowTitle(title);
    vmButtonDialog.setVmData(vmData);
    int ret = vmButtonDialog.exec();
    return ret;

}




