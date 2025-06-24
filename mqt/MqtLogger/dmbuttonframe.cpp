#include <QString>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>
#include <QPushButton>
#include <QFileDialog>
#include <QFileSystemWatcher>

#include "ContestApp.h"
#include "LoggerContest.h"
#include "MShowMessageDlg.h"
#include "MinosLoggerEvents.h"
#include "MinosParameters.h"

#include "MinosRPC.h"
#include "SendRPCDM.h"
#include "fileutils.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "MTrace.h"
#include "dmkeyseditdlg.h"

#include "dmbuttonframe.h"
#include "ui_dmbuttonframe.h"

DMButtonFrame::DMButtonFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::DMButtonFrame)
{
    ui->setupUi(this);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::fKey, this, &DMButtonFrame::fKey);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::SandPChanged, this, &DMButtonFrame::sandPChanged);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::DMMess, this, &DMButtonFrame::DMMess);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::modeChange, this, &DMButtonFrame::onModeChange);

    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpDigiFunctionKeyFile, fkeyFileName );

    txKeyer = nullptr;
    voiceKeyerType = keyerTypes[VoiceKeyerId::None];

    voiceKeyerFactory = new VoiceKeyerFactory(this);

    msgDurTimer = new QTimer(this);
    connect(msgDurTimer, &QTimer::timeout, this, &TxVmButtonsFrame::onMsgDurTimerTimeout);

    repeatPauseTimer = new QTimer(this);
    connect(repeatPauseTimer, &QTimer::timeout, this, &TxVmButtonsFrame::onRepeatPauseTimerTimeout);

    extKeyerConnectTimer = new QTimer(this);
    connect(extKeyerConnectTimer, &QTimer::timeout, this, &TxVmButtonsFrame::onExtConnectTimer);
    connect(LogContainer->sendDM, &TSendDM::keyerReport, this, &TxVmButtonsFrame::onExtConnectTimer);

    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICEKEYER_COMMON_PARAMS_FILENAME;
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup(VOICEKEYER_COMMON_PARAMS_GROUPNAME);

    QString voiceKeyerName = config.value("KeyerName").toString();

    voiceKeyerFactory->populateComboKeyerList(ui->voiceKeyerSelect, voiceKeyerName);
    connect(ui->voiceKeyerSelect, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TxVmButtonsFrame::onVoiceKeyerSelect);

    trace(QString("start keyer name = %1").arg(ui->voiceKeyerSelect->currentText()));

    onVoiceKeyerSelect(ui->voiceKeyerSelect->currentIndex());

    // If we haven't already done so, copy issue fkey file to a local copy
    // so that an installation can overwrite the original without losing
    // our changes.

    QSharedPointer<ProfileEntry> &dfkd = TContestApp::getContestApp() ->loggerBundle.bundleFile->GetKey( elpDigiFunctionKeyFile );

    if (dfkd->sdefaultval == fkeyFileName)
    {
        QString dstItemPath = ExtractFileDir(fkeyFileName);
        QString fname = ExtractFileName(fkeyFileName);
        QString dname = "my_" + fname;
        dstItemPath = dstItemPath + "/" + dname;
        if (!FileExists(dstItemPath))
        {
            QFile::copy(fkeyFileName, dstItemPath);
            fkeyFileName = dstItemPath;
            TContestApp::getContestApp() ->loggerBundle.setStringProfile( elpDigiFunctionKeyFile, fkeyFileName );
        }
    }
    fButtons << ui->F1Button << ui->F2Button << ui->F3Button << ui->F4Button << ui->F5Button << ui->F6Button;
    fButtons << ui->F7Button << ui->F8Button << ui->F9Button << ui->F10Button << ui->F11Button << ui->F12Button;

    int i = Qt::Key_F1;
    for (auto b: QASCONST(fButtons))
    {
        b->setProperty("KeyNo", i++);
        b->setText("");
        connect(b, &QPushButton::clicked, this, &DMButtonFrame::fButtonClicked);
    }

    ui->nameLabel->setText(tr("Data Modes Buttons from %1").arg(fkeyFileName));

    ui->FButtonFrame->setEnabled(false);

    ui->fkeysetCombo->addItem(currentName);

    fkeyFileChanged();
}

DMButtonFrame::~DMButtonFrame()
{
    delete ui;
}

bool  TxVmButtonsFrame::isVoiceMode()
{
    return  curMode == rigcommon::convertModeToQString(MODE::USB)
    || curMode == rigcommon::convertModeToQString(MODE::LSB)
        || curMode == rigcommon::convertModeToQString(MODE::FM)
        || curMode == PH;

}

void TxVmButtonsFrame::setPttTypeLabelsVisible(bool visible)
{
    ui->pttTypeLabel->setVisible(visible);
    ui->pttTypeText->setVisible(visible);
}

void TxVmButtonsFrame::setPttTypeText(serialCommonData::MINOS_PTT_TYPES pttType)
{

    ui->pttTypeText->setText(serialCommonData::pttTypeStr[static_cast<int>(pttType)]);
}


void TxVmButtonsFrame::onVmSetupClicked()
{

    if ((voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl] && !isVoiceMemAvail(selectedRadio))
        || (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl] && !isCwMemTypeAvail(selectedRadio)))
    {
        logMessage(QString("Setup Selected rigControl Keyer Selected, but not available for this radio or no keyer selected"));
        return;
    }


    if (voiceKeyerType != keyerTypes[VoiceKeyerId::None])
    {
        int oldnb = txKeyer->numButtons;


        int maxNumOfVoiceMessages = MAXIMUM_BUTTONS;

        if (voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl])
        {
            maxNumOfVoiceMessages = getNumVoiceMessages(selectedRadio);
        }



        if (txKeyer->setup(voiceKeyerFactory, maxNumOfVoiceMessages, txKeyer->numButtons, selectedRadio.getLocalName()) == QDialog::Accepted)
        {
            if (txKeyer->numButtons != oldnb)
            {
                int columns = 4;
                createButtonsForKeyer(txKeyer->numButtons, columns);

            }

            setEomLabelText(txKeyer->getSelectedEomType());

        }
    }


}


