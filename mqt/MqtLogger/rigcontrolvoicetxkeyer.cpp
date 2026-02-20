#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "txKeyerFactory.h"
#include "txkeyerCommonConstants.h"
#include "txvmrigsetupdialog.h"
#include "txvmrigbuttondialog.h"
#include "rigcontrolvoicetxkeyer.h"

using namespace TxKeyerCommon;

RigControlVoiceTxKeyer::RigControlVoiceTxKeyer(QObject *parent) : TxKeyerBase(parent)
{

}


RigControlVoiceTxKeyer::~RigControlVoiceTxKeyer()
{

}


void RigControlVoiceTxKeyer::registerTxKeyer(TxKeyerFactory::TxKeyers* vmKeyersList)
{
    QString keyerName = getTxKeyerDisplayName(TxKeyerId::RigControl);

    TxKeyerCapabilities voiceMemCap;

    voiceMemCap.setTxKeyerId(TxKeyerId::RigControl);
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


void RigControlVoiceTxKeyer::setPttOnOff(bool onOff)
{
    Q_UNUSED(onOff)
}



TxKeyerCommon::KeyerEomTypes RigControlVoiceTxKeyer::getSelectedEomType()
{
    return selectedEomType;
}


void RigControlVoiceTxKeyer::txKeyerInit(int &numButtons)
{
    int userNumberButtons = 0;
    getRadioCommonData(selectedEomType, userNumberButtons, radioMaxNumButtons);
    numButtons = userNumberButtons;

}

void RigControlVoiceTxKeyer::sendMsgNum(TxKeyerParams &vkParams)
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    tslf->sendRigTxVoiceMessage(QString::number(vkParams.getRigVoiceMemNum()));

}

void RigControlVoiceTxKeyer::stopMsg(TxKeyerParams &vkParams)
{
    Q_UNUSED(vkParams)
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    tslf->sendRigStopTxVoiceMessage(STOP_VOICE_MESSAGE);
}



bool RigControlVoiceTxKeyer::readTxKeyerButtonParams(int buttonNum, TxKeyerParams &txKeyerParams)
{
/*
    bool saveByRadioName = readSaveVoiceCWMemoryButtonByRadioNameFromIni(TxKeyerId::RigControl);


    QString fileName = TX_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + vmParams.getType() + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    if (saveByRadioName && !vmParams.getSelRadioName().isEmpty())
    {
        config.beginGroup(vmParams.getSelRadioName().replace('/', '_'));
    }
    else
    {
        config.beginGroup(ALL_RADIOS_GROUP_NAME);
    }

    QString newKey = "button" + QString::number(buttonNum);


    vmParams.setType(config.value(newKey + "/type", "").toString());
    vmParams.setKeyerName(config.value(newKey + "/name", "").toString());
    vmParams.setKeyerRepeatFlag(config.value(newKey + "/repeatFlag", false).toBool());
    vmParams.setKeyerDuration(config.value(newKey + "/messageDuration", 0).toInt());
    vmParams.setKeyerRepeatPauseDur(config.value(newKey + "/repeatPauseDuration", 0).toInt());
    vmParams.setKeyerButtonNum(config.value(newKey + "/buttonNum", buttonNum).toInt());
    config.endGroup();
*/
    return true;
}


void RigControlVoiceTxKeyer::saveTxKeyerButtonParams(const TxKeyerParams &txKeyerParams_ )
{
    TxKeyerParams txKeyerParams = txKeyerParams_;
/*
    bool saveByRadioName = readSaveVoiceCWMemoryButtonByRadioNameFromIni(TxKeyerId::RigControl);

    QString fileName = TX_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + vmParams.getType() + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    if (saveByRadioName  && !vmParams.getSelRadioName().isEmpty())
    {
       config.beginGroup(vmParams.getSelRadioName().replace('/', '_'));
    }
    else
    {
       config.beginGroup(ALL_RADIOS_GROUP_NAME);
    }

    QString newKey = "button" + QString::number(vmParams.getKeyerButtonNum());
    config.setValue(newKey + "/type", vmParams.getType());
    config.setValue(newKey + "/name", vmParams.getKeyerName());
    config.setValue(newKey + "/repeatFlag", vmParams.getKeyerRepeatFlag());
    config.setValue(newKey + "/messageDuration", vmParams.getKeyerDuration());
    config.setValue(newKey + "/repeatPauseDuration", vmParams.getKeyerRepeatPauseDur());
    config.setValue(newKey + "/buttonNum", vmParams.getKeyerButtonNum());
    config.endGroup();

*/

}


