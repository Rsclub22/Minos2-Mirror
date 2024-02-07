#include "MShowMessageDlg.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "SendRPCDM.h"
#include "txVmExternalButtonDialog.h"
#include "KeyerJson.h"
#include "MTrace.h"

#include "ExternalMqtKeyer.h"

/*static*/ QString ExternalMqtKeyer::keyerName = "ExternalMqtKeyer";


ExternalMqtKeyer::ExternalMqtKeyer(QObject *parent) : VoiceKeyerBase(parent)
{
}

ExternalMqtKeyer::~ExternalMqtKeyer()
{
}

void ExternalMqtKeyer::registerVoiceKeyer(VoiceKeyerFactory::VmKeyers* vmKeyersList)
{
   // QString keyerName = "ExternalMqtKeyer";

    VoiceKeyerCapabilities voiceMemCap;

    voiceMemCap.setVmIdNum(VoiceKeyerId::ExternalVoiceKeyer);
    voiceMemCap.setKeyerType(keyerTypes[VoiceKeyerId::ExternalVoiceKeyer]);
    voiceMemCap.setKeyerName(keyerName);
    voiceMemCap.setNumVoiceKeys(KEYERKEYS);
    voiceMemCap.setsupportSerial(false);
    voiceMemCap.setUseCatPTTForEom(false);
    voiceMemCap.setEnableCwMode(false);
    voiceMemCap.setSupportRepeatMsg(true);
    voiceMemCap.setHasPip(true);
    voiceMemCap.setHasTxStatus(false);
    voiceMemCap.setSetupButton(false);
    voiceMemCap.setHasAvailStatus(false);
    voiceMemCap.setHasMessageRepeat(false);


    (*vmKeyersList)[keyerName] = voiceMemCap;

}

void ExternalMqtKeyer::voiceKeyerInit(int &numButtons)
{
    trace("ExternalMqtKeyer::voiceKyerInit");

    numButtons = VOICEKEYER_MAX_NUMBUTTONS;
    connect(LogContainer->sendDM, &TSendDM::keyerConfig, this, &ExternalMqtKeyer::onKeyerConfig, Qt::UniqueConnection);
    connect(LogContainer->sendDM, &TSendDM::keyerReport, this, &ExternalMqtKeyer::onKeyerReport, Qt::UniqueConnection);
    LogContainer->sendDM->publishKeyerConfig(rpcConstants::keyerConfig);

}
void ExternalMqtKeyer::sendMsgNum(int msgNum)
{
    trace("ExternalMqtKeyer::sendMsgNum");
    emit LogContainer->sendKeyerPlay( msgNum );
}
void ExternalMqtKeyer::stopMsg(VoiceKeyerParams */*vkParam*/)
{
    trace("ExternalMqtKeyer::stopMsg");
    emit LogContainer->sendKeyerStop();
}
void ExternalMqtKeyer::doRecording(VoiceKeyerParams *vkParam)
{
    emit LogContainer->sendKeyerRecord( vkParam->getvmButtonNum() );
}

bool ExternalMqtKeyer::readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams)
{
    // This info should have come from the external keyer
    vmParams.setVmRepeatFlag(remoteConfig.kjj[buttonNum].autoRepeat);
    vmParams.setVmRepeatPauseDur(remoteConfig.kjj[buttonNum].autoRepeatDelay);
    vmParams.setVmName(remoteConfig.kjj[buttonNum].CQName);
    vmParams.setVmDuration(remoteConfig.kjj[buttonNum].CQLength);
    vmParams.setvmButtonNum(buttonNum);

    return true;
}
void ExternalMqtKeyer::saveVmButtonParams(const VoiceKeyerParams &vmParams_ )
{
    // We should send this config to the external keyer

    trace("ExternalMqtKeyer::saveVmButtonParams");
    int buttonNum = vmParams_.getvmButtonNum();
    KeyerKeyJson &kkj = remoteConfig.kjj[buttonNum];

    kkj.keyno = buttonNum;
    kkj.CQLength = vmParams_.getVmDuration();
    kkj.CQName = vmParams_.getVmName();
    kkj.autoRepeat = vmParams_.getVmRepeatFlag();
    kkj.autoRepeatDelay = vmParams_.getVmRepeatPauseDur();

    QString config = remoteConfig.makeConfig(QJsonDocument::Compact, false, false);
    LogContainer->sendDM->publishKeyerConfig(config);

}

void ExternalMqtKeyer::setPttOnOff(bool onOff)
{
    Q_UNUSED(onOff)
}

bool ExternalMqtKeyer::getUsePttForEomFlag()
{
    return usePttForEom;
}

int ExternalMqtKeyer::setup(VoiceKeyerFactory *voiceKeyerFactory, int &maxNumButtons, int &numButtons, QString selectedRadioName)
{
    // setup button shouldn't be visible, so shouldn't be called


    Q_UNUSED(voiceKeyerFactory)
    Q_UNUSED(maxNumButtons)
    Q_UNUSED(numButtons)
    Q_UNUSED(selectedRadioName)

    // This should never happen, so no tranlation required
    mShowMessage("No setup available for external keyer ", nullptr);

    return QDialog::Rejected;
}
int ExternalMqtKeyer::editButton(VoiceKeyerParams* vmData, QString title)
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    TxVmExternalButtonDialog vmButtonDialog(tslf->txVmButtonsFrame);

    vmButtonDialog.setWindowTitle(title);
    vmButtonDialog.setVmData(vmData);
    int ret = vmButtonDialog.exec();
    return ret;

}

void ExternalMqtKeyer::setPip(bool p)
{
    trace("ExternalMqtKeyer::setPip");
    remoteConfig.pipEnable = p;

    QString config = remoteConfig.makeConfig(QJsonDocument::Compact, false, false);
    LogContainer->sendDM->publishKeyerConfig(config);

}

void ExternalMqtKeyer::onKeyerConfig(QString key, QString val)
{
    if (key == rpcConstants::keyerConfig)
    {
        remoteConfig.parseConfig(val, false);
        // and now use it!

        emit remoteConfigChanged();

    }

}
void ExternalMqtKeyer::onKeyerReport(QString val)
{
    int bracket = val.lastIndexOf('[');
    int k = val.indexOf('K', bracket);
    if (k > 0)
    {
        int key = val[k + 1].toLatin1() - '0';
        emit remoteKeyerStarted(key - 1);

    }
    else
    {
        emit remoteKeyerStopped();
    }
}
