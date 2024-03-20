/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      CW Message Keyer
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2023
//
//
//
//
/////////////////////////////////////////////////////////////////////////////


#include <QSettings>
#include <QString>
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "voicekeyerfactory.h"
#include "txvmrigsetupdialog.h"
#include "txvmrigbuttondialog.h"
#include "rigcontrolcwmessagekeyer.h"
#include "voicekeyerCommonConstants.h"
#include "MTrace.h"

const char * STOPCW = "\xFF";

using namespace voiceKeyerCommon;

RigControlCwMessageKeyer::RigControlCwMessageKeyer(QObject *parent) : VoiceKeyerBase(parent)
{

}


RigControlCwMessageKeyer::~RigControlCwMessageKeyer()
{

}


void RigControlCwMessageKeyer::registerVoiceKeyer(VoiceKeyerFactory::VmKeyers* vmKeyersList)
{
    QString keyerName = "cwRigControl";

    VoiceKeyerCapabilities voiceMemCap;

    voiceMemCap.setVmIdNum(VoiceKeyerId::CW_RigControl);
    voiceMemCap.setKeyerType(keyerTypes[VoiceKeyerId::CW_RigControl]);
    voiceMemCap.setKeyerName(keyerName);
    voiceMemCap.setNumVoiceKeys(8);
    voiceMemCap.setsupportSerial(false);
    voiceMemCap.setUseCatPTTForEom(true);
    voiceMemCap.setEnableCwMode(true);
    voiceMemCap.setSupportRepeatMsg(true);
    voiceMemCap.setHasPip(false);
    voiceMemCap.setHasTxStatus(true);
    voiceMemCap.setSetupButton(true);
    voiceMemCap.setHasAvailStatus(true);
    voiceMemCap.setHasMessageRepeat(true);



    (*vmKeyersList)[keyerName] = voiceMemCap;

}

void RigControlCwMessageKeyer::setPttOnOff(bool onOff)
{
    Q_UNUSED(onOff)
}

bool RigControlCwMessageKeyer::getUsePttForEomFlag()
{
    return usePttForEom;
}

bool RigControlCwMessageKeyer::getSetCwModeAndRestoreFlag()
{
    return setCwModeAndRestoreCurrentMode;
}

void RigControlCwMessageKeyer::voiceKeyerInit(int &numButtons)
{
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::CW_RigControl] + ".ini";
    QSettings config(fileName, QSettings::IniFormat);
    numButtons = config.value("Common/NumButtons", VOICEKEYER_MAX_NUMBUTTONS).toInt();
    usePttForEom = config.value("Common/UseCatPttForEom", true).toBool();
    setCwModeAndRestoreCurrentMode = config.value("Common/SwitchToCwMode", true).toBool();
}



void RigControlCwMessageKeyer::sendCwMsg(VoiceKeyerParams &vmData)
{
    if (!vmData.getVmCwMessage().isEmpty())
    {
        QString cwMessageToTx;
        QString radioManufacturer;
        QString radioModel;

        if (readSaveVoiceCWMemoryButtonByRadioNameFromIni(VoiceKeyerId::CW_RigControl))
        {

            radioManufacturer = getCwRadioManufacturer(cwMemType);
            radioModel = getRadioModel(vmData.getSelRadioName());

        }
        else
        {
            radioManufacturer = "AllRadios";

        }

        QStringList listOfRadioModels;
        getRigCWKeyerRadiosSupportSpecialCharacters(listOfRadioModels, radioManufacturer);
        if (!listOfRadioModels.isEmpty())
        {
            if (listOfRadioModels.contains(radioModel) && vmData.getVmCwMessage().contains(voiceKeyerCommon::specialCwCharEscapeChar))
            {
                // this radio supports special chars
                QMap<QString, QChar> specialCharMap;
                getRigCWKeyerSupportedSpecialCharacters(specialCharMap, radioManufacturer);
                int currentIndex = 0;
                while (currentIndex < vmData.getVmCwMessage().length())
                {
                    QString c = vmData.getVmCwMessage().mid(currentIndex, 1);
                    if (c != voiceKeyerCommon::specialCwCharEscapeChar)
                    {
                        cwMessageToTx.append(c);
                        currentIndex = currentIndex + 1;
                    }
                    else
                    {
                        QString sp = vmData.getVmCwMessage().mid(currentIndex + 1, 2);
                        if (specialCharMap.contains(sp))
                        {
                            cwMessageToTx.append(specialCharMap[sp]);
                        }

                        currentIndex = currentIndex + 3;
                    }
                }


            }
        }
        else
        {
            cwMessageToTx = vmData.getVmCwMessage();
        }

        if (cwMemType == hamlibData::CW_MEMORY_TYPES::YAESU_MEM_RECALL)
        {
            cwMessageToTx.append('}');
        }
        else if (cwMemType == hamlibData::CW_MEMORY_TYPES::KENWOOD_MEM_RECALL)
        {
            cwMessageToTx.append(';');
        }

        TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
        tslf->sendRigTxCwMessage(cwMessageToTx);
    }

}