int RigControlVoiceTxKeyer::setup(TxKeyerFactory *voiceKeyerFactory, int &maxNumButtons, int &numButtons, QString selectedRadioName)
{
    TxKeyerCapabilities voiceCap = voiceKeyerFactory->supportedTxKeyers()->value("rigControl");
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    TxVmRigSetupDialog txVmSetupDialog(voiceCap, maxNumButtons, numButtons, tslf->dmKeyerContainer);

    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICE_KEYER_BASE_FILE_NAME + getTxKeyerTypeFromTxKeyerId(TxKeyerId::RigControl) + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    fileName = TX_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + getTxKeyerTypeFromTxKeyerId(TxKeyerId::RigControl) + ".ini";
    QSettings buttonConfig(fileName, QSettings::IniFormat);


    txVmSetupDialog.setWindowTitle(tr("Rig Control Voice Memory Setup"));


    txVmSetupDialog.setMaxNumOfButtonsLabel(maxNumButtons);

    QString allRadiosGrpName = ALL_RADIOS_GROUP_NAME;

    if (readSaveVoiceCWMemoryButtonByRadioNameFromIni(TxKeyerId::RigControl))
    {
        txVmSetupDialog.setSetupRadioGroupBoxTitle(selectedRadioName);
    }
    else
    {        
        txVmSetupDialog.setSetupRadioGroupBoxTitle(allRadiosGrpName);
    }


    if (voiceCap.getUseCatPTTForEom())
    {

        txVmSetupDialog.setPttEomGroupBoxVisible(true);

        if (pttType == serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_CAT)
        {

            if (readSaveVoiceCWMemoryButtonByRadioNameFromIni(TxKeyerId::RigControl))
            {
                buttonConfig.beginGroup(selectedRadioName.replace('/', '_'));
                TxKeyerCommon::KeyerEomTypes eom = static_cast<TxKeyerCommon::KeyerEomTypes>(buttonConfig.value("endOfMessageType",
                                             static_cast<int>(TxKeyerCommon::KeyerEomTypes::Eom_None)).toInt());

                txVmSetupDialog.setEomRadioButtons(eom);
                buttonConfig.endGroup();
            }
            else
            {
                config.beginGroup(allRadiosGrpName);
                TxKeyerCommon::KeyerEomTypes eom = static_cast<TxKeyerCommon::KeyerEomTypes>(buttonConfig.value("endOfMessageType",
                                                                                                                static_cast<int>(TxKeyerCommon::KeyerEomTypes::Eom_None)).toInt());

                txVmSetupDialog.setEomRadioButtons(eom);
                buttonConfig.endGroup();
            }
        }
    }
    else
    {
        txVmSetupDialog.setPttEomGroupBoxVisible(false);
    }


    txVmSetupDialog.setSwitchToCwVisible(false);


    int ret = txVmSetupDialog.exec();

    if (ret == QDialog::Accepted)
    {
        numButtons = txVmSetupDialog.getNumButtons();

        // save these values by radio name in the buttons ini file


        if (readSaveVoiceCWMemoryButtonByRadioNameFromIni(TxKeyerId::RigControl))
        {
            buttonConfig.beginGroup(selectedRadioName.replace('/', '_'));
        }
        else
        {
            buttonConfig.beginGroup("AllRadios");
        }

        buttonConfig.setValue("NumButtons", numButtons);
        buttonConfig.setValue("endOfMessageType", static_cast<int>(txVmSetupDialog.getSelectedEomType()));
        buttonConfig.endGroup();

        selectedEomType = txVmSetupDialog.getSelectedEomType();

    }
    return ret;
}


void RigControlVoiceTxKeyer::setRadioParams(int radioMaxNumButtons_, QString selectedRadioName_, serialCommonData::MINOS_PTT_TYPES pttType_, bool pttEnabled_)
{

    selectedRadioName = selectedRadioName_;
    radioMaxNumButtons = radioMaxNumButtons_;
    pttType = pttType_;
    pttEnabled = pttEnabled_;
}


void RigControlVoiceTxKeyer::getRadioCommonData(TxKeyerCommon::KeyerEomTypes &selectedEomType, int &userNumberButtons, int radioMaxNumButtons)
{
    int numButtons = 0;

    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICE_KEYER_BASE_FILE_NAME + getTxKeyerTypeFromTxKeyerId(TxKeyerId::RigControl) + ".ini";
    QSettings readCommonConfig(fileName, QSettings::IniFormat);

    QString groupName;
    if (readCommonConfig.value("Common/SaveButtonByRadioName", false).toBool())
    {
        groupName = selectedRadioName.replace('/', '_');
    }
    else
    {
        groupName = ALL_RADIOS_GROUP_NAME;
    }

    fileName = TX_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + getTxKeyerTypeFromTxKeyerId(TxKeyerId::RigControl) + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    config.beginGroup(groupName);
    numButtons = config.value("NumButtons", -1).toInt();
    selectedEomType = static_cast<TxKeyerCommon::KeyerEomTypes>(config.value("endOfMessageType", static_cast<int>(TxKeyerCommon::KeyerEomTypes::Eom_None)).toInt());
    config.endGroup();

    if (numButtons == -1)   // no user button number saved
    {
        numButtons = radioMaxNumButtons;  // radio specific number of voice messages
    }

    userNumberButtons =  numButtons;
}




/*
int RigControlVoiceTxKeyer::editButton(TxKeyerParams *vmData, QString title)
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    TxVmRigButtonDialog vmButtonDialog(tslf->dmButtonFrame);

    if (readSaveVoiceCWMemoryButtonByRadioNameFromIni(TxKeyerId::RigControl))
    {
        vmButtonDialog.setRadioNameLbl(vmData->getSelRadioName());
    }
    else
    {
        vmButtonDialog.setRadioNameLbl("All Radios");
    }

    vmButtonDialog.setWindowTitle(title);
    vmButtonDialog.setVmData(vmData);
    vmButtonDialog.setVmTypeAndRadioModelLabel(vmData->getRigModel());
    vmButtonDialog.setCwMessageTextBoxVisible(false);
    vmButtonDialog.setDialogForEomType(selectedEomType);
    vmButtonDialog.setSerialMessageTextBoxVisible(false);
    vmButtonDialog.setCwInfoPanelVisible(false);

    int ret = vmButtonDialog.exec();
    return ret;

}
*/


