#include <QString>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>
#include <QPushButton>
#include <QFileDialog>
#include <QFileSystemWatcher>
#include <QTimer>

#include "ContestApp.h"
#include "LoggerContest.h"
#include "MShowMessageDlg.h"
#include "MinosLoggerEvents.h"
#include "MinosParameters.h"

#include "MinosRPC.h"
#include "SendRPCDM.h"
#include "delayedaction.h"
#include "fileutils.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "MTrace.h"
#include "dmkeyseditdlg.h"

#include "cwspeedcontrol.h"
#include "rigCommon.h"
#include "serialCommonData.h"
#include "pccwmessagekeyer.h"
#include "cwrigkeyervalidator.h"
#include "dmkeyercontainer.h"

#include <QThread>



#include "dmbuttonframe.h"
#include "ui_dmbuttonframe.h"

using namespace TxKeyerCommon;


inline const QStringList vmButtonShortCutKeys = {
    "Shift+F1", "Shift+F2",
    "Shift+F3", "Shift+F4",
    "Shift+F5", "Shift+F6",
    "Shift+F7", "Shift+F8",
    "Shift+F9", "Shift+F10",
    "Shift+F11", "Shift+F12",
};

/*
// Defines the layout of the Keyer Ui
struct UiElementCfg
{
    bool showCwEntryBox;
    bool showTxStatus;
    bool showKeyerIndicators;
    bool showPttIndicators;
    bool showStopButton;
    bool showRadioLabel;
    bool showPip;
    bool showAvailIndicator;
    bool showRepeatIndicator;
    bool showCwSpeedSlider;
};

std::map<TxKeyerId, UiElementCfg> keyerUiMap = {
//                          CwEntryBox  TxStatus    KeyIndicator    PTTIndicator    StopBtn RadioLbl    Pip     AvailInd    RepeatInd   CwSpeedSlider
    { TxKeyerId::None,           { false,   false,      false,          false,          false,  false,      false,  false,      false,      false } },
    { TxKeyerId::RigControl,     { false,   true,       true,           true,           true,   true,       false,   true,       true,      false } },
    { TxKeyerId::CW_RigControl,  { true,    true,       true,           true,           true,   true,       false,   true,       true,      false } },
    { TxKeyerId::PcCwKeyer,      { true,    true,       true,           true,           true,   false,      false,   true,       true,      true } },
    { TxKeyerId::InternalVoiceKeyer, { false, true,     true,           true,           true,   false,      false,   true,       true,      false } },
    // Add other keyers as needed
};
*/

inline const QString DIGIMODE = "DigitalMode";

const int CW_FREE_TEXT_BUTTON_NUMBER = 13;

const int ERROR_MSG_TIMEOUT_DURATION = 20000;

DMButtonFrame::DMButtonFrame(DMKeyerContainer* keyerContainer_, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::DMButtonFrame),
    fixedMode(false),
    keyerContainer(keyerContainer_)
{
    ui->setupUi(this);

    createKeyerForms();         // using stackedwidget

    keyerSettings = keyerContainer->keyerSettings;

    initKeyerSettings();   // get settings from container frame

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::fKey, this, &DMButtonFrame::fKey);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::SandPChanged, this, &DMButtonFrame::sandPChanged);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::DMMess, this, &DMButtonFrame::DMMess);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::modeChange, this, &DMButtonFrame::onModeChange);

    connect(keyerContainer, &DMKeyerContainer::activeKeyerChanged, this, &DMButtonFrame::onActiveKeyerChanged);
    connect(keyerContainer, &DMKeyerContainer::contestChanged, this, &DMButtonFrame::onContestChanged);
    connect(keyerContainer, &DMKeyerContainer::selectedRadioChanged, this, &DMButtonFrame::onSelectedRadioChanged);
    connect(keyerContainer, &DMKeyerContainer::isRadioConnectedChanged, this, &DMButtonFrame::onIsRadioConnectedChanged);
    connect(keyerContainer, &DMKeyerContainer::pttEnabledChanged, this, &DMButtonFrame::onPttEnabledChanged);
    connect(keyerContainer, &DMKeyerContainer::pttTypeChanged, this, &DMButtonFrame::onPttTypeChanged);
    connect(keyerContainer, &DMKeyerContainer::pttStateChanged, this, &DMButtonFrame::onPttStateChanged);
    connect(keyerContainer, &DMKeyerContainer::voiceMemAvailChanged, this, &DMButtonFrame::onVoiceMemAvailChanged);
    connect(keyerContainer, &DMKeyerContainer::numVoiceMessagesChanged, this, &DMButtonFrame::onNumVoiceMessagesChanged);
    connect(keyerContainer, &DMKeyerContainer::rigModelChanged, this, &DMButtonFrame::onRigModelChanged);
    connect(keyerContainer, &DMKeyerContainer::cwMemTypeChanged, this, &DMButtonFrame::onCwMemTypeChanged);
    connect(keyerContainer, &DMKeyerContainer::rigVoiceKeyerSupportStopFlagChanged, this, &DMButtonFrame::onRigVoiceKeyerSupportStopFlagChanged);
    connect(keyerContainer, &DMKeyerContainer::rigCwKeyerSupportStopCmdChanged, this, &DMButtonFrame::onRigCwKeyerSupportStopFlagChanged);
    connect(keyerContainer, &DMKeyerContainer::pcCwKeyerComportChanged, this, &DMButtonFrame::onPcCwKeyerComportChanged);
    connect(keyerContainer, &DMKeyerContainer::pcCwKeyerConnectionStateChanged, this, &DMButtonFrame::onPcCwKeyerConnectionStateChanged);
    connect(keyerContainer, &DMKeyerContainer::pcCwKeyerErrorMessageChanged, this, &DMButtonFrame::onPcCwKeyerErrorMsgChanged);
    connect(keyerContainer, &DMKeyerContainer::pcCwKeyerPttEnabledChanged, this, &DMButtonFrame::onPcCwKeyerPttEnabledChanged);
    connect(keyerContainer, &DMKeyerContainer::pcCwKeyerTxOnStateChanged, this, &DMButtonFrame::onPcCwKeyerTxOnStateChanged);
    connect(keyerContainer, &DMKeyerContainer::pcCwKeyerCurrentWpmChanged, this, &DMButtonFrame::onPcCwKeyerCurrentWpmChanged);
    connect(keyerContainer, &DMKeyerContainer::loggerRadioSettingsChanged, this, &DMButtonFrame::onLoggerRadioSettingsChanged);


    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpDigiFunctionKeyFile, fkeyFileName );

    txKeyer = nullptr;
    selectedKeyerCap.clear();

    txKeyerFactory = new TxKeyerFactory(this);

    msgDurTimer = new QTimer(this);
    connect(msgDurTimer, &QTimer::timeout, this, &DMButtonFrame::onMsgDurTimerTimeout);

    repeatPauseTimer = new QTimer(this);
    connect(repeatPauseTimer, &QTimer::timeout, this, &DMButtonFrame::onRepeatPauseTimerTimeout);

    extKeyerConnectTimer = new QTimer(this);
    connect(extKeyerConnectTimer, &QTimer::timeout, this, &DMButtonFrame::onExtConnectTimer);
    connect(LogContainer->sendDM, &TSendDM::keyerReport, this, &DMButtonFrame::onExtConnectTimer);



    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICEKEYER_COMMON_PARAMS_FILENAME;
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup(VOICEKEYER_COMMON_PARAMS_GROUPNAME);

    QString txKeyerName = config.value("KeyerName").toString();

    //connect(ui->txKeyerSetupPb, &QPushButton::clicked, this, &DMButtonFrame::onTxKeyerSetupClicked);

    txKeyerFactory->populateComboKeyerList(ui->txKeyerSelect, txKeyerName);

    // we add digi modes to list, though it will not act as keyer
    ui->txKeyerSelect->addItem(txKeyerNames[DigitalModes]);

    connect(ui->txKeyerSelect, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DMButtonFrame::onTxKeyerSelect);

    logMessage(QString("start keyer name = %1").arg(ui->txKeyerSelect->currentText()));

    onTxKeyerSelect(ui->txKeyerSelect->currentIndex());

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

    buildFkeyButtons(12);

/*
    fButtons << ui->F1Button << ui->F2Button << ui->F3Button << ui->F4Button << ui->F5Button << ui->F6Button;
    fButtons << ui->F7Button << ui->F8Button << ui->F9Button << ui->F10Button << ui->F11Button << ui->F12Button;

    int i = 0;
    for (auto b: QASCONST(fButtons))
    {
        b->setProperty("KeyNo", i++);
        b->setText("");
        connect(b, &QPushButton::clicked, this, &DMButtonFrame::fButtonClicked);
    }

    // ui->nameLabel->setText(tr("Data Modes & Keyer Buttons from %1").arg(fkeyFileName));

    ui->selectedRadioLabel->setVisible(false);
*/

}

DMButtonFrame::~DMButtonFrame()
{

    if (keyerContainer)
    {
        disconnect(keyerContainer, nullptr, this, nullptr);
        keyerContainer = nullptr;
    }

    delete ui;


}


void DMButtonFrame::clearFkeyLayout()
{
    QLayoutItem *child;
    while ((child = ui->FkeyGridLayout->takeAt(0)) != nullptr)
    {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
}

void DMButtonFrame::buildFkeyButtons(int count)
{
    clearFkeyLayout();
    fButtons.clear();

    int perRow = count / 2;           // e.g. 12→6, 8→4, 10→5
    if (count % 2 != 0) perRow += 1;  // if odd number, make top row larger

    int row = 0;
    int col = 0;

    for (int i = 0; i < count; ++i)
    {
        QPushButton *b = new QPushButton(this);
        b->setProperty("KeyNo", i);
        b->setText("");
        b->setObjectName(QString("FKeyButton%1").arg(i+1));

        connect(b, &QPushButton::clicked, this, &DMButtonFrame::fButtonClicked);

        fButtons << b;

        // Add to grid
        ui->FkeyGridLayout->addWidget(b, row, col);

        col++;
        if (col >= perRow) {
            col = 0;
            row++;
        }
    }
}


// ****************** Brought in from TxVmButtonsFrame *************************************//

bool  DMButtonFrame::isVoiceMode()
{
    return  curMode == rigcommon::convertModeToQString(MODE::USB)
    || curMode == rigcommon::convertModeToQString(MODE::LSB)
        || curMode == rigcommon::convertModeToQString(MODE::FM)
        || curMode == PH;

}

void DMButtonFrame::setPttTypeLabelsVisible(bool visible)
{
    // ui->pttTypeLabel->setVisible(visible);
    // ui->pttTypeText->setVisible(visible);
}

void DMButtonFrame::setPttTypeText(serialCommonData::MINOS_PTT_TYPES pttType)
{

    // ui->pttTypeText->setText(serialCommonData::pttTypeStr[static_cast<int>(pttType)]);
}

/*
void DMButtonFrame::onTxKeyerSetupClicked()
{

    if ((txKeyerType == keyerTypes[TxKeyerId::RigControl] && !isVoiceMemAvail(selectedRadio))
        || (txKeyerType == keyerTypes[TxKeyerId::CW_RigControl] && !isCwMemTypeAvail(selectedRadio)))
    {
        logMessage(QString("Setup Selected rigControl Keyer Selected, but not available for this radio or no keyer selected"));
        return;
    }


    if (txKeyerType != keyerTypes[TxKeyerId::None])
    {
        int oldnb = txKeyer->numButtons;


        int maxNumOfVoiceMessages = MAXIMUM_BUTTONS;

        if (txKeyerType == keyerTypes[TxKeyerId::RigControl])
        {
            maxNumOfVoiceMessages = getNumVoiceMessages(selectedRadio);
        }



        if (txKeyer->setup(txKeyerFactory, maxNumOfVoiceMessages, txKeyer->numButtons, selectedRadio.getLocalName()) == QDialog::Accepted)
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
*/



// also PcCwKeyer
void DMButtonFrame::setRadioParams()
{
    if (selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::CW_RigControl])
    {
        txKeyer->setRadioParams(MAXIMUM_BUTTONS, keyerSettings->getSelectedRadio().getLocalName(), keyerSettings->getPttType(keyerSettings->getSelectedRadio()), keyerSettings->getPttEnabled(keyerSettings->getSelectedRadio()));
    }
    else if (selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::RigControl])
    {
        txKeyer->setRadioParams(keyerSettings->getNumVoiceMessages(keyerSettings->getSelectedRadio()), keyerSettings->getSelectedRadio().getLocalName(), keyerSettings->getPttType(keyerSettings->getSelectedRadio()), keyerSettings->getPttEnabled(keyerSettings->getSelectedRadio()));
    }
    else if (selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer])
    {
        txKeyer->setRadioParams(PC_CW_KEYER_MAXIMUM_BUTTONS, keyerSettings->getSelectedRadio().getLocalName(), serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_NONE, false);
    }
}