void TxVmButtonsFrame::logRadioSettingsChanged(QSharedPointer<RadioSettingsDialogChangeFlag> logRadioSettingsFlags)
{
    Q_UNUSED(logRadioSettingsFlags)

    if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl] || voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer] || voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl] || voiceKeyerType == keyerTypes[VoiceKeyerId::InternalVoiceKeyer])
    {
        setSaveButtonByRadionameText(selectedRadio.getLocalName());
        setRadioParams();

        if (voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
        {
            setEomTypeLabelsVisible(false);

            setPttTypeText(serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_NONE);
            setPttEnabledIndicatorOnOff(false);

        }
        else
        {
            setPttTypeText(getPttType(selectedRadio));
            setPttEnabledIndicatorOnOff(getPttEnabled(selectedRadio));
            setEomLabelText(txKeyer->getSelectedEomType());
        }



        txKeyer->voiceKeyerInit(txKeyer->numButtons);

        loadButtonData();
    }
}


// also PcCwKeyer
void TxVmButtonsFrame::setRadioParams()
{
    if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl])
    {
        txKeyer->setRadioParams(MAXIMUM_BUTTONS, selectedRadio.getLocalName(), getPttType(selectedRadio), getPttEnabled(selectedRadio));
    }
    else if (voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl])
    {
        txKeyer->setRadioParams(getNumVoiceMessages(selectedRadio), selectedRadio.getLocalName(), getPttType(selectedRadio), getPttEnabled(selectedRadio));
    }
    else if (voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {
        txKeyer->setRadioParams(PC_CW_KEYER_MAXIMUM_BUTTONS, selectedRadio.getLocalName(), serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_NONE, false);
    }
}

void TxVmButtonsFrame::createKeyer(QString voiceKeyerName)
{
    if (!voiceKeyerName.isEmpty())
    {


        VoiceKeyerCapabilities voiceCap = voiceKeyerFactory->supportedVoiceKeyers()->value(voiceKeyerName);
        voiceKeyerType = voiceCap.getKeyerType();

        if (voiceKeyerType != keyerTypes[VoiceKeyerId::None])
        {
            txKeyer = QSharedPointer<VoiceKeyerBase>(voiceKeyerFactory->createVoiceKeyer(voiceCap.getVmIdNum()));
            if (txKeyer)
            {
                logMessage(QString("Voice Keyer type selected = %1").arg(voiceCap.getKeyerName()));

                connect(txKeyer.data(), &VoiceKeyerBase::remoteConfigChanged, this, &TxVmButtonsFrame::onRemoteConfigChanged, Qt::UniqueConnection);
                connect(txKeyer.data(), &VoiceKeyerBase::remoteKeyerStopped, this, &TxVmButtonsFrame::onRemoteKeyerStopped, Qt::UniqueConnection);
                connect(txKeyer.data(), &VoiceKeyerBase::remoteKeyerStarted, this, &TxVmButtonsFrame::onRemoteKeyerStarted, Qt::UniqueConnection);
                connect(txKeyer.data(), &VoiceKeyerBase::internalVoiceMemoryKeyerPlayState, this, &TxVmButtonsFrame::onInternalVoiceMemoryPlayState);

                setRadioParams();
                txKeyer->voiceKeyerInit(txKeyer->numButtons);

                // create the buttons
                if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl]
                    || voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl]
                    || voiceKeyerType == keyerTypes[VoiceKeyerId::InternalVoiceKeyer])
                {

                    int columns = 4;
                    createButtonsForKeyer(txKeyer->numButtons, columns);
                }
                else if (voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
                {
                    int numButtons = 12;    // fixed at 12!
                    int columns = 6;
                    createButtonsForKeyer(numButtons, columns);
                }

                vmKeyParamList.clear();
                buttonNumSent = NO_VM_BUTTON_ON;



                if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl] || voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl])
                {
                    // convert to version 2 ini type
                    checkButtonIniFileVersion(voiceKeyerType);
                    checkCommonIniFileVersion(voiceKeyerType);
                }
                else
                {

                    txKeyer->voiceKeyerInit(txKeyer->numButtons);

                    for (int i = 0; i < voiceMemButtonList.count(); i++)
                    {
                        VoiceKeyerParams vmData;
                        if (vmData.getType().isEmpty())
                        {
                            vmData.setType(voiceKeyerType);
                        }

                        //vmData.setRigModel(getRigModel(selectedRadio));
                        txKeyer->readVmButtonParams(i, vmData);
                        vmKeyParamList.append(vmData);
                        setRunButtonText(i, vmData.getVmName());

                    }


                }



            }

        }

    }
}


void TxVmButtonsFrame::onExtConnectTimer()
{
    QString voiceKeyerName = ui->voiceKeyerSelect->currentText();

    VoiceKeyerCapabilities voiceCap = voiceKeyerFactory->supportedVoiceKeyers()->value(voiceKeyerName);
    voiceKeyerType = voiceCap.getKeyerType();

    if (LogContainer->sendDM->isKeyerLoaded())
    {
        notifyComboChange = false;
        voiceKeyerFactory->populateComboKeyerList(ui->voiceKeyerSelect, voiceKeyerName);
        notifyComboChange = true;
    }

    if (!txKeyer && voiceKeyerType == keyerTypes[VoiceKeyerId::ExternalVoiceKeyer])
    {
        createKeyer(voiceKeyerName);
        if (txKeyer)
        {
            ui->noExtKeyerLabel->clear();
            extKeyerConnectTimer->stop();
        }
        else
        {
            voiceKeyerType = keyerTypes[VoiceKeyerId::None];
        }
    }
    else if (txKeyer)
    {
        ui->noExtKeyerLabel->clear();
        extKeyerConnectTimer->stop();
    }
}


void TxVmButtonsFrame::onVoiceKeyerSelect(int idx)
{
    Q_UNUSED(idx)

    if (!notifyComboChange)
        return;

    QString voiceKeyerName = ui->voiceKeyerSelect->currentText();
    logMessage(QString("onVoiceKeyerSelect - keyer select name = %1").arg( ui->voiceKeyerSelect->currentText()));

    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICEKEYER_COMMON_PARAMS_FILENAME;
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup(VOICEKEYER_COMMON_PARAMS_GROUPNAME);

    config.setValue("KeyerName", voiceKeyerName);

    config.endGroup();

    txKeyer.clear();

    delayedAction(this, [=]{
        createKeyer(voiceKeyerName);
        setFrameState(voiceKeyerName);
    });


    extKeyerConnectTimer->start(1000);

    ui->voiceKeyerSelect->repaint();   // or the combo doesn't update

}


