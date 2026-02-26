#include <QMessageBox>

#include "cwspeedcontrol.h"
#include "tlogcontainer.h"
#include "delayedaction.h"
#include "SendRPCDM.h"
#include "cutils.h"
#include "MinosLoggerEvents.h"
#include "MTrace.h"
#include "contest.h"

#include "rigcommon.h"
#include "serialCommonData.h"
#include "txvmbuttonsframe.h"
#include "ui_txvmbuttonsframe.h"
#include "pccwmessagekeyer.h"
#include "cwrigkeyervalidator.h"


const char * VM_BUTTON_ON_STYLE = "QToolButton { background-color: orange; }"
                                     "QToolButton::menu-indicator { image: none; }";


const char * VM_BUTTON_OFF_STYLE = "QToolButton { background-color: white; }"
                                     "QToolButton::menu-indicator { image: none; }";

const int NO_VM_BUTTON_ON = -1;
const int CW_FREE_TEXT_BUTTON_NUMBER = 13;

const int ERROR_MSG_TIMEOUT_DURATION = 20000;

const QStringList vmButtonShortCutKeys = {
                                    "Shift+F1", "Shift+F2",
                                    "Shift+F3", "Shift+F4",
                                    "Shift+F5", "Shift+F6",
                                    "Shift+F7", "Shift+F8",
                                    "Shift+F9", "Shift+F10",
                                    "Shift+F11", "Shift+F12",
                                };

//const char * vmStopButtonShortCutKey = "Shift+F10";


TxVmButtonsFrame::TxVmButtonsFrame(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::TxVmButtonsFrame),
    buttonNumSent(NO_VM_BUTTON_ON),
    radioConnected(false),
    pttState(false)

{
    ui->setupUi(this);

    txVoiceKeyer = nullptr;
    voiceKeyerType = keyerTypes[VoiceKeyerId::None];

    voiceKeyerFactory = new VoiceKeyerFactory(this);

    msgDurTimer = new QTimer(this);
    connect(msgDurTimer, &QTimer::timeout, this, &TxVmButtonsFrame::onMsgDurTimerTimeout);

    repeatPauseTimer = new QTimer(this);
    connect(repeatPauseTimer, &QTimer::timeout, this, &TxVmButtonsFrame::onRepeatPauseTimerTimeout);

    extKeyerConnectTimer = new QTimer(this);
    connect(extKeyerConnectTimer, &QTimer::timeout, this, &TxVmButtonsFrame::onExtConnectTimer);
    connect(LogContainer->sendDM, &TSendDM::keyerReport, this, &TxVmButtonsFrame::onExtConnectTimer);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::fKey, this, &TxVmButtonsFrame::fKey);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::SandPChanged, this, &TxVmButtonsFrame::sandPChanged);
    //connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::modeChange, this, &TxVmButtonsFrame::onModeChange);


    ui->buttonGridContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);


    gridLayout = qobject_cast<QGridLayout *>(ui->buttonGridContainer->layout());
    if (!gridLayout)
    {
        // Defensive fallback in case layout was not set
        gridLayout = new QGridLayout(ui->buttonGridContainer);
        ui->buttonGridContainer->setLayout(gridLayout);
    }

    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICEKEYER_COMMON_PARAMS_FILENAME;
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup(VOICEKEYER_COMMON_PARAMS_GROUPNAME);

    QString voiceKeyerName = config.value("KeyerName").toString();

    config.endGroup();

    connect(ui->vmSetupPb, &QPushButton::clicked, this, &TxVmButtonsFrame::onVmSetupClicked);
    connect(ui->vmStopPb, &QPushButton::clicked, this, &TxVmButtonsFrame::onVmStopClicked);

    voiceKeyerFactory->populateComboKeyerList(ui->voiceKeyerSelect, voiceKeyerName);
    connect(ui->voiceKeyerSelect, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TxVmButtonsFrame::onVoiceKeyerSelect);

    trace(QString("start keyer name = %1").arg(ui->voiceKeyerSelect->currentText()));

    onVoiceKeyerSelect(ui->voiceKeyerSelect->currentIndex());

    qApp->installEventFilter(this); // look for esc key to stop playing




}

TxVmButtonsFrame::~TxVmButtonsFrame()
{
    delete ui;

    for(const auto &b: QASCONST(txVmButtonMap))
    {
        delete b;
    }
}


// create the tool buttons
void TxVmButtonsFrame::createButtonsForKeyer(int numButtons, int columns)
{
    clearButtons(); // Ensure old buttons are gone
    voiceMemButtonList.clear();
    txVmButtonMap.clear();

    for (int i = 0; i < numButtons; ++i)
    {
        QToolButton *toolButton = new QToolButton();
        toolButton->setText(QString("VM %1").arg(i + 1));
        toolButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        gridLayout->addWidget(toolButton, i / columns, i % columns);
        voiceMemButtonList.append(toolButton);
        txVmButtonMap[i] = new TxVoiceMemButton(voiceMemButtonList[i], this, i);

    }


}


void TxVmButtonsFrame::clearButtons()
{
    QLayoutItem *child;
    while ((child = gridLayout->takeAt(0)) != nullptr)
    {
        if (child->widget())
        {
            delete child->widget(); // delete button
        }
        delete child; // delete layout item
    }
}




bool  TxVmButtonsFrame::isVoiceMode()
{
    if (curMode.isEmpty())
    {
        curMode = ct->currentMode.getValue();
    }
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
        int oldnb = txVoiceKeyer->numButtons;


        int maxNumOfVoiceMessages = MAXIMUM_BUTTONS;

        if (voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl])
        {
           maxNumOfVoiceMessages = getNumVoiceMessages(selectedRadio);
        }



        if (txVoiceKeyer->setup(voiceKeyerFactory, maxNumOfVoiceMessages, txVoiceKeyer->numButtons, selectedRadio.getLocalName()) == QDialog::Accepted)
        {
            if (txVoiceKeyer->numButtons != oldnb)
            {
                int columns = 4;
                createButtonsForKeyer(txVoiceKeyer->numButtons, columns);

            }

            setEomLabelText(txVoiceKeyer->getSelectedEomType());

        }
    }


}