void DMButtonFrame::createKeyer(QString voiceKeyerName)
{
    if (!voiceKeyerName.isEmpty())
    {


        selectedKeyerCap = txKeyerFactory->supportedTxKeyers()->value(voiceKeyerName);
        //txKeyerType = selectedKeyerCap.getKeyerType();

        if ( selectedKeyerCap.getKeyerType() != txKeyerTypes[TxKeyerId::None])
        {
            txKeyer = QSharedPointer<TxKeyerBase>(txKeyerFactory->createTxKeyer(selectedKeyerCap.getVmIdNum()));
            if (txKeyer)
            {
                logMessage(QString("Voice Keyer type selected = %1").arg(selectedKeyerCap.getKeyerName()));

                connect(txKeyer.data(), &TxKeyerBase::remoteConfigChanged, this, &DMButtonFrame::onRemoteConfigChanged, Qt::UniqueConnection);
                connect(txKeyer.data(), &TxKeyerBase::remoteKeyerStopped, this, &DMButtonFrame::onRemoteKeyerStopped, Qt::UniqueConnection);
                connect(txKeyer.data(), &TxKeyerBase::remoteKeyerStarted, this, &DMButtonFrame::onRemoteKeyerStarted, Qt::UniqueConnection);
                connect(txKeyer.data(), &TxKeyerBase::internalVoiceMemoryKeyerPlayState, this, &DMButtonFrame::onInternalVoiceMemoryPlayState);
                connect(txKeyer.data(), &TxKeyerBase::cwMacroExpandedText, this, &DMButtonFrame::onCwMacroTextProcessed);
                setRadioParams();

/*
                txKeyer->txKeyerInit(txKeyer->numButtons);

                // create the buttons
                if (txKeyerType == keyerTypes[TxKeyerId::CW_RigControl]
                    || txKeyerType == keyerTypes[TxKeyerId::RigControl]
                    || txKeyerType == keyerTypes[TxKeyerId::InternalVoiceKeyer])
                {

                    int columns = 4;
                    createButtonsForKeyer(txKeyer->numButtons, columns);
                }
                else if (txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
                {
                    int numButtons = 12;    // fixed at 12!
                    int columns = 6;
                    createButtonsForKeyer(numButtons, columns);
                }
*/
                txKeyParamList.clear();


                buttonNumSent = NO_TXKEYER_BUTTON_SELECTED;



                if (selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::CW_RigControl] ||  selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::RigControl])
                {
                    // convert to version 2 ini type
                    checkButtonIniFileVersion( selectedKeyerCap.getKeyerType());
                    checkCommonIniFileVersion( selectedKeyerCap.getKeyerType());
                }
                else
                {

                    txKeyer->txKeyerInit(txKeyer->numButtons);



/**************************************************
                    for (int i = 0; i < voiceMemButtonList.count(); i++)
                    {
                        TxKeyerParams vmData;
                        if (vmData.getType().isEmpty())
                        {
                            vmData.setType(txKeyerType);
                        }

                        //vmData.setRigModel(getRigModel(selectedRadio));
                        txKeyer->readVmButtonParams(i, vmData);
                        txKeyParamList.append(vmData);
                        setRunButtonText(i, vmData.getVmName());

                    }
*/

                }



            }

        }

    }
}


void DMButtonFrame::onExtConnectTimer()
{
/*    QString keyerName = ui->txKeyerSelect->currentText();

    if (keyerName == DIGIMODE)
    {
        return;
    }

    //selectedKeyerCap = txKeyerFactory->supportedTxKeyers()->value(keyerName);
    //txKeyerType = selectedKeyerCap.getKeyerType();

    if (LogContainer->sendDM->isKeyerLoaded())
    {
        notifyComboChange = false;
        txKeyerFactory->populateComboKeyerList(ui->txKeyerSelect, keyerName);
        notifyComboChange = true;
    }

    if (!txKeyer &&  selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::ExternalMqtKeyer])
    {
        createKeyer(keyerName);
        if (txKeyer)
        {
            ui->noExtKeyerLabel->clear();
            extKeyerConnectTimer->stop();
        }
        else
        {
             selectedKeyerCap.getKeyerType() = txKeyerTypes[TxKeyerId::None];
        }
    }
    else if (txKeyer)
    {
        ui->noExtKeyerLabel->clear();
        extKeyerConnectTimer->stop();
    }*/
}


void DMButtonFrame::onTxKeyerSelect(int idx)
{
    Q_UNUSED(idx)

    if (fixedMode) return;


    if (!notifyComboChange)
        return;

/*
    QString txKeyerName = ui->txKeyerSelect->currentText();
    logMessage(QString("onVoiceKeyerSelect - keyer select name = %1").arg( ui->txKeyerSelect->currentText()));

    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICEKEYER_COMMON_PARAMS_FILENAME;
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup(VOICEKEYER_COMMON_PARAMS_GROUPNAME);

    config.setValue("KeyerName", txKeyerName);

    config.endGroup();

    txKeyer.clear();
*/
/*

    selectedKeyerCap.clear();

    if (txKeyerNameToId(txKeyerName) != DigitalModes)
    {
        // flag we are in Digital Mode
         selectedKeyerCap.getKeyerType() = txKeyerTypes[TxKeyerId::DigitalModes];

    }
    else
    {
        selectedKeyerCap = txKeyerFactory->supportedTxKeyers()->value(txKeyerName);

    }
*/
/*
    delayedAction(this, [=]{
        if (txKeyerName != DIGIMODE)
        {
            createKeyer(txKeyerName);   // don't create a keyer when in Digimode
        }
        //setFrameState(txKeyerName);
        setFrameStateForKeyer(txKeyerName);
    });
*/

    extKeyerConnectTimer->start(1000);

    // ui->txKeyerSelect->repaint();   // or the combo doesn't update

}

void  DMButtonFrame::onCwMacroTextProcessed(const QString &cwTextSent)
{
    clearCwMessagePlayingDisplay();
    displayCwMessagePlaying(cwTextSent);

    if (buttonNumSent == CW_FREE_TEXT_BUTTON_NUMBER)
    {
        setCwFreeTextIndicatorOnOff(true);
    }

}


void DMButtonFrame::createKeyerForms()
{

    keyerFormsStack = new QStackedWidget(this);

    // Create all the keyer forms
    noneForm = new TxKeyerNoneForm(this);
    voiceRigControlForm = new TxVoiceRigControlForm(this);
    cwRigControlForm = new TxKeyerCwRigControlForm(this);
    cwDtrForm = new TxKeyerCwDtrForm(this);

    // Add forms to stacked widget
    keyerFormsStack->addWidget(noneForm);           // Index 0
    keyerFormsStack->addWidget(voiceRigControlForm); // Index 1
    keyerFormsStack->addWidget(cwRigControlForm);    // Index 2
    keyerFormsStack->addWidget(cwDtrForm);           // Index 3

    // Add stacked widget to your main layout
    // IMPORTANT: You need to have a layout in your .ui file where this will go
    // Let's assume you have a QVBoxLayout called "keyerFormLayout" in your UI
    ui->mainContentLayout->addWidget(keyerFormsStack);

    // Connect signals from forms
    connectFormSignals();

    // Start with none form visible
    keyerFormsStack->setCurrentWidget(noneForm);

    qDebug() << "Stacked widget created with" << keyerFormsStack->count() << "forms";
}

void DMButtonFrame::connectFormSignals()
{
    // For now, just add debug output when forms are created
    qDebug() << "Connecting form signals";

    // We'll add the actual signal connections later
    // For now, just verify the forms exist
    if (voiceRigControlForm)
        qDebug() << "Voice Rig Control form created";
    if (cwRigControlForm)
        qDebug() << "CW Rig Control form created";
    if (cwDtrForm)
        qDebug() << "CW DTR form created";
    if (noneForm)
        qDebug() << "None form created";
}




void DMButtonFrame::setFrameStateForKeyer(QString txKeyerName)
{
    if (!ct) return;

    TxKeyerId txKeyerId;

    //if (txKeyerName.isEmpty())
    //{
    //    txKeyerId =TxKeyerId::None;
    //    logMessage(QString("setFrameState =  None"));
    //    keyerFormsStack->setCurrentWidget(noneForm);
    //}
    //else
    //{
        // empty will give txKeyerId = None
        txKeyerId = txKeyerNameToId(txKeyerName);
    //}



    // Clear UI elements first
    clearAll_Ui_Elements();
    // ///////ui->noExtKeyerLabel->clear();
    clearErrorMessage();


    switch (txKeyerId)
    {
    case TxKeyerId::RigControl:
        logMessage(QString("setFrameState =  Voice RigControl"));
        currentName = ct->rigControlCurrentFKeySetContest.getValue();
        fkeyFileName = TX_KEYER_PATH().append(rigControlKeyerConfigFilename);
        keyerFormsStack->setCurrentWidget(voiceRigControlForm);
        break;
    case TxKeyerId::CW_RigControl:
        logMessage(QString("setFrameState =  CW RigControl"));
        currentName = ct->cwRigControlCurrentFKeySetContest.getValue();
        fkeyFileName = TX_KEYER_PATH().append(cwRigControlKeyerConfigFilename);
        keyerFormsStack->setCurrentWidget(cwRigControlForm);
        break;
    case TxKeyerId::PcCwKeyer:
        logMessage(QString("setFrameState =  CW DTR Keyer"));
        currentName = ct->pcCwKeyerCurrentFKeySetContest.getValue();
        fkeyFileName = TX_KEYER_PATH().append(pcCwKeyerKeyerConfigFilename);
        keyerFormsStack->setCurrentWidget(cwDtrForm);
        break;
    case TxKeyerId::InternalVoiceKeyer:
        currentName = ct->internalVoiceKeyerCurrentFKeySetContest.getValue();
        fkeyFileName = TX_KEYER_PATH().append(InternalKeyerConfigFilename);
        break;
    case TxKeyerId::None:
        logMessage(QString("setFrameState =  None"));
        keyerFormsStack->setCurrentWidget(noneForm);
        break;
    default:
        txKeyParamList.clear();
        currentName.clear();
        return;     // exit.....
        break;
    }

    // Load the keyer configuration file
    readSingleKeyerFile(fkeyFileName, selectedKeyerCap.getKeyerType());

    // Apply the UI config map
    //if (!keyerUiMap.count(txKeyerId)) return;
    //const UiElementCfg &cfg = keyerUiMap[txKeyerId];

    // setCWEntryElementsVisible(cfg.showCwEntryBox);
    // setTXStatusVisible(cfg.showTxStatus);
    // setKeyerIndicatorGroupBoxVisible(cfg.showKeyerIndicators);
    // setPttIndicatorGroupBoxVisible(cfg.showPttIndicators);

    // ui->stopButton->setVisible(cfg.showStopButton);
    // ui->selectedRadioLabel->setVisible(cfg.showRadioLabel);
    // ui->pipCb->setVisible(cfg.showPip);

    // setAvailIndicatorVisible(cfg.showAvailIndicator);
    // setRepeatIndicatorVisible(cfg.showRepeatIndicator);

    // Handle CW speed slider dynamically
    // if (cfg.showCwSpeedSlider)
    // {

    //     cwSpeedSlider->show();
    // }
    // else
    // {

    //    cwSpeedSlider->hide();
    // }

    // Radio & keyer setup
    txKeyer->setContest(ct);
    txKeyer->setRadioParams(
        keyerSettings->getNumVoiceMessages(keyerSettings->getSelectedRadio()),
        keyerSettings->getSelectedRadio().getLocalName(),
        keyerSettings->getPttType(keyerSettings->getSelectedRadio()),
        keyerSettings->getPttEnabled(keyerSettings->getSelectedRadio())
        );

    txKeyer->txKeyerInit(txKeyer->numButtons);

    setPttTypeLabelsVisible(true);
    setPttTypeText(keyerSettings->getPttType(keyerSettings->getSelectedRadio()));
    setPttEnabledIndicatorOnOff(keyerSettings->getPttEnabled(keyerSettings->getSelectedRadio()));

    setEomTypeLabelsVisible(true);
    setEomLabelText(txKeyer->getSelectedEomType());

    // setMessagePlayingFlag(cfg.showCwEntryBox);
    // setCwMessagePlayingVisible(cfg.showCwEntryBox);

    setLogItButtonVisible(true);

    // Optional: initialize CW text entry if applicable
    // if (cfg.showCwEntryBox)
    // {
    //    initCwTextEntryBox(
    //         getCwRadioManufacturer(keyerSettings->getCwMemType(keyerSettings->getSelectedRadio())),
    //         CWKEYER_RADIO_COMMON_PARAMS_FILENAME
    //         );
    // }

    displayButtons();
}