void TxVmButtonsFrame::updateFrameState()
{
    setFrameState(ui->voiceKeyerSelect->currentText());
}



void TxVmButtonsFrame::initCwTextEntryBox(QString radioManufacturer, QString fileName)
{
    ui->cwEntry->setVisible(true);
    ui->cwEntry->installEventFilter(this);

    //

    QString cwMacroCharList;
    bool cwMacroCharOk;
    if (getRigCWKeyerMacroCharacter(cwMacroCharList, radioManufacturer, CWKEYER_RADIO_COMMON_PARAMS_FILENAME))
    {
        cwMacroCharOk = true;
        trace(QString("[TxVmButtonsFrame] Retrieved CW Macro Chars %1 for manufacturer %2").arg(cwMacroCharList).arg(radioManufacturer));
    }
    else
    {
        cwMacroCharOk = false;
        trace(QString("[TxVmButtonsFrame] Error retrieving CW Macro Chars for manufacturer %1").arg(radioManufacturer));
    }



    QString validCharCwList;
    if (getRigCWKeyerSupportedCharacters(validCharCwList, radioManufacturer, CWKEYER_RADIO_COMMON_PARAMS_FILENAME))
    {
        if (cwMacroCharOk)
        {
            validCharCwList = validCharCwList.append(cwMacroCharList);
            trace(QString("[TxButtons Frame] Supported CW Chars and Macro chars %1 for manufacturer %2").arg(validCharCwList).arg(radioManufacturer));
        }
        else
        {

            trace(QString("[TxVmButtonsFrame] Supported CW Chars with no Macro chars %1 for manufacturer %2").arg(validCharCwList).arg(radioManufacturer));
        }

    }
    else
    {
        trace(QString("[TxVmButtonsFrame] Error retrieving supported CW Chars for manufacturer %1, no validator set").arg(radioManufacturer));
    }


    int maxNumChars = 0;
    if (getRigCWKeyerMaxMessageLength(maxNumChars, radioManufacturer, fileName))
    {


        trace(QString("[TxVmButtonsFrame] set max number of CW Chars = %1 for manufacturer %2").arg(maxNumChars).arg(radioManufacturer));
    }
    else
    {
        trace(QString("[TxVmButtonsFrame] Error retrieving max CW Message Length for manufacturer %1").arg(radioManufacturer));

    }
    // we need better error handling here!
    auto *validator = new CWRigKeyerValidator(this);
    validator->setValidCwCharStr(validCharCwList);
    validator->setMaxNumCwChars(maxNumChars);
    ui->cwEntry->setValidator(validator);


}


void TxVmButtonsFrame::onCwEntryReturnPressed()
{


    if  (txKeyer)
    {
        if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl] || voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
        {
            QString message = ui->cwEntry->text().trimmed();

            if (!message.isEmpty())
            {
                ui->cwEntry->selectAll();
                txKeyer->sendCwFreeTextMsg(message);
            }

        }

    }

}

void TxVmButtonsFrame::readActionSelected(int buttonNumber)
{
    if ((voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl] && !isVoiceMemAvail(selectedRadio))
        || (voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer] && !isPcCwKeyerLoaded())
        || (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl] && !isCwMemTypeAvail(selectedRadio)))
    {
        logMessage(QString("- readActionSelected rigControl Voice CW Keyer Selected, but not available for this radio or no keyer selected"));
        return;
    }
    logMessage(QString("- readActionSelected"));

    if (voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl])
    {
        if(!isVoiceMode())
        {
            logMessage(QString("Mode needs to be a phone type for rigcontrol Voice Message, current mode = %1").arg(curMode));
            return;
        }
    }

    if (buttonNumSent != NO_VM_BUTTON_ON)
    {
        onVmStopClicked();

    }

    startVMMsg(buttonNumber);
}


void TxVmButtonsFrame::startVMMsg(int buttonNumber)
{
    logMessage(QString("- startVMMsg button Number = %1").arg(buttonNumber));
    buttonNumSent = buttonNumber;
    if (buttonNumber >= voiceMemButtonList.count())
    {
        return;
    }

    selectedEomType = voiceKeyerCommon::VoiceCwKeyerEomTypes::Eom_None;

    VoiceKeyerParams vmData;

    if ((voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl]
         || voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl]
         || voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
        && !selectedRadio.getLocalName().isEmpty())
    {
        vmData.setSelRadioName(selectedRadio.getLocalName());
        vmData.setRigModel(getRigModel(selectedRadio));
        vmData.setSAndPState(sAndPState);

    }


    vmData.setType(voiceKeyerType);
    txKeyer->readVmButtonParams(buttonNumber, vmData);
    setRepeatIndicatorOnOff(vmData.getVmRepeatFlag());



    if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl]
        || voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {
        if (vmData.getVmCwMessage().isEmpty())
        {
            logMessage(QString("Cw Message is empty, ignore"));
            return;
        }


        if (getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::ICOM
            || getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::ELECRAFT
            || getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::KENWOOD
            || getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::YAESU
            || getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::FLEX_RADIO
            || getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::FLEX_RADIO_APACHE
            || getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::OPENHPSDR
            || getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::QRPLABS
            || getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::THETIS
            || voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
        {
            if (curMode != rigcommon::convertModeToQString(MODE::CW) && txKeyer->getSetCwModeAndRestoreFlag())
            {
                savedMode = curMode;
                sendModeToRadio(rigcommon::convertModeToQString(MODE::CW));
            }
            else
            {
                savedMode = curMode;        // keep current mode if CW
            }

            txKeyer->sendCwMsg(vmData);
            setMessagePlayingFlag(true);
        }

    }
    else
    {

        txKeyer->sendMsgNum(buttonNumSent);
        setMessagePlayingFlag(true);
    }

    selectedEomType = txKeyer->getSelectedEomType();

    if (selectedEomType == voiceKeyerCommon::VoiceCwKeyerEomTypes::Timer)
    {
        int msgDur = vmKeyParamList[buttonNumber].getVmDuration() * 1000;
        if (msgDur > 0)
        {
            logMessage(QString("- msgDurTimer->start(%1)").arg(msgDur));
            msgDurTimer->start(msgDur);
        }
    }

    txVmButtonMap[buttonNumber]->showButtonOnOff(true);

}


