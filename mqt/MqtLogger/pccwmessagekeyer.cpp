/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Logger PC DTR CW Message Keyer
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2025
//
//
//
//
/////////////////////////////////////////////////////////////////////////////

#include <QSettings>
#include <QString>



#include "LoggerContest.h"
#include "tlogcontainer.h"
#include "txKeyerfactory.h"
#include "txvmrigsetupdialog.h"
#include "txvmrigbuttondialog.h"
#include "txkeyerCommonConstants.h"
#include "MTrace.h"
#include "pccwmessagekeyer.h"

// Note we have forced to use "AllRadios" as this is radio agnostic...


PcCWMessageKeyer::PcCWMessageKeyer(QObject *parent) : TxKeyerBase(parent)
{

}

PcCWMessageKeyer::~PcCWMessageKeyer()
{

}


void PcCWMessageKeyer::registerTxKeyer(TxKeyerFactory::TxKeyers* vmKeyersList)
{
    QString keyerName = "pcCwKeyer";

    TxKeyerCapabilities voiceMemCap;

    voiceMemCap.setVmIdNum(TxKeyerId::PcCwKeyer);
    voiceMemCap.setKeyerType(keyerTypes[TxKeyerId::PcCwKeyer]);
    voiceMemCap.setKeyerName(keyerName);
    voiceMemCap.setNumVoiceKeys(12);
    voiceMemCap.setsupportSerial(false);
    voiceMemCap.setUseCatPTTForEom(false);
    voiceMemCap.setEnableCwMode(true);
    voiceMemCap.setSupportRepeatMsg(true);
    voiceMemCap.setHasPip(false);
    voiceMemCap.setHasTxStatus(true);
    voiceMemCap.setSetupButton(true);
    voiceMemCap.setHasAvailStatus(true);
    voiceMemCap.setHasMessageRepeat(true);



    (*vmKeyersList)[keyerName] = voiceMemCap;

}


void PcCWMessageKeyer::setSelectedEomType(int selectedEomType_)
{
    selectedEomType = selectedEomType_;
}

int PcCWMessageKeyer::getSelectedEomType()
{
    return selectedEomType;
}

void PcCWMessageKeyer::setPttOnOff(bool onOff)
{
    Q_UNUSED(onOff)
}

bool PcCWMessageKeyer::getSetCwModeAndRestoreFlag()
{
    return setCwModeAndRestoreCurrentMode;
}


void PcCWMessageKeyer::txKeyerInit(int &numButtons)
{
    int userNumberButtons = 0;
    getRadioCommonData(selectedEomType, userNumberButtons, radioMaxNumButtons);
    numButtons = userNumberButtons;


}

void PcCWMessageKeyer::getRadioCommonData(int &selectedEomType, int &userNumberButtons, int radioMaxNumButtons)
{
    int numButtons = 0;

    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[TxKeyerId::PcCwKeyer] + ".ini";
    QSettings readCommonConfig(fileName, QSettings::IniFormat);

    QString groupName;
    //if (readCommonConfig.value("Common/SaveButtonByRadioName", false).toBool())
    //{
   //     groupName = selectedRadioName.replace('/', '_');
   // }
   // else
   // {
        groupName = ALL_RADIOS_GROUP_NAME;
  //  }

    fileName = VOICE_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[TxKeyerId::CW_RigControl] + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    config.beginGroup(groupName);
    numButtons = config.value("NumButtons", -1).toInt();
    selectedEomType = config.value("endOfMessageType", TxKeyerCommon::KeyerEomTypes::Eom_None).toInt();
    setCwModeAndRestoreCurrentMode = config.value("SwitchToCwMode", true).toBool();
    config.endGroup();

    if (numButtons == -1)   // no user button number saved
    {
        numButtons = radioMaxNumButtons;  // radio specific number of voice messages
    }

    userNumberButtons =  numButtons;
}

// we don't change these parameters through Setup
void PcCWMessageKeyer::saveFixedRadioCommonData()
{
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[TxKeyerId::PcCwKeyer] + ".ini";
    QSettings readCommonConfig(fileName, QSettings::IniFormat);

    QString groupName = ALL_RADIOS_GROUP_NAME;

    fileName = VOICE_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[TxKeyerId::CW_RigControl] + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    config.beginGroup(groupName);
    config.setValue("NumButtons", PC_CW_KEYER_MAXIMUM_BUTTONS);
    config.setValue("endOfMessageType", TxKeyerCommon::KeyerEomTypes::DTRKeyerTXStatus);
    config.endGroup();


}