void DMButtonFrame::setFrameState(QString txKeyerName)
{

    if (txKeyerNameToId(txKeyerName) == DigitalModes)
    {

        set_DigiMode_FrameState();
        return;
    }



    //TxKeyerCapabilities voiceCap = txKeyerFactory->supportedTxKeyers()->value(txKeyerName);

    if (txKeyer == nullptr)
    {

        set_None_FrameState();


    }
    else
    {
        if (txKeyerNameToId(txKeyerName) == RigControl)
        {
            set_rigControl_FrameState();
        }
        else if (txKeyerNameToId(txKeyerName) == CW_RigControl)
        {
            set_cwRigControl_FrameState();
        }
        else if (txKeyerNameToId(txKeyerName) == PcCwKeyer)
        {
            set_pcCwKeyer_FrameState();
        }
        else if (txKeyerNameToId(txKeyerName) == InternalVoiceKeyer)
        {
            set_Internal_FrameState();
        }
        else if (InternalVoiceKeyer == ExternalMqtKeyer)
        {
           set_External_FrameState();
        }

    }
}


void DMButtonFrame::set_DigiMode_FrameState()
{
    txKeyParamList.clear();
    selectedKeyerCap.getKeyerType() = txKeyerTypes[TxKeyerId::DigitalModes];

    clearButtonLabels();
    setCwEntryBoxVisible(false);

    setAvailIndicatorVisible(false);
    setRepeatIndicatorVisible(false);


    // ui->pipCb->setVisible(false);
    // ui->stopButton->setVisible(true);
    // ui->selectedRadioLabel->setVisible(false);
    setTXStatusVisible(false);
    setEomTypeLabelsVisible(false);
    setKeyerIndicatorGroupBoxVisible(false);
    setPttIndicatorGroupBoxVisible(false);
    setErrorMessageVisible(false);
    setCwMessagePlayingVisible(false);
    setMessagePlayingFlag(false);

    setLogItButtonVisible(true);
    setLogItButtonVisible(true);

    if (!ct)
    {
        return;
    }

    currentName = ct->digitalModesCurrentFKeySetContest.getValue();
    bool currentNameOk = false;
    populateFksetCombo(selectedKeyerCap.getKeyerType(), currentName, currentNameOk);

    if (!currentNameOk)
    {
        currentName = KEYER_DEFAULT_CONTEST;  // current contest name is not in the list
    }

    fkeyFileChanged();
}


void DMButtonFrame::set_None_FrameState()
{


    clearButtonLabels();
    txKeyParamList.clear();

    if ( selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::ExternalMqtKeyer])
    {
        // ui->noExtKeyerLabel->setText(HtmlFontColour(Qt::red) +  tr("To use the external keyer mqtKeyer must be running and connected"));

    }

     selectedKeyerCap.getKeyerType() = txKeyerTypes[TxKeyerId::None];


    clearAll_Ui_Elements();
}

void DMButtonFrame::clearAll_Ui_Elements()
{
    clearButtonLabels();
    setCWEntryElementsVisible(false);

    setAvailIndicatorVisible(false);
    setRepeatIndicatorVisible(false);


    // ui->pipCb->setVisible(false);
    // ui->stopButton->setVisible(false);
    // ui->selectedRadioLabel->setVisible(false);
    setTXStatusVisible(false);
    setEomTypeLabelsVisible(false);
    setKeyerIndicatorGroupBoxVisible(false);
    setPttIndicatorGroupBoxVisible(false);
    setErrorMessageVisible(false);
    setCwMessagePlayingVisible(false);
    setMessagePlayingFlag(false);
    clearErrorMessage();
    //cwSpeedSlider->hide();

}

void DMButtonFrame::creatCwSlider()
{
    // cwSpeedSlider = new CwSpeedControl(ui->cwSpeedSliderFrame);
    // ui->cwSpeedSliderHorizontalLayout->addWidget(cwSpeedSlider);

    // cwSpeedSlider->setSpeedRange(
    //     TxKeyerCommon::PC_CW_KEYER_MIN_WPM,
    //     TxKeyerCommon::PC_CW_KEYER_MAX_WPM
    //    );

/*
    connect(cwSpeedSlider, &CwSpeedControl::cwSpeedChanged, this,
            [this](int wpm){ emit sendWpmToPcCwkeyer(wpm); });


    cwSpeedSlider->hide();
*/
}

void DMButtonFrame::set_rigControl_FrameState()
{

    clearAll_Ui_Elements(); // we do this to try and make it clear that the radio definition
                            // is missing for the contest

    // ui->noExtKeyerLabel->clear();

    if (!ct)
    {
        return;
    }

    currentName = ct->rigControlCurrentFKeySetContest.getValue();

    logMessage(QString("set_rigControl_framestate - current contest = %1").arg(currentName));

    fkeyFileName = TX_KEYER_PATH().append(rigControlKeyerConfigFilename);

    readSingleKeyerFile(fkeyFileName, selectedKeyerCap.getKeyerType());  // also populates FkSetCombo

    //parseFKeyFile(fkeyFileName);


    //populateFksetCombo(txKeyerName, currentName);

    logMessage(QString("Set RigControl Frame State for Contest Name %1, Radio %2").arg(currentName, keyerSettings->getSelectedRadio().key()));

    int checkContestRadioErrorCode = CHECK_RAD_CONT_CONTEST_OK;
    if (!checkContestAndRadioAvailable(checkContestRadioErrorCode))    // check keyer, contest and radio if applicable have been retrieved from json file
    {
        // error
        if (checkContestRadioErrorCode == CHECK_RAD_CONT_CONTEST_MISSING)
        {
            logMessage(QString("Contest %1 is missing set default").arg(currentName));
            currentName = KEYER_DEFAULT_CONTEST;
        }
        else if (checkContestRadioErrorCode == CHECK_RAD_CONT_RADIO_MISSING)
        {
            logMessage(QString("Radio %1 is missing set default radio ").arg(keyerSettings->getSelectedRadio().key()));
            return;     // may be we need to set the radio to noRadio???

        }
        else if (checkContestRadioErrorCode == CHECK_RAD_CONT_KEYER_MISING)
        {
            logMessage(QString("set rigControl frame state - keyerType missing %1").arg(selectedKeyerCap.getKeyerType()));
            return;
        }
    }

    logMessage(QString("Contest %1 Exists").arg(currentName));
    clearErrorMessage();
    setupRigControl_Ui_Elements();
    //    populateRadioNameCombo(currentName);
    displayButtons();


}


void DMButtonFrame::setupRigControl_Ui_Elements()
{


    // ui->pipCb->setVisible(selectedKeyerCap.getHasPip());
    setTXStatusVisible(selectedKeyerCap.getHasTxStatus());

    // ui->selectedRadioLabel->setVisible(true);
    // ui->selectedRadioLabel->setText(keyerSettings->getSelectedRadio().key());

    setKeyerIndicatorGroupBoxVisible(true);
    setPttIndicatorGroupBoxVisible(true);
    setMessagePlayingFlag(false);
    setCwMessagePlayingVisible(false);

    setLogItButtonVisible(false);
    setLogItButtonVisible(false);


    if (selectedKeyerCap.getHasAvailStatus())
    {
        setAvailIndicatorVisible(selectedKeyerCap.getHasAvailStatus());
        setAvailIndicatorForRadioOnOff(keyerSettings->getSelectedRadio());
    }
    else
    {
        setAvailIndicatorVisible(false);
    }

    setRepeatIndicatorVisible(selectedKeyerCap.getHasMessageRepeat());

    // ui->stopButton->setVisible(true);


    //setSaveButtonByRadionameText(keyerSettings->getSelectedRadio().getLocalName());



    txKeyer->setRadioParams(keyerSettings->getNumVoiceMessages(keyerSettings->getSelectedRadio()), keyerSettings->getSelectedRadio().getLocalName(), keyerSettings->getPttType(keyerSettings->getSelectedRadio()), keyerSettings->getPttEnabled(keyerSettings->getSelectedRadio()));
    txKeyer->txKeyerInit(txKeyer->numButtons);
    setPttTypeLabelsVisible(true);
    setPttTypeText(keyerSettings->getPttType(keyerSettings->getSelectedRadio()));
    setPttEnabledIndicatorOnOff(keyerSettings->getPttEnabled(keyerSettings->getSelectedRadio()));
    setEomTypeLabelsVisible(true);
    setEomLabelText(txKeyer->getSelectedEomType());
    setCwEntryBoxVisible(false);
    //loadButtonData();

    if (!keyerSettings->getRigVoiceKeyerSupportStopFlag(keyerSettings->getSelectedRadio().getLocalName()))
    {
       // ui->stopButton->setVisible(false);
    }
    else
    {
        //ui->stopButton->setVisible(true);
    }

}

void DMButtonFrame::set_cwRigControl_FrameState()
{
    clearAll_Ui_Elements(); // we do this to try and make it clear that the radio definition
                            // is missing for the contest

    // ui->noExtKeyerLabel->clear();

    if (!ct)
    {
        return;
    }

    currentName = ct->cwRigControlCurrentFKeySetContest.getValue();

    fkeyFileName = TX_KEYER_PATH().append(cwRigControlKeyerConfigFilename);
    readSingleKeyerFile(fkeyFileName, selectedKeyerCap.getKeyerType());  // also populates FkSetCombo

    //parseFKeyFile(fkeyFileName);    // also populates FkSetCombo

    logMessage(QString("Set Cw RigControl Frame State for Contest Name %1, Radio %2").arg(currentName, keyerSettings->getSelectedRadio().key()));

    int checkContestRadioErrorCode = CHECK_RAD_CONT_CONTEST_OK;
    if (!checkContestAndRadioAvailable(checkContestRadioErrorCode))    // check keyer, contest and radio if applicable have been retrieved from json file
    {
        // error
        if (checkContestRadioErrorCode == CHECK_RAD_CONT_CONTEST_MISSING)
        {
            logMessage(QString("Contest %1 is missing set default").arg(currentName));
            currentName = KEYER_DEFAULT_CONTEST;
        }
        else if (checkContestRadioErrorCode == CHECK_RAD_CONT_RADIO_MISSING)
        {
            logMessage(QString("Radio %1 is missing set default radio ").arg(keyerSettings->getSelectedRadio().key()));
            return;     // may be we need to set the radio to noRadio???

        }
        else if (checkContestRadioErrorCode == CHECK_RAD_CONT_KEYER_MISING)
        {
            logMessage(QString("set cw_rigControl frame state - keyerType missing %1").arg(selectedKeyerCap.getKeyerType()));
            return;
        }
    }


    clearErrorMessage();
    setupCw_RigControl_Ui_Elements();
    displayButtons();

}


void DMButtonFrame::setupCw_RigControl_Ui_Elements()
{

    // ui->pipCb->setVisible(selectedKeyerCap.getHasPip());
    setTXStatusVisible(selectedKeyerCap.getHasTxStatus());

    // ui->selectedRadioLabel->setVisible(true);
    // ui->selectedRadioLabel->setText(keyerSettings->getSelectedRadio().key());

    setKeyerIndicatorGroupBoxVisible(true);
    setPttIndicatorGroupBoxVisible(true);
    setMessagePlayingFlag(false);
    setCwMessagePlayingVisible(false);



    if (selectedKeyerCap.getHasAvailStatus())
    {
        setAvailIndicatorVisible(selectedKeyerCap.getHasAvailStatus());
        setAvailIndicatorForRadioOnOff(keyerSettings->getSelectedRadio());
    }
    else
    {
        setAvailIndicatorVisible(false);
    }

    setRepeatIndicatorVisible(selectedKeyerCap.getHasMessageRepeat());
    txKeyer->setCwMemType(keyerSettings->getCwMemType(keyerSettings->getSelectedRadio()));

    txKeyer->setContest(ct);

    // ui->stopButton->setVisible(true);

    txKeyer->setRadioParams(keyerSettings->getNumVoiceMessages(keyerSettings->getSelectedRadio()), keyerSettings->getSelectedRadio().getLocalName(), keyerSettings->getPttType(keyerSettings->getSelectedRadio()), keyerSettings->getPttEnabled(keyerSettings->getSelectedRadio()));
    txKeyer->txKeyerInit(txKeyer->numButtons);
    setPttTypeLabelsVisible(true);
    setPttTypeText(keyerSettings->getPttType(keyerSettings->getSelectedRadio()));
    setPttEnabledIndicatorOnOff(keyerSettings->getPttEnabled(keyerSettings->getSelectedRadio()));
    setEomTypeLabelsVisible(true);
    setEomLabelText(txKeyer->getSelectedEomType());
    //loadButtonData();

    setLogItButtonVisible(true);
    setLogItButtonVisible(true);

    setCwEntryBoxVisible(true);
    setCwMessagePlayingVisible(true);

    if (!keyerSettings->getRigCwKeyerSupportStopFlag(keyerSettings->getSelectedRadio().getLocalName()))
    {
       // ui->stopButton->setVisible(false);
    }
    else
    {
        // ui->stopButton->setVisible(true);
    }

    initCwTextEntryBox(getCwRadioManufacturer(keyerSettings->getCwMemType(keyerSettings->getSelectedRadio())), CWKEYER_RADIO_COMMON_PARAMS_FILENAME);

}