void TxVmButtonsFrame::logRadioSettingsChanged(QSharedPointer<RadioSettingsDialogChangeFlag> logRadioSettingsFlags)
{
    trace("TxVmButtonsFrame::logRadioSettingsChanged");
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
            setEomLabelText(txVoiceKeyer->getSelectedEomType());
        }



        txVoiceKeyer->voiceKeyerInit(txVoiceKeyer->numButtons);

        loadButtonData();
    }
}

// also PcCwKeyer
void TxVmButtonsFrame::setRadioParams()
{
    if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl])
    {
        txVoiceKeyer->setRadioParams(MAXIMUM_BUTTONS, selectedRadio.getLocalName(), getPttType(selectedRadio), getPttEnabled(selectedRadio));
    }
    else if (voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl])
    {
        txVoiceKeyer->setRadioParams(getNumVoiceMessages(selectedRadio), selectedRadio.getLocalName(), getPttType(selectedRadio), getPttEnabled(selectedRadio));
    }
    else if (voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {
        txVoiceKeyer->setRadioParams(PC_CW_KEYER_MAXIMUM_BUTTONS, selectedRadio.getLocalName(), serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_NONE, false);
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
            txVoiceKeyer = QSharedPointer<VoiceKeyerBase>(voiceKeyerFactory->createVoiceKeyer(voiceCap.getVmIdNum()));
            if (txVoiceKeyer)
            {
                logMessage(QString("Voice Keyer type selected = %1").arg(voiceCap.getKeyerName()));

                connect(txVoiceKeyer.data(), &VoiceKeyerBase::remoteConfigChanged, this, &TxVmButtonsFrame::onRemoteConfigChanged, Qt::UniqueConnection);
                connect(txVoiceKeyer.data(), &VoiceKeyerBase::remoteKeyerStopped, this, &TxVmButtonsFrame::onRemoteKeyerStopped, Qt::UniqueConnection);
                connect(txVoiceKeyer.data(), &VoiceKeyerBase::remoteKeyerStarted, this, &TxVmButtonsFrame::onRemoteKeyerStarted, Qt::UniqueConnection);
                connect(txVoiceKeyer.data(), &VoiceKeyerBase::internalVoiceMemoryKeyerPlayState, this, &TxVmButtonsFrame::onInternalVoiceMemoryPlayState);
                connect(txVoiceKeyer.data(), &VoiceKeyerBase::cwMacroExpandedText, this, &TxVmButtonsFrame::onCwMacroTextProcessed);

                setRadioParams();
                txVoiceKeyer->voiceKeyerInit(txVoiceKeyer->numButtons);

                // create the buttons
                if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl]
                    || voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl]
                    || voiceKeyerType == keyerTypes[VoiceKeyerId::InternalVoiceKeyer])
                {

                    int columns = 4;
                    createButtonsForKeyer(txVoiceKeyer->numButtons, columns);
                }
                else if (voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
                {
                    int numButtons = 12;    // fixed at 12!
                    int columns = 6;
                    createButtonsForKeyer(numButtons, columns);
                }
                else if (voiceKeyerType == keyerTypes[VoiceKeyerId::ExternalVoiceKeyer])
                {
                    int numButtons = 8;    // fixed at 12!
                    int columns = 4;
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

                    txVoiceKeyer->voiceKeyerInit(txVoiceKeyer->numButtons);

                    for (int i = 0; i < voiceMemButtonList.count(); i++)
                   {
                       VoiceKeyerParams vmData;
                       if (vmData.getType().isEmpty())
                       {
                           vmData.setType(voiceKeyerType);
                       }

                       //vmData.setRigModel(getRigModel(selectedRadio));
                       txVoiceKeyer->readVmButtonParams(i, vmData);
                       vmKeyParamList.append(vmData);
                       setRunButtonText(i, vmData.getVmName());

                   }


                }



            }

        }

    }
}

void TxVmButtonsFrame::loadButtonData()
{

    for (int i = 0; i < voiceMemButtonList.count(); i++)
    {
        VoiceKeyerParams vmData;

        if ((voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl] || voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer] || voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl])
            && !selectedRadio.getLocalName().isEmpty())
        {
            vmData.setSelRadioName(selectedRadio.getLocalName());
            vmData.setRigModel(getRigModel(selectedRadio));

        }

        if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl] || voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
        {
            vmData.setSAndPState(sAndPState);
        }

        if (vmData.getType().isEmpty())
        {
            vmData.setType(voiceKeyerType);
        }

        if (!selectedRadio.getLocalName().isEmpty())
        {
            vmData.setSelRadioName(selectedRadio.getLocalName());
        }


        txVoiceKeyer->readVmButtonParams(i, vmData);
        vmKeyParamList.append(vmData);
        setRunButtonText(i, vmData.getVmName());
    }

    //setVoiceNumMemButtonsVisible(txVoiceKeyer->numButtons);
}


