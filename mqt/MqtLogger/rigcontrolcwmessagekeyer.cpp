/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      CW Message Keyer
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2024
//
//
//
//
/////////////////////////////////////////////////////////////////////////////


#include <QSettings>
#include <QString>



#include "LoggerContest.h"
#include "tlogcontainer.h"
#include "voicekeyerfactory.h"
#include "txvmrigsetupdialog.h"
#include "txvmrigbuttondialog.h"
#include "rigcontrolcwmessagekeyer.h"
#include "voicekeyerCommonConstants.h"
#include "MTrace.h"



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

void RigControlCwMessageKeyer::setSelectedEomType(int selectedEomType_)
{
    selectedEomType = selectedEomType_;
}

void RigControlCwMessageKeyer::setPttOnOff(bool onOff)
{
    Q_UNUSED(onOff)
}

int RigControlCwMessageKeyer::getSelectedEomType()
{
    return selectedEomType;
}

bool RigControlCwMessageKeyer::getSetCwModeAndRestoreFlag()
{
    return setCwModeAndRestoreCurrentMode;
}

void RigControlCwMessageKeyer::voiceKeyerInit(int &numButtons)
{
    int userNumberButtons = 0;
    getRadioCommonData(selectedEomType, userNumberButtons, radioMaxNumButtons);
    numButtons = userNumberButtons;
}


void RigControlCwMessageKeyer::getRadioCommonData(int &selectedEomType, int &userNumberButtons, int radioMaxNumButtons)
{
    int numButtons = 0;

    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::CW_RigControl] + ".ini";
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

    fileName = VOICE_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::CW_RigControl] + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    config.beginGroup(groupName);
    numButtons = config.value("NumButtons", -1).toInt();
    selectedEomType = config.value("endOfMessageType", voiceKeyerCommon::VoiceCwKeyerEomTypes::Eom_None).toInt();
    setCwModeAndRestoreCurrentMode = config.value("SwitchToCwMode", true).toBool();
    config.endGroup();

    if (numButtons == -1)   // no user button number saved
    {
        numButtons = radioMaxNumButtons;  // radio specific number of voice messages
    }

    userNumberButtons =  numButtons;
}



void RigControlCwMessageKeyer::sendCwMsg(VoiceKeyerParams &vmData)
{
    if (!vmData.getVmCwMessage().isEmpty())
    {
        QString cwMessageToTx;
        QString radioManufacturer;
        QStringList rmList;
        QString radioModel;


        TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();


        if (readSaveVoiceCWMemoryButtonByRadioNameFromIni(VoiceKeyerId::CW_RigControl))
        {

            radioManufacturer = getCwRadioManufacturer(cwMemType);
            rmList = vmData.getRigModel().split(' ');
            if (rmList.length() == 2)
            {
                radioModel = rmList[1].remove('-');
            }


        }
        else
        {
            radioManufacturer = "AllRadios";

        }

        QStringList listOfRadioModels;
        getRigCWKeyerListOfRadiosSupportSpecialCharacters(listOfRadioModels, radioManufacturer);

        if (!listOfRadioModels.isEmpty()
            && listOfRadioModels.contains(radioModel)
            && vmData.getVmCwMessage().contains(voiceKeyerCommon::specialCwCharEscapeChar)
            && cwMemType != hamlibData::CW_MEMORY_TYPES::ICOM)          // we don't need to convert for Icom radios
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
        else
        {
            cwMessageToTx = vmData.getVmCwMessage();
        }



        cwMessageToTx = parseMacrosInMessage(tslf, cwMessageToTx);

        trace(QString("Send Morse Message to radio : %1").arg(cwMessageToTx));

        tslf->sendRigTxCwMessage(cwMessageToTx);
    }

}