void DMButtonFrame::set_pcCwKeyer_FrameState()
{
    // ui->noExtKeyerLabel->clear();
    clearErrorMessage();

    if (!ct)
    {
        return;
    }

    currentName = ct->pcCwKeyerCurrentFKeySetContest.getValue();

    fkeyFileName = TX_KEYER_PATH().append(pcCwKeyerKeyerConfigFilename);

    readSingleKeyerFile(fkeyFileName, selectedKeyerCap.getKeyerType());  // also populates FkSetCombo

    //parseFKeyFile(fkeyFileName);    // also populates FkSetCombo



    // ui->pipCb->setVisible(selectedKeyerCap.getHasPip());
    setTXStatusVisible(selectedKeyerCap.getHasTxStatus());

    // ui->selectedRadioLabel->setVisible(false);

    setKeyerIndicatorGroupBoxVisible(true);
    setPttIndicatorGroupBoxVisible(true);
    setMessagePlayingFlag(false);
    setCwMessagePlayingVisible(false);


    setAvailIndicatorVisible(selectedKeyerCap.getHasAvailStatus());
    setAvailIndicatorOnOffForPcCwKeyer();


    setRepeatIndicatorVisible(selectedKeyerCap.getHasMessageRepeat());

    txKeyer->setContest(ct);

    // ui->stopButton->setVisible(true);
    txKeyer->txKeyerInit(txKeyer->numButtons);
    setPttTypeLabelsVisible(true);
    setPttTypeText(keyerSettings->getPttType(keyerSettings->getSelectedRadio()));
    setPttEnabledIndicatorOnOff(keyerSettings->getPttEnabled(keyerSettings->getSelectedRadio()));
    setEomTypeLabelsVisible(true);
    setEomLabelText(txKeyer->getSelectedEomType());


    setLogItButtonVisible(true);
    setLogItButtonVisible(true);

    setCwEntryBoxVisible(true);
    setCwMessagePlayingVisible(true);

    // ui->stopButton->setVisible(true);


    initCwTextEntryBox(getCwRadioManufacturer(keyerSettings->getCwMemType(keyerSettings->getSelectedRadio())), CWKEYER_RADIO_COMMON_PARAMS_FILENAME);

    displayButtons();
    //currentName = ct->pcCwKeyerCurrentFKeySetContest.getValue();
    //populateFksetCombo(txKeyerName, currentName);
    //fkeyFileChanged();


}




void DMButtonFrame::set_Internal_FrameState()
{
    // ui->noExtKeyerLabel->clear();
    clearErrorMessage();

    if (cwSpeedSlider)
    {
        cwSpeedSlider->hide();
    }

    currentName = ct->internalVoiceKeyerCurrentFKeySetContest.getValue();

    fkeyFileName = TX_KEYER_PATH().append(InternalKeyerConfigFilename);

    readSingleKeyerFile(fkeyFileName, selectedKeyerCap.getKeyerType());  // also populates FkSetCombo


    //ui->txKeyerSetupPb->setVisible(txKeyerCap.getSetupButton());
    // ui->pipCb->setVisible(selectedKeyerCap.getHasPip());
    setTXStatusVisible(selectedKeyerCap.getHasTxStatus());

    // ui->selectedRadioLabel->setVisible(false);

    setKeyerIndicatorGroupBoxVisible(true);
    setPttIndicatorGroupBoxVisible(true);
    setMessagePlayingFlag(false);
    setCwEntryBoxVisible(false);
    setCwMessagePlayingVisible(false);

    setAvailIndicatorVisible(selectedKeyerCap.getHasAvailStatus());


    setRepeatIndicatorVisible(selectedKeyerCap.getHasMessageRepeat());

    // ui->stopButton->setVisible(true);

    //setSaveButtonByRadionameText(selectedRadio.getLocalName());

    txKeyer->setRadioParams(keyerSettings->getNumVoiceMessages(keyerSettings->getSelectedRadio()), keyerSettings->getSelectedRadio().getLocalName(), keyerSettings->getPttType(keyerSettings->getSelectedRadio()), keyerSettings->getPttEnabled(keyerSettings->getSelectedRadio()));
    txKeyer->txKeyerInit(txKeyer->numButtons);
    setPttTypeLabelsVisible(true);
    setPttTypeText(keyerSettings->getPttType(keyerSettings->getSelectedRadio()));
    setPttEnabledIndicatorOnOff(keyerSettings->getPttEnabled(keyerSettings->getSelectedRadio()));
    setEomTypeLabelsVisible(true);
    setEomLabelText(txKeyer->getSelectedEomType());
    //    loadButtonData();


    setPttTypeLabelsVisible(true);
    setPttTypeText(keyerSettings->getPttType(keyerSettings->getSelectedRadio()));
    setPttEnabledIndicatorOnOff(keyerSettings->getPttEnabled(keyerSettings->getSelectedRadio()));

    //currentName = ct->internalVoiceKeyerCurrentFKeySetContest.getValue();
    //populateFksetCombo(selectedKeyerCap.getKeyerType(), currentName);
    //fkeyFileChanged();
}


void DMButtonFrame::set_External_FrameState()
{
    // not sure what this should be....
    if (cwSpeedSlider)
    {
        cwSpeedSlider->hide();
    }

    setCwEntryBoxVisible(false);
    setCwMessagePlayingVisible(false);
    clearErrorMessage();
    // ui->selectedRadioLabel->setVisible(false);

     currentName = ct->externalVoiceKeyerCurrentFKeySetContest.getValue();

     bool currentNameOk = false;
     populateFksetCombo(selectedKeyerCap.getKeyerType(), currentName, currentNameOk);

     if (!currentNameOk)
     {
         currentName = KEYER_DEFAULT_CONTEST; // current contest name is not in the list
     }



     fkeyFileChanged();
}


void DMButtonFrame::DMButtonFrame::updateFrameState()
{
    // if (!ui->txKeyerSelect->currentText().isEmpty())
    // {
    //   setFrameStateForKeyer(ui->txKeyerSelect->currentText());
    // }


}


void DMButtonFrame::setFixedKeyerType(const QString &keyerType)
{
    fixedMode = true;
    fixedKeyerType = keyerType;

    // ui->txKeyerSelect->setVisible(false);

    notifyComboChange = false;
    createKeyer(keyerType);
    setFrameState(keyerType);
    notifyComboChange = true;
}

QString DMButtonFrame::getCurrentKeyerType() const
{
    if (fixedMode) {
        return fixedKeyerType;
    }
    return "None"/* ui->txKeyerSelect->currentText()*/;
}




void DMButtonFrame::initCwTextEntryBox(QString radioManufacturer, QString fileName)
{
    // ui->cwEntry->setVisible(true);
    // ui->cwEntry->installEventFilter(this);

    //

    QString cwMacroCharList;
    bool cwMacroCharOk;
    if (getRigCWKeyerMacroCharacter(TXKEYER_COMMON_PARAMS_PATH(), cwMacroCharList, radioManufacturer, CWKEYER_RADIO_COMMON_PARAMS_FILENAME))
    {
        cwMacroCharOk = true;
        logMessage(QString("Retrieved CW Macro Chars %1 for manufacturer %2").arg(cwMacroCharList).arg(radioManufacturer));
    }
    else
    {
        cwMacroCharOk = false;
        logMessage(QString("Error retrieving CW Macro Chars for manufacturer %1").arg(radioManufacturer));
    }



    QString validCharCwList;
    if (getRigCWKeyerSupportedCharacters(TXKEYER_COMMON_PARAMS_PATH(), validCharCwList, radioManufacturer, CWKEYER_RADIO_COMMON_PARAMS_FILENAME))
    {
        if (cwMacroCharOk)
        {
            validCharCwList = validCharCwList.append(cwMacroCharList);
            logMessage(QString("Supported CW Chars and Macro chars %1 for manufacturer %2").arg(validCharCwList).arg(radioManufacturer));
        }
        else
        {

            logMessage(QString("Supported CW Chars with no Macro chars %1 for manufacturer %2").arg(validCharCwList).arg(radioManufacturer));
        }

    }
    else
    {
        logMessage(QString("Error retrieving supported CW Chars for manufacturer %1, no validator set").arg(radioManufacturer));
    }


    int maxNumChars = 0;
    if (getRigCWKeyerMaxMessageLength(TXKEYER_COMMON_PARAMS_PATH(), maxNumChars, radioManufacturer, fileName))
    {


        logMessage(QString("set max number of CW Chars = %1 for manufacturer %2").arg(maxNumChars).arg(radioManufacturer));
    }
    else
    {
        logMessage(QString("Error retrieving max CW Message Length for manufacturer %1").arg(radioManufacturer));

    }
    // we need better error handling here!
    auto *validator = new CWRigKeyerValidator(this);
    validator->setValidCwCharStr(validCharCwList);
    validator->setMaxNumCwChars(maxNumChars);
    // ui->cwEntry->setValidator(validator);


}


void DMButtonFrame::onCwEntryReturnPressed()
{
/*

    if  (txKeyer)
    {

        if (!checkRadioAndKeyerState())
        {
            return;         // error radio keyer state missing
        }


        if ( selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::CW_RigControl] ||  selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer])
        {
            QString message = ui->cwEntry->text().trimmed();

            if (!message.isEmpty())
            {
                ui->cwEntry->selectAll();

                if (selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::CW_RigControl])
                {

                    buttonNumSent = CW_FREE_TEXT_BUTTON_NUMBER;

                    if (curMode != rigcommon::convertModeToQString(MODE::CW) && txKeyer->getSetCwModeAndRestoreFlag())
                    {
                        savedMode = curMode;
                        emit sendModeToRadio(rigcommon::convertModeToQString(MODE::CW));
                    }
                    else
                    {
                        savedMode = curMode;        // keep current mode if CW
                    }

                }

                txKeyer->sendCwFreeTextMsg(message);
            }

        }

    }
*/
}



bool DMButtonFrame::checkRadioAndKeyerState()
{

    if (selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::RigControl] && !keyerSettings->isVoiceMemAvail(keyerSettings->getSelectedRadio()))
    {
        QString msg = tr("rigControl Voice Selected, but not available for this radio");
        logMessage(QString(msg));
        showTemporaryErrorMessage(msg, ERROR_MSG_TIMEOUT_DURATION);
        return false;
    }
    else if (selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::CW_RigControl] && !keyerSettings->isCwMemTypeAvail(keyerSettings->getSelectedRadio()))
    {
        QString msg = tr("rigControl CW Keyer selected, but not available for this radio");
        logMessage(QString(msg));
        showTemporaryErrorMessage(msg, ERROR_MSG_TIMEOUT_DURATION);
        return false;
    }
    else if (selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer] && !isPcCwKeyerLoaded())
    {
        QString msg = tr("PC CW DTR Server is not available.");
        logMessage(QString(msg));
        showTemporaryErrorMessage(msg, ERROR_MSG_TIMEOUT_DURATION);
        return false;
    }


    if (!keyerSettings->getPttEnabled(keyerSettings->getSelectedRadio()))
    {
        QString msg = tr("radio ptt is not enabled, please enable");
        logMessage(QString(msg));
        showTemporaryErrorMessage(msg, ERROR_MSG_TIMEOUT_DURATION);
        return false;
    }


    if (selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::RigControl])
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