void TxVmButtonsFrame::checkButtonIniFileVersion(QString voiceKeyerType)
{
    QString fileName = VOICE_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + voiceKeyerType + ".ini";
    QSettings config(fileName, QSettings::IniFormat);

    //if (config.value("version", 0).toInt() != 2)
    //{
        // this is not a version 2 file

    QStringList keys = config.childGroups();
    if (!keys.isEmpty())
    {
        // check if this is a version 2 file
        if (config.value("version", 0).toInt() != 2)
        {
            // no convert file to version 2
            for (const QString& key : QASCONST(keys))
            {
                QString buttonNumStr = key.mid( 6, 1);
                // read settings for this key
                config.beginGroup(key);


                QString type = config.value("type", "").toString();
                QString name = config.value("name", "").toString();
                bool repeatFlag = config.value("repeatFlag", false).toBool();
                int duration = config.value("messageDuration", 0).toInt();
                int repeatPauseDur = config.value("repeatPauseDuration", 0).toInt();


                config.remove("");
                config.endGroup();

                config.beginGroup("AllRadios");

                QString buttonKey = "button" + buttonNumStr;
                config.setValue(buttonKey + "/type", type);
                config.setValue(buttonKey + "/name", name);
                config.setValue(buttonKey + "/repeatFlag", repeatFlag);
                config.setValue(buttonKey + "/messageDuration", duration);
                config.setValue(buttonKey + "/repeatPauseDuration", repeatPauseDur);
                config.setValue(buttonKey + "/buttonNum", buttonNumStr.toInt());
                config.endGroup();


            }

            config.setValue("version", 2);
        }
    }
}


void TxVmButtonsFrame::checkCommonIniFileVersion(QString voiceKeyerType)
{

    QString fileName = VOICE_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + voiceKeyerType + ".ini";
    QSettings buttonConfig(fileName, QSettings::IniFormat);

    fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[VoiceKeyerId::RigControl] + ".ini";
    QSettings commonConfig(fileName, QSettings::IniFormat);

    QStringList keys = commonConfig.childGroups();

    if (!keys.empty())
    {
        // check if this is version 2 file
        if (commonConfig.value("version", 0).toInt() != 2)
        {
            // convert file to version 2 and move some keys from common to button Ini file

            commonConfig.beginGroup("Common");

            int numButtons = commonConfig.value("NumButtons", 8).toInt();
            commonConfig.remove("NumButtons");

            bool useCatPttForEom = false;
            bool switchToCwMode = false;



            if (voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl])
            {
                useCatPttForEom = commonConfig.value("UseCatPttForEom", false).toBool();
                commonConfig.remove("UseCatPttForEom");
            }

            if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl])
            {
                switchToCwMode = commonConfig.value("SwitchToCwMode", true).toBool();
                commonConfig.remove("SwitchToCwMode");
            }

            commonConfig.endGroup();

            // move these keys to the button ini file

            buttonConfig.beginGroup("AllRadios");

            buttonConfig.setValue("NumButtons", numButtons);

            if (voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl])
            {
                if (useCatPttForEom)
                {
                    buttonConfig.setValue("endOfMessageType", voiceKeyerCommon::VoiceCwKeyerEomTypes::CAT);
                }
                else
                {
                    buttonConfig.setValue("endOfMessageType", voiceKeyerCommon::VoiceCwKeyerEomTypes::Eom_None);
                }

            }

            if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl])
            {
                buttonConfig.setValue("SwitchToCwMode", switchToCwMode);
            }

            buttonConfig.endGroup();

            commonConfig.setValue("version", 2);

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

    if (!txVoiceKeyer && voiceKeyerType == keyerTypes[VoiceKeyerId::ExternalVoiceKeyer])
    {
        createKeyer(voiceKeyerName);
        if (txVoiceKeyer)
        {
            ui->noExtKeyerLabel->clear();
            extKeyerConnectTimer->stop();
        }
        else
        {
            voiceKeyerType = keyerTypes[VoiceKeyerId::None];
        }
    }
    else if (txVoiceKeyer)
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

    txVoiceKeyer.clear();

    delayedAction(this, [=]{
        createKeyer(voiceKeyerName);
        setFrameState(voiceKeyerName);
    });


    extKeyerConnectTimer->start(1000);

    ui->voiceKeyerSelect->repaint();   // or the combo doesn't update

}

void  TxVmButtonsFrame::onCwMacroTextProcessed(const QString &cwTextSent)
{
    clearCwMessagePlayingDisplay();
    displayCwMessagePlaying(cwTextSent);

    if (buttonNumSent == CW_FREE_TEXT_BUTTON_NUMBER)
    {
        setCwFreeTextIndicatorOnOff(true);
    }

}

void TxVmButtonsFrame::updateFrameState()
{
    setFrameState(ui->voiceKeyerSelect->currentText());
}


