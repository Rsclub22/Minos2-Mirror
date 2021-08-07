#include "base_pch.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "SendRPCDM.h"
#include "txVmExternalButtonDialog.h"
#include "txVmExternalSetupDialog.h"

#include "ExternalMqtKeyer.h"

ExternalMqtKeyer::ExternalMqtKeyer(QObject *parent) : VoiceKeyerBase(parent)
{

}

ExternalMqtKeyer::~ExternalMqtKeyer()
{

}

void ExternalMqtKeyer::registerVoiceKeyer(VoiceKeyerFactory::VmKeyers* vmKeyersList)
{
    QString keyerName = "ExternalMqtKeyer";

    VoiceKeyerCapabilities voiceMemCap;

    voiceMemCap.setVmIdNum(VoiceKeyerId::ExternalVoiceKeyer);
    voiceMemCap.setKeyerType(keyerTypes[VoiceKeyerId::ExternalVoiceKeyer]);
    voiceMemCap.setKeyerName(keyerName);
    voiceMemCap.setNumVoiceKeys(8);
    voiceMemCap.setSupportRepeatMsg(true);
    voiceMemCap.setSetupButton(false);


    (*vmKeyersList)[keyerName] = voiceMemCap;

}

void ExternalMqtKeyer::voiceKeyerInit(int numButtons)
{
    Q_UNUSED(numButtons)
}
void ExternalMqtKeyer::sendMsgNum(int msgNum)
{
    emit LogContainer->sendKeyerPlay( msgNum );
}
void ExternalMqtKeyer::stopMsg()
{
    emit LogContainer->sendKeyerStop();
}
void ExternalMqtKeyer::doRecording(VoiceKeyerParams *vkParam)
{
    emit LogContainer->sendKeyerRecord( vkParam->getvmButtonNum() );
}

bool ExternalMqtKeyer::readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams)
{

}
void ExternalMqtKeyer::saveVmButtonParams(const VoiceKeyerParams &vmParams )
{

}

void ExternalMqtKeyer::setPttOnOff(bool onOff)
{

}

int ExternalMqtKeyer::setup(VoiceKeyerFactory *voiceKeyerFactory, VoiceKeyerCommonParams &vmCommonParams)
{
    VoiceKeyerCapabilities voiceCap = voiceKeyerFactory->supportedVoiceKeyers()->value("ExternalMqt");
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    TxVmExternalSetupDialog setup(voiceCap, tslf->txVmButtonsFrame);
    setup.setWindowTitle(tr("Internal Voice Memory Setup"));

    setup.setVmCommonParamsData(&vmCommonParams);

    return setup.exec();
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

void ExternalMqtKeyer::onDoPTT(bool onOff)
{

}