void TxVmButtonsFrame::onVmStopClicked()
{
    if (voiceKeyerType == keyerTypes[VoiceKeyerId::None])
    {
        return;
    }



    if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl]
        || voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {

        //if (getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::ICOM)
        // {
        txKeyer->stopCwMsg();

        if (curMode != savedMode && txKeyer->getSetCwModeAndRestoreFlag())       // restore mode?
        {
            sendModeToRadio(savedMode);
        }
        // }
    }
    else
    {
        txKeyer->stopMsg(nullptr);
    }

    msgDurTimer->stop();
    repeatPauseTimer->stop();
    if (buttonNumSent != NO_VM_BUTTON_ON && buttonNumSent <= txVmButtonMap.count())
    {
        TxVoiceMemButton *b = txVmButtonMap[buttonNumSent];
        if (b)
        {
            b->showButtonOnOff(false);
        }
    }
    setRepeatIndicatorOnOff(false);
    buttonNumSent = NO_VM_BUTTON_ON;
    setMessagePlayingFlag(false);

}


void TxVmButtonsFrame::onRemoteConfigChanged()
{
    bool s = txKeyer->getPip();
    if (ui->pipCb->isChecked() != s)
    {
        ui->pipCb->setChecked(s);
    }

    for (int i = 0; i < voiceMemButtonList.count(); i++)
    {
        VoiceKeyerParams vmData;
        if (vmData.getType().isEmpty())
        {
            vmData.setType(voiceKeyerType);
        }

        txKeyer->readVmButtonParams(i, vmData);
        vmKeyParamList[i] = vmData;
        setRunButtonText(i, vmData.getVmName());
    }
}
void TxVmButtonsFrame::onRemoteKeyerStarted(int key)
{
    onRemoteKeyerStopped();

    buttonNumSent = key;
    if (voiceKeyerType == keyerTypes[VoiceKeyerId::None] || buttonNumSent == NO_VM_BUTTON_ON)
    {
        return;
    }

    int msgDur = vmKeyParamList[buttonNumSent].getVmDuration() * 1000;
    if (msgDur > 0)
    {
        logMessage(QString("- msgDurTimer->start(%1)").arg(msgDur));
        msgDurTimer->start(msgDur);
    }
    TxVoiceMemButton *b = txVmButtonMap[buttonNumSent];
    if (b)
    {
        b->showButtonOnOff(true);
    }
}
void TxVmButtonsFrame::onRemoteKeyerStopped()
{
    if (voiceKeyerType == keyerTypes[VoiceKeyerId::None] || buttonNumSent == NO_VM_BUTTON_ON)
    {
        return;
    }
    //txKeyer->stopMsg();
    msgDurTimer->stop();
    repeatPauseTimer->stop();
    TxVoiceMemButton *b = txVmButtonMap[buttonNumSent];
    if (b)
    {
        b->showButtonOnOff(false);
    }
    buttonNumSent = NO_VM_BUTTON_ON;

}

void TxVmButtonsFrame::onMsgDurTimerTimeout()
{
    if (buttonNumSent >= 0)
    {
        if (vmKeyParamList[buttonNumSent].getVmDuration() > 0
            || selectedEomType == voiceKeyerCommon::VoiceCwKeyerEomTypes::CAT
            || txKeyer->getSelectedEomType() == voiceKeyerCommon::VoiceCwKeyerEomTypes::InternalSoundCardVoiceKeyer
            || selectedEomType == voiceKeyerCommon::VoiceCwKeyerEomTypes::DTRKeyerTXStatus)
        {
            if (voiceKeyerType == keyerTypes[VoiceKeyerId::InternalVoiceKeyer])
            {
                txKeyer->stopMsg(nullptr); // ensure the sbdriver is stopped
            }

            // message duration of zero means that there shouldn't be a message timer running
            if (vmKeyParamList[buttonNumSent].getVmRepeatFlag())
            {
                int repeatPauseDur = vmKeyParamList[buttonNumSent].getVmRepeatPauseDur() * 1000;
                repeatPauseTimer->start(repeatPauseDur);
            }
            else
            {
                turnOffVMButton();
            }
        }

    }


    msgDurTimer->stop();


    if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl]
        || voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {

        if ((getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::ICOM
             ||  getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::YAESU
             || getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::KENWOOD
             || getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::ELECRAFT
             || getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::FLEX_RADIO
             || getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::FLEX_RADIO_APACHE
             || getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::OPENHPSDR
             || getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::QRPLABS
             || getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::THETIS
             || voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
            && txKeyer->getSetCwModeAndRestoreFlag())
        {

            if (curMode != savedMode)       // restore mode?
            {
                sendModeToRadio(savedMode);
            }

        }

    }

}


void TxVmButtonsFrame::turnOffVMButton()
{
    TxVoiceMemButton *b = txVmButtonMap[buttonNumSent];
    if (b)
    {
        b->showButtonOnOff(false);
    }

    setRepeatIndicatorOnOff(false);
    buttonNumSent = NO_VM_BUTTON_ON;
}



void TxVmButtonsFrame::onRepeatPauseTimerTimeout()
{

    repeatPauseTimer->stop();

    if (txKeyer->doRepeatFromLogger())
    {
        if (buttonNumSent >= 0)
        {
            if (vmKeyParamList[buttonNumSent].getVmRepeatFlag())
            {
                logMessage("- onRepeatPauseTimerTimeout()");
                startVMMsg(buttonNumSent);

            }
            else
            {
                onVmStopClicked();
            }
        }
    }
}

void TxVmButtonsFrame::setRadioIsConnected(bool connected)
{
    radioConnected = connected;
}

void TxVmButtonsFrame::setSelectedRadio(PubSubName selectedRadio_)
{
    if (voiceKeyerType.isEmpty() || voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {
        // ignore as we don't want to update framestate
        return;
    }

    if (selectedRadio != selectedRadio_)
    {
        selectedRadio = selectedRadio_;

        updateFrameState();
    }
}

void TxVmButtonsFrame::setPttEnabled(bool state, PubSubName psn)
{
    if (voiceKeyerType.isEmpty() || voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {
        // ignore as we don't want to update framestate
        return;
    }


    logMessage(QString("setPttEnabled = %1, radio = %2").arg(state ? "Enabled" : "Disabled").arg(psn.getLocalName()));

    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        rd.setPttEnabled(state);
        allRadioDetails[psn] = rd;
    }
    else
    {
        rd.setPttEnabled (state);
        allRadioDetails[psn] = rd;
    }

    updateFrameState();
}

bool TxVmButtonsFrame::getPttEnabled(PubSubName psn)
{

    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        return rd.getPttEnabled();
    }


    return false;
}

