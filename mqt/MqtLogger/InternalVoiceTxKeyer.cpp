#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "SendRPCDM.h"
#include "txvminternalbuttondialog.h"
#include "txVmInternalSetupDialog.h"
#include "txkeyerCommonConstants.h"

#include "sbdriver.h"
#include "keyerlog.h"
#include "MTrace.h"

#include "InternalVoiceTxKeyer.h"

using namespace TxKeyerCommon;

InternalVoiceTxKeyer::InternalVoiceTxKeyer(QObject *parent) : TxKeyerBase(parent)
{

}

InternalVoiceTxKeyer::~InternalVoiceTxKeyer()
{
    SoundSystemDriver::sbdvp_unload();
}
void InternalVoiceTxKeyer::registerTxKeyer(TxKeyerFactory::TxKeyers* vmKeyersList)
{
    QString keyerName = "Internal";

    TxKeyerCapabilities voiceMemCap;

    voiceMemCap.setVmIdNum(TxKeyerId::InternalVoiceKeyer);
    voiceMemCap.setKeyerType(keyerTypes[TxKeyerId::InternalVoiceKeyer]);
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
void InternalVoiceTxKeyer::txKeyerInit(int &numButtons)
{
    sblog = true;

    QString fileName = TX_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + "Internal" + ".ini";
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

       QString inifileName = TX_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[TxKeyerId::InternalVoiceKeyer] + ".ini";
        QSettings config(inifileName, QSettings::IniFormat);
        config.beginGroup("button" + QString::number(i));

        config.setValue("messageDuration", msgLen);

        config.endGroup();
    }
    connect(SoundSystemDriver::getSbDriver(), &SoundSystemDriver::ptt, this, &InternalVoiceTxKeyer::onDoPTT);
}

void InternalVoiceTxKeyer::sendMsgNum(TxKeyerParams &vkParam)
{
    // play message - we need a PTT/NoPtt switch
    QString fileName;
 /*
    fileName = QString("CQF%1.WAV").arg(msgNum + 1);
*/
    if ( !SoundSystemDriver::getSbDriver() ->play_file( fileName, true/*xmit*/, 0/*clipRecord*/ ))
    {

    }

}

void InternalVoiceTxKeyer::stopMsg(TxKeyerParams &vkParam)
{
    // stop recording/playing message
    SoundSystemDriver::getSbDriver() ->stoprec();
    SoundSystemDriver::getSbDriver() ->stopDMA();

    int msgLen = SoundSystemDriver::getSbDriver() ->getMessageLen(vkParam.getKeyerButtonNum());
    vkParam.setKeyerDuration(msgLen);
    QString inifileName = TX_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + vkParam.getType() + ".ini";
    QSettings config(inifileName, QSettings::IniFormat);
    config.beginGroup("button" + QString::number(vkParam.getKeyerButtonNum()));

    config.setValue("messageDuration", vkParam.getKeyerDuration());

    config.endGroup();

}


void InternalVoiceTxKeyer::doRecording(TxKeyerParams * vkParam)
{
    // execute recording from params dialog
    // button number and filename are in vkParam

    // Can we work it to use space bar as PTT for recording?
    QString fileName = QString("CQF%1.WAV").arg(vkParam->getKeyerButtonNum() + 1);
    SoundSystemDriver::getSbDriver() ->record_file( fileName );

}


/*
bool InternalVoiceTxKeyer::readVmButtonParams(int buttonNum, TxKeyerParams &vmParams)
{
    QString fileName = TX_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + vmParams.getType() + ".ini";
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("button" + QString::number(buttonNum));

    vmParams.setType(config.value("type", "").toString());
    vmParams.setKeyerName(config.value("name", "").toString());
    vmParams.setKeyerRepeatFlag(config.value("repeatFlag", false).toBool());
    vmParams.setKeyerDuration(config.value("messageDuration", 0).toInt());
    vmParams.setKeyerRepeatPauseDur(config.value("repeatPauseDuration", 0).toInt());
    vmParams.setKeyerButtonNum(config.value("buttonNum", buttonNum).toInt());
    config.endGroup();

    return true;
}

void InternalVoiceTxKeyer::saveVmButtonParams(const TxKeyerParams &vmParams_)
{
    TxKeyerParams vmParams = vmParams_;

    QString fileName = TX_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + vmParams.getType() + ".ini";
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("button" + QString::number(vmParams.getKeyerButtonNum()));

    config.setValue("type", vmParams.getType());
    config.setValue("name", vmParams.getKeyerName());
    config.setValue("repeatFlag", vmParams.getKeyerRepeatFlag());
    config.setValue("messageDuration", vmParams.getKeyerDuration());
    config.setValue("repeatPauseDuration", vmParams.getKeyerRepeatPauseDur());
    config.setValue("buttonNum", vmParams.getKeyerButtonNum());
    config.endGroup();
}
*/

void InternalVoiceTxKeyer::setPttOnOff(bool onOff)
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    if (tslf)
    {
        trace(QString("InternalVoiceMemory setPttOnOff = %1").arg((onOff ? "On" : "Off")));
        LogContainer->sendDM->sendRigControlPttOnOff(tslf, onOff);
    }
}
void InternalVoiceTxKeyer::onDoPTT(bool onOff)
{

    setPttOnOff(onOff);
    emit internalVoiceMemoryKeyerPlayState(onOff);


}



int InternalVoiceTxKeyer::setup(TxKeyerFactory *txKeyerFactory, int &maxNumButtons, int &numButtons, QString selectedRadio)
{
    Q_UNUSED(selectedRadio)
    Q_UNUSED(maxNumButtons)

    TxKeyerCapabilities voiceCap = txKeyerFactory->supportedTxKeyers()->value("internal");
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    txVmInternalSetupDialog txvmSetup(voiceCap, maxNumButtons, numButtons, tslf->dmButtonFrame);
    txvmSetup.setWindowTitle(tr("Internal Voice Memory Setup"));

    int ret = txvmSetup.exec();

    if (ret == QDialog::Accepted)
    {
        numButtons = txvmSetup.getNumButtons();
        QString fileName = TX_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[TxKeyerId::RigControl] + ".ini";
        QSettings config(fileName, QSettings::IniFormat);
        config.setValue("Common/NumButtons", numButtons);
    }
    return ret;
}
/*
int InternalVoiceTxKeyer::editButton(TxKeyerParams *vmData, QString title)
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    TxVmInternalButtonDialog vmButtonDialog(tslf->dmButtonFrame);

    vmButtonDialog.setWindowTitle(title);
    vmButtonDialog.setVmData(vmData);
    int ret = vmButtonDialog.exec();
    return ret;

}
*/



