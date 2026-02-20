/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Logger WinKeyer
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2025
//
//
//
//
/////////////////////////////////////////////////////////////////////////////

#include <QSettings>
#include <QString>
#include "winkeyertxkeyer.h"
#include "LoggerContest.h"
#include "tlogcontainer.h"
#include "txvmrigsetupdialog.h"
#include "txkeyerCommonConstants.h"
#include "MTrace.h"

WinKeyerTxKeyer::WinKeyerTxKeyer(QObject *parent)
    : TxKeyerBase{parent}
{

}

WinKeyerTxKeyer::~WinKeyerTxKeyer()
{

}



void WinKeyerTxKeyer::registerTxKeyer(TxKeyerFactory::TxKeyers* vmKeyersList)
{
    QString keyerName = getTxKeyerDisplayName(TxKeyerId::WinKeyer);

    TxKeyerCapabilities voiceMemCap;

    voiceMemCap.setTxKeyerId(TxKeyerId::WinKeyer);
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


void WinKeyerTxKeyer::setSelectedEomType(TxKeyerCommon::KeyerEomTypes selectedEomType_)
{
    selectedEomType = selectedEomType_;
}

TxKeyerCommon::KeyerEomTypes WinKeyerTxKeyer::getSelectedEomType()
{
    return selectedEomType;
}

void WinKeyerTxKeyer::setPttOnOff(bool onOff)
{
    Q_UNUSED(onOff)
}

bool WinKeyerTxKeyer::getSetCwModeAndRestoreFlag()
{
    return setCwModeAndRestoreCurrentMode;
}


void WinKeyerTxKeyer::txKeyerInit(int &numButtons)
{
    int userNumberButtons = 0;
    getRadioCommonData(selectedEomType, userNumberButtons, radioMaxNumButtons);
    numButtons = userNumberButtons;


}

bool WinKeyerTxKeyer::readTxKeyerButtonParams(int buttonNum, TxKeyerParams &txKeyerParams)
{
    return true;
}
void WinKeyerTxKeyer::saveTxKeyerButtonParams(const TxKeyerParams &txKeyerParams)
{

}


void WinKeyerTxKeyer::getRadioCommonData(TxKeyerCommon::KeyerEomTypes &selectedEomType, int &userNumberButtons, int radioMaxNumButtons)
{
    int numButtons = 0;

    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICE_KEYER_BASE_FILE_NAME + getTxKeyerTypeFromTxKeyerId(TxKeyerId::PcCwKeyer) + ".ini";
    QSettings readCommonConfig(fileName, QSettings::IniFormat);

    QString groupName = ALL_RADIOS_GROUP_NAME;

    fileName = TX_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + getTxKeyerTypeFromTxKeyerId(TxKeyerId::CW_RigControl) + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    config.beginGroup(groupName);
    numButtons = config.value("NumButtons", -1).toInt();
    selectedEomType = static_cast<TxKeyerCommon::KeyerEomTypes>(config.value("endOfMessageType", static_cast<int>(TxKeyerCommon::KeyerEomTypes::Eom_None)).toInt());
    setCwModeAndRestoreCurrentMode = config.value("SwitchToCwMode", true).toBool();
    config.endGroup();

    if (numButtons == -1)   // no user button number saved
    {
        numButtons = radioMaxNumButtons;  // radio specific number of voice messages
    }

    userNumberButtons =  numButtons;
}

// we don't change these parameters through Setup
void WinKeyerTxKeyer::saveFixedRadioCommonData()
{
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICE_KEYER_BASE_FILE_NAME + getTxKeyerTypeFromTxKeyerId(TxKeyerId::PcCwKeyer) + ".ini";
    QSettings readCommonConfig(fileName, QSettings::IniFormat);

    QString groupName = ALL_RADIOS_GROUP_NAME;

    fileName = TX_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + getTxKeyerTypeFromTxKeyerId(TxKeyerId::CW_RigControl) + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    config.beginGroup(groupName);
    config.setValue("NumButtons", PC_CW_KEYER_MAXIMUM_BUTTONS);
    config.setValue("endOfMessageType", static_cast<int>(TxKeyerCommon::KeyerEomTypes::DTRKeyerTXStatus));
    config.endGroup();


}


void WinKeyerTxKeyer::sendCwMsg(TxKeyerParams &vmData)
{
    if (!vmData.getKeyerCwMessage().isEmpty())
    {
        TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

        QString cwMessageToTx = vmData.getKeyerCwMessage();

        logMessage(QString("PcCwMessageKeyer Send Morse Message to Pc DTR Keyer : %1").arg(cwMessageToTx));

        tslf->sendPcKeyerTxCwMessage(cwMessageToTx);
    }
    else
    {
        logMessage(QString("PcCwMessageKeyer Send Morse Message to radio, message is empty"));
    }

}

void  WinKeyerTxKeyer::sendCwFreeTextMsg(QString message)
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    QString cwMessageToTx = parseMacrosInMessage(tslf, message);

    logMessage(QString("Send Free Text Morse Message to PcCwKeyer : %1").arg(cwMessageToTx));

    tslf->sendPcKeyerTxCwMessage(cwMessageToTx);
}