void TxVmButtonsFrame::setFrameState(QString voiceKeyerName)
{
    trace("TxVmButtonsFrame::setFrameState");

    VoiceKeyerCapabilities voiceCap = voiceKeyerFactory->supportedVoiceKeyers()->value(voiceKeyerName);

    ui->sAndPLabel->clear();

    // set buttons to centre
    setButtonsJustification(false);

    if (cwSpeedSlider)
    {
        ui->cwSpeedSliderHorizontalLayout->removeWidget(cwSpeedSlider);
        cwSpeedSlider->deleteLater();
        cwSpeedSlider = nullptr;
    }

    setCwEntryBoxVisible(false);
    setCwMessagePlayingVisible(false);
    setCwFreeTextIndicatorVisible(false);
    setErrorMessageVisible(false);

    ui->cwEntry->clear();

    if (txVoiceKeyer == nullptr)
    {
       clearButtonLabels();
       vmKeyParamList.clear();
       //setVoiceNumMemButtonsVisible(0);
       if (voiceKeyerType == keyerTypes[VoiceKeyerId::ExternalVoiceKeyer])
       {
           ui->noExtKeyerLabel->setText(HtmlFontColour(Qt::red) +  tr("To use the external keyer mqtKeyer must be running and connected"));

       }
       voiceKeyerType = keyerTypes[VoiceKeyerId::None];

       clearButtons();



       setAvailIndicatorVisible(false);
       setRepeatIndicatorVisible(false);

       ui->vmSetupPb->setVisible(false);
       ui->pipCb->setVisible(false);
       ui->vmStopPb->setVisible(false);
       setTXStatusVisible(false);
       ui->buttonSelectionLbl->setVisible(false);
       ui->saveByRadioNameText->setVisible(false);
       setEomTypeLabelsVisible(false);
       setKeyerIndicatorGroupBoxVisible(false);
       setPttIndicatorGroupBoxVisible(false);
       setCwMessagePlayingVisible(false);
       setMessagePlayingFlag(false);

    }
    else
    {
        ui->noExtKeyerLabel->clear();

        VoiceKeyerCapabilities voiceCap = voiceKeyerFactory->supportedVoiceKeyers()->value(voiceKeyerName);

        ui->vmSetupPb->setVisible(voiceCap.getSetupButton());
        ui->pipCb->setVisible(voiceCap.getHasPip());
        setTXStatusVisible(voiceCap.getHasTxStatus());

        setKeyerIndicatorGroupBoxVisible(true);
        setPttIndicatorGroupBoxVisible(true);
        setMessagePlayingFlag(false);


        if (voiceCap.getHasAvailStatus())
        {
            setAvailIndicatorVisible(voiceCap.getHasAvailStatus());

            if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl] || voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl])
            {

                setAvailIndicatorForRadioOnOff(selectedRadio);
            }
            else if (voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
            {

                setAvailIndicatorOnOffForPcCwKeyer();
            }


        }
        else
        {
            setAvailIndicatorVisible(false);
        }

        setRepeatIndicatorVisible(voiceCap.getHasMessageRepeat());

        if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl])
        {

            txVoiceKeyer->setCwMemType(getCwMemType(selectedRadio));
            ui->sAndPLabel->setText("| S&P");       // init S&P/Run Label
            txVoiceKeyer->setContest(ct);

            // do we need to left justify the buttons
            if (getVmButtonsLeftJustifyFlag(VOICEKEYER_COMMON_PARAMS_FILENAME))
            {
                setButtonsJustification(true);

            }
        }

        if (voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
        {
            ui->sAndPLabel->setText("| S&P");       // init S&P/Run Label
            txVoiceKeyer->setContest(ct);
            // don't really need to save every time, but ensures the data is correct
            // we don't allow change of number of buttons or EOM type.
            if (auto keyed = qobject_cast<PcCWMessageKeyer*>(txVoiceKeyer.data()))
            {
                keyed->saveFixedRadioCommonData();
            }
        }

        ui->vmStopPb->setVisible(true);

        if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl]
            || voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl]
            || voiceKeyerType == keyerTypes[VoiceKeyerId::InternalVoiceKeyer]
            || voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
        {
            setSaveButtonByRadionameText(selectedRadio.getLocalName());

            txVoiceKeyer->setRadioParams(getNumVoiceMessages(selectedRadio), selectedRadio.getLocalName(), getPttType(selectedRadio), getPttEnabled(selectedRadio));
            txVoiceKeyer->voiceKeyerInit(txVoiceKeyer->numButtons);
            setPttTypeLabelsVisible(true);
            setPttTypeText(getPttType(selectedRadio));
            setPttEnabledIndicatorOnOff(getPttEnabled(selectedRadio));
            setEomTypeLabelsVisible(true);
            setEomLabelText(txVoiceKeyer->getSelectedEomType());
            loadButtonData();

            if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl])
            {
                setCwFreeTextIndicatorVisible(true);
                setCwFreeTextIndicatorOnOff(false);

                if (!getRigCwKeyerSupportStopFlag(selectedRadio.getLocalName()))
                {
                    ui->vmStopPb->setVisible(false);
                }
                else
                {
                    ui->vmStopPb->setVisible(true);
                }
                setCwEntryBoxVisible(true);
                setCwMessagePlayingVisible(true);
                initCwTextEntryBox(getCwRadioManufacturer(getCwMemType(selectedRadio)), CWKEYER_RADIO_COMMON_PARAMS_FILENAME);
            }
            else if (voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
            {
                setCwEntryBoxVisible(true);
                setCwMessagePlayingVisible(true);
                setPttTypeLabelsVisible(false);
                setPttEnabledIndicatorOnOff(false);
                setEomTypeLabelsVisible(false);
                setCwFreeTextIndicatorVisible(true);
                setCwFreeTextIndicatorOnOff(false);
                initCwTextEntryBox("AllRadios", PC_CW_KEYER_COMMON_PARAMS_FILENAME);

                cwSpeedSlider = new CwSpeedControl(ui->cwSpeedSliderFrame);
                ui->cwSpeedSliderHorizontalLayout->addWidget(cwSpeedSlider);
                cwSpeedSlider->setSpeedRange(voiceKeyerCommon::PC_CW_KEYER_MIN_WPM, voiceKeyerCommon::PC_CW_KEYER_MAX_WPM);

                connect(cwSpeedSlider, &CwSpeedControl::cwSpeedChanged, this, [this](int wpm){
                    emit sendWpmToPcCwkeyer(wpm);
                });

            }
            else if (voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl])
            {
                if (!getRigVoiceKeyerSupportStopFlag(selectedRadio.getLocalName()))
                {
                    ui->vmStopPb->setVisible(false);
                }
                else
                {
                    ui->vmStopPb->setVisible(true);
                }


            }
        }
        else if (voiceKeyerType == keyerTypes[VoiceKeyerId::InternalVoiceKeyer])
        {
            setPttTypeLabelsVisible(true);
            setPttTypeText(getPttType(selectedRadio));
            setPttEnabledIndicatorOnOff(getPttEnabled(selectedRadio));
            loadButtonData();

        }
        else
        {
            ui->buttonSelectionLbl->setVisible(false);
            ui->saveByRadioNameText->setVisible(false);
            setPttTypeLabelsVisible(false);

        }
    }
}