void DMButtonFrame::startKeyerMsg(int key)
{
    logMessage(QString("- start keyerMsg button Number = %1").arg(key));

    if (!checkRadioAndKeyerState())
    {
        return;         // error radio keyer state missing
    }


    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    int msgOffset = tslf->GJVQSOLogFrame->getSandP()?12:0;

    int messageNumber = key + msgOffset; // e.g. F1=0, F12=11 (Run keys), then S&P keys 12..23

    selectedEomType = KeyerEomTypes::Eom_None;

    TxKeyerParams vmData;

    if (( selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::CW_RigControl]
         ||  selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::RigControl]
         ||  selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer])
        && !keyerSettings->getSelectedRadio().key().isEmpty())
    {

        QString rigKey;
        if ( selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer])
        {
            rigKey = KEYER_NO_RADIO;    // pcCWKeyer has noRadio in the macro definitions
        }
        else
        {
            rigKey = keyerSettings->getSelectedRadio().key();
            if (rigKey.isEmpty())
                rigKey = KEYER_NO_RADIO;  // fallback if needed
        }




        auto &contestMap = allKeyConfigs[selectedKeyerCap.getKeyerType()];
        if (contestMap.contains(currentName) && contestMap[currentName].contains(rigKey))
        {
            const ContestSection &section = contestMap[currentName][rigKey];

            const int runCount = section.run.size();
            const int spCount = section.sp.size();

            const KeyVal *kv = nullptr;

            if (messageNumber < runCount)
            {
                kv = &section.run[messageNumber];
            }
            else if (messageNumber < runCount + spCount)
            {
                kv = &section.sp[messageNumber - runCount];
            }

            if (kv)
            {
                vmData.setKeyerCwMessage(kv->kval());
                vmData.setKeyerButtonNum(messageNumber);
                vmData.setRigVoiceMemNum(kv->rigVoiceMemNum());
                vmData.setKeyerRepeatPauseDur(kv->rptDur());
                vmData.setKeyerRepeatFlag(kv->rptEnable());
                vmData.setSelRadioName(keyerSettings->getSelectedRadio().getLocalName());
                vmData.setRigModel(keyerSettings->getRigModel(keyerSettings->getSelectedRadio()));
                vmData.setSAndPState(sAndPState);
            }
        }
    }


    vmData.setType( selectedKeyerCap.getKeyerType());

    setRepeatIndicatorOnOff(vmData.getKeyerRepeatFlag());



    if ( selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::CW_RigControl]
        ||  selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer])
    {
        if (vmData.getKeyerCwMessage().isEmpty())
        {
            logMessage(QString("Cw Message is empty, ignore"));
            return;
        }


        if (keyerSettings->getCwMemType(keyerSettings->getSelectedRadio()) == hamlibData::CW_MEMORY_TYPES::ICOM
            || keyerSettings->getCwMemType(keyerSettings->getSelectedRadio()) == hamlibData::CW_MEMORY_TYPES::ELECRAFT
            || keyerSettings->getCwMemType(keyerSettings->getSelectedRadio()) == hamlibData::CW_MEMORY_TYPES::KENWOOD
            || keyerSettings->getCwMemType(keyerSettings->getSelectedRadio()) == hamlibData::CW_MEMORY_TYPES::YAESU
            || keyerSettings->getCwMemType(keyerSettings->getSelectedRadio()) == hamlibData::CW_MEMORY_TYPES::FLEX_RADIO
            || keyerSettings->getCwMemType(keyerSettings->getSelectedRadio()) == hamlibData::CW_MEMORY_TYPES::FLEX_RADIO_APACHE
            || keyerSettings->getCwMemType(keyerSettings->getSelectedRadio()) == hamlibData::CW_MEMORY_TYPES::OPENHPSDR
            || keyerSettings->getCwMemType(keyerSettings->getSelectedRadio()) == hamlibData::CW_MEMORY_TYPES::QRPLABS
            || keyerSettings->getCwMemType(keyerSettings->getSelectedRadio()) == hamlibData::CW_MEMORY_TYPES::THETIS
            ||  selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer])
        {
            if (curMode != rigcommon::convertModeToQString(MODE::CW) && txKeyer->getSetCwModeAndRestoreFlag())
            {
                savedMode = curMode;
                emit sendModeToRadio(rigcommon::convertModeToQString(MODE::CW));
            }
            else
            {
                savedMode = curMode;        // keep current mode if CW
            }

            txKeyer->sendCwMsg(vmData);

        }

    }
    else
    {
        txKeyer->sendMsgNum(vmData);

    }

    selectedEomType = txKeyer->getSelectedEomType();

//    if (selectedEomType == TxKeyerCommon::KeyerEomTypes::Timer)
//    {
//        int msgDur = vmKeyParamList[buttonNumber].getVmDuration() * 1000;
//        if (msgDur > 0)
//        {
//            logMessage(QString("- msgDurTimer->start(%1)").arg(msgDur));
//            msgDurTimer->start(msgDur);
//        }
//    }

//   txVmButtonMap[buttonNumber]->showButtonOnOff(true);

}





void DMButtonFrame::onRemoteConfigChanged()
{
    bool s = txKeyer->getPip();
    //  if (ui->pipCb->isChecked() != s)
    // {
    //    ui->pipCb->setChecked(s);
    //}
//*******************************************************************************
//    for (int i = 0; i < voiceMemButtonList.count(); i++)
//    {
//        VoiceKeyerParams vmData;
//        if (vmData.getType().isEmpty())
//        {
//            vmData.setType(txKeyerType);
//        }

//        txKeyer->readVmButtonParams(i, vmData);
//        vmKeyParamList[i] = vmData;
//        setRunButtonText(i, vmData.getVmName());
//    }

}
void DMButtonFrame::onRemoteKeyerStarted(int key)
{
    onRemoteKeyerStopped();
//*********************************************************************************************
//    buttonNumSent = key;
//    if (txKeyerType == keyerTypes[TxKeyerId::None] || buttonNumSent == NO_VM_BUTTON_ON)
//    {
//        return;
//    }

//    int msgDur = vmKeyParamList[buttonNumSent].getVmDuration() * 1000;
//    if (msgDur > 0)
//    {
//        logMessage(QString("- msgDurTimer->start(%1)").arg(msgDur));
//        msgDurTimer->start(msgDur);
//    }
//    TxVoiceMemButton *b = txVmButtonMap[buttonNumSent];
//    if (b)
//    {
//        b->showButtonOnOff(true);
//    }

}
void DMButtonFrame::onRemoteKeyerStopped()
{
//*************************************************************************************
//
//    if (txKeyerType == keyerTypes[TxKeyerId::None] || buttonNumSent == NO_VM_BUTTON_ON)
//    {
//        return;
//    }
//    //txKeyer->stopMsg();
//    msgDurTimer->stop();
//    repeatPauseTimer->stop();


//    TxVoiceMemButton *b = txVmButtonMap[buttonNumSent];
//    if (b)
//    {
//        b->showButtonOnOff(false);
//    }
//    buttonNumSent = NO_VM_BUTTON_ON;

}

void DMButtonFrame::onMsgDurTimerTimeout()
{
//**************************************************************************

//    if (buttonNumSent >= 0)
//    {
//        if (vmKeyParamList[buttonNumSent].getVmDuration() > 0
//            || selectedEomType == KeyerEomTypes::CAT
//            || txKeyer->getSelectedEomType() == KeyerEomTypes::InternalSoundCardVoiceKeyer
//            || selectedEomType == KeyerEomTypes::DTRKeyerTXStatus)
//        {
//            if (txKeyerType == keyerTypes[TxKeyerId::InternalVoiceKeyer])
//            {
//                txKeyer->stopMsg(nullptr); // ensure the sbdriver is stopped
//            }

//            // message duration of zero means that there shouldn't be a message timer running
//            if (vmKeyParamList[buttonNumSent].getVmRepeatFlag())
//            {
//                int repeatPauseDur = vmKeyParamList[buttonNumSent].getVmRepeatPauseDur() * 1000;
//                repeatPauseTimer->start(repeatPauseDur);
//            }
//            else
//            {
//                turnOffVMButton();
//            }
//        }

//    }


//    msgDurTimer->stop();


//    if (txKeyerType == keyerTypes[TxKeyerId::CW_RigControl]
//        || txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
//    {
//
//        if ((getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::ICOM
//             ||  getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::YAESU
//             || getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::KENWOOD
//             || getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::ELECRAFT
//             || getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::FLEX_RADIO
//             || getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::FLEX_RADIO_APACHE
//             || getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::OPENHPSDR
//             || getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::QRPLABS
//             || getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::THETIS
//             || txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
//            && txKeyer->getSetCwModeAndRestoreFlag())
//        {
//
//            if (curMode != savedMode)       // restore mode?
//            {
//                sendModeToRadio(savedMode);
//            }

//        }

//    }


}





void DMButtonFrame::onRepeatPauseTimerTimeout()
{

    repeatPauseTimer->stop();
//***********************************************************************************
//    if (txKeyer->doRepeatFromLogger())
//    {
//        if (buttonNumSent >= 0)
//        {
//            if (vmKeyParamList[buttonNumSent].getVmRepeatFlag())
//            {
//                logMessage("- onRepeatPauseTimerTimeout()");
//                startVMMsg(buttonNumSent);
//
//            }
//            else
//            {
//                onVmStopClicked();
//            }
//        }
//    }

}

void DMButtonFrame::setWipeButtonVisible(bool visible)
{
    // ui->wipeButton->setVisible(visible);
}

void DMButtonFrame::setLogItButtonVisible(bool visible)
{
    // ui->logitButton->setVisible(visible);
}

void DMButtonFrame::onActiveKeyerChanged()
{
    QString activeKeyer = keyerSettings->getActiveKeyerType();
    if (getCurrentKeyerType() == activeKeyer)
    {
        // Enable/disable widgets based on is Active
        setFrameStateForKeyer(activeKeyer);
    }


}

void DMButtonFrame::onContestChanged()
{
    auto contest = keyerContainer->keyerSettings->getContest();

    qDebug() << "onContestChanged contest = %1 " << contest;
    if (!contest) {
        ct = nullptr;
        // clear UI elements if needed
        return;
    }

    ct = dynamic_cast<LoggerContestLog *>(contest);
    if (!ct) {
        qWarning() << "Contest is not a LoggerContestLog";
        return;
    }

    // Only update the UI or internal values that actually changed
    QString mode = ct->currentMode.getValue();
    onModeChange(mode);

    // Other dynamic values can be updated here as needed
}

void DMButtonFrame::onLoggerRadioSettingsChanged()
{


    if (selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::CW_RigControl]
        || selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer]
        || selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::RigControl]
        || selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::InternalVoiceKeyer])
    {

        setRadioParams();

        if (selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer])
        {
            setEomTypeLabelsVisible(false);

            setPttTypeText(serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_NONE);
            setPttEnabledIndicatorOnOff(false);

        }
        else
        {
            setPttTypeText(keyerSettings->getPttType(keyerSettings->getSelectedRadio()));
            setPttEnabledIndicatorOnOff(keyerSettings->getPttEnabled(keyerSettings->getSelectedRadio()));
            setEomLabelText(txKeyer->getSelectedEomType());
        }



        txKeyer->txKeyerInit(txKeyer->numButtons);

        loadButtonData();
    }
}



void DMButtonFrame::onIsRadioConnectedChanged(bool connected)
{
    radioConnected = connected;
}

void DMButtonFrame::onSelectedRadioChanged()
{

    if (selectedKeyerCap.getKeyerType().isEmpty() || selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer])
    {
        // ignore as we don't want to update framestate
        return;
    }

    keyerSettings->getSelectedRadio();

    updateFrameState();

}

void DMButtonFrame::onPttEnabledChanged()
{

    if ( selectedKeyerCap.getKeyerType().isEmpty() ||  selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer])
    {
        // ignore as we don't want to update framestate
        return;
    }

    updateFrameState();

}



void DMButtonFrame::onPttTypeChanged()
{
    int type = static_cast<int>(keyerSettings->getPttType(keyerSettings->getSelectedRadio()));

    if ( selectedKeyerCap.getKeyerType().isEmpty() ||  selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer])
    {
        // ignore as we don't want to update framestate
        return;
    }

    logMessage(QString("setPttType = %1, selectedRadio = %2").arg(type).arg(keyerSettings->getSelectedRadio().getLocalName()));

    updateFrameState();
}


void DMButtonFrame::onVoiceMemAvailChanged()
{
    bool avail = keyerSettings->isVoiceMemAvail(keyerSettings->getSelectedRadio());

    if ( selectedKeyerCap.getKeyerType().isEmpty() ||  selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer])
    {
        // ignore as we don't want to update framestate
        return;
    }
    logMessage(QString("setVoiceMemAvail = %1, radio = %2").arg(avail ? "Yes" : "No", keyerSettings->getSelectedRadio().getLocalName()));



    if ( selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::RigControl])
    {
         updateFrameState();
    }


}


void DMButtonFrame::getVoiceCwMemSupportedRadios(const QStringList &listOfRadios, QStringList& listOfRadioSupportKeyer)
{
    listOfRadioSupportKeyer.clear();

    for (auto &radio: listOfRadios)
    {
        PubSubName radName(radio);

        if ( selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::RigControl])
        {
            if(keyerSettings->isVoiceMemAvail(radName))
            {
                listOfRadioSupportKeyer.append(radio);
            }
        }
        else if ( selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::CW_RigControl])
        {
            if(keyerSettings->isCwMemTypeAvail(radName))
            {
                listOfRadioSupportKeyer.append(radio);
            }
        }
    }
}



void DMButtonFrame::onNumVoiceMessagesChanged()
{

    int numMsgs = keyerSettings->getNumVoiceMessages(keyerSettings->getSelectedRadio());

    logMessage(QString("setNumVoiceMessages = %1, radio = %2").arg(QString::number(numMsgs), keyerSettings->getSelectedRadio().getLocalName()));

    if ( selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::RigControl])
    {
        updateFrameState(  );
    }

}



void DMButtonFrame::onRigVoiceKeyerSupportStopFlagChanged()
{
    bool supportStopCmd = keyerSettings->getRigVoiceKeyerSupportStopFlag(keyerSettings->getSelectedRadio());

    if ( selectedKeyerCap.getKeyerType().isEmpty() ||  selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer])
    {
        // ignore as we don't want to update framestate
        return;
    }

    logMessage(QString("setRigVoiceKeyerSupportStopFlag = %1, radio = %2").arg(supportStopCmd ? "Yes" : "No").arg(keyerSettings->getSelectedRadio().getLocalName()));


    if ( selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::RigControl])
    {
        updateFrameState();
    }



}