QString RigControlCwMessageKeyer::parseMacrosInMessage(TSingleLogFrame *tslf, QString mess)
{
    // make sure screenContact is up to date

    tslf->GJVQSOLogFrame->getScreenEntry();
    ScreenContact *sc = &tslf->GJVQSOLogFrame->screenContact;

    // data is taken now; an {ENTER} may log the call, and clear it
    QString call = sc->cs.getFullCall();
    QString serials = sc->serials;
    QString reps = sc->reps;

    QString txMess;

    // and parse the message

    for (int i = 0; i < mess.length(); i++)
    {
        QChar c = mess[i];


        if (c == '*')
        {
            txMess += ct->mycall.getFullCall();
        }
        else if (c == '#')
        {
            txMess += serials;
        }
        else if (c == '!')
        {
            txMess += call;
        }
        else if (c == '{')
        {
            int lb = mess.indexOf('}', i);
            if (lb)
            {
                QString macro = mess.mid(i + 1, lb - i - 1).toUpper().trimmed();
                i = lb;
                if (macro == "MYCALL")
                {
                    txMess += ct->mycall.getFullCall();
                }
                else if (macro == "CALL")
                {
                    txMess += call;
                }
                else if (macro == "SN")
                {
                    txMess += serials;
                }
                else if (macro == "EXCH")
                {
                    // This is whatever exchange is required
                    // May have multiple elements!
                    // in particular, includes serial number

                    // we need an exchange definition somewhere
                    // to be able to do this properly

                    bool needSpace = false;
                    if (ct->serialMandatoryField.getValue() || ct->asymmetricMult.getValue())
                    {
                        txMess += serials;
                        needSpace = true;
                    }
                    if (!ct->asymmetricMult.getValue() && ct->exchangeRequired.getValue())
                    {
                        QString exch = ct->location.getValue();
                        if (!exch.isEmpty() && exch != "-")
                        {
                            if (needSpace)
                            {
                                txMess += ' ';
                            }
                            txMess +=exch;
                        }
                    }

                }
                else if (macro == "GRID")
                {
                    txMess += ct->myloc.getLoc();
                }
                else if (macro == "SPACE")
                {
                    txMess += ' ';
                }
                else if (macro == "SENTRST")
                {
                    txMess += reps;
                }
                else if (macro == "TIME2")
                {
                    QString t2 = sc->sentExchange.getValue();
                    if (t2.isEmpty())
                    {
                        t2 = QDateTime::currentDateTimeUtc().toString("HHmm");
                    }
                    txMess += t2;
                    tslf->GJVQSOLogFrame->sentExchange = t2;
                }
                else if (macro == "LOG")
                {
                    // simulate "Enter" key
                    // This may well log the contact, leaving nothing
                    // useful in screenContact
                    // which is why N1MM has various "last contact" macros

                    tslf->GJVQSOLogFrame->doGJVOKButton_clicked();
                }
                else if (macro == "WIPE")
                {
                    // wipe QSO - like ESC key
                    tslf->GJVQSOLogFrame->doGJVCancelButton_clicked();
                }
                else if (macro == "CALLFIELD")
                {
                    tslf->GJVQSOLogFrame->selectCallField();
                }
                else if (macro == "SERIALFIELD")
                {
                    tslf->GJVQSOLogFrame->selectSnRxField();
                }
                else if (macro == "EXCHANGEFIELD")
                {
                    tslf->GJVQSOLogFrame->selectExchField();
                }
                else
                {
                    trace(QString("Message <%1> contains unknown macro {%2}").arg(mess, macro));
                }
            }
        }
        else
        {
            txMess += c;
        }
    }
    return txMess;

}




void RigControlCwMessageKeyer::stopCwMsg()
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    tslf->sendRigStopTxVoiceMessage(STOP_CW_MESSAGE);
}



