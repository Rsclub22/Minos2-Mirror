#include "MShowMessageDlg.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "SendRPCDM.h"
#include "txVmExternalButtonDialog.h"
#include "KeyerJson.h"
#include "MTrace.h"

#include "ExternalMqtKeyer.h"

using namespace TxKeyerCommon;

/*static*/ QString ExternalMqtKeyer::keyerName = txKeyerNames[TxKeyerId::ExternalMqtKeyer];


ExternalMqtKeyer::ExternalMqtKeyer(QObject *parent) : TxKeyerBase(parent)
{
}

ExternalMqtKeyer::~ExternalMqtKeyer()
{
}

void ExternalMqtKeyer::registerTxKeyer(TxKeyerFactory::TxKeyers* vmKeyersList)
{
    QString keyerName = getTxKeyerDisplayName(TxKeyerId::ExternalMqtKeyer);

    TxKeyerCapabilities voiceMemCap;

    voiceMemCap.setTxKeyerId(TxKeyerId::ExternalMqtKeyer);
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

void ExternalMqtKeyer::txKeyerInit(int &numButtons)
{


    trace("ExternalMqtKeyer::voiceKyerInit");

    numButtons = VOICEKEYER_MAX_NUMBUTTONS;
    connect(LogContainer->sendDM, &TSendDM::keyerConfig, this, &ExternalMqtKeyer::onKeyerConfig, Qt::UniqueConnection);
    connect(LogContainer->sendDM, &TSendDM::keyerReport, this, &ExternalMqtKeyer::onKeyerReport, Qt::UniqueConnection);
    LogContainer->sendDM->publishKeyerConfig(rpcConstants::keyerConfig);

}
void ExternalMqtKeyer::sendMsgNum(TxKeyerParams &vkParam)
{
    trace("ExternalMqtKeyer::sendMsgNum");
    //*******************************************************emit LogContainer->sendKeyerPlay( msgNum );
}
void ExternalMqtKeyer::stopMsg(TxKeyerParams &vkParam)
{
    Q_UNUSED(vkParam)
    trace("ExternalMqtKeyer::stopMsg");
    emit LogContainer->sendKeyerStop();
}
void ExternalMqtKeyer::doRecording(TxKeyerParams *vkParam)
{
    emit LogContainer->sendKeyerRecord( vkParam->getKeyerButtonNum() );
}



bool ExternalMqtKeyer::readTxKeyerButtonParams(int buttonNum, TxKeyerParams &TxKeyerParams)
{
/*    // This info should have come from the external keyer
    vmParams.setKeyerRepeatFlag(remoteConfig.kjj[buttonNum].autoRepeat);
    vmParams.setKeyerRepeatPauseDur(remoteConfig.kjj[buttonNum].autoRepeatDelay);
    vmParams.setKeyerName(remoteConfig.kjj[buttonNum].CQName);
    vmParams.setKeyerDuration(remoteConfig.kjj[buttonNum].CQLength);
    vmParams.setKeyerButtonNum(buttonNum);
*/
    return true;
}
void ExternalMqtKeyer::saveTxKeyerButtonParams(const TxKeyerParams &txKeyerParams_ )
{
    // We should send this config to the external keyer
/*
    trace("ExternalMqtKeyer::saveVmButtonParams");
    int buttonNum = vmParams_.getKeyerButtonNum();
    KeyerKeyJson &kkj = remoteConfig.kjj[buttonNum];

    kkj.keyno = buttonNum;
    kkj.CQLength = vmParams_.getKeyerDuration();
    kkj.CQName = vmParams_.getKeyerName();
    kkj.autoRepeat = vmParams_.getKeyerRepeatFlag();
    kkj.autoRepeatDelay = vmParams_.getKeyerRepeatPauseDur();

    QString config = remoteConfig.makeConfig(QJsonDocument::Compact, false, false);
    LogContainer->sendDM->publishKeyerConfig(config);
*/
}

void ExternalMqtKeyer::setPttOnOff(bool onOff)
{
    Q_UNUSED(onOff)
}



int ExternalMqtKeyer::setup(TxKeyerFactory *txKeyerFactory, int &maxNumButtons, int &numButtons, QString selectedRadioName)
{
    // setup button shouldn't be visible, so shouldn't be called


    Q_UNUSED(txKeyerFactory)
    Q_UNUSED(maxNumButtons)
    Q_UNUSED(numButtons)
    Q_UNUSED(selectedRadioName)

    // This should never happen, so no tranlation required
    mShowMessage("No setup available for external keyer ", nullptr);

    return QDialog::Rejected;
}
/*
int ExternalMqtKeyer::editButton(TxKeyerParams* vmData, QString title)
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    TxVmExternalButtonDialog vmButtonDialog(tslf->dmButtonFrame);

    vmButtonDialog.setWindowTitle(title);
    vmButtonDialog.setVmData(vmData);
    int ret = vmButtonDialog.exec();
    return ret;

}
*/
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