QString WinKeyerTxKeyer::parseMacrosInMessage(TSingleLogFrame *tslf, QString mess)
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
            continue;
        }
        else if (c == '#')
        {
            txMess += serials;
            continue;
        }
        else if (c == '!')
        {
            txMess += call;
            continue;
        }
        else if (c == '%')
        {
            txMess += ct->myloc.getLoc();
            continue;
        }
        else if (c == '$')
        {
            txMess += reps;
            continue;
        }
        else if (c == '{')
        {
            int lb = mess.indexOf('}', i);
            if (lb != -1)
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
                    logMessage(QString("Message <%1> contains unknown macro {%2}").arg(mess, macro));
                }
            }

            continue;
        }
        else
        {
            txMess += c;
        }
    }
    return txMess;

}




void WinKeyerTxKeyer::stopCwMsg()
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    tslf->sendPcKeyerTxCwStop(STOP_CW_MESSAGE);
}



int WinKeyerTxKeyer::setup(TxKeyerFactory *txKeyerFactory, int &maxNumButtons, int &numButtons, QString selectedRadioName)
{

    Q_UNUSED(selectedRadioName)

    TxKeyerCapabilities voiceCap = txKeyerFactory->supportedTxKeyers()->value("pcCwKeyer");
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

    TxVmRigSetupDialog txVmSetupDialog(voiceCap, maxNumButtons, numButtons, tslf->dmKeyerContainer);

    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICE_KEYER_BASE_FILE_NAME + getTxKeyerTypeFromTxKeyerId(TxKeyerId::PcCwKeyer) + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    fileName = TX_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + getTxKeyerTypeFromTxKeyerId(TxKeyerId::PcCwKeyer) + ".ini";
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


void WinKeyerTxKeyer::setRadioParams(int radioMaxNumButtons_, QString selectedRadioName_, serialCommonData::MINOS_PTT_TYPES pttType_, bool pttEnabled_)
{
    selectedRadioName = selectedRadioName_;
    radioMaxNumButtons = radioMaxNumButtons_;
    //pttType = pttType_;
    //pttEnabled = pttEnabled_;
}


void WinKeyerTxKeyer::setCwMemType(int _cwMemType)
{
    cwMemType = _cwMemType;
}


void WinKeyerTxKeyer::setContest(BaseContestLog *c)
{
    ct = dynamic_cast<LoggerContestLog *>( c);

}

void WinKeyerTxKeyer::logMessage(QString msg)
{
    trace(QString("[TxVmButtonsFrame - PcCWMessageKeyer] %1").arg(msg));
}