bool RigControlCwMessageKeyer::readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams)
{



    bool saveByRadioName = readSaveVoiceCWMemoryButtonByRadioNameFromIni(VoiceKeyerId::CW_RigControl);

    QString runStateTxt;
    if(vmParams.getSAndPState())
    {
        runStateTxt = "_SANDP";
    }
    else
    {
        runStateTxt = "_RUN";
    }

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



    QString newKey = "button" +  QString::number(buttonNum) + runStateTxt;

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

    QString runStateTxt;
    if(vmParams.getSAndPState())
    {
        runStateTxt = "_SANDP";
    }
    else
    {
        runStateTxt = "_RUN";
    }

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

    QString newKey = "button" + QString::number(vmParams.getvmButtonNum()) + runStateTxt;

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
        txVmSetupDialog.setPttEomGroupBoxVisible(true);

        if (readSaveVoiceCWMemoryButtonByRadioNameFromIni(VoiceKeyerId::CW_RigControl))
        {
            buttonConfig.beginGroup(selectedRadioName.replace('/', '_'));
            txVmSetupDialog.setEomRadioButtons(buttonConfig.value("endOfMessageType", voiceKeyerCommon::VoiceCwKeyerEomTypes::CAT).toInt());

            buttonConfig.endGroup();
        }
        else
        {
            config.beginGroup(allRadiosGrpName);
            txVmSetupDialog.setEomRadioButtons(buttonConfig.value("endOfMessageType", voiceKeyerCommon::VoiceCwKeyerEomTypes::CAT).toInt());
            buttonConfig.endGroup();
        }


    }
    else
    {
        txVmSetupDialog.setPttEomGroupBoxVisible(false);
    }




    if (voiceCap.getEnableCwMode())
    {
        txVmSetupDialog.setSwitchToCwVisible(true);

        if (readSaveVoiceCWMemoryButtonByRadioNameFromIni(VoiceKeyerId::CW_RigControl))
        {
            buttonConfig.beginGroup(selectedRadioName.replace('/', '_'));
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
        buttonConfig.setValue("endOfMessageType", txVmSetupDialog.getSelectedEomType());
        buttonConfig.setValue("SwitchToCwMode", txVmSetupDialog.getSetCwModeAndRestoreState());
        buttonConfig.endGroup();

        selectedEomType = txVmSetupDialog.getSelectedEomType();
        setCwModeAndRestoreCurrentMode = txVmSetupDialog.getSetCwModeAndRestoreState();


    }

    return ret;


}


void RigControlCwMessageKeyer::setRadioParams(int radioMaxNumButtons_, QString selectedRadioName_, serialCommonData::MINOS_PTT_TYPES pttType_, bool pttEnabled_)
{
    selectedRadioName = selectedRadioName_;
    radioMaxNumButtons = radioMaxNumButtons_;
    pttType = pttType_;
    pttEnabled = pttEnabled_;
}


void RigControlCwMessageKeyer::setCwMemType(int _cwMemType)
{
    cwMemType = _cwMemType;
}


void RigControlCwMessageKeyer::setContest(BaseContestLog *c)
{
    ct = dynamic_cast<LoggerContestLog *>( c);

}


int RigControlCwMessageKeyer::editButton(VoiceKeyerParams *vmData, QString title)
{

    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    TxVmRigButtonDialog vmButtonDialog(tslf->txVmButtonsFrame);

    QString radioManufacturer;   // used as group name
    QStringList rmList;
    QString radioModel;

    if (readSaveVoiceCWMemoryButtonByRadioNameFromIni(VoiceKeyerId::CW_RigControl))
    {
        vmButtonDialog.setRadioNameLbl(vmData->getSelRadioName());
        radioManufacturer = getCwRadioManufacturer(cwMemType);
        rmList = vmData->getRigModel().split(' ');
        if (rmList.length() == 2)
        {
            radioModel = rmList[1].remove('-');
        }

    }
    else
    {
        vmButtonDialog.setRadioNameLbl("All Radios");
        radioManufacturer = "AllRadios";

    }




    vmButtonDialog.setWindowTitle(title);
    vmButtonDialog.setVmData(vmData);
    vmButtonDialog.setVmTypeAndRadioModelLabel(vmData->getRigModel());
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
    getRigCWKeyerListOfRadiosSupportSpecialCharacters(listOfRadioModels, radioManufacturer);

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

    if (cwMemType == hamlibData::CW_MEMORY_TYPES::YAESU
        || cwMemType == hamlibData::CW_MEMORY_TYPES::KENWOOD
        || cwMemType == hamlibData::CW_MEMORY_TYPES::ELECRAFT
        || cwMemType == hamlibData::CW_MEMORY_TYPES::ICOM)
    {
        vmButtonDialog.setCwMessageTextBoxVisible(true);
    }


    vmButtonDialog.setDialogForEomType(selectedEomType);

    int ret = vmButtonDialog.exec();
    return ret;

}