void PcCWMessageKeyer::sendCwMsg(TxKeyerParams &vmData)
{
    if (!vmData.getKeyerCwMessage().isEmpty())
    {
        TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

        QString cwMessageToTx = vmData.getKeyerCwMessage();

        trace(QString("PcCwMessageKeyer Send Morse Message to Pc DTR Keyer : %1").arg(cwMessageToTx));

        tslf->sendPcKeyerTxCwMessage(cwMessageToTx);
    }
    else
    {
        trace(QString("PcCwMessageKeyer Send Morse Message to radio, message is empty"));
    }

}

void  PcCWMessageKeyer::sendCwFreeTextMsg(QString message)
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    QString cwMessageToTx = parseMacrosInMessage(tslf, message);

    trace(QString("Send Free Text Morse Message to PcCwKeyer : %1").arg(cwMessageToTx));

    tslf->sendPcKeyerTxCwMessage(cwMessageToTx);
}




QString PcCWMessageKeyer::parseMacrosInMessage(TSingleLogFrame *tslf, QString mess)
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
        else if (c == '%')
        {
            txMess += ct->myloc.getLoc();
        }
        else if (c == '$')
        {
            txMess += reps;
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




void PcCWMessageKeyer::stopCwMsg()
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    tslf->sendPcKeyerTxCwStop(STOP_CW_MESSAGE);
}



bool PcCWMessageKeyer::readVmButtonParams(int buttonNum, TxKeyerParams &vmParams)
{
/*


    //bool saveByRadioName = readSaveVoiceCWMemoryButtonByRadioNameFromIni(VoiceKeyerId::CW_RigControl);

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


    //if (saveByRadioName && !vmParams.getSelRadioName().isEmpty())
   // {
    //    config.beginGroup(vmParams.getSelRadioName().replace('/', '_'));
    //}
   // else
   // {
        config.beginGroup(ALL_RADIOS_GROUP_NAME);
   // }



    QString newKey = "button" +  QString::number(buttonNum) + runStateTxt;

    vmParams.setType(config.value(newKey + "/type", "").toString());
    vmParams.setKeyerName(config.value(newKey + "/name", "").toString());
    vmParams.setKeyerCwMessage(config.value(newKey + "/cwMessageText", "").toString());
    vmParams.setKeyerRepeatFlag(config.value(newKey + "/repeatFlag", false).toBool());
    vmParams.setKeyerDuration(config.value(newKey + "/messageDuration", 0).toInt());
    vmParams.setKeyerRepeatPauseDur(config.value(newKey + "/repeatPauseDuration", 0).toInt());
    vmParams.setKeyerButtonNum(config.value(newKey + "/buttonNum", buttonNum).toInt());
    config.endGroup();

    return true;
*/ return true;
}


void PcCWMessageKeyer::saveVmButtonParams(const TxKeyerParams &vmParams_ )
{
/*    TxKeyerParams vmParams = vmParams_;

    //bool saveByRadioName = readSaveVoiceCWMemoryButtonByRadioNameFromIni(VoiceKeyerId::CW_RigControl);

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

    //if (saveByRadioName  && !vmParams.getSelRadioName().isEmpty())
   // {
   //     config.beginGroup(vmParams.getSelRadioName().replace('/', '_'));
   // }
   // else
   // {
        config.beginGroup(ALL_RADIOS_GROUP_NAME);
   // }

    QString newKey = "button" + QString::number(vmParams.getKeyerButtonNum()) + runStateTxt;

    config.setValue(newKey + "/type", vmParams.getType());
    config.setValue(newKey + "/name", vmParams.getKeyerName());
    config.setValue(newKey + "/cwMessageText", vmParams.getKeyerCwMessage());
    config.setValue(newKey + "/repeatFlag", vmParams.getKeyerRepeatFlag());
    config.setValue(newKey + "/messageDuration", vmParams.getKeyerDuration());
    config.setValue(newKey + "/repeatPauseDuration", vmParams.getKeyerRepeatPauseDur());
    config.setValue(newKey + "/buttonNum", vmParams.getKeyerButtonNum());
    config.endGroup();
*/
}


