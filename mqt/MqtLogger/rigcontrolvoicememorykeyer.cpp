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
    voiceMemCap.setsupportSerial(false);
    voiceMemCap.setUseCatPTTForEom(true);
    voiceMemCap.setEnableCwMode(false);
    voiceMemCap.setSupportRepeatMsg(true);
    voiceMemCap.setHasPip(false);
    voiceMemCap.setHasTxStatus(true);
    voiceMemCap.setSetupButton(true);
    voiceMemCap.setHasAvailStatus(true);
    voiceMemCap.setHasMessageRepeat(true);


    (*vmKeyersList)[keyerName] = voiceMemCap;

}


void RigControlVoiceMemoryKeyer::setPttOnOff(bool onOff)
{
    Q_UNUSED(onOff)
}

bool RigControlVoiceMemoryKeyer::getUsePttForEomFlag()
{
    return usePttForEom;
}


void RigControlVoiceMemoryKeyer::voiceKeyerInit(int &numButtons)
{
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::RigControl] + ".ini";
    QSettings config(fileName, QSettings::IniFormat);
    numButtons = config.value("Common/NumButtons", VOICEKEYER_MAX_NUMBUTTONS).toInt();
    usePttForEom = config.value("Common/UseCatPttForEom", true).toBool();
}

void RigControlVoiceMemoryKeyer::sendMsgNum(int buttonNum)
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    tslf->sendRigTxVoiceMessage(QString::number(buttonNum +1));  // add for Icom message Number

}

void RigControlVoiceMemoryKeyer::stopMsg(VoiceKeyerParams *vkParams)
{
    Q_UNUSED(vkParams)
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    tslf->sendRigStopTxVoiceMessage("Stop Voice Msg");
}



bool RigControlVoiceMemoryKeyer::readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams)
{

    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::RigControl] + ".ini";
    QSettings readConfig(fileName, QSettings::IniFormat);

    bool saveByRadioName = readConfig.value("Common/SaveButtonByRadioName", false).toBool();


    fileName = VOICE_KEYER_PATH + VOICE_KEYER_BASE_FILE_NAME + vmParams.getType() + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    if (saveByRadioName && !vmParams.getSelRadioName().isEmpty())
    {
        config.beginGroup(vmParams.getSelRadioName());
    }
    else
    {
        config.beginGroup("AllRadios");
    }

    //config.beginGroup("button" + QString::number(buttonNum));


    vmParams.setType(config.value("button" + QString::number(buttonNum) + "/type", "").toString());
    vmParams.setVmName(config.value("button" + QString::number(buttonNum) + "/name", "").toString());
    vmParams.setVmRepeatFlag(config.value("button" + QString::number(buttonNum) + "/repeatFlag", false).toBool());
    vmParams.setVmDuration(config.value("button" + QString::number(buttonNum) + "/messageDuration", 0).toInt());
    vmParams.setVmRepeatPauseDur(config.value("button" + QString::number(buttonNum) + "/repeatPauseDuration", 0).toInt());
    vmParams.setvmButtonNum(config.value("button" + QString::number(buttonNum) + "/buttonNum", buttonNum).toInt());
    config.endGroup();

    return true;
}

void RigControlVoiceMemoryKeyer::saveVmButtonParams(const VoiceKeyerParams &vmParams_ )
{
    VoiceKeyerParams vmParams = vmParams_;

    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::RigControl] + ".ini";
    QSettings readConfig(fileName, QSettings::IniFormat);

    bool saveByRadioName = readConfig.value("Common/SaveButtonByRadioName", false).toBool();

    fileName = VOICE_KEYER_PATH + VOICE_KEYER_BASE_FILE_NAME + vmParams.getType() + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    if (saveByRadioName  && !vmParams.getSelRadioName().isEmpty())
    {
       config.beginGroup(vmParams.getSelRadioName());
    }
    else
    {
       config.beginGroup("AllRadios");
    }


    config.setValue("button" + QString::number(vmParams.getvmButtonNum()) + "/type", vmParams.getType());
    config.setValue("button" + QString::number(vmParams.getvmButtonNum()) + "/name", vmParams.getVmName());
    config.setValue("button" + QString::number(vmParams.getvmButtonNum()) + "/repeatFlag", vmParams.getVmRepeatFlag());
    config.setValue("button" + QString::number(vmParams.getvmButtonNum()) + "/messageDuration", vmParams.getVmDuration());
    config.setValue("button" + QString::number(vmParams.getvmButtonNum()) + "/repeatPauseDuration", vmParams.getVmRepeatPauseDur());
    config.setValue("button" + QString::number(vmParams.getvmButtonNum()) + "/buttonNum", vmParams.getvmButtonNum());
    config.endGroup();

}

int RigControlVoiceMemoryKeyer::setup(VoiceKeyerFactory *voiceKeyerFactory, int &numButtons)
{
    VoiceKeyerCapabilities voiceCap = voiceKeyerFactory->supportedVoiceKeyers()->value("rigControl");
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    TxVmRigSetupDialog txVmSetupDialog(voiceCap, numButtons, tslf->txVmButtonsFrame);
    txVmSetupDialog.setWindowTitle(tr("Rig Control Voice Memory Setup"));

    int ret = txVmSetupDialog.exec();

    if (ret == QDialog::Accepted)
    {
        numButtons = txVmSetupDialog.getNumButtons();
        QString fileName = VOICEKEYER_COMMON_PARAMS_PATH + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::RigControl] + ".ini";
        QSettings config(fileName, QSettings::IniFormat);
        config.setValue("Common/NumButtons", numButtons);
        config.setValue("Common/UseCatPttForEom", txVmSetupDialog.getCatPttForEomState() );
        config.setValue("Common/SaveButtonByRadioName", txVmSetupDialog.getSaveButtonsByRadioNameState());
        usePttForEom = txVmSetupDialog.getCatPttForEomState();

    }
    return ret;
}



int RigControlVoiceMemoryKeyer::editButton(VoiceKeyerParams *vmData, QString title)
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    TxVmRigButtonDialog vmButtonDialog(tslf->txVmButtonsFrame);

    vmButtonDialog.setWindowTitle(title);
    vmButtonDialog.setVmData(vmData);
    vmButtonDialog.setCwMessageTextBoxVisible(false);
    vmButtonDialog.setDialogForCatPttEom(usePttForEom);
    vmButtonDialog.setSerialMessageTextBoxVisible(false);

    int ret = vmButtonDialog.exec();
    return ret;

}