void TxVmButtonsFrame::setButtonsJustification(bool leftJustify)
{
    if (leftJustify)
    {
        // Left justify - collapse left spacers
        ui->vmButtonsLeftSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
        ui->stopButtonLeftSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
    else
    {
        // Center - restore left spacers
        ui->vmButtonsLeftSpacer->changeSize(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        ui->stopButtonLeftSpacer->changeSize(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    }

    ui->vmButtonsHorizLayout->invalidate();
    ui->stopButtonHorizLayout->invalidate();
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
    if  (txVoiceKeyer)
    {

        if (!checkRadioAndKeyerState())
        {
            return;         // error radio keyer state missing
        }


        if (voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer] || voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl])
        {
            QString message = ui->cwEntry->text().trimmed();

            if (!message.isEmpty())
            {
                ui->cwEntry->selectAll();

                buttonNumSent = CW_FREE_TEXT_BUTTON_NUMBER;

                if (curMode != rigcommon::convertModeToQString(MODE::CW) && txVoiceKeyer->getSetCwModeAndRestoreFlag())
                {
                    savedMode = curMode;
                    sendModeToRadio(rigcommon::convertModeToQString(MODE::CW));
                }
                else
                {
                    savedMode = curMode;        // keep current mode if CW
                }

                txVoiceKeyer->sendCwFreeTextMsg(message);
            }
        }
    }

}



void TxVmButtonsFrame::setSaveButtonByRadionameText(QString selectedRadioName)
{

    if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl] || voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer] ||voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl])
    {
        QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICE_KEYER_BASE_FILE_NAME + voiceKeyerType + ".ini";
        QSettings readConfig(fileName, QSettings::IniFormat);

        ui->saveByRadioNameText->setVisible(true);
        ui->buttonSelectionLbl->setVisible(true);

        if (readConfig.value("Common/SaveButtonByRadioName", false).toBool())
        {

            ui->saveByRadioNameText->setText(selectedRadioName);

        }
        else
        {

            ui->saveByRadioNameText->setText("All Radios");
        }
    }
    else
    {
        ui->saveByRadioNameText->setVisible(false);
        ui->buttonSelectionLbl->setVisible(false);

    }
}

void TxVmButtonsFrame::clearButtonLabels()
{
    for(int i = 0;  i < voiceMemButtonList.count(); i++)
    {
        setRunButtonText(i, "");
    }
}

void TxVmButtonsFrame::editActionSelected(int buttonNumber)
{
    if((voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl] && !isVoiceMemAvail(selectedRadio))
        || (voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer] && !isPcCwKeyerLoaded())
        || (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl] && !isCwMemTypeAvail(selectedRadio)))
    {
        if (voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl])
        {
            logMessage(QString("editActionSelected - voice rigcontrol - selectedRadio = %1, voiceMemAvail = %2").arg(selectedRadio.getLocalName()).arg(isVoiceMemAvail(selectedRadio) ? "True" : "False"));
        }
        else if ((voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer]))
        {
            logMessage(QString("editActionSelected - pcCwKeyer - isPcCwKeyerLoaded = %1").arg(isPcCwKeyerLoaded() ? "True" : "False"));
        }
        else if ((voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl]))
        {
            logMessage(QString("editActionSelected - cwRigControl - selectedRadio = %1, isCwMemTypeAvail = %2").arg(selectedRadio.getLocalName()).arg(isCwMemTypeAvail(selectedRadio) ? "True" : "False"));
        }


        return;
    }


    logMessage(QString("editActionSelected"));

    VoiceKeyerParams vmData;
    vmData.setType(voiceKeyerType);



    if ((voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl] || voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl] || voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl])
        && !selectedRadio.getLocalName().isEmpty())
    {
        vmData.setSelRadioName(selectedRadio.getLocalName());
        vmData.setRigModel(getRigModel(selectedRadio));
        if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl])
        {
            txVoiceKeyer->setCwMemType(getCwMemType(selectedRadio));    // ensure the cwType is updated
        }

        vmData.setSAndPState(sAndPState);
    }

    if (txVoiceKeyer)
    {
        txVoiceKeyer->readVmButtonParams(buttonNumber, vmData);
        if (vmData.getType().isEmpty())    // in case read data is empty
        {
            vmData.setType(voiceKeyerType);
        }
        vmData.setVkBase(txVoiceKeyer);

        if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl])
        {
            vmData.setCwKeyerType(CW_KEYER_TYPE::RIG);
        }
        else if (voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
        {
            vmData.setCwKeyerType(CW_KEYER_TYPE::PC_CwKeyer);
        }
        else
        {
            vmData.setCwKeyerType(CW_KEYER_TYPE::KEYER_NONE);
        }


        logMessage(QString("- edit selected button no = %1").arg(buttonNumber));

        QString title1 = "";
        if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl] || voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
        {
            QString t;

            if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl])
            {
               t = tr("Rig CW Message - ");
            }
            else
            {
               t = tr("PC CW Keyer Message - ");
            }


            QString runSandPTxt;
            if (sAndPState)
            {
                runSandPTxt = "S&P";
            }
            else
            {
                runSandPTxt = "Run";
            }
            title1 = t + runSandPTxt;
        }
        else
        {
            title1 = tr("Voice Memory");
        }
        QString title(tr("%1 %2 - Edit").arg(title1).arg(buttonNumber + 1));

        int ret = txVoiceKeyer->editButton(&vmData, title);
        if (ret == QDialog::Accepted)
        {
            if ( txVoiceKeyer)
            {
                txVoiceKeyer->saveVmButtonParams(vmData);
                setRunButtonText(buttonNumber, vmData.getVmName());
                vmKeyParamList[buttonNumber] = vmData;
            }

        }

    }

}
void TxVmButtonsFrame::readActionSelected(int buttonNumber)
{
    logMessage(QString("- readActionSelected"));

    if (!checkRadioAndKeyerState())
    {
        return;         // error radio keyer state missing
    }

    if (buttonNumSent != NO_VM_BUTTON_ON)
    {
        onVmStopClicked();
        return;

    }

    startVMMsg(buttonNumber);
}