int PcCWMessageKeyer::setup(TxKeyerFactory *txKeyerFactory, int &maxNumButtons, int &numButtons, QString selectedRadioName)
{

    Q_UNUSED(selectedRadioName)

    TxKeyerCapabilities voiceCap = txKeyerFactory->supportedTxKeyers()->value("pcCwKeyer");
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    TxVmRigSetupDialog txVmSetupDialog(voiceCap, maxNumButtons, numButtons, tslf->dmButtonFrame);

    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[TxKeyerId::PcCwKeyer] + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    fileName = VOICE_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[TxKeyerId::PcCwKeyer] + ".ini";
    QSettings buttonConfig(fileName, QSettings::IniFormat);



    txVmSetupDialog.setWindowTitle(tr("PC CW Keyer Setup"));

    //txVmSetupDialog.setMaxNumOfButtonsLabel(maxNumButtons);


    QString allRadiosGrpName = ALL_RADIOS_GROUP_NAME;
    //if (readSaveVoiceCWMemoryButtonByRadioNameFromIni(VoiceKeyerId::PcCwKeyer))
   // {
   //     txVmSetupDialog.setSetupRadioGroupBoxTitle(selectedRadioName);
  //  }
  //  else
  //  {
        txVmSetupDialog.setSetupRadioGroupBoxTitle(allRadiosGrpName);
  //  }

  /*
        if (voiceCap.getUseCatPTTForEom())
    {
        txVmSetupDialog.setPttEomGroupBoxVisible(true);

        if (readSaveVoiceCWMemoryButtonByRadioNameFromIni(VoiceKeyerId::PcCwKeyer))
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

*/

    // we don't allow change number of buttons
    txVmSetupDialog.setButtonWidgetsVisible(false);

    if (voiceCap.getEnableCwMode())
    {
        txVmSetupDialog.setSwitchToCwVisible(true);

        //if (readSaveVoiceCWMemoryButtonByRadioNameFromIni(VoiceKeyerId::PcCwKeyer))
        //{
        //    buttonConfig.beginGroup(selectedRadioName.replace('/', '_'));
        //    txVmSetupDialog.setSwitchToCwChecked(buttonConfig.value("SwitchToCwMode", true).toBool());
        //    buttonConfig.endGroup();
        //}
       // else
       // {
            config.beginGroup(allRadiosGrpName);
            txVmSetupDialog.setSwitchToCwChecked(buttonConfig.value("SwitchToCwMode", true).toBool());
            buttonConfig.endGroup();
       // }

    }
    else
    {
        txVmSetupDialog.setSwitchToCwVisible(false);
    }







    cwMemType = hamlibData::CW_MEMORY_TYPES::NONE;

    int ret = txVmSetupDialog.exec();

    if (ret == QDialog::Accepted)
    {
        //numButtons = txVmSetupDialog.getNumButtons();

        // save these values by radio name in the buttons ini file


        //if (readSaveVoiceCWMemoryButtonByRadioNameFromIni(VoiceKeyerId::PcCwKeyer))
        //{
        //    buttonConfig.beginGroup(selectedRadioName.replace('/', '_'));
        //}
        //else
        //{
            buttonConfig.beginGroup("AllRadios");
       // }


        //buttonConfig.setValue("NumButtons", numButtons);
        //buttonConfig.setValue("endOfMessageType", txVmSetupDialog.getSelectedEomType());
        buttonConfig.setValue("SwitchToCwMode", txVmSetupDialog.getSetCwModeAndRestoreState());
        buttonConfig.endGroup();

        //selectedEomType = txVmSetupDialog.getSelectedEomType();
        setCwModeAndRestoreCurrentMode = txVmSetupDialog.getSetCwModeAndRestoreState();


    }

    return ret;


}


void PcCWMessageKeyer::setRadioParams(int radioMaxNumButtons_, QString selectedRadioName_, serialCommonData::MINOS_PTT_TYPES pttType_, bool pttEnabled_)
{
    selectedRadioName = selectedRadioName_;
    radioMaxNumButtons = radioMaxNumButtons_;
    //pttType = pttType_;
    //pttEnabled = pttEnabled_;
}


void PcCWMessageKeyer::setCwMemType(int _cwMemType)
{
    cwMemType = _cwMemType;
}


void PcCWMessageKeyer::setContest(BaseContestLog *c)
{
    ct = dynamic_cast<LoggerContestLog *>( c);

}