void TxVmButtonsFrame::setPttType(int type, PubSubName psn)
{

    if (voiceKeyerType.isEmpty() || voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {
        // ignore as we don't want to update framestate
        return;
    }

    logMessage(QString("setPttType = %1, selectedRadio = %2").arg(type).arg(psn.getLocalName()));

    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        rd.setPttType(type);
        allRadioDetails[psn] = rd;
    }
    else
    {
        rd.setPttType(type);
        allRadioDetails[psn] = rd;
    }

    updateFrameState();
}

serialCommonData::MINOS_PTT_TYPES TxVmButtonsFrame::getPttType(PubSubName psn)
{

    // convert int back to MINOS_PTT_TYPES

    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        return static_cast<serialCommonData::MINOS_PTT_TYPES>(rd.getPttType());
    }


    return serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_NONE;

}

void TxVmButtonsFrame::setVoiceMemAvail(bool avail, PubSubName psn)
{
    if (voiceKeyerType.isEmpty() || voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {
        // ignore as we don't want to update framestate
        return;
    }
    logMessage(QString("setVoiceMemAvail = %1, radio = %2").arg(avail ? "Yes" : "No", psn.getLocalName()));

    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        rd.setVoiceMemAvail(avail);
        allRadioDetails[psn] = rd;
    }
    else
    {
        rd.setVoiceMemAvail(avail);
        allRadioDetails[psn] = rd;
    }

    updateFrameState();
}

bool TxVmButtonsFrame::isVoiceMemAvail(PubSubName psn)
{
    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        return rd.getVoiceMemAvail();
    }

    return false;
}

void TxVmButtonsFrame::setNumVoiceMessages(int numMsgs, PubSubName psn)
{

    if (voiceKeyerType.isEmpty() || voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {
        // ignore as we don't want to update framestate
        return;
    }

    logMessage(QString("setNumVoiceMessages = %1, radio = %2").arg(QString::number(numMsgs), psn.getLocalName()));

    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        rd.setNumVoiceMessages(numMsgs);
        allRadioDetails[psn] = rd;
    }
    else
    {
        rd.setNumVoiceMessages(numMsgs);
        allRadioDetails[psn] = rd;
    }

    updateFrameState();
}

// This is max number of voice messages available on a radio
int TxVmButtonsFrame::getNumVoiceMessages(PubSubName psn)
{

    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        return rd.getNumVoiceMessages();
    }
    else
    {
        return MAXIMUM_BUTTONS;
    }
}

void TxVmButtonsFrame::setRigVoiceKeyerSupportStopFlag(bool supportStopCmd, PubSubName psn)
{

    if (voiceKeyerType.isEmpty() || voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {
        // ignore as we don't want to update framestate
        return;
    }

    logMessage(QString("setRigVoiceKeyerSupportStopFlag = %1, radio = %2").arg(supportStopCmd ? "Yes" : "No").arg(psn.getLocalName()));

    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        rd.setRigVoiceKeyerSupportStopCmd(supportStopCmd);
        allRadioDetails[psn] = rd;
    }
    else
    {
        rd.setRigVoiceKeyerSupportStopCmd(supportStopCmd);
        allRadioDetails[psn] = rd;
    }

    updateFrameState();
}


bool TxVmButtonsFrame::getRigVoiceKeyerSupportStopFlag(PubSubName psn)
{
    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        return rd.getRigVoiceKeyerSupportStopCmd();
    }

    return true;

}


void TxVmButtonsFrame::setRigCwKeyerSupportStopFlag(bool supportStopCmd, PubSubName psn)
{
    if (voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {
        // ignore as we don't want to update framestate
        return;
    }


    logMessage(QString("setRigCwKeyerSupportStopFlag = %1, radio = %2").arg(supportStopCmd ? "Yes" : "No").arg(psn.getLocalName()));


    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        rd.setRigCwKeyerSupportStopCmd(supportStopCmd);
        allRadioDetails[psn] = rd;
    }
    else
    {
        rd.setRigCwKeyerSupportStopCmd(supportStopCmd);
        allRadioDetails[psn] = rd;
    }

    updateFrameState();
}


bool TxVmButtonsFrame::getRigCwKeyerSupportStopFlag(PubSubName psn)
{
    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        return rd.getRigCwKeyerSupportStopCmd();
    }

    return true;

}

void TxVmButtonsFrame::setRigModel(QString rigModel, PubSubName psn)
{

    if (voiceKeyerType.isEmpty() ||voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {
        // ignore as we don't want to update framestate
        return;
    }

    logMessage(QString("setRigModel = %1, radio = %2").arg(rigModel).arg(psn.getLocalName()));


    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        rd.setRigModel(rigModel);
        allRadioDetails[psn] = rd;
    }
    else
    {
        rd.setRigModel(rigModel);
        allRadioDetails[psn] = rd;
    }

    updateFrameState();
}

QString TxVmButtonsFrame::getRigModel(PubSubName psn)
{
    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        return rd.getRigModel();
    }

    return "";

}



bool TxVmButtonsFrame::isCwMemTypeAvail(PubSubName psn)
{
    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        if (rd.getCwMemType() == hamlibData::CW_MEMORY_TYPES::KENWOOD
            || rd.getCwMemType() == hamlibData::CW_MEMORY_TYPES::YAESU
            || rd.getCwMemType() == hamlibData::CW_MEMORY_TYPES::ICOM
            || rd.getCwMemType() == hamlibData::CW_MEMORY_TYPES::ELECRAFT
            || rd.getCwMemType() == hamlibData::CW_MEMORY_TYPES::FLEX_RADIO
            || rd.getCwMemType() == hamlibData::CW_MEMORY_TYPES::FLEX_RADIO_APACHE
            || rd.getCwMemType() == hamlibData::CW_MEMORY_TYPES::OPENHPSDR
            || rd.getCwMemType() == hamlibData::CW_MEMORY_TYPES::QRPLABS
            || rd.getCwMemType() == hamlibData::CW_MEMORY_TYPES::THETIS)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return false;
}