bool TxVmButtonsFrame::checkRadioAndKeyerState()
{
    if (voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl] && !isVoiceMemAvail(selectedRadio))
    {
        QString msg = tr("rigControl Voice Selected, but not available for this radio");
        logMessage(QString(msg));
        showTemporaryErrorMessage(msg, ERROR_MSG_TIMEOUT_DURATION);
        return false;
    }
    else if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl] && !isCwMemTypeAvail(selectedRadio))
    {
        QString msg = tr("rigControl CW Keyer selected, but not available for this radio");
        logMessage(QString(msg));
        showTemporaryErrorMessage(msg, ERROR_MSG_TIMEOUT_DURATION);
        return false;
    }
    else if (voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer] && !isPcCwKeyerLoaded())
    {
        QString msg = tr("PC CW DTR Server is not available.");
        logMessage(QString(msg));
        showTemporaryErrorMessage(msg, ERROR_MSG_TIMEOUT_DURATION);
        return false;
    }


    if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl] && voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl])
    {
        if (!getPttEnabled(selectedRadio) )
        {
            QString msg = tr("radio ptt is not enabled, please enable");
            logMessage(QString(msg));
            showTemporaryErrorMessage(msg, ERROR_MSG_TIMEOUT_DURATION);
            return false;
        }
    }



    if (voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl])
    {
        if(!isVoiceMode())
        {
            QString msg = tr("Mode needs to be a phone type for rigcontrol Voice Message, current mode = %1").arg(curMode);
            logMessage(QString(msg));
            showTemporaryErrorMessage(msg, ERROR_MSG_TIMEOUT_DURATION);
            return false;
        }
    }

    return true;
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

    txVoiceKeyer->readVmButtonParams(buttonNumber, vmData);
    setRepeatIndicatorOnOff(vmData.getVmRepeatFlag());



    if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl]
        || voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {
        if (vmData.getVmCwMessage().isEmpty())
        {
            logMessage(QString("Cw Message is empty, ignore"));
            displayCwMessagePlaying(QString("CW Message Empty!"));
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
            if (curMode != rigcommon::convertModeToQString(MODE::CW) && txVoiceKeyer->getSetCwModeAndRestoreFlag())
            {
                savedMode = curMode;
                sendModeToRadio(rigcommon::convertModeToQString(MODE::CW));
            }
            else
            {
                savedMode = curMode;        // keep current mode if CW
            }

            txVoiceKeyer->sendCwMsg(vmData);
            //setMessagePlayingFlag(true);

        }

    }
    else
    {

        txVoiceKeyer->sendMsgNum(buttonNumSent);
        //setMessagePlayingFlag(true);
    }

    selectedEomType = txVoiceKeyer->getSelectedEomType();

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

        txVoiceKeyer->stopCwMsg();
        clearCwMessagePlayingDisplay();
        setCwFreeTextIndicatorOnOff(false);

        if (curMode != savedMode && txVoiceKeyer->getSetCwModeAndRestoreFlag())       // restore mode?
        {
            sendModeToRadio(savedMode);
        }

    }
    else
    {
        txVoiceKeyer->stopMsg(nullptr);
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
    //setMessagePlayingFlag(false);

}

void TxVmButtonsFrame::newActionSelected(int buttonNumber)
{

    if (voiceKeyerType == keyerTypes[VoiceKeyerId::None]
        || (voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl] && !isVoiceMemAvail(selectedRadio))
        || (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl] && !isCwMemTypeAvail(selectedRadio))
        || (voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer] && !isPcCwKeyerLoaded())
        )
    {
        logMessage(QString("- newActionSelected rigControl Voice Keyer Selected, but not available for this radio or no keyer selected keyerType = %1").arg(voiceKeyerType));
        return;
    }

    logMessage(QString("- newActionSelected Keyer Selected = %1").arg(voiceKeyerType));

    VoiceKeyerParams vmData;
    vmData.clear();


    logMessage(QString("- write selected button no = %1").arg(buttonNumber));

    QString title1 = "";
    if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl]
        || voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {
        vmData.setSAndPState(sAndPState);

        QString t;
        if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl])
        {
            t = tr("Rig CW Message - ");
        }
        else
        {
           t = tr("PC CW Keyer Message - ");
        }

        QString runSandPTxt;
        if (sAndPState)
        {
            runSandPTxt = "S&P";
        }
        else
        {
            runSandPTxt = "Run";
        }
        title1 = t + runSandPTxt;
    }
    else
    {
        title1 = tr("Voice Memory");
    }
    QString title(tr("%1 %2 - New").arg(title1).arg(buttonNumber + 1));

    vmData.setSelRadioName(selectedRadio.getLocalName());
    vmData.setvmButtonNum(buttonNumber);
    vmData.setType(voiceKeyerType);
    vmData.setVkBase(txVoiceKeyer);
    vmData.setRigModel(getRigModel(selectedRadio));



    if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl])
    {
        vmData.setCwKeyerType(CW_KEYER_TYPE::RIG);
    }
    else if (voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {
        vmData.setCwKeyerType(CW_KEYER_TYPE::PC_CwKeyer);
    }
    else
    {
        vmData.setCwKeyerType(CW_KEYER_TYPE::KEYER_NONE);
    }

    int ret = txVoiceKeyer->editButton(&vmData, title);
    if (ret == QDialog::Accepted)
    {
        if (txVoiceKeyer)
        {
            txVoiceKeyer->saveVmButtonParams(vmData);
            setRunButtonText(buttonNumber, vmData.getVmName());
            vmKeyParamList[buttonNumber] = vmData;
        }

    }

}

void TxVmButtonsFrame::onRemoteConfigChanged()
{
    bool s = txVoiceKeyer->getPip();
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

        txVoiceKeyer->readVmButtonParams(i, vmData);
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
    //txVoiceKeyer->stopMsg();
    msgDurTimer->stop();
    repeatPauseTimer->stop();
    TxVoiceMemButton *b = txVmButtonMap[buttonNumSent];
    if (b)
    {
        b->showButtonOnOff(false);
    }
    buttonNumSent = NO_VM_BUTTON_ON;

}
void TxVmButtonsFrame::setRunButtonText(const int buttonNumber, const QString name)
{
    QString buttonText = QString("%1: %2").arg(buttonNumber + 1).arg(name);
    voiceMemButtonList[buttonNumber]->setText(buttonText);
}