void DMButtonFrame::onRigCwKeyerSupportStopFlagChanged()
{

    bool supportStopCmd = keyerSettings->getRigCwKeyerSupportStopFlag(keyerSettings->getSelectedRadio());

    if ( selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer])
    {
        // ignore as we don't want to update framestate
        return;
    }

    logMessage(QString("setRigCwKeyerSupportStopFlag = %1, radio = %2").arg(supportStopCmd ? "Yes" : "No").arg(keyerSettings->getSelectedRadio().getLocalName()));

    if ( selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::CW_RigControl])
    {
        updateFrameState();
    }


}



void DMButtonFrame::onRigModelChanged()
{

    QString rigModel = keyerSettings->getRigModel(keyerSettings->getSelectedRadio());    logMessage(QString("setRigModel = %1, radio = %2").arg(rigModel, keyerSettings->getSelectedRadio().getLocalName()));

    if ( selectedKeyerCap.getKeyerType().isEmpty() || selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer])
    {
        // ignore as we don't want to update framestate
        return;
    }

    updateFrameState();



}

/*
bool DMButtonFrame::isCwMemTypeAvail(PubSubName psn)
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

*/
void DMButtonFrame::onCwMemTypeChanged()
{
    int cwMemType = keyerSettings->getCwMemType(keyerSettings->getSelectedRadio());

    logMessage(QString("setCwMemType = %1, radio = %2").arg(cwMemType).arg(keyerSettings->getSelectedRadio().getLocalName()));

    if (!selectedKeyerCap.getKeyerType().isEmpty() || selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::CW_RigControl])
    {
        updateFrameState();
    }

}


/*
int DMButtonFrame::getCwMemType(PubSubName psn)
{
    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        return rd.getCwMemType();
    }

    return hamlibData::CW_MEMORY_TYPES::NONE;

}
*/


void DMButtonFrame::setCWEntryElementsVisible(bool visible)
{
    setCwEntryBoxVisible(visible);
    setCwFreeTextIndicatorVisible(visible);
}

void DMButtonFrame::setCwEntryBoxVisible(bool visible)
{
    // ui->cwEntry->setVisible(visible);
    // ui->cwEntryLabel->setVisible(visible);
}


void DMButtonFrame::setCwFreeTextIndicatorOnOff(bool on)
{
    if (on)
    {
       // ui->cwFreeTextPlayingIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);
       // ui->cwFreeTextPlayingIndicator->setToolTip(tr("CW FreeText Playing"));
    }
    else
    {
       // ui->cwFreeTextPlayingIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);
       // ui->cwFreeTextPlayingIndicator->setToolTip(tr("No CW FreeText Playing"));
    }
}

void DMButtonFrame::setCwFreeTextIndicatorVisible(bool visible)
{
    // ui->cwFreeTextPlayingIndicator->setVisible(visible);
}

void DMButtonFrame::setAvailIndicatorVisible(bool visible)
{

    // ui->availLabel->setVisible(visible);
    // ui->availIndicator->setVisible(visible);
    // ui->vmAvailIndicatorLine->setVisible(visible);

}

void DMButtonFrame:: setAvailIndicatorOnOffForPcCwKeyer()
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


void DMButtonFrame::setMessagePlayingFlag(bool playing)
{
    messagePlaying = playing;
}

bool DMButtonFrame::isMessagePlaying()
{
    return messagePlaying;
}

bool DMButtonFrame::isPcCwKeyerLoaded()
{
    return LogContainer->sendDM->isPcCWkeyerLoaded();
}

bool DMButtonFrame::isPcCwKeyerConnected()
{
    return LogContainer->sendDM->isPcCwKeyerConnected();
}

void DMButtonFrame::setAvailIndicatorOnOff(bool on)
{
    if (on)
    {
        // ui->availIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);
    }
    else
    {
        // ui->availIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);

    }

    if ( selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::RigControl])
    {
        // ui->availIndicator->setToolTip(tr("Rig Voice Keyer Available"));
    }
    else if ( selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::CW_RigControl])
    {
        // ui->availIndicator->setToolTip(tr("Rig CW Messages Available"));
    }
    else
    {
        // ui->availIndicator->setToolTip(tr(""));
    }
}

void DMButtonFrame::setAvailIndicatorForRadioOnOff(PubSubName radName)
{
    if ( selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::CW_RigControl] || selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::RigControl])
    {
        if ( selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::CW_RigControl])
        {
            setAvailIndicatorOnOff(keyerSettings->isCwMemTypeAvail(radName));
        }
        else if ( selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::RigControl])
        {
            setAvailIndicatorOnOff(keyerSettings->isVoiceMemAvail(radName));
        }
        else
        {
            setAvailIndicatorOnOff(false);
        }
    }

}

void DMButtonFrame::setRepeatIndicatorVisible(bool visible)
{
    // ui->repeatIndicator->setVisible(visible);
    // ui->repeatLabel->setVisible(visible);
}

void DMButtonFrame::setTXStatusVisible(bool visible)
{
    // ui->txStatusIndicator->setVisible(visible);
    // ui->txStatusLabel->setVisible(visible);
}

void DMButtonFrame::setRepeatIndicatorForMessageOnOff(bool state)
{
    if (state)
    {
       // ui->repeatIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);
    }
    else
    {
        //ui->repeatIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);
    }
}


void DMButtonFrame::setRepeatIndicatorOnOff(bool on)
{
    if (on)
    {
        // ui->repeatIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);
    }
    else
    {
        // ui->repeatIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);

    }

}

void DMButtonFrame::setKeyerIndicatorGroupBoxVisible(bool visible)
{
    // ui->keyerIndicatorGroupBox->setVisible(visible);
}

void DMButtonFrame::setPttIndicatorGroupBoxVisible(bool visible)
{
    // ui->pttIndicatorGroupBox->setVisible(visible);
}

void DMButtonFrame::setErrorMessageVisible(bool visible)
{
    // ui->errorTitleLabel->setVisible(visible);
    //ui->errorMessageDisplay->setVisible(visible);
}


void DMButtonFrame::showTemporaryErrorMessage(const QString &msg, int timeoutMs, const QColor &colour)
{
/*    setErrorMessageVisible(true);
    // Save the current stylesheet
    // QString oldStyle = ui->errorMessageDisplay->styleSheet();

    // Set error color and text
    // ui->errorMessageDisplay->setStyleSheet(QString("color: %1;").arg(colour.name()));

    // ui->errorMessageDisplay->setText(msg);

    // Clear after timeoutMs milliseconds
    QTimer::singleShot(timeoutMs, this, [this, oldStyle]() {
       // ui->errorMessageDisplay->clear();
       // ui->errorMessageDisplay->setStyleSheet(oldStyle);
        setErrorMessageVisible(false);
    });*/
}



void DMButtonFrame::displayErrorMessage(QString msg)
{
    setErrorMessageVisible(true);
    // ui->errorMessageDisplay->setText(msg);
}

void DMButtonFrame::clearErrorMessage()
{
    // ui->errorMessageDisplay->clear();
    setErrorMessageVisible(false);
}

void DMButtonFrame::onPttStateChanged()
{
    bool state = keyerSettings->getPttState();

    if (selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer])
    {
        // ignore as we don't want to update framestate
        return;
    }

}


void DMButtonFrame::pttStopMessage(bool state)
{
    logMessage(QString("pttStopMessage state = %1").arg(state ? "true" : "false"));

    if (selectedEomType == TxKeyerCommon::KeyerEomTypes::CAT
        || selectedEomType == TxKeyerCommon::KeyerEomTypes::DTRKeyerTXStatus)
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

void DMButtonFrame::on_pipCb_stateChanged(int /*arg1*/)
{
    // txKeyer->setPip(ui->pipCb->isChecked());
}

void DMButtonFrame::setPttStatusIndicatorOnOff(bool on)
{
    if (on)
    {
        setMessagePlayingFlag(true);
       // ui->txStatusIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);
       // ui->txStatusIndicator->setToolTip(tr("TX On"));

    }
    else
    {
        setMessagePlayingFlag(false);
        // ui->txStatusIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);
        // ui->txStatusIndicator->setToolTip(tr("TX Off"));
    }

}

void DMButtonFrame::setPttEnabledIndicatorOnOff(bool on)
{
    if (on)
    {
        // ui->pttEnabledIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);
        // ui->pttEnabledIndicator->setToolTip(tr("PTT Enabled"));

    }
    else
    {
        // ui->pttEnabledIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);
        // ui->pttEnabledIndicator->setToolTip(tr("PTT Disabled"));
    }

}

void DMButtonFrame::setEomTypeLabelsVisible(bool visible)
{
    // ui->eomLabel->setVisible(visible);
    // ui->eomText->setVisible(visible);
}

void DMButtonFrame::setEomLabelText(int selectedEomType)
{
    if (selectedEomType == TxKeyerCommon::KeyerEomTypes::CAT)
    {
       // ui->eomText->setText("CAT");
    }
    else if (selectedEomType == TxKeyerCommon::KeyerEomTypes::Timer)
    {
       // ui->eomText->setText("Timer");
    }
    else if (selectedEomType == TxKeyerCommon::KeyerEomTypes::InternalSoundCardVoiceKeyer)
    {
        // ui->eomText->setText("EOF");
    }
    else if (selectedEomType == TxKeyerCommon::KeyerEomTypes::Eom_None)
    {
        // ui->eomText->setText("None");
    }
}





bool DMButtonFrame::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        // -- Only handle Return if it's from cwEntry --
        // if (obj == ui->cwEntry &&
           // (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter))
        // {
        //     if (!keyEvent->isAutoRepeat())
        //        onCwEntryReturnPressed();

        //    return true; // consume
        //}

        // -- Handle Escape globally --
        if (keyEvent->key() == Qt::Key_Escape)
        {
            if (isMessagePlaying())
            {
                on_stopButton_clicked();
                return true;                // consume Esc
            }
            // else let Esc propagate
        }
    }

    return QWidget::eventFilter(obj, event); // default
}


// ****************************************************** **********************************************//