void TxVmButtonsFrame::setCwMemType(int cwMemType, PubSubName psn)
{
    if (voiceKeyerType.isEmpty() || voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {
        // ignore as we don't want to update framestate
        return;
    }

    logMessage(QString("setCwMemType = %1, radio = %2").arg(cwMemType).arg(psn.getLocalName()));


    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        rd.setCwMemType(cwMemType);
        allRadioDetails[psn] = rd;
    }
    else
    {
        rd.setCwMemType(cwMemType);
        allRadioDetails[psn] = rd;
    }

    updateFrameState();
}

int TxVmButtonsFrame::getCwMemType(PubSubName psn)
{
    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        return rd.getCwMemType();
    }

    return hamlibData::CW_MEMORY_TYPES::NONE;
}


void TxVmButtonsFrame::setCwEntryBoxVisible(bool visible)
{
    ui->cwEntry->setVisible(visible);
    ui->cwEntryLabel->setVisible(visible);
}

void TxVmButtonsFrame::setAvailIndicatorVisible(bool visible)
{

    ui->availLabel->setVisible(visible);
    ui->availIndicator->setVisible(visible);
    ui->vmAvailIndicatorLine->setVisible(visible);

}

void TxVmButtonsFrame:: setAvailIndicatorOnOffForPcCwKeyer()
{

    if (isPcCwKeyerLoaded() && isPcCwKeyerConnected())
    {
        setAvailIndicatorOnOff(true);
    }
    else
    {
        setAvailIndicatorOnOff(false);
    }
}

void TxVmButtonsFrame::setMessagePlayingFlag(bool playing)
{
    messagePlaying = playing;
}

bool TxVmButtonsFrame::isMessagePlaying()
{
    return messagePlaying;
}

bool TxVmButtonsFrame::isPcCwKeyerLoaded()
{
    return LogContainer->sendDM->isPcCWkeyerLoaded();
}

bool TxVmButtonsFrame::isPcCwKeyerConnected()
{
    return LogContainer->sendDM->isPcCwKeyerConnected();
}

void TxVmButtonsFrame::setAvailIndicatorOnOff(bool on)
{
    if (on)
    {
        ui->availIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);
    }
    else
    {
        ui->availIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);

    }

    if (voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl])
    {
        ui->availIndicator->setToolTip(tr("Rig Voice Keyer Available"));
    }
    else if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl])
    {
        ui->availIndicator->setToolTip(tr("Rig CW Messages Available"));
    }
    else
    {
        ui->availIndicator->setToolTip(tr(""));
    }
}

void TxVmButtonsFrame::setAvailIndicatorForRadioOnOff(PubSubName radName)
{
    if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl] || voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl])
    {
        if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl])
        {
            setAvailIndicatorOnOff(isCwMemTypeAvail(radName));
        }
        else if (voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl])
        {
            setAvailIndicatorOnOff(isVoiceMemAvail(radName));
        }
        else
        {
            setAvailIndicatorOnOff(false);
        }
    }

}

void TxVmButtonsFrame::setRepeatIndicatorVisible(bool visible)
{
    ui->repeatIndicator->setVisible(visible);
    ui->repeatLabel->setVisible(visible);
}

void TxVmButtonsFrame::setTXStatusVisible(bool visible)
{
    ui->txStatusIndicator->setVisible(visible);
    ui->txStatusLabel->setVisible(visible);
}

void TxVmButtonsFrame::setRepeatIndicatorForMessageOnOff(bool state)
{
    if (state)
    {
        ui->repeatIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);
    }
    else
    {
        ui->repeatIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);
    }
}


void TxVmButtonsFrame::setRepeatIndicatorOnOff(bool on)
{
    if (on)
    {
        ui->repeatIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);
    }
    else
    {
        ui->repeatIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);

    }

}

void TxVmButtonsFrame::setKeyerIndicatorGroupBoxVisible(bool visible)
{
    ui->keyerIndicatorGroupBox->setVisible(visible);
}

void TxVmButtonsFrame::setPttIndicatorGroupBoxVisible(bool visible)
{
    ui->pttIndicatorGroupBox->setVisible(visible);
}

void TxVmButtonsFrame::setErrorMessageVisible(bool visible)
{
    ui->errorTitleLabel->setVisible(visible);
    ui->errorMeassageLabel->setVisible(visible);
}

void TxVmButtonsFrame::setRadioPttState(bool state)
{
    if (voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {
        // ignore as we don't want to update framestate
        return;
    }

    if (pttState != state)
    {
        pttState = state;
        setPttStatusIndicatorOnOff(state);
        pttStopMessage(state);
        emit pttStatus(pttState);
    }

}


void TxVmButtonsFrame::pttStopMessage(bool state)
{
    logMessage(QString("pttStopMessage state = %1").arg(state ? "true" : "false"));

    if (selectedEomType == voiceKeyerCommon::VoiceCwKeyerEomTypes::CAT
        || selectedEomType == voiceKeyerCommon::VoiceCwKeyerEomTypes::DTRKeyerTXStatus)
    {
        logMessage(QString("Using PTT or DTR Keyer TX Status for EOM Flag"));
        if (txKeyer && txKeyer->doRepeatFromLogger() && !state)
        {
            onMsgDurTimerTimeout();
        }
    }
    else
    {
        logMessage(QString("Ignoring PTT for EOM Flag"));
    }


}

void TxVmButtonsFrame::on_pipCb_stateChanged(int /*arg1*/)
{
    txKeyer->setPip(ui->pipCb->isChecked());
}

void TxVmButtonsFrame::setPttStatusIndicatorOnOff(bool on)
{
    if (on)
    {
        ui->txStatusIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);
        ui->txStatusIndicator->setToolTip(tr("TX On"));

    }
    else
    {
        ui->txStatusIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);
        ui->txStatusIndicator->setToolTip(tr("TX Off"));
    }

}

void TxVmButtonsFrame::setPttEnabledIndicatorOnOff(bool on)
{
    if (on)
    {
        ui->pttEnabledIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);
        ui->pttEnabledIndicator->setToolTip(tr("PTT Enabled"));

    }
    else
    {
        ui->pttEnabledIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);
        ui->pttEnabledIndicator->setToolTip(tr("PTT Disabled"));
    }

}

void TxVmButtonsFrame::setEomTypeLabelsVisible(bool visible)
{
    ui->eomLabel->setVisible(visible);
    ui->eomText->setVisible(visible);
}