void RigControlCwMessageKeyer::stopCwMsg()
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    tslf->sendRigTxCwMessage(STOPCW);
}



bool RigControlCwMessageKeyer::readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams)
{



    bool saveByRadioName = readSaveVoiceCWMemoryButtonByRadioNameFromIni(VoiceKeyerId::CW_RigControl);


    QString fileName = VOICE_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + vmParams.getType() + ".ini";
    QSettings config(fileName, QSettings::IniFormat);


    if (saveByRadioName && !vmParams.getSelRadioName().isEmpty())
    {
        config.beginGroup(vmParams.getSelRadioName().replace('/', '_'));
    }
    else
    {
        config.beginGroup(ALL_RADIOS_GROUP_NAME);
    }



    QString newKey = "button" +  QString::number(buttonNum);

    vmParams.setType(config.value(newKey + "/type", "").toString());
    vmParams.setVmName(config.value(newKey + "/name", "").toString());
    vmParams.setVmCwMessage(config.value(newKey + "/cwMessageText", "").toString());
    vmParams.setVmRepeatFlag(config.value(newKey + "/repeatFlag", false).toBool());
    vmParams.setVmDuration(config.value(newKey + "/messageDuration", 0).toInt());
    vmParams.setVmRepeatPauseDur(config.value(newKey + "/repeatPauseDuration", 0).toInt());
    vmParams.setvmButtonNum(config.value(newKey + "/buttonNum", buttonNum).toInt());
    config.endGroup();

    return true;
}


void RigControlCwMessageKeyer::saveVmButtonParams(const VoiceKeyerParams &vmParams_ )
{
    VoiceKeyerParams vmParams = vmParams_;

    bool saveByRadioName = readSaveVoiceCWMemoryButtonByRadioNameFromIni(VoiceKeyerId::CW_RigControl);

    QString fileName = VOICE_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + vmParams.getType() + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    if (saveByRadioName  && !vmParams.getSelRadioName().isEmpty())
    {
       config.beginGroup(vmParams.getSelRadioName().replace('/', '_'));
    }
    else
    {
       config.beginGroup(ALL_RADIOS_GROUP_NAME);
    }

    QString newKey = "button" + QString::number(vmParams.getvmButtonNum());

    config.setValue(newKey + "/type", vmParams.getType());
    config.setValue(newKey + "/name", vmParams.getVmName());
    config.setValue(newKey + "/cwMessageText", vmParams.getVmCwMessage());
    config.setValue(newKey + "/repeatFlag", vmParams.getVmRepeatFlag());
    config.setValue(newKey + "/messageDuration", vmParams.getVmDuration());
    config.setValue(newKey + "/repeatPauseDuration", vmParams.getVmRepeatPauseDur());
    config.setValue(newKey + "/buttonNum", vmParams.getvmButtonNum());
    config.endGroup();

}