int PcCWMessageKeyer::editButton(TxKeyerParams *vmData, QString title)
{
/*
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    TxVmRigButtonDialog vmButtonDialog(tslf->dmButtonFrame);

    QString radioManufacturer;   // used as group name
    QStringList rmList;
    //QString radioModel;

   if (readSaveVoiceCWMemoryButtonByRadioNameFromIni(VoiceKeyerId::PcCwKeyer))
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

        vmButtonDialog.setRadioNameLbl("All Radios");
        radioManufacturer = "AllRadios";

    }




    vmButtonDialog.setWindowTitle(title);
    vmButtonDialog.setVmData(vmData);
    vmButtonDialog.setVmTypeAndRadioModelLabel(vmData->getRigModel());
    vmButtonDialog.setCwInfoPanelVisible(true);

    QString cwMacroCharList;
    bool cwMacroCharOk;

    QString rm = "AllRadios";

    // we still support saving message under radioname, but retrieve common data from common file
    //if (radioManufacturer != "AllRadios")
    //{
    //   rm = "PC_CWKEYER";
    //}

    if (getRigCWKeyerMacroCharacter(cwMacroCharList, rm, PC_CW_KEYER_COMMON_PARAMS_FILENAME))
    {
        cwMacroCharOk = true;
        trace(QString("Retrieved CW Macro Chars %1 for pcCwKeyer").arg(cwMacroCharList));
    }
    else
    {
        cwMacroCharOk = false;
        trace(QString("Error retrieving CW Macro Chars for pcCwKeyer"));
    }



    QString validCharCwList;
    if (getRigCWKeyerSupportedCharacters(validCharCwList, rm, PC_CW_KEYER_COMMON_PARAMS_FILENAME))
    {
        if (cwMacroCharOk)
        {
            QString vc = validCharCwList.append(cwMacroCharList);
            vmButtonDialog.setCwValidatorCwCharList(vc);
            trace(QString("Supported CW Chars and Macro chars %1 for pcCwKeyer").arg(vc));
        }
        else
        {
            vmButtonDialog.setCwValidatorCwCharList(validCharCwList);
            trace(QString("Supported CW Chars with no Macro chars %1 for ").arg(validCharCwList));
        }

    }
    else
    {
        trace(QString("Error retrieving supported CW Chars for pcCwKeyer"));
    }


    int maxNumChars = 0;
    if (getRigCWKeyerMaxMessageLength(maxNumChars, rm, PC_CW_KEYER_COMMON_PARAMS_FILENAME))
    {
        vmButtonDialog.setCwValidatorMaxCwMessageLength(maxNumChars);
        vmButtonDialog.setMaxNumberCwCharactersText(maxNumChars);
    }
    else
    {
        trace(QString("Error retrieving max CW Message Length for pcCwKeyer"));
        vmButtonDialog.setMaxNumberCwCharactersText(0);
    }

    QStringList listOfRadioModels;
    getRigCWKeyerListOfRadiosSupportSpecialCharacters(listOfRadioModels, radioManufacturer, PC_CW_KEYER_COMMON_PARAMS_FILENAME);

    bool radioSupportSpecialChar = false;

    if (!listOfRadioModels.isEmpty())
    {
        if (listOfRadioModels.contains(radioModel))
        {
            radioSupportSpecialChar = true;
        }
    }


    bool radioSupportSpecialChar = false;

    vmButtonDialog.setCwSupportSpecialChar(radioSupportSpecialChar);

    QMap<QString, QChar> specialCharMap;
    if (radioSupportSpecialChar)
    {
        if (getRigCWKeyerSupportedSpecialCharacters(specialCharMap, rm, PC_CW_KEYER_COMMON_PARAMS_FILENAME))
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

    //if (cwMemType == hamlibData::CW_MEMORY_TYPES::YAESU
    //    || cwMemType == hamlibData::CW_MEMORY_TYPES::KENWOOD
    //    || cwMemType == hamlibData::CW_MEMORY_TYPES::ELECRAFT
    //    || cwMemType == hamlibData::CW_MEMORY_TYPES::ICOM)
   // {
        vmButtonDialog.setCwMessageTextBoxVisible(true);
   // }


    vmButtonDialog.setDialogForEomType(selectedEomType);

    int ret = vmButtonDialog.exec();
    return ret;
*/ return 0;
}