void TxVmButtonsFrame::setEomLabelText(int selectedEomType)
{
    if (selectedEomType == voiceKeyerCommon::VoiceCwKeyerEomTypes::CAT)
    {
        ui->eomText->setText("CAT");
    }
    else if (selectedEomType == voiceKeyerCommon::VoiceCwKeyerEomTypes::Timer)
    {
        ui->eomText->setText("Timer");
    }
    else if (selectedEomType == voiceKeyerCommon::VoiceCwKeyerEomTypes::InternalSoundCardVoiceKeyer)
    {
        ui->eomText->setText("EOF");
    }
    else if (selectedEomType == voiceKeyerCommon::VoiceCwKeyerEomTypes::Eom_None)
    {
        ui->eomText->setText("None");
    }
}


void TxVmButtonsFrame::sandPChanged(bool s)
{
    if (txKeyer)
    {

        if (s != sAndPState)
        {
            sAndPState = s;
            loadButtonData();
        }


        if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl])
        {
            if (s)
            {
                ui->sAndPLabel->setText("| S&P");
            }
            else
            {
                ui->sAndPLabel->setText("| Run");
            }
        }
        else
        {
            ui->sAndPLabel->setText("");
        }
    }

}


bool TxVmButtonsFrame::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        // -- Only handle Return if it's from cwEntry --
        if (obj == ui->cwEntry &&
            (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter))
        {
            if (!keyEvent->isAutoRepeat())
                onCwEntryReturnPressed();

            return true; // consume
        }

        // -- Handle Escape globally --
        if (keyEvent->key() == Qt::Key_Escape)
        {
            if (isMessagePlaying())
            {
                onVmStopClicked();
                return true;                // consume Esc
            }
            // else let Esc propagate
        }
    }

    return QWidget::eventFilter(obj, event); // default
}





void DMButtonFrame::setFreq(Frequency freq)
{
    curFreq = freq;
}
void DMButtonFrame::DMMess(AnalysePubSubNotify an)
{
    if (an.getKey() == rpcConstants::DMSender)
    {
        dataSender = an.getPublisherProgram() + "@" + an.getPublisherRouter();

        trace(QString("Datasender set to %1").arg(dataSender));

        fkeyFileChanged();
        qfsw = new QFileSystemWatcher(this);
        qfsw->addPath(fkeyFileName);
        connect(qfsw, &QFileSystemWatcher::fileChanged, this, &DMButtonFrame::fkeyFileChanged);
    }
}

void DMButtonFrame::onModeChange(QString mode)
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    if (tslf)
    {
        curMode = mode;
        MinosRPC *rpc = MinosRPC::getMinosRPC();
        rpc->publish( rpcConstants::DMCat, rpcConstants::DMMode, mode, psPublished );

        emit sendFreqControl(curFreq);
    }
}
void DMButtonFrame::fkeyFileChanged()
{
    parseFKeyFile(fkeyFileName);

    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    if (tslf)
    {
        bool sandp = tslf->GJVQSOLogFrame->getSandP();
        showFButtons(sandp);
    }
}
void DMButtonFrame::fButtonClicked()
{
    QPushButton *b = dynamic_cast<QPushButton *>(sender());
    int kno = b->property("KeyNo").toInt();
    fKey(ct, kno, 0);
}
void DMButtonFrame::setContest(BaseContestLog *c)
{
    ct = dynamic_cast<LoggerContestLog *>( c);
    if (ct)
    {
        currentName = ct->currentFKeySet.getValue();
        ui->fkeysetCombo->setCurrentText(currentName);
        QString mode = ct->currentMode.getValue();
        onModeChange(mode);
    }
}
bool  DMButtonFrame::isDataMode()
{
    return  curMode == PSK
           || curMode == RY;

}void DMButtonFrame::fKey(BaseContestLog *c, int key, int carr)
{
    if (c && c == ct && isDataMode())
    {
        if (key >= Qt::Key_F1 && key <= Qt::Key_F12 && fkeys[currentName].size() == 24)
        {
            TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
            int spoffset = tslf->GJVQSOLogFrame->getSandP()?12:0;
            KeyVal mess = fkeys[currentName][key - Qt::Key_F1 + spoffset];

            QString toSend = parseFKeyMessage(mess.kval);

            // send transmission to sender app

            RPCGeneralClient rpc(rpcConstants::DMTransmit);
            QSharedPointer<RPCParam>st(new RPCParamStruct);
            st->addMember( toSend, rpcConstants::DMTransmit );
            st->addMember(carr, rpcConstants::DMMarkFreq);
            rpc.getCallArgs() ->addParam( st );
            rpc.queueCall( dataSender );

        }
    }
}