void TxVmButtonsFrame::onMsgDurTimerTimeout()
{
    if (buttonNumSent >= 0)
    {
        if (vmKeyParamList[buttonNumSent].getVmDuration() > 0
            || selectedEomType == voiceKeyerCommon::VoiceCwKeyerEomTypes::CAT
            || txVoiceKeyer->getSelectedEomType() == voiceKeyerCommon::VoiceCwKeyerEomTypes::InternalSoundCardVoiceKeyer
            || selectedEomType == voiceKeyerCommon::VoiceCwKeyerEomTypes::DTRKeyerTXStatus)
        {
            if (voiceKeyerType == keyerTypes[VoiceKeyerId::InternalVoiceKeyer])
            {
                txVoiceKeyer->stopMsg(nullptr); // ensure the sbdriver is stopped
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


    restoreRadioMode();

}


void TxVmButtonsFrame::restoreRadioMode()
{
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
            && txVoiceKeyer->getSetCwModeAndRestoreFlag())
        {
            logMessage(QString("Restore Radio Mode, current mode = %1, savedMode = %2").arg(curMode, savedMode));
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

    if (txVoiceKeyer->doRepeatFromLogger())
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
    //qDebug() << "set messagePlaying " << (playing ? "True" : "False");
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
    ui->errorMeassageDisplay->setVisible(visible);
}


void TxVmButtonsFrame::showTemporaryErrorMessage(const QString &msg, int timeoutMs, const QColor &colour)
{
    setErrorMessageVisible(true);
    // Save the current stylesheet
    QString oldStyle = ui->errorMeassageDisplay->styleSheet();

    // Set error color and text
    ui->errorMeassageDisplay->setStyleSheet(QString("color: %1;").arg(colour.name()));

    ui->errorMeassageDisplay->setText(msg);

    // Clear after timeoutMs milliseconds
    QTimer::singleShot(timeoutMs, this, [this, oldStyle]() {
        ui->errorMeassageDisplay->clear();
        ui->errorMeassageDisplay->setStyleSheet(oldStyle);
        setErrorMessageVisible(false);
    });
}

void TxVmButtonsFrame::setCwMessagePlayingVisible(bool visible)
{
    ui->cwMesssagePlayingLabel->setVisible(visible);
    ui->cwMessagePlayingDisplay->setVisible(visible);
}

void TxVmButtonsFrame::displayCwMessagePlaying(const QString msg)
{
    ui->cwMessagePlayingDisplay->setText(msg);
}

void TxVmButtonsFrame::clearCwMessagePlayingDisplay()
{
    ui->cwMessagePlayingDisplay->clear();
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

   if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl])
    {
       if(!state)
       {
          clearCwMessagePlayingDisplay();
          setCwFreeTextIndicatorOnOff(false);
       }


        if (buttonNumSent == CW_FREE_TEXT_BUTTON_NUMBER)
        {
            logMessage(QString("CW Freetext sent - stop tx and restore mode."));


            restoreRadioMode();

            return;
        }
    }

   if (selectedEomType == voiceKeyerCommon::VoiceCwKeyerEomTypes::CAT
       || selectedEomType == voiceKeyerCommon::VoiceCwKeyerEomTypes::DTRKeyerTXStatus)
   {
       logMessage(QString("Using PTT or DTR Keyer TX Status for EOM Flag"));
       if (txVoiceKeyer && txVoiceKeyer->doRepeatFromLogger() && !state)
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
    txVoiceKeyer->setPip(ui->pipCb->isChecked());
}

void TxVmButtonsFrame::setPttStatusIndicatorOnOff(bool on)
{
    if (on)
    {
        ui->txStatusIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);
        ui->txStatusIndicator->setToolTip(tr("TX On"));
        setMessagePlayingFlag(true);

    }
    else
    {
       ui->txStatusIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);
       ui->txStatusIndicator->setToolTip(tr("TX Off"));
       setMessagePlayingFlag(false);
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

void TxVmButtonsFrame::setCwFreeTextIndicatorOnOff(bool on)
{
    if (on)
    {
        ui->cwFreeTextPlayingIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);
        ui->cwFreeTextPlayingIndicator->setToolTip(tr("CW FreeText Playing"));
    }
    else
    {
       ui->cwFreeTextPlayingIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);
       ui->cwFreeTextPlayingIndicator->setToolTip(tr("No CW FreeText Playing"));
    }
}