int RigControlCwMessageKeyer::setup(VoiceKeyerFactory *voiceKeyerFactory, int &maxNumButtons, int &numButtons, QString selectedRadioName)
{


    VoiceKeyerCapabilities voiceCap = voiceKeyerFactory->supportedVoiceKeyers()->value("cwRigControl");
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    TxVmRigSetupDialog txVmSetupDialog(voiceCap, maxNumButtons, numButtons, tslf->txVmButtonsFrame);

    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::CW_RigControl] + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    fileName = VOICE_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::CW_RigControl] + ".ini";
    QSettings buttonConfig(fileName, QSettings::IniFormat);



    txVmSetupDialog.setWindowTitle(tr("Rig Control CW Memory Setup"));    

    txVmSetupDialog.setMaxNumOfButtonsLabel(maxNumButtons);


    QString allRadiosGrpName = ALL_RADIOS_GROUP_NAME;
    if (readSaveVoiceCWMemoryButtonByRadioNameFromIni(VoiceKeyerId::CW_RigControl))
    {
        txVmSetupDialog.setSetupRadioGroupBoxTitle(selectedRadioName);
    }
    else
    {
        txVmSetupDialog.setSetupRadioGroupBoxTitle(allRadiosGrpName);
    }

    if (voiceCap.getUseCatPTTForEom())
    {
        txVmSetupDialog.setPttEOMChkBoxVisible(true);

        if (readSaveVoiceCWMemoryButtonByRadioNameFromIni(VoiceKeyerId::CW_RigControl))
        {
            buttonConfig.beginGroup(selectedRadioName);
            txVmSetupDialog.setPttEOMChkBoxChecked(buttonConfig.value("UseCatPttForEom", false).toBool());

            buttonConfig.endGroup();
        }
        else
        {
            config.beginGroup(allRadiosGrpName);
            txVmSetupDialog.setPttEOMChkBoxChecked(buttonConfig.value("UseCatPttForEom", false).toBool());
            buttonConfig.endGroup();
        }


    }
    else
    {
        txVmSetupDialog.setPttEOMChkBoxVisible(false);
    }




    if (voiceCap.getEnableCwMode())
    {
        txVmSetupDialog.setSwitchToCwVisible(true);

        if (readSaveVoiceCWMemoryButtonByRadioNameFromIni(VoiceKeyerId::CW_RigControl))
        {
            buttonConfig.beginGroup(selectedRadioName);
            txVmSetupDialog.setSwitchToCwChecked(buttonConfig.value("SwitchToCwMode", true).toBool());
            buttonConfig.endGroup();
        }
        else
        {
            config.beginGroup(allRadiosGrpName);
            txVmSetupDialog.setSwitchToCwChecked(buttonConfig.value("SwitchToCwMode", true).toBool());
            buttonConfig.endGroup();
        }

    }
    else
    {
        txVmSetupDialog.setSwitchToCwVisible(false);
    }







    cwMemType = hamlibData::CW_MEMORY_TYPES::NONE;

    int ret = txVmSetupDialog.exec();

    if (ret == QDialog::Accepted)
    {
        numButtons = txVmSetupDialog.getNumButtons();

        // save these values by radio name in the buttons ini file


        if (readSaveVoiceCWMemoryButtonByRadioNameFromIni(VoiceKeyerId::CW_RigControl))
        {
            buttonConfig.beginGroup(selectedRadioName.replace('/', '_'));
        }
        else
        {
            buttonConfig.beginGroup("AllRadios");
        }


        buttonConfig.setValue("NumButtons", numButtons);
        buttonConfig.setValue("UseCatPttForEom", txVmSetupDialog.getCatPttForEomState() );
        buttonConfig.setValue("SwitchToCwMode", txVmSetupDialog.getSetCwModeAndRestoreState());
        buttonConfig.endGroup();

        usePttForEom = txVmSetupDialog.getCatPttForEomState();
        setCwModeAndRestoreCurrentMode = txVmSetupDialog.getSetCwModeAndRestoreState();


    }

    return ret;


}