void DMButtonFrame::sandPChanged(bool s)
{
    showFButtons(s);
}
void DMButtonFrame::showFButtons(bool s)
{
    ui->FButtonFrame->setEnabled(false);
    MinosRPC *rpc = MinosRPC::getMinosRPC();

    if (fkeys[currentName].size() == 24)
    {
        for (int i = 0; i < 12; i++)
        {
            QString keytop = QString("F%1: %2").arg(i + 1).arg(fkeys[currentName][i + (s?12:0)].ktop);

            fButtons[i]->setText(keytop);
        }
        ui->FButtonFrame->setEnabled(true);

        QString fkeystring = getFKeysString();

        rpc->publish( rpcConstants::DMCat, rpcConstants::DMFKeys, fkeystring, psPublished );

    }
    else if (fkeys[currentName].size() == 0)
    {
        for (int i = 0; i < 12; i++)
        {
            fButtons[i]->setText(QString("F%1").arg(i, 1));
        }
        rpc->publish( rpcConstants::DMCat, rpcConstants::DMFKeys, "", psRevoked );
    }
    else {
        mShowMessage(tr("Not enough key definitions in %1").arg(fkeyFileName), this);
        rpc->publish( rpcConstants::DMCat, rpcConstants::DMFKeys, "", psRevoked );
    }
}
QString DMButtonFrame::parseFKeyMessage(QString mess)
{
    // make sure screenContact is up to date
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
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
                QString macro = mess.mid(i + 1, lb - i - 1).toUpper();
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
void DMButtonFrame::parseFKeyFile(QString fname)
{
    fkeys.clear();
    ui->fkeysetCombo->clear();
    nameList.clear();

    QFile lf(fname);

    if (!lf.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QString ebuff = QString( "Failed to open Function Key file %1" ).arg(fname );
        MinosParameters::getMinosParameters() ->mshowMessage( ebuff );
        return;
    }
    bool retval = false;

    QString s = lf.readAll();
    retval = parseFKeyString(s);
    if (retval == false)
    {
        mShowMessage(tr("Invalid or missing FKey definitions"), this);
    }
    else
    {
        ui->fkeysetCombo->clear();
        ui->fkeysetCombo->addItems(nameList);
        ui->fkeysetCombo->setCurrentText(currentName);
    }
}
bool DMButtonFrame::parseFKeyArray(QJsonArray s, QString keyset)
{
    KeySet &ks = fkeys[keyset];
    for (const auto &v:QASCONST(s))
    {
        if (v.isArray())
        {
            QJsonArray a = v.toArray();
            if (a.size() == 3)
            {
                QString fk = a[0].toString();
                QString keytop = a[1].toString();
                QString val = a[2].toString();

                KeyVal p;
                p.fk = fk;
                p.ktop = keytop;
                p.kval = val;
                ks.append(p);
            }
        }
    }
    return true;
}

bool DMButtonFrame::parseFKeyString(QString s)
{
    QJsonParseError err;
    QJsonDocument json = QJsonDocument::fromJson(s.toUtf8(), &err);
    if (err.error)
    {
        return false;
    }
    else
    {
        if( json.isArray())
        {
            QJsonArray namearray = json.array();
            for (auto const &n: QASCONST(namearray))
            {
                QJsonObject namestruct = n.toObject();
                QString name = namestruct.value("Name").toString();
                nameList.push_back(name);

                QJsonArray run = namestruct.value("Run").toArray();

                if (!parseFKeyArray(run, name) )
                {
                    // always returns true
                }

                QJsonArray sandp = namestruct.value("SandP").toArray();
                if (!parseFKeyArray(sandp, name) )
                {
                    // always returns true
                }
            }
        }
    }
    return true;
}
void DMButtonFrame::rewriteFKeyFile()
{
    QJsonDocument json;

    QJsonArray keys;
    for (Keys::const_iterator i = fkeys.constBegin();
         i != fkeys.constEnd(); i++)
    {
        QString setName = i.key();

        const KeySet &eles = i.value();

        QJsonArray korun;
        QJsonArray kosp;
        {
            for (int i = 0; i < 12; i++)
            {
                const KeyVal &k = eles[i];

                QJsonArray kor;
                kor.append(QJsonValue(k.fk));
                kor.append(QJsonValue(k.ktop));
                kor.append(QJsonValue(k.kval));
                korun.append(kor);
            }
        }
        {
            for (int i = 12; i < 24; i++)
            {
                const KeyVal &k = eles[i];

                QJsonArray ksp;
                ksp.append(QJsonValue(k.fk));
                ksp.append(QJsonValue(k.ktop));
                ksp.append(QJsonValue(k.kval));
                kosp.append(ksp);
            }
        }

        QJsonObject ks;
        ks.insert("Name", setName);
        ks.insert("Run", korun);
        ks.insert("SandP", kosp);


        keys.append(ks);

     }
    json.setArray(keys);

     QByteArray s = json.toJson(QJsonDocument::Indented);

    QFile jf(fkeyFileName);
    if (!jf.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        trace("Failed to open " +  fkeyFileName);
        return;
    }
    jf.write(s);

    jf.close();

}
void DMButtonFrame::on_stopButton_clicked()
{
    // send stop transmission to sender app
    RPCGeneralClient rpc(rpcConstants::DMStopTransmit);
    rpc.queueCall( dataSender );
}

void DMButtonFrame::on_editButton_clicked()
{
    // bring up default file editor on "fkeyFileName"

    //   QDesktopServices::openUrl(QUrl::fromLocalFile(fkeyFileName));

    // Use built in fKey editor on "fkeyFileName"

    Keys nfk = fkeys;
    DMKeysEditDlg jed(this, fkeyFileName, currentName, nfk);
    if (jed.exec() == QDialog::Accepted)
    {
        fkeys = nfk;
        // and we have to regenerate the JSON file
        rewriteFKeyFile();
    }
}

void DMButtonFrame::on_logitButton_clicked()
{
    // simulate "Enter" key
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    tslf->GJVQSOLogFrame->doGJVOKButton_clicked();
}

void DMButtonFrame::on_wipeButton_clicked()
{
    // wipe QSO - like ESC key
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    tslf->GJVQSOLogFrame->doGJVCancelButton_clicked();
}


void DMButtonFrame::on_chooseButton_clicked()
{
    // choose which file to load from
    QString lastf = fkeyFileName;

    QString InitialDir = fkeyFileName;

    QFileInfo qf(InitialDir);

    InitialDir = qf.canonicalFilePath();
    QString Filter = tr("Function Key Definitions Files (*.json);;"
                     "All Files (*.*)") ;

    QString fName = QFileDialog::getOpenFileName( this,
                       tr("Open Function Key Definitions File"),
                       InitialDir,
                       Filter
                       );

    if (!fName.isEmpty())
    {
        fkeyFileName = fName;
        TContestApp::getContestApp() ->loggerBundle.setStringProfile( elpDigiFunctionKeyFile, fkeyFileName );

        ui->nameLabel->setText(tr("Data Modes Buttons from %1").arg(fkeyFileName));

        if (qfsw)
        {
            qfsw->removePath(lastf);
        }
        else
        {
            qfsw = new QFileSystemWatcher(this);
            connect(qfsw, &QFileSystemWatcher::fileChanged, this, &DMButtonFrame::fkeyFileChanged);
        }

        fkeyFileChanged();
        qfsw->addPath(fkeyFileName);
    }
}

QString DMButtonFrame::getFKeysString() const
{
    QJsonArray ja;
    for (int i = 0; i < 12; i++)
    {
        QString val = fButtons[i]->text();

        QJsonObject jv;
        jv.insert(QString("F%1").arg(i + 1), val);

        ja.append(jv);
    }
    QJsonDocument json(ja);

    QString message(json.toJson(QJsonDocument::Compact));
    return message;

}

void DMButtonFrame::on_fkeysetCombo_textActivated(const QString &arg1)
{
    currentName = arg1;
    ct->currentFKeySet.setValue(currentName);
    ct->commonSave(false);
}