void TxVmButtonsFrame::setCwFreeTextIndicatorVisible(bool visible)
{
    ui->cwFreeTextPlayingIndicator->setVisible(visible);
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


void TxVmButtonsFrame::sandPChanged(BaseContestLog *c, bool s)
{
    if (ct == c && txVoiceKeyer)
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



void TxVmButtonsFrame::fKey(BaseContestLog *c, int key, int /*carrier*/)
{
    // FKey event received by log frame (or ctrl/FKey)

    if (c && ct == c && isVoiceMode())
    {
        int mem = key - Qt::Key_F1;
        if (mem > 10)
        {

        }
        else if (mem == 10)
        {
            onVmStopClicked();
        }
        else
        {
            readActionSelected(mem);
        }
    }
    else if (c && ct == c && voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {
        // for test...
        int mem = key - Qt::Key_F1;
        if (mem > 10)
        {

        }
        else if (mem == 10)
        {
            onVmStopClicked();
        }
        else
        {
            readActionSelected(mem);
        }
    }
}

void TxVmButtonsFrame::setMode(const QString m)
{
    QString mode = m;
    if (curMode != mode.remove(':'))
    {
        curMode = mode;
    }
}

void TxVmButtonsFrame::setContest(BaseContestLog *c)
{
    ct = c;
    if (txVoiceKeyer && voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl])
    {
        txVoiceKeyer->setContest(ct);
    }
}

void TxVmButtonsFrame::sendModeToRadio(const QString m)
{
    emit sendRadioMode(m);
}


void TxVmButtonsFrame::onInternalVoiceMemoryPlayState(bool playing)
{
    if (playing)
    {
        //qDebug() << "internal is playing";
    }
    else
    {
       //qDebug() << "internal has stopped";
        onMsgDurTimerTimeout();
    }
}


void TxVmButtonsFrame::setPcCwKeyerComport(QString comportStr)
{
    if (txVoiceKeyer && voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {

    }
}

void TxVmButtonsFrame::setPcCwKeyerConnectionState(QString stateStr)
{

    logMessage(QString("PcCwKeyerConnection state = %1").arg(stateStr));
    if (txVoiceKeyer && voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {
        if (stateStr == "Open")
        {
            setAvailIndicatorOnOff(true);
        }
        else
        {
            setAvailIndicatorOnOff(false);
        }
    }
}

void TxVmButtonsFrame::setPcCwKeyerErrorMsg(QString errorMsg)
{
    logMessage(QString("PcCwKeyerConnection error message = %1").arg(errorMsg));
    if (txVoiceKeyer && voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {

    }
}

void TxVmButtonsFrame::setPcCwKeyerPttEnabled(QString enabled)
{
    logMessage(QString("PcCwKeyer Ptt enabled = %1").arg(enabled));

    if (txVoiceKeyer && voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {
        if (enabled == "On")
        {
            setPttEnabledIndicatorOnOff(true);
        }
        else
        {
            setPttEnabledIndicatorOnOff(false);
        }
    }
}


void TxVmButtonsFrame::setPcCwKeyerTxOnState(QString state)
{

    logMessage(QString("TX State received = %1").arg(state));

    if (txVoiceKeyer && voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {
        if (state == "On")
        {
            setPttStatusIndicatorOnOff(true);
        }
        else
        {
            setPttStatusIndicatorOnOff(false);
            clearCwMessagePlayingDisplay();
            setCwFreeTextIndicatorOnOff(false);
            onMsgDurTimerTimeout();
        }
    }
}


void TxVmButtonsFrame::setPcCwKeyerCurrentWpm(QString wpm)
{
    logMessage(QString("Current WPM from PcCwKeyer = %1").arg(wpm));
    if (wpm.isEmpty())
    {
        return;
    }

    if (txVoiceKeyer && voiceKeyerType == keyerTypes[VoiceKeyerId::PcCwKeyer])
    {

        cwSpeedSlider->setValue(wpm.toInt()); // this will not emit and send back to keyer

    }
}

void TxVmButtonsFrame::logMessage(QString msg)
{
    trace(QString("[TxVmButtonsFrame] %1").arg(msg));
}

/*
// this to prevent multiple return event signals with connect statement

bool TxVmButtonsFrame::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->cwEntry && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
        {
            if (keyEvent->isAutoRepeat())
                return true;  // block repeated Return

            onCwEntryReturnPressed();
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}
*/


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



//*******************TX Voice Memory Button *************************//

TxVoiceMemButton::TxVoiceMemButton(QToolButton *b, TxVmButtonsFrame *tvmbf, int no)
{
    memNo = no;
    txVmButtonsFrame = tvmbf;

    vmButton = b;

    vmMenu = new QMenu(vmButton);

    vmButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    vmButton->setPopupMode(QToolButton::MenuButtonPopup);
    vmButton->setFocusPolicy(Qt::NoFocus);
    vmButton->setFixedHeight(vmButton->sizeHint().height());

    shortKey = new QShortcut(QKeySequence(vmButtonShortCutKeys[memNo]), vmButton);

    newAction = new QAction(tr("&New"),vmButton);
    editAction = new QAction(tr("&Edit"), vmButton);
    vmMenu->addAction(newAction);
    vmMenu->addAction(editAction);


    vmButton->setMenu(vmMenu);

    connect(shortKey, &QShortcut::activated, this, &TxVoiceMemButton::readActionSelected);
    connect(vmButton, &QToolButton::clicked, this, &TxVoiceMemButton::readActionSelected);
    connect(vmButton, &QToolButton::clicked, this, &TxVoiceMemButton::buttonSelected);
    connect( newAction, &QAction::triggered, this, &TxVoiceMemButton::newActionSelected);
    connect( editAction, &QAction::triggered, this, &TxVoiceMemButton::editActionSelected);


}
TxVoiceMemButton::~TxVoiceMemButton()
{

}

void TxVoiceMemButton::memoryShortCutSelected()
{
//    rigControlFrame->memoryShortCutSelected(memNo);
    vmButton->showMenu();
    //emit lostFocus();
}
void TxVoiceMemButton::readActionSelected()
{
    trace(QString("TxVoiceMemButton::readActionSelected from %1 %2").arg(sender()->metaObject()->className(), sender()->objectName()));
    txVmButtonsFrame->readActionSelected(memNo);
}
void TxVoiceMemButton::editActionSelected()
{

    trace(QString("TxVoiceMemButton::editActionSelected from %1 %2").arg(sender()->metaObject()->className(), sender()->objectName()));
    txVmButtonsFrame->editActionSelected(memNo);

}
void TxVoiceMemButton::newActionSelected()
{
    trace(QString("TxVoiceMemButton::newActionSelected from %1 %2").arg(sender()->metaObject()->className(), sender()->objectName()));
    txVmButtonsFrame->newActionSelected(memNo);
}


void TxVoiceMemButton::buttonSelected()
{
    emit buttonActivated(memNo);
}

void TxVoiceMemButton::showButtonOnOff(bool state)
{
    if (vmButton)
    {
        if (state)
        {
            vmButton->setStyleSheet(VM_BUTTON_ON_STYLE);
        }
        else
        {
            vmButton->setStyleSheet(VM_BUTTON_OFF_STYLE);
        }
    }
}