void RigControlCwMessageKeyer::setCwMemType(int _cwMemType)
{
    cwMemType = _cwMemType;
}


int RigControlCwMessageKeyer::editButton(VoiceKeyerParams *vmData, QString title)
{

    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    TxVmRigButtonDialog vmButtonDialog(tslf->txVmButtonsFrame);

    QString radioManufacturer;   // used as group name
    QString radioModel;

    if (readSaveVoiceCWMemoryButtonByRadioNameFromIni(VoiceKeyerId::CW_RigControl))
    {
        vmButtonDialog.setRadioNameLbl(vmData->getSelRadioName());
        radioManufacturer = getCwRadioManufacturer(cwMemType);
        radioModel = getRadioModel(vmData->getSelRadioName());

    }
    else
    {
        vmButtonDialog.setRadioNameLbl("All Radios");
        radioManufacturer = "AllRadios";

    }




    vmButtonDialog.setWindowTitle(title);
    vmButtonDialog.setVmData(vmData);
    vmButtonDialog.setVmTypeLabelcwMemType(radioManufacturer);
    vmButtonDialog.setCwInfoPanelVisible(true);

    QString validCharCwList;
    if (getRigCWKeyerSupportedCharacters(validCharCwList, radioManufacturer))
    {
        vmButtonDialog.setCwValidatorCwCharList(validCharCwList);
    }
    else
    {
        trace(QString("Error retrieving supported CW Chars for manufacturer %1").arg(radioManufacturer));
    }


    int maxNumChars = 0;
    if (getRigCWKeyerMaxMessageLength(maxNumChars, radioManufacturer))
    {
        vmButtonDialog.setCwValidatorMaxCwMessageLength(maxNumChars);
        vmButtonDialog.setMaxNumberCwCharactersText(maxNumChars);
    }
    else
    {
        trace(QString("Error retrieving max CW Message Length for manufacturer %1").arg(radioManufacturer));
        vmButtonDialog.setMaxNumberCwCharactersText(0);
    }

    QStringList listOfRadioModels;
    getRigCWKeyerRadiosSupportSpecialCharacters(listOfRadioModels, radioManufacturer);

    bool radioSupportSpecialChar = false;

    if (!listOfRadioModels.isEmpty())
    {
        if (listOfRadioModels.contains(radioModel))
        {
            radioSupportSpecialChar = true;
        }
    }

    vmButtonDialog.setCwSupportSpecialChar(radioSupportSpecialChar);

    QMap<QString, QChar> specialCharMap;
    if (radioSupportSpecialChar)
    {
        if (getRigCWKeyerSupportedSpecialCharacters(specialCharMap, radioManufacturer))
        {
            vmButtonDialog.setSpecialCwCharLists(specialCharMap);
            vmButtonDialog.setCwSupportSpecialCharsGroupBoxVisible(true);
        }


    }
    else
    {
        vmButtonDialog.setCwSupportSpecialCharsGroupBoxVisible(false);
    }

    vmButtonDialog.setSerialMessageTextBoxVisible(false);
    vmButtonDialog.setCwCharInputValidator();

    if (cwMemType == hamlibData::CW_MEMORY_TYPES::YAESU_MEM_RECALL
        || cwMemType == hamlibData::CW_MEMORY_TYPES::KENWOOD_MEM_RECALL
        || cwMemType == hamlibData::CW_MEMORY_TYPES::ELECRAFT
        || cwMemType == hamlibData::CW_MEMORY_TYPES::ICOM)
    {
        vmButtonDialog.setCwMessageTextBoxVisible(true);
    }


    vmButtonDialog.setDialogForCatPttEom(usePttForEom);

    int ret = vmButtonDialog.exec();
    return ret;

}





QString RigControlCwMessageKeyer::getRadioModel(QString selectedRadio)
{
    QStringList rl = selectedRadio.split('/');
    if (rl.length() == 2)
    {
        return rl[1];
    }

    return "";
}