void DMButtonFrame::setFreq(Frequency freq)
{
    curFreq = freq;
}
void DMButtonFrame::DMMess(AnalysePubSubNotify an)
{
    if (an.getKey() == rpcConstants::DMSender)
    {
        dataSender = an.getPublisherProgram() + "@" + an.getPublisherRouter();

        logMessage(QString("Datasender set to %1").arg(dataSender));

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
    readSingleKeyerFile(fkeyFileName, selectedKeyerCap.getKeyerType());
    //parseFKeyFile(fkeyFileName);

    displayButtons();
}


void DMButtonFrame::displayButtons()
{
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



void DMButtonFrame::initKeyerSettings()
{


    auto contest = keyerSettings->getContest();
    if (!contest) {
        ct = nullptr;   // clear previous pointer
        return;
    }

    // Safe dynamic cast from QSharedPointer
    ct = dynamic_cast<LoggerContestLog *>(contest);
    if (!ct) {
        qWarning() << "initKeyerSettings: contest is not a LoggerContestLog";
        return;
    }

    // Contest is valid, proceed
    QString mode = ct->currentMode.getValue();
    onModeChange(mode);


}




void DMButtonFrame::setContest(BaseContestLog *c)
{
    ct = dynamic_cast<LoggerContestLog *>( c);
    if (ct)
    {
        //currentName = ct->currentFKeySet.getValue();
        //ui->fkeysetCombo->setCurrentText(currentName);
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

    if (fixedMode && !isVisible()) {
        return;
    }


    if (c && c == ct)
    {
        if (isDataMode() &&  selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::DigitalModes])
        {
            // digital mode isn't a keyer..
            actionDigitalModeKeyPress(key, carr);
        }
        else if (txKeyer)
        {
            startKeyerMsg(key);

        }
    }
}



void DMButtonFrame::actionDigitalModeKeyPress(int key, int carr)
{
 /*   if (key >= Qt::Key_F1 && key <= Qt::Key_F12 && fkeys[currentName].size() == 24)
    {
        TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
        int spoffset = tslf->GJVQSOLogFrame->getSandP()?12:0;
        KeyVal mess = fkeys[currentName][key - Qt::Key_F1 + spoffset];

        QString toSend = parseFKeyMessage(mess.kval);

 */
        if (key >= Qt::Key_F1 && key <= Qt::Key_F12)
        {
            TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
            bool sandp = tslf->GJVQSOLogFrame->getSandP();
            //int spoffset = sandp ? 12 : 0;

            QString rigKey = KEYER_NO_RADIO;

            auto &contestMap = allKeyConfigs[selectedKeyerCap.getKeyerType()];
            if (contestMap.contains(currentName) && contestMap[currentName].contains(rigKey))
            {
                const ContestSection &section = contestMap[currentName][rigKey];

                int index = key - Qt::Key_F1;
                KeyVal mess;

                if (!sandp)
                {
                    if (index < section.run.size())
                        mess = section.run[index];
                }
                else
                {
                    if (index < section.sp.size())
                        mess = section.sp[index];
                }

                QString toSend = parseFKeyMessage(mess.kval());
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
    //ui->FButtonFrame->setEnabled(false);


    MinosRPC *rpc = MinosRPC::getMinosRPC();

    QString rigKey = KEYER_NO_RADIO;
    if ( selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::RigControl] || selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::CW_RigControl])
    {
        rigKey = keyerSettings->getSelectedRadio().key();
    }

    auto &contestMap = allKeyConfigs[selectedKeyerCap.getKeyerType()];
    if (contestMap.contains(currentName) && contestMap[currentName].contains(rigKey))
    {
        const ContestSection &section = contestMap[currentName][rigKey];

        const KeySet &keysToShow = s ? section.sp : section.run;

        if (keysToShow.size() >= 12)
        {
            for (int i = 0; i < 12; i++)
            {
                QString keytop = QString("F%1: %2").arg(i + 1).arg(keysToShow[i].ktop());
                fButtons[i]->setText(keytop);
            }
          //  ui->FButtonFrame->setEnabled(true);

            QString fkeystring = getFKeysString();

            rpc->publish(rpcConstants::DMCat, rpcConstants::DMFKeys, fkeystring, psPublished);
            return;
        }
        else if (keysToShow.size() == 0)
        {
            for (int i = 0; i < 12; i++)
            {
                fButtons[i]->setText(QString("F%1").arg(i + 1));
            }
            rpc->publish(rpcConstants::DMCat, rpcConstants::DMFKeys, "", psRevoked);
            return;
        }
        else
        {
            mShowMessage(tr("Not enough key definitions in %1").arg(fkeyFileName), this);
            rpc->publish(rpcConstants::DMCat, rpcConstants::DMFKeys, "", psRevoked);
        }
    }


}

/*
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
*/


void DMButtonFrame::populateFksetCombo(QString txKeyerType, QString currentName, bool &contestNameOk)
{
    ignoreFkComboSignal = true;
    contestNameOk = false;

    nameList.clear();
    // ui->fkeysetCombo->clear();

    nameList = getContestNamesForKeyerType(txKeyerType);
    // ui->fkeysetCombo->addItems(nameList);

    if (nameList.contains(currentName))
    {
       // ui->fkeysetCombo->setCurrentText(currentName);
        contestNameOk = true;
    }


    ignoreFkComboSignal = false;
}


QStringList DMButtonFrame::getContestNamesForKeyerType(const QString &keyerType)
{
    QStringList contestNames;

    auto keyerIt = allKeyConfigs.constFind(keyerType);
    if (keyerIt != allKeyConfigs.constEnd())
    {
        const ContestMap &contestMap = keyerIt.value();
        contestNames = contestMap.keys();
    }

    return contestNames;
}


/*
void DMButtonFrame::populateRadioNameCombo(const QString &contestName)
{

    ui->radioSelectCombo->clear();

    QStringList radioList = getRadioNamesForSelectedContestName(contestName);
    ui->radioSelectCombo->addItems(radioList);
    ui->radioSelectCombo->setCurrentText(selectedRadio.getLocalName());

}

void DMButtonFrame::connectFkeySetComboToPopulateRadioNameCombo()
{
    disconnect(ui->fkeysetCombo, &QComboBox::currentTextChanged, this, &DMButtonFrame::onFkeysetComboSelected);
}


void DMButtonFrame::disConnectFkeySetComboToPopulateRadioNameCombo()
{
    connect(ui->fkeysetCombo, &QComboBox::currentTextChanged, this, &DMButtonFrame::onFkeysetComboSelected);
}

void DMButtonFrame::onFkeysetComboSelected()
{
    populateRadioNameCombo(currentName);
}

*/


QStringList DMButtonFrame::getRadioNamesForSelectedContestName(const QString &contestName)
{

    QStringList radioList;

    // Check if the contest exists for this keyer type
    if (!allKeyConfigs.contains(selectedKeyerCap.getKeyerType()))
        return radioList;

    auto &contestMap = allKeyConfigs[selectedKeyerCap.getKeyerType()];

    if (!contestMap.contains(contestName))
        return radioList;

    // Get all rig keys for this contest
    auto &rigMap = contestMap[contestName];
    radioList = rigMap.keys();

    return radioList;


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
                    logMessage(QString("Message <%1> contains unknown macro {%2}").arg(mess, macro));
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


bool DMButtonFrame::readSingleKeyerFile(const QString &filePath, const QString &keyerType)
{

    KeyerMap tempKeyConfigs;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Cannot open" << filePath;
        return false;
    }
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError)
    {
        qWarning() << "JSON parse error in" << filePath << ":" << err.errorString();
        return false;
    }
    QJsonObject root = doc.object();
    if (!root.contains("KeyerConfig"))
    {
        qWarning() << "Missing KeyerConfig in" << filePath;
        return false;
    }
    QJsonObject keyerConfig = root["KeyerConfig"].toObject();
    if (!keyerConfig.contains(keyerType))
    {
        qWarning() << "Keyer type" << keyerType << "not found in" << filePath;
        return false;
    }
    QJsonObject contestMapJson = keyerConfig[keyerType].toObject();

    // Build into tempKeyConfigs instead
    ContestMap &contestMapRef = tempKeyConfigs[keyerType];

    const QStringList contestNames = contestMapJson.keys();
    for (const QString &contestName : contestNames)
    {
        QJsonObject rigMapJson = contestMapJson[contestName].toObject();
        RigMap &rigMapRef = contestMapRef[contestName];


        const QStringList rigModels = rigMapJson.keys();
        for (const QString &rigModel : rigModels)
        {
            QJsonObject section = rigMapJson[rigModel].toObject();
            ContestSection cs;

            if (section.contains("Run"))
            {
                QJsonArray runArr = section["Run"].toArray();
                cs.run.clear();

                for (const QJsonValue &val : std::as_const(runArr))
                {
                    if (!val.isObject()) continue;
                    KeyVal kv;
                    kv.fromJson(val.toObject());
                    cs.run.append(kv);
                }
            }
            if (section.contains("SandP"))
            {
                QJsonArray spArr = section["SandP"].toArray();
                cs.sp.clear();
                for (const QJsonValue &val :std::as_const(spArr))
                {
                    if (!val.isObject()) continue;
                    KeyVal kv;
                    kv.fromJson(val.toObject());
                    cs.sp.append(kv);
                }
            }
            if (section.contains("Common"))
            {
                cs.common.fromJson(section["Common"].toObject());
            }
            else
            {
                cs.common = CommonVal();
            }

            rigMapRef[rigModel] = cs;
        }
    }

    // Only at the very end, after everything is built, assign to the output parameter
    allKeyConfigs = tempKeyConfigs;

    bool currentNameOk = false;
    populateFksetCombo(selectedKeyerCap.getKeyerType(), currentName, currentNameOk);
    if (!currentNameOk)
    {
        currentName = KEYER_DEFAULT_CONTEST;
        ct->rigControlCurrentFKeySetContest.setValue(currentName);  // save back the contest name
        ct->commonSave(false);

    }
    clearAllDirtyFlags();
    return true;
}



bool DMButtonFrame::writeSingleKeyerFile(const QString &filePath, const QString &keyerType, TxKeyerId keyerId)
{
    if (!allKeyConfigs.contains(keyerType))
    {

        logMessage(QString("writeKeyerFile - No data for keyer type: %1").arg(keyerType));
        return false;
    }

    QJsonObject keyerTypeObj;

    const auto &contestMap = allKeyConfigs[keyerType];
    for (auto contestIt = contestMap.constBegin(); contestIt != contestMap.constEnd(); ++contestIt) {
        QJsonObject rigObj;
        for (auto rigIt = contestIt.value().constBegin(); rigIt != contestIt.value().constEnd(); ++rigIt) {
            const ContestSection &cs = rigIt.value();

            // Run array
            QJsonArray runArr;
            for (const KeyVal &kv : cs.run)
                runArr.append(kv.toJson(keyerId));

            // S&P array
            QJsonArray spArr;
            for (const KeyVal &kv : cs.sp)
                spArr.append(kv.toJson(keyerId));

            QJsonObject sectionObj;
            sectionObj["Run"] = runArr;
            sectionObj["SandP"] = spArr;
            sectionObj["Common"] = cs.common.toJson();

            rigObj[rigIt.key()] = sectionObj;
        }
        keyerTypeObj[contestIt.key()] = rigObj;
    }

    QJsonObject root;
    QJsonObject keyerConfigObj;
    keyerConfigObj[keyerType] = keyerTypeObj;
    root["KeyerConfig"] = keyerConfigObj;

    QJsonDocument doc(root);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qWarning() << "Cannot write" << filePath;
        return false;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    return true;
}


TxKeyerId DMButtonFrame::txKeyerNameToId(const QString &name)
{
    for (int i = 0; i < txKeyerNames.size(); ++i)
    {
        if (txKeyerNames[i] == name)
            return static_cast<TxKeyerId>(i);
    }
    return TxKeyerId::None; // fallback
}



ValidationResult DMButtonFrame::validateKeyConfigs(const KeyerMap &configs)
{
    ValidationResult result;

    for (auto keyerIt = configs.constBegin(); keyerIt != configs.constEnd(); ++keyerIt)
    {
        const QString &keyerType = keyerIt.key();
        const auto &contestMap = keyerIt.value();

        if (keyerType.isEmpty())
        {
            result.addError("Found empty keyer type");
            continue;
        }

        for (auto contestIt = contestMap.constBegin(); contestIt != contestMap.constEnd(); ++contestIt)
        {
            const QString &contestName = contestIt.key();
            const auto &rigMap = contestIt.value();

            if (contestName.isEmpty())
            {
                result.addWarning(QString("Empty contest name in keyer '%1'").arg(keyerType));
            }

            if (rigMap.isEmpty())
            {
                result.addWarning(QString("Contest '%1' (keyer '%2') has no rigs")
                                      .arg(contestName, keyerType));
            }

            for (auto rigIt = rigMap.constBegin(); rigIt != rigMap.constEnd(); ++rigIt)
            {
                const QString &rigName = rigIt.key();
                const ContestSection &section = rigIt.value();

                if (rigName.isEmpty())
                {
                    result.addWarning(QString("Empty rig name in contest '%1', keyer '%2'")
                                          .arg(contestName, keyerType));
                }

                // Validate Run KeySet
                if (section.run.isEmpty())
                {
                    result.addWarning(QString("Run section is empty for rig '%1' in contest '%2', keyer '%3'")
                                          .arg(rigName, contestName, keyerType));
                }
                else
                {
                    for (int i = 0; i < section.run.size(); ++i)
                    {
                        const KeyVal &k = section.run[i];

                        if (k.fk().isEmpty())
                            result.addWarning(QString("Run[%1]: Empty key for rig '%2', contest '%3'")
                                                  .arg(i).arg(rigName, contestName));

                        // Check optional fields and set defaults if missing
                        // Example: messageDuration, repeatEnable, etc.
                        // (Could be done during parseFKeyArray instead)
                    }
                }

                // Validate S&P KeySet
                if (section.sp.isEmpty())
                {
                    result.addWarning(QString("SandP section is empty for rig '%1' in contest '%2', keyer '%3'")
                                          .arg(rigName, contestName, keyerType));
                }
                else
                {
                    for (int i = 0; i < section.sp.size(); ++i)
                    {
                        const KeyVal &k = section.sp[i];

                        if (k.fk().isEmpty())
                            result.addWarning(QString("S&P[%1]: Empty key for rig '%2', contest '%3'")
                                                  .arg(i).arg(rigName, contestName));
                    }
                }
            }
        }
    }

    return result;
}



bool DMButtonFrame::checkContestAndRadioAvailable(int &errorCode)
{
    int contestErrorCode = CHECK_RAD_CONT_CONTEST_OK;
    int radioErrorCode = CHECK_RAD_CONT_CONTEST_OK;
    QString errorMsg;

    clearErrorMessage();

    checkSavedContestExists(contestErrorCode);

    if (contestErrorCode == CHECK_RAD_CONT_CONTEST_OK)
    {
        if ( selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::RigControl]  || selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::CW_RigControl])
        {
            checkRadioExists(keyerSettings->getSelectedRadio().key(), radioErrorCode);
            if (radioErrorCode == CHECK_RAD_CONT_CONTEST_OK)
            {
                errorCode = CHECK_RAD_CONT_CONTEST_OK;
                return true;
            }
            else
            {
                errorCode = radioErrorCode;
                // display radio error codes
                switch (radioErrorCode) {
                case  CHECK_RAD_CONT_CONTEST_MISSING:
                    errorMsg = QObject::tr(checkContestRadioErrorCodeStr[CHECK_RAD_CONT_CONTEST_MISSING].toUtf8().constData())
                                   .arg(currentName, selectedKeyerCap.getKeyerType());
                    break;

                case CHECK_RAD_CONT_RADIO_MISSING:
                    errorMsg = QObject::tr(checkContestRadioErrorCodeStr[CHECK_RAD_CONT_RADIO_MISSING].toUtf8().constData())
                                   .arg(keyerSettings->getSelectedRadio().key(), currentName);
                    break;

                case CHECK_RAD_CONT_KEYER_MISING:
                    errorMsg = QObject::tr(checkContestRadioErrorCodeStr[CHECK_RAD_CONT_KEYER_MISING].toUtf8().constData())
                                   .arg(selectedKeyerCap.getKeyerType());
                    break;


                case CHECK_RAD_CONT_RADIO_NAME_EMPTY:
                    errorMsg = QObject::tr(checkContestRadioErrorCodeStr[CHECK_RAD_CONT_RADIO_NAME_EMPTY].toUtf8().constData());
                    break;

                default:
                    errorMsg = tr("Check Radio Unknown error");
                    break;

                }

                displayErrorMessage(errorMsg);

                return false;

            }
        }
        else
        {
            // no radio in this keyer
            errorCode = CHECK_RAD_CONT_CONTEST_OK;
            return true;
        }

    }
    else
    {
        errorCode = contestErrorCode;
        // display contest error codes
        switch (contestErrorCode) {

        case  CHECK_RAD_CONT_CONTEST_MISSING:
            errorMsg = QObject::tr(checkContestRadioErrorCodeStr[CHECK_RAD_CONT_CONTEST_MISSING].toUtf8().constData())
                           .arg(currentName, selectedKeyerCap.getKeyerType());
            break;

        case  CHECK_RAD_CONT_KEYER_MISING:
            errorMsg = QObject::tr(checkContestRadioErrorCodeStr[CHECK_RAD_CONT_KEYER_MISING].toUtf8().constData())
                           .arg(selectedKeyerCap.getKeyerType());
            break;

        case  CHECK_RAD_CONT_CONTEST_NAME_EMPTY:
            errorMsg = QObject::tr(checkContestRadioErrorCodeStr[CHECK_RAD_CONT_CONTEST_NAME_EMPTY].toUtf8().constData())
                           .arg(currentName, selectedKeyerCap.getKeyerType());
            break;

        default:
            errorMsg = tr("Check Contest Unknown error");
            break;

        }

        displayErrorMessage(errorMsg);
    }

    return false;


}


void DMButtonFrame::checkSavedContestExists(int &errorCode)
{

    if (currentName.isEmpty())
    {
        logMessage("checkSaveContestExists - contest name is empty");
        errorCode = CHECK_RAD_CONT_CONTEST_NAME_EMPTY;
        return;
    }

    if (allKeyConfigs.contains(selectedKeyerCap.getKeyerType()))
    {
        const ContestMap &contestMap = allKeyConfigs.value(selectedKeyerCap.getKeyerType());

        QStringList contests = contestMap.keys();
        if (contests.contains(currentName))
        {
            logMessage(QString("checkSaveContestExists - contest name %1 exists in json file for this keyerType %2").arg(currentName, selectedKeyerCap.getKeyerType()));
            errorCode = CHECK_RAD_CONT_CONTEST_OK;
            return;
        }
        else
        {

            errorCode = CHECK_RAD_CONT_CONTEST_MISSING;
            logMessage(QString("checkSaveContestExists - contest name %1 does not exist in json file for this keyerType %2").arg(currentName, selectedKeyerCap.getKeyerType()));
            return;

        }


    }
    else
    {
        logMessage(QString("checkSaveContestExists - keyerType %1 does not exist in json file").arg(selectedKeyerCap.getKeyerType()));
        errorCode  = CHECK_RAD_CONT_KEYER_MISING;
        return;
    }
}

// keyername and contest should have been checked when checking contest existed..
void DMButtonFrame::checkRadioExists(QString radioName, int &errorCode)
{

    if (radioName.isEmpty())
    {
         logMessage("checkRadioExists - radio name is empty");
        errorCode = CHECK_RAD_CONT_RADIO_NAME_EMPTY;
         return;
    }

    if (allKeyConfigs.contains(selectedKeyerCap.getKeyerType()))
    {
        const ContestMap &contestMap = allKeyConfigs.value(selectedKeyerCap.getKeyerType());

        if (contestMap.contains(currentName))
        {
            const RigMap &rigMap = contestMap.value(currentName);

            if (rigMap.contains(radioName))
            {
                logMessage(QString("checkRadioExists - radio name %1 exists in json file for this  for this contest name %2 and keyerType %3").arg(radioName, selectedKeyerCap.getKeyerType()));
                errorCode = CHECK_RAD_CONT_CONTEST_OK;
                return;
            }
            else
            {
                logMessage(QString("checkRadioExists - radio name %1 does not exist in json file for this contest name %2 and keyerType %3").arg(radioName, currentName, selectedKeyerCap.getKeyerType()));
                errorCode = CHECK_RAD_CONT_RADIO_MISSING;
                return;
            }


        }
        else
        {
            logMessage(QString("checkRadioExists - contest name %2 does not exist for this keyerType %2").arg(currentName, selectedKeyerCap.getKeyerType()));
            errorCode =  CHECK_RAD_CONT_CONTEST_MISSING;
            return;
        }
    }
    else
    {
        logMessage(QString("checkRadioExists - contest name %2 does not exist for this keyerType does not exist in json file %1").arg(selectedKeyerCap.getKeyerType()));
        errorCode = CHECK_RAD_CONT_KEYER_MISING;


    }

    return;
}



void DMButtonFrame::on_stopButton_clicked()
{
    if (selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::None])
    {
        return;
    }
    else if (selectedKeyerCap.getKeyerType() != txKeyerTypes[TxKeyerId::DigitalModes])
    {

        logMessage("Digimode - send stop transmission to sender app");
        RPCGeneralClient rpc(rpcConstants::DMStopTransmit);
        rpc.queueCall( dataSender );
    }
    else if (selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::CW_RigControl]
        || selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer])
    {
        txKeyer->stopCwMsg();

        if (curMode != savedMode && txKeyer->getSetCwModeAndRestoreFlag())       // restore mode?
        {
            emit sendModeToRadio(savedMode);
        }

    }
    else
    {
        //*******************************txKeyer->stopMsg(nullptr);
    }

    msgDurTimer->stop();
    repeatPauseTimer->stop();
    /*****************************************************************************************
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
*/


}

void DMButtonFrame::on_configEditButton_clicked()
{
    // bring up default file editor on "fkeyFileName"

    //   QDesktopServices::openUrl(QUrl::fromLocalFile(fkeyFileName));

    // Use built in fKey editor on "fkeyFileName"

    KeyerMap nfk = allKeyConfigs;


    if (selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::RigControl] || selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::CW_RigControl])
    {

        listOfRadios = LogContainer->sendDM->rigs();

        getVoiceCwMemSupportedRadios(listOfRadios, listOfRadioSupportKeyer);
        mapUniqueNames(listOfRadioSupportKeyer, radioMap);
    }

   //  QString txKeyerName = ui->txKeyerSelect->currentText();

   // if (txKeyerName.isEmpty())
   // {
   //     return;
   // }

    DMKeysEditDlgConfig editDlgConfig(nfk, selectedKeyerCap, radioMap, listOfRadioSupportKeyer);

    editDlgConfig.fKeyFileName = fkeyFileName;
    editDlgConfig.minosSelectedContestName = currentName;
    editDlgConfig.txKeyerFactory = txKeyerFactory;
    editDlgConfig.txKeyer = txKeyer;
    editDlgConfig.txKeyerType = selectedKeyerCap.getKeyerType();
    editDlgConfig.minosSelectedRadio = keyerSettings->getSelectedRadio();

    DMKeysEditDlg jed(this, editDlgConfig);

    if (jed.exec() == QDialog::Accepted)
    {
        allKeyConfigs = nfk;
        // and we have to regenerate the JSON file
        writeSingleKeyerFile(fkeyFileName, selectedKeyerCap.getKeyerType(), txKeyerNameToId(selectedKeyerCap.getKeyerName()));
        updateFrameState();
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

       // ui->nameLabel->setText(tr("Data Modes Buttons from %1").arg(fkeyFileName));

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
    if (ignoreFkComboSignal)
        return;

    currentName = arg1;

    if (selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::RigControl])
    {
        ct->rigControlCurrentFKeySetContest.setValue(currentName);

    }
    else  if (selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::CW_RigControl])
    {
        ct->cwRigControlCurrentFKeySetContest.setValue(currentName);

    }
    else  if (selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::SerialControl])
    {
        ct->serialControlCurrentFKeySetContest.setValue(currentName);
    }
    else  if (selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer])
    {
        ct->pcCwKeyerCurrentFKeySetContest.setValue(currentName);
    }
    else  if (selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::DigitalModes])
    {
        ct->digitalModesCurrentFKeySetContest.setValue(currentName);
    }
    else  if (selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::InternalVoiceKeyer])
    {
        ct->internalVoiceKeyerCurrentFKeySetContest.setValue(currentName);
    }
    else  if (selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::ExternalMqtKeyer])
    {
        ct->externalVoiceKeyerCurrentFKeySetContest.setValue(currentName);
    }

    ct->commonSave(false);

    displayButtons();
}

void DMButtonFrame::setCwMessagePlayingVisible(bool visible)
{
    // ui->cwMesssagePlayingLabel->setVisible(visible);
    // ui->cwMessagePlayingDisplay->setVisible(visible);
}


void DMButtonFrame::clearCwMessagePlayingDisplay()
{
    // ui->cwMessagePlayingDisplay->clear();
}

void DMButtonFrame::displayCwMessagePlaying(const QString msg)
{
    // ui->cwMessagePlayingDisplay->setText(msg);
}


void DMButtonFrame::logMessage(QString msg)
{
    trace(QString("[DMButtonFrame] %1").arg(msg));
}

// these many not be needed

void DMButtonFrame::createButtonsForKeyer(int numButtons, int columns)
{

}




void DMButtonFrame::loadButtonData()
{

}

void DMButtonFrame::onInternalVoiceMemoryPlayState(bool playing)
{

}

void DMButtonFrame::checkButtonIniFileVersion(QString txKeyerType)
{

}

void DMButtonFrame::checkCommonIniFileVersion(QString txKeyerType)
{

}

void DMButtonFrame::clearButtonLabels()
{
    for(auto &b: fButtons)
    {
        b->setText("");
    }
}


/*
void DMButtonFrame::sendModeToRadio(const QString m)
{

}
*/

void DMButtonFrame::setMode(const QString m)
{

}


void DMButtonFrame::onPcCwKeyerComportChanged()
{
    QString comportStr = keyerSettings->getPcCwKeyerComport();

    if (txKeyer && selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer])
    {

    }
}
void DMButtonFrame::onPcCwKeyerConnectionStateChanged()
{
    QString stateStr = keyerSettings->getPcCwKeyerConnectionState();

    logMessage(QString("PcCwKeyerConnection state = %1").arg(stateStr));
    if (txKeyer && selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer])
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
void DMButtonFrame::onPcCwKeyerErrorMsgChanged()
{
    QString errorMsg = keyerSettings->getPcCwKeyerErrorMessage();

    logMessage(QString("PcCwKeyerConnection error message = %1").arg(errorMsg));
    if (txKeyer && selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer])
    {

    }
}
void DMButtonFrame::onPcCwKeyerPttEnabledChanged()
{
    QString enabled = keyerSettings->getPcCwKeyerPttEnabled();

    logMessage(QString("PcCwKeyer Ptt enabled = %1").arg(enabled));

    if (txKeyer && selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer])
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
void DMButtonFrame::onPcCwKeyerTxOnStateChanged()
{

    QString state = keyerSettings->getPcCwKeyerTxOnState();

    logMessage(QString("TX State received = %1").arg(state));

    if (txKeyer && selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer])
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
void DMButtonFrame::onPcCwKeyerCurrentWpmChanged()
{
    QString wpm = keyerSettings->getPcCwKeyerCurrentWpm();

    logMessage(QString("Current WPM from PcCwKeyer = %1").arg(wpm));

    if (wpm.isEmpty())
    {
        return;
    }

    if (txKeyer && selectedKeyerCap.getKeyerType() == txKeyerTypes[TxKeyerId::PcCwKeyer])
    {

        cwSpeedSlider->setValue(wpm.toInt()); // this will not emit and send back to keyer

    }
}

void DMButtonFrame::clearAllDirtyFlags()
{
    for (auto &contestMap : allKeyConfigs)
    {
        for (auto &rigMap : contestMap)
        {
            for (auto &section : rigMap)
            {
                for (auto &k : section.run)
                    k.clearDirty();
                for (auto &k : section.sp)
                    k.clearDirty();
            }
        }
    }
}

