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



#include "dmbuttonframe.h"
#include "ui_dmbuttonframe.h"

using namespace TxKeyerCommon;


const QStringList vmButtonShortCutKeys = {
    "Shift+F1", "Shift+F2",
    "Shift+F3", "Shift+F4",
    "Shift+F5", "Shift+F6",
    "Shift+F7", "Shift+F8",
    "Shift+F9", "Shift+F10",
    "Shift+F11", "Shift+F12",
};

const QString DIGIMODE = "DigiMode";

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
    txKeyerType = keyerTypes[TxKeyerId::None];

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

    connect(ui->txKeyerSetupPb, &QPushButton::clicked, this, &DMButtonFrame::onTxKeyerSetupClicked);

    txKeyerFactory->populateComboKeyerList(ui->txKeyerSelect, txKeyerName);

    // we add digi modes to list, though it will not act as keyer
    ui->txKeyerSelect->addItem(DIGIMODE);

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
    fButtons << ui->F1Button << ui->F2Button << ui->F3Button << ui->F4Button << ui->F5Button << ui->F6Button;
    fButtons << ui->F7Button << ui->F8Button << ui->F9Button << ui->F10Button << ui->F11Button << ui->F12Button;

    int i = 0;
    for (auto b: QASCONST(fButtons))
    {
        b->setProperty("KeyNo", i++);
        b->setText("");
        connect(b, &QPushButton::clicked, this, &DMButtonFrame::fButtonClicked);
    }

    ui->nameLabel->setText(tr("Data Modes & Keyer Buttons from %1").arg(fkeyFileName));

    ui->selectedRadioLabel->setVisible(false);


}

DMButtonFrame::~DMButtonFrame()
{
    delete ui;
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
    ui->pttTypeLabel->setVisible(visible);
    ui->pttTypeText->setVisible(visible);
}

void DMButtonFrame::setPttTypeText(serialCommonData::MINOS_PTT_TYPES pttType)
{

    ui->pttTypeText->setText(serialCommonData::pttTypeStr[static_cast<int>(pttType)]);
}


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


void DMButtonFrame::logRadioSettingsChanged(QSharedPointer<RadioSettingsDialogChangeFlag> logRadioSettingsFlags)
{
    Q_UNUSED(logRadioSettingsFlags)

    if (txKeyerType == keyerTypes[TxKeyerId::CW_RigControl]
                        || txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer]
                        || txKeyerType == keyerTypes[TxKeyerId::RigControl]
                        || txKeyerType == keyerTypes[TxKeyerId::InternalVoiceKeyer])
    {

        setRadioParams();

        if (txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
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



        txKeyer->txKeyerInit(txKeyer->numButtons);

        loadButtonData();
    }
}


// also PcCwKeyer
void DMButtonFrame::setRadioParams()
{
    if (txKeyerType == keyerTypes[TxKeyerId::CW_RigControl])
    {
        txKeyer->setRadioParams(MAXIMUM_BUTTONS, selectedRadio.getLocalName(), getPttType(selectedRadio), getPttEnabled(selectedRadio));
    }
    else if (txKeyerType == keyerTypes[TxKeyerId::RigControl])
    {
        txKeyer->setRadioParams(getNumVoiceMessages(selectedRadio), selectedRadio.getLocalName(), getPttType(selectedRadio), getPttEnabled(selectedRadio));
    }
    else if (txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
    {
        txKeyer->setRadioParams(PC_CW_KEYER_MAXIMUM_BUTTONS, selectedRadio.getLocalName(), serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_NONE, false);
    }
}


void DMButtonFrame::createKeyer(QString voiceKeyerName)
{
    if (!voiceKeyerName.isEmpty())
    {


        TxKeyerCapabilities voiceCap = txKeyerFactory->supportedTxKeyers()->value(voiceKeyerName);
        txKeyerType = voiceCap.getKeyerType();

        if (txKeyerType != keyerTypes[TxKeyerId::None])
        {
            txKeyer = QSharedPointer<TxKeyerBase>(txKeyerFactory->createTxKeyer(voiceCap.getVmIdNum()));
            if (txKeyer)
            {
                logMessage(QString("Voice Keyer type selected = %1").arg(voiceCap.getKeyerName()));

                connect(txKeyer.data(), &TxKeyerBase::remoteConfigChanged, this, &DMButtonFrame::onRemoteConfigChanged, Qt::UniqueConnection);
                connect(txKeyer.data(), &TxKeyerBase::remoteKeyerStopped, this, &DMButtonFrame::onRemoteKeyerStopped, Qt::UniqueConnection);
                connect(txKeyer.data(), &TxKeyerBase::remoteKeyerStarted, this, &DMButtonFrame::onRemoteKeyerStarted, Qt::UniqueConnection);
                connect(txKeyer.data(), &TxKeyerBase::internalVoiceMemoryKeyerPlayState, this, &DMButtonFrame::onInternalVoiceMemoryPlayState);

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



                if (txKeyerType == keyerTypes[TxKeyerId::CW_RigControl] || txKeyerType == keyerTypes[TxKeyerId::RigControl])
                {
                    // convert to version 2 ini type
                    checkButtonIniFileVersion(txKeyerType);
                    checkCommonIniFileVersion(txKeyerType);
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
    QString keyerName = ui->txKeyerSelect->currentText();

    if (keyerName == DIGIMODE)
    {
        return;
    }

    TxKeyerCapabilities voiceCap = txKeyerFactory->supportedTxKeyers()->value(keyerName);
    txKeyerType = voiceCap.getKeyerType();

    if (LogContainer->sendDM->isKeyerLoaded())
    {
        notifyComboChange = false;
        txKeyerFactory->populateComboKeyerList(ui->txKeyerSelect, keyerName);
        notifyComboChange = true;
    }

    if (!txKeyer && txKeyerType == keyerTypes[TxKeyerId::ExternalVoiceKeyer])
    {
        createKeyer(keyerName);
        if (txKeyer)
        {
            ui->noExtKeyerLabel->clear();
            extKeyerConnectTimer->stop();
        }
        else
        {
            txKeyerType = keyerTypes[TxKeyerId::None];
        }
    }
    else if (txKeyer)
    {
        ui->noExtKeyerLabel->clear();
        extKeyerConnectTimer->stop();
    }
}


void DMButtonFrame::onTxKeyerSelect(int idx)
{
    Q_UNUSED(idx)

    if (!notifyComboChange)
        return;

    QString txKeyerName = ui->txKeyerSelect->currentText();
    logMessage(QString("onVoiceKeyerSelect - keyer select name = %1").arg( ui->txKeyerSelect->currentText()));

    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICEKEYER_COMMON_PARAMS_FILENAME;
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup(VOICEKEYER_COMMON_PARAMS_GROUPNAME);

    config.setValue("KeyerName", txKeyerName);

    config.endGroup();

    txKeyer.clear();

    if (txKeyerName == DIGIMODE)
    {
        // flag we are in Digital Mode
        txKeyerType = keyerTypes[TxKeyerId::DigitalModes];

    }

    delayedAction(this, [=]{
        if (txKeyerName != DIGIMODE)
        {
            createKeyer(txKeyerName);   // don't create a keyer when in Digimode
        }
        setFrameState(txKeyerName);
    });


    extKeyerConnectTimer->start(1000);

    ui->txKeyerSelect->repaint();   // or the combo doesn't update

}


void DMButtonFrame::setFrameState(QString txKeyerName)
{

    if (txKeyerName == DIGIMODE)
    {

        set_DigiMode_FrameState(txKeyerName);
        return;
    }



    //TxKeyerCapabilities voiceCap = txKeyerFactory->supportedTxKeyers()->value(txKeyerName);

    if (txKeyer == nullptr)
    {

        set_None_FrameState(txKeyerName);


    }
    else
    {
        if (txKeyerName == "rigControl")
        {
            set_rigControl_FrameState(txKeyerName);
        }
        else if (txKeyerName == "cwRigControl")
        {
            set_cwRigControl_FrameState(txKeyerName);
        }
        else if (txKeyerName == "pcCwKeyer")
        {
            set_pcCwKeyer_FrameState(txKeyerName);
        }
        else if (txKeyerName == "Internal")
        {
            set_Internal_FrameState(txKeyerName);
        }
        else if (txKeyerName == "External")
        {
           set_External_FrameState(txKeyerName);
        }

    }
}


void DMButtonFrame::set_DigiMode_FrameState(QString txKeyerName)
{
    txKeyParamList.clear();
    txKeyerType = keyerTypes[TxKeyerId::DigitalModes];

    clearButtonLabels();
    setCwEntryBoxVisible(false);

    setAvailIndicatorVisible(false);
    setRepeatIndicatorVisible(false);

    ui->txKeyerSetupPb->setVisible(false);
    ui->pipCb->setVisible(false);
    ui->stopButton->setVisible(true);
    ui->selectedRadioLabel->setVisible(false);
    setTXStatusVisible(false);
    setEomTypeLabelsVisible(false);
    setKeyerIndicatorGroupBoxVisible(false);
    setPttIndicatorGroupBoxVisible(false);
    setErrorMessageVisible(false);
    setCwMessagePlayingVisible(false);
    setMessagePlayingFlag(false);

    setLogItButtonVisible(true);
    setLogItButtonVisible(true);

    currentName = ct->digitalModesCurrentFKeySetContest.getValue();
    populateFksetCombo(txKeyerName, currentName);
    fkeyFileChanged();
}


void DMButtonFrame::set_None_FrameState(QString txKeyerName)
{

    Q_UNUSED(txKeyerName)

    clearButtonLabels();
    txKeyParamList.clear();

    if (txKeyerType == keyerTypes[TxKeyerId::ExternalVoiceKeyer])
    {
        ui->noExtKeyerLabel->setText(HtmlFontColour(Qt::red) +  tr("To use the external keyer mqtKeyer must be running and connected"));

    }

    txKeyerType = keyerTypes[TxKeyerId::None];


    clearAll_Ui_Elements();
}

void DMButtonFrame::clearAll_Ui_Elements()
{
    clearButtonLabels();
    setCwEntryBoxVisible(false);


    setAvailIndicatorVisible(false);
    setRepeatIndicatorVisible(false);

    ui->txKeyerSetupPb->setVisible(false);
    ui->pipCb->setVisible(false);
    ui->stopButton->setVisible(false);
    ui->selectedRadioLabel->setVisible(false);
    setTXStatusVisible(false);
    setEomTypeLabelsVisible(false);
    setKeyerIndicatorGroupBoxVisible(false);
    setPttIndicatorGroupBoxVisible(false);
    setErrorMessageVisible(false);
    setCwMessagePlayingVisible(false);
    setMessagePlayingFlag(false);
    clearErrorMessage();

    if (cwSpeedSlider)
    {
        cwSpeedSlider->hide();
    }
}

void DMButtonFrame::set_rigControl_FrameState(QString txKeyerName)
{

    clearAll_Ui_Elements(); // we do this to try and make it clear that the radio definition
                            // is missing for the contest

    ui->noExtKeyerLabel->clear();

    currentName = ct->rigControlCurrentFKeySetContest.getValue();
    parseFKeyFile(fkeyFileName);    // also populates FkSetCombo


    //populateFksetCombo(txKeyerName, currentName);

    logMessage(QString("Set RigControl Frame State for Contest Name %1, Radio %2").arg(currentName, selectedRadio.key()));

    bool radioExists = false;
    if (checkRadioExists(selectedRadio.key(), radioExists))    // check radio exists in keyerConfigs
    {
        if (!radioExists)
        {
            displayErrorMessage(
                tr("Radio %1 has not been defined in contest %2, please add the radio")
                    .arg(selectedRadio.key())
                    .arg(currentName)
            );
            logMessage(QString("Radio %1 has not been defined in %2, please add the radio").arg(selectedRadio.key().arg(currentName)));
        }
        else
        {
            logMessage(QString("Radio %1 Exists for Contest Name %2").arg(selectedRadio.key(), currentName));
            clearErrorMessage();
            setupRigControl_Ui_Elements(txKeyerName);



            //    populateRadioNameCombo(currentName);
            displayButtons();
        }

    }
    else
    {
        qDebug() << "check radio exists failed";
        logMessage(QString("checkRadio exists failed for contest name %1, radio %2").arg(currentName, selectedRadio.key()));
    }
}


void DMButtonFrame::setupRigControl_Ui_Elements(QString txKeyerName)
{

    TxKeyerCapabilities txKeyerCap = txKeyerFactory->supportedTxKeyers()->value(txKeyerName);


    ui->txKeyerSetupPb->setVisible(txKeyerCap.getSetupButton());
    ui->pipCb->setVisible(txKeyerCap.getHasPip());
    setTXStatusVisible(txKeyerCap.getHasTxStatus());

    ui->selectedRadioLabel->setVisible(true);
    ui->selectedRadioLabel->setText(selectedRadio.key());

    setKeyerIndicatorGroupBoxVisible(true);
    setPttIndicatorGroupBoxVisible(true);
    setMessagePlayingFlag(false);
    setCwMessagePlayingVisible(false);

    setLogItButtonVisible(false);
    setLogItButtonVisible(false);


    if (txKeyerCap.getHasAvailStatus())
    {
        setAvailIndicatorVisible(txKeyerCap.getHasAvailStatus());
        setAvailIndicatorForRadioOnOff(selectedRadio);
    }
    else
    {
        setAvailIndicatorVisible(false);
    }

    setRepeatIndicatorVisible(txKeyerCap.getHasMessageRepeat());

    ui->stopButton->setVisible(true);


    //setSaveButtonByRadionameText(selectedRadio.getLocalName());



    txKeyer->setRadioParams(getNumVoiceMessages(selectedRadio), selectedRadio.getLocalName(), getPttType(selectedRadio), getPttEnabled(selectedRadio));
    txKeyer->txKeyerInit(txKeyer->numButtons);
    setPttTypeLabelsVisible(true);
    setPttTypeText(getPttType(selectedRadio));
    setPttEnabledIndicatorOnOff(getPttEnabled(selectedRadio));
    setEomTypeLabelsVisible(true);
    setEomLabelText(txKeyer->getSelectedEomType());
    setCwEntryBoxVisible(false);
    //loadButtonData();

    if (!getRigVoiceKeyerSupportStopFlag(selectedRadio.getLocalName()))
    {
        ui->stopButton->setVisible(false);
    }
    else
    {
        ui->stopButton->setVisible(true);
    }

}

void DMButtonFrame::set_cwRigControl_FrameState(QString txKeyerName)
{
    clearAll_Ui_Elements(); // we do this to try and make it clear that the radio definition
                            // is missing for the contest

    ui->noExtKeyerLabel->clear();

    currentName = ct->cwRigControlCurrentFKeySetContest.getValue();
    parseFKeyFile(fkeyFileName);    // also populates FkSetCombo

    logMessage(QString("Set Cw RigControl Frame State for Contest Name %1, Radio %2").arg(currentName, selectedRadio.key()));



    bool radioExists = false;
    if (checkRadioExists(selectedRadio.key(), radioExists))    // check radio exists in keyerConfigs
    {
        if (!radioExists)
        {
            displayErrorMessage(
                tr("Radio %1 has not been defined in contest %2, please add the radio")
                    .arg(selectedRadio.key())
                    .arg(currentName)
            );
            logMessage(QString("Radio %1 has not been defined in %2, please add the radio").arg(selectedRadio.key().arg(currentName)));
        }
        else
        {
            logMessage(QString("Radio %1 Exists for Contest Name %2").arg(selectedRadio.key(), currentName));

            clearErrorMessage();
            setupCw_RigControl_Ui_Elements(txKeyerName);
            displayButtons();
        }

    }
    else
    {
        qDebug() << "check radio exists failed";
        logMessage(QString("checkRadio exists failed for contest name %1, radio %2").arg(currentName, selectedRadio.key()));
    }




}


void DMButtonFrame::setupCw_RigControl_Ui_Elements(QString txKeyerName)
{
    TxKeyerCapabilities txKeyerCap = txKeyerFactory->supportedTxKeyers()->value(txKeyerName);

    ui->txKeyerSetupPb->setVisible(txKeyerCap.getSetupButton());
    ui->pipCb->setVisible(txKeyerCap.getHasPip());
    setTXStatusVisible(txKeyerCap.getHasTxStatus());

    ui->selectedRadioLabel->setVisible(true);
    ui->selectedRadioLabel->setText(selectedRadio.key());

    setKeyerIndicatorGroupBoxVisible(true);
    setPttIndicatorGroupBoxVisible(true);
    setMessagePlayingFlag(false);
    setCwMessagePlayingVisible(false);



    if (txKeyerCap.getHasAvailStatus())
    {
        setAvailIndicatorVisible(txKeyerCap.getHasAvailStatus());
        setAvailIndicatorForRadioOnOff(selectedRadio);
    }
    else
    {
        setAvailIndicatorVisible(false);
    }

    setRepeatIndicatorVisible(txKeyerCap.getHasMessageRepeat());
    txKeyer->setCwMemType(getCwMemType(selectedRadio));
    txKeyer->setContest(ct);

    ui->stopButton->setVisible(true);




    txKeyer->setRadioParams(getNumVoiceMessages(selectedRadio), selectedRadio.getLocalName(), getPttType(selectedRadio), getPttEnabled(selectedRadio));
    txKeyer->txKeyerInit(txKeyer->numButtons);
    setPttTypeLabelsVisible(true);
    setPttTypeText(getPttType(selectedRadio));
    setPttEnabledIndicatorOnOff(getPttEnabled(selectedRadio));
    setEomTypeLabelsVisible(true);
    setEomLabelText(txKeyer->getSelectedEomType());
    //loadButtonData();

    setLogItButtonVisible(true);
    setLogItButtonVisible(true);

    setCwEntryBoxVisible(true);
    setCwMessagePlayingVisible(true);

    if (!getRigCwKeyerSupportStopFlag(selectedRadio.getLocalName()))
    {
        ui->stopButton->setVisible(false);
    }
    else
    {
        ui->stopButton->setVisible(true);
    }

    initCwTextEntryBox(getCwRadioManufacturer(getCwMemType(selectedRadio)), CWKEYER_RADIO_COMMON_PARAMS_FILENAME);

}


void DMButtonFrame::set_pcCwKeyer_FrameState(QString txKeyerName)
{
    ui->noExtKeyerLabel->clear();
    clearErrorMessage();

    currentName = ct->pcCwKeyerCurrentFKeySetContest.getValue();
    parseFKeyFile(fkeyFileName);    // also populates FkSetCombo


    TxKeyerCapabilities txKeyerCap = txKeyerFactory->supportedTxKeyers()->value(txKeyerName);

    ui->txKeyerSetupPb->setVisible(txKeyerCap.getSetupButton());
    ui->pipCb->setVisible(txKeyerCap.getHasPip());
    setTXStatusVisible(txKeyerCap.getHasTxStatus());

     ui->selectedRadioLabel->setVisible(false);

    setKeyerIndicatorGroupBoxVisible(true);
    setPttIndicatorGroupBoxVisible(true);
    setMessagePlayingFlag(false);
    setCwMessagePlayingVisible(false);


    setAvailIndicatorVisible(txKeyerCap.getHasAvailStatus());
    setAvailIndicatorOnOffForPcCwKeyer();


    setRepeatIndicatorVisible(txKeyerCap.getHasMessageRepeat());

    txKeyer->setContest(ct);

    ui->stopButton->setVisible(true);
    txKeyer->txKeyerInit(txKeyer->numButtons);
    setPttTypeLabelsVisible(true);
    setPttTypeText(getPttType(selectedRadio));
    setPttEnabledIndicatorOnOff(getPttEnabled(selectedRadio));
    setEomTypeLabelsVisible(true);
    setEomLabelText(txKeyer->getSelectedEomType());


    setLogItButtonVisible(true);
    setLogItButtonVisible(true);

    setCwEntryBoxVisible(true);
    setCwMessagePlayingVisible(true);

    ui->stopButton->setVisible(true);

    if (!cwSpeedSlider) // check already added to frame
    {

        cwSpeedSlider = new CwSpeedControl(ui->cwSpeedSliderFrame);
        ui->cwSpeedSliderHorizontalLayout->addWidget(cwSpeedSlider);
        cwSpeedSlider->setSpeedRange(TxKeyerCommon::PC_CW_KEYER_MIN_WPM, TxKeyerCommon::PC_CW_KEYER_MAX_WPM);

        connect(cwSpeedSlider, &CwSpeedControl::cwSpeedChanged, this, [this](int wpm){
            emit sendWpmToPcCwkeyer(wpm);
        });
    }
    else
    {
        cwSpeedSlider->show();

    }


    initCwTextEntryBox(getCwRadioManufacturer(getCwMemType(selectedRadio)), CWKEYER_RADIO_COMMON_PARAMS_FILENAME);

    displayButtons();
    //currentName = ct->pcCwKeyerCurrentFKeySetContest.getValue();
    //populateFksetCombo(txKeyerName, currentName);
    //fkeyFileChanged();


}




void DMButtonFrame::set_Internal_FrameState(QString txKeyerName)
{
    ui->noExtKeyerLabel->clear();
    clearErrorMessage();

    TxKeyerCapabilities txKeyerCap = txKeyerFactory->supportedTxKeyers()->value(txKeyerName);

    if (cwSpeedSlider)
    {
        cwSpeedSlider->hide();
    }


    ui->txKeyerSetupPb->setVisible(txKeyerCap.getSetupButton());
    ui->pipCb->setVisible(txKeyerCap.getHasPip());
    setTXStatusVisible(txKeyerCap.getHasTxStatus());

     ui->selectedRadioLabel->setVisible(false);

    setKeyerIndicatorGroupBoxVisible(true);
    setPttIndicatorGroupBoxVisible(true);
    setMessagePlayingFlag(false);
    setCwEntryBoxVisible(false);
    setCwMessagePlayingVisible(false);

    setAvailIndicatorVisible(txKeyerCap.getHasAvailStatus());


    setRepeatIndicatorVisible(txKeyerCap.getHasMessageRepeat());

    ui->stopButton->setVisible(true);

    //setSaveButtonByRadionameText(selectedRadio.getLocalName());

    txKeyer->setRadioParams(getNumVoiceMessages(selectedRadio), selectedRadio.getLocalName(), getPttType(selectedRadio), getPttEnabled(selectedRadio));
    txKeyer->txKeyerInit(txKeyer->numButtons);
    setPttTypeLabelsVisible(true);
    setPttTypeText(getPttType(selectedRadio));
    setPttEnabledIndicatorOnOff(getPttEnabled(selectedRadio));
    setEomTypeLabelsVisible(true);
    setEomLabelText(txKeyer->getSelectedEomType());
    //    loadButtonData();


    setPttTypeLabelsVisible(true);
    setPttTypeText(getPttType(selectedRadio));
    setPttEnabledIndicatorOnOff(getPttEnabled(selectedRadio));

    currentName = ct->internalVoiceKeyerCurrentFKeySetContest.getValue();
    populateFksetCombo(txKeyerName, currentName);
    fkeyFileChanged();
}


void DMButtonFrame::set_External_FrameState(QString txKeyerName)
{
    // not sure what this should be....
    if (cwSpeedSlider)
    {
        cwSpeedSlider->hide();
    }

    setCwEntryBoxVisible(false);
    setCwMessagePlayingVisible(false);
    clearErrorMessage();
     ui->selectedRadioLabel->setVisible(false);

     currentName = ct->externalVoiceKeyerCurrentFKeySetContest.getValue();
     populateFksetCombo(txKeyerName, currentName);
     fkeyFileChanged();
}


void DMButtonFrame::DMButtonFrame::updateFrameState()
{
    if (!ui->txKeyerSelect->currentText().isEmpty())
    {
       setFrameState(ui->txKeyerSelect->currentText());
    }


}



void DMButtonFrame::initCwTextEntryBox(QString radioManufacturer, QString fileName)
{
    ui->cwEntry->setVisible(true);
    ui->cwEntry->installEventFilter(this);

    //

    QString cwMacroCharList;
    bool cwMacroCharOk;
    if (getRigCWKeyerMacroCharacter(cwMacroCharList, radioManufacturer, CWKEYER_RADIO_COMMON_PARAMS_FILENAME))
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
    if (getRigCWKeyerSupportedCharacters(validCharCwList, radioManufacturer, CWKEYER_RADIO_COMMON_PARAMS_FILENAME))
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
    if (getRigCWKeyerMaxMessageLength(maxNumChars, radioManufacturer, fileName))
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
    ui->cwEntry->setValidator(validator);


}


void DMButtonFrame::onCwEntryReturnPressed()
{


    if  (txKeyer)
    {
        if (txKeyerType == keyerTypes[TxKeyerId::CW_RigControl] || txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
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

void DMButtonFrame::startKeyerMsg(int key)
{
    logMessage(QString("- start keyerMsg button Number = %1").arg(key));

    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    int msgOffset = tslf->GJVQSOLogFrame->getSandP()?12:0;

    int messageNumber = key + msgOffset; // e.g. F1=0, F12=11 (Run keys), then S&P keys 12..23

    selectedEomType = KeyerEomTypes::Eom_None;

    TxKeyerParams vmData;

    if ((txKeyerType == keyerTypes[TxKeyerId::CW_RigControl]
         || txKeyerType == keyerTypes[TxKeyerId::RigControl]
         || txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
        && !selectedRadio.key().isEmpty())
    {

        QString rigKey;
        if (txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
        {
            rigKey = KEYER_NO_RADIO;    // pcCWKeyer has noRadio in the macro definitions
        }
        else
        {
            rigKey = selectedRadio.key();
            if (rigKey.isEmpty())
                rigKey = KEYER_NO_RADIO;  // fallback if needed
        }




        auto &contestMap = allKeyConfigs[txKeyerType];
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
                vmData.setSelRadioName(selectedRadio.getLocalName());
                vmData.setRigModel(getRigModel(selectedRadio));
                vmData.setSAndPState(sAndPState);
            }
        }
    }


    vmData.setType(txKeyerType);

    setRepeatIndicatorOnOff(vmData.getKeyerRepeatFlag());



    if (txKeyerType == keyerTypes[TxKeyerId::CW_RigControl]
        || txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
    {
        if (vmData.getKeyerCwMessage().isEmpty())
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
            || txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
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

        }

    }
    else
    {
        txKeyer->sendMsgNum(vmData);

    }

    selectedEomType = txKeyer->getSelectedEomType();
/*
    if (selectedEomType == TxKeyerCommon::KeyerEomTypes::Timer)
    {
        int msgDur = vmKeyParamList[buttonNumber].getVmDuration() * 1000;
        if (msgDur > 0)
        {
            logMessage(QString("- msgDurTimer->start(%1)").arg(msgDur));
            msgDurTimer->start(msgDur);
        }
    }

   txVmButtonMap[buttonNumber]->showButtonOnOff(true);
*/
}





void DMButtonFrame::onRemoteConfigChanged()
{
    bool s = txKeyer->getPip();
    if (ui->pipCb->isChecked() != s)
    {
        ui->pipCb->setChecked(s);
    }
/*******************************************************************************
    for (int i = 0; i < voiceMemButtonList.count(); i++)
    {
        VoiceKeyerParams vmData;
        if (vmData.getType().isEmpty())
        {
            vmData.setType(txKeyerType);
        }

        txKeyer->readVmButtonParams(i, vmData);
        vmKeyParamList[i] = vmData;
        setRunButtonText(i, vmData.getVmName());
    }
*/
}
void DMButtonFrame::onRemoteKeyerStarted(int key)
{
    onRemoteKeyerStopped();
/*********************************************************************************************
    buttonNumSent = key;
    if (txKeyerType == keyerTypes[TxKeyerId::None] || buttonNumSent == NO_VM_BUTTON_ON)
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
*/
}
void DMButtonFrame::onRemoteKeyerStopped()
{
/*************************************************************************************

    if (txKeyerType == keyerTypes[TxKeyerId::None] || buttonNumSent == NO_VM_BUTTON_ON)
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
*/
}

void DMButtonFrame::onMsgDurTimerTimeout()
{
/**************************************************************************

    if (buttonNumSent >= 0)
    {
        if (vmKeyParamList[buttonNumSent].getVmDuration() > 0
            || selectedEomType == KeyerEomTypes::CAT
            || txKeyer->getSelectedEomType() == KeyerEomTypes::InternalSoundCardVoiceKeyer
            || selectedEomType == KeyerEomTypes::DTRKeyerTXStatus)
        {
            if (txKeyerType == keyerTypes[TxKeyerId::InternalVoiceKeyer])
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


    if (txKeyerType == keyerTypes[TxKeyerId::CW_RigControl]
        || txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
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
             || txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
            && txKeyer->getSetCwModeAndRestoreFlag())
        {

            if (curMode != savedMode)       // restore mode?
            {
                sendModeToRadio(savedMode);
            }

        }

    }
*/

}





void DMButtonFrame::onRepeatPauseTimerTimeout()
{

    repeatPauseTimer->stop();
/***********************************************************************************
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
*/
}

void DMButtonFrame::setWipeButtonVisible(bool visible)
{
    ui->wipeButton->setVisible(visible);
}

void DMButtonFrame::setLogItButtonVisible(bool visible)
{
    ui->logitButton->setVisible(visible);
}

void DMButtonFrame::setRadioIsConnected(bool connected)
{
    radioConnected = connected;
}

void DMButtonFrame::setSelectedRadio(PubSubName selectedRadio_)
{
    //if (txKeyerType.isEmpty() || txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
    //{
        // ignore as we don't want to update framestate
    //    return;
   // }

    if (selectedRadio != selectedRadio_)
    {
        selectedRadio = selectedRadio_;

        updateFrameState();
    }
}

void DMButtonFrame::setPttEnabled(bool state, PubSubName psn)
{
    if (txKeyerType.isEmpty() || txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
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

    //if (txKeyerType.isEmpty() || txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
    //{
        updateFrameState();
    //}


}

bool DMButtonFrame::getPttEnabled(PubSubName psn)
{

    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        return rd.getPttEnabled();
    }


    return false;
}

void DMButtonFrame::setPttType(int type, PubSubName psn)
{

    if (txKeyerType.isEmpty() || txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
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

serialCommonData::MINOS_PTT_TYPES DMButtonFrame::getPttType(PubSubName psn)
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

void DMButtonFrame::setVoiceMemAvail(bool avail, PubSubName psn)
{
    if (txKeyerType.isEmpty() || txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
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

    if (txKeyerType == keyerTypes[TxKeyerId::RigControl])
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

        if (txKeyerType == keyerTypes[TxKeyerId::RigControl])
        {
            if(isVoiceMemAvail(radName))
            {
                listOfRadioSupportKeyer.append(radio);
            }
        }
        else if (txKeyerType == keyerTypes[TxKeyerId::CW_RigControl])
        {
            if(isCwMemTypeAvail(radName))
            {
                listOfRadioSupportKeyer.append(radio);
            }
        }
    }
}

bool DMButtonFrame::isVoiceMemAvail(PubSubName psn)
{
    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        return rd.getVoiceMemAvail();
    }

    return false;
}

void DMButtonFrame::setNumVoiceMessages(int numMsgs, PubSubName psn)
{

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

    if (txKeyerType == keyerTypes[TxKeyerId::RigControl])
    {
        updateFrameState(  );
    }

}

// This is max number of voice messages available on a radio
int DMButtonFrame::getNumVoiceMessages(PubSubName psn)
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

void DMButtonFrame::setRigVoiceKeyerSupportStopFlag(bool supportStopCmd, PubSubName psn)
{

    if (txKeyerType.isEmpty() || txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
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

    if (txKeyerType == keyerTypes[TxKeyerId::RigControl])
    {
        updateFrameState();
    }



}


bool DMButtonFrame::getRigVoiceKeyerSupportStopFlag(PubSubName psn)
{
    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        return rd.getRigVoiceKeyerSupportStopCmd();
    }

    return true;

}


void DMButtonFrame::setRigCwKeyerSupportStopFlag(bool supportStopCmd, PubSubName psn)
{
    if (txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
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

    if (txKeyerType == keyerTypes[TxKeyerId::CW_RigControl])
    {
        updateFrameState();
    }


}


bool DMButtonFrame::getRigCwKeyerSupportStopFlag(PubSubName psn)
{
    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        return rd.getRigCwKeyerSupportStopCmd();
    }

    return true;

}

void DMButtonFrame::setRigModel(QString rigModel, PubSubName psn)
{

    if (txKeyerType.isEmpty() ||txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
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

    //if (txKeyerType.isEmpty() ||txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
    //{
        updateFrameState();
    //}



}

QString DMButtonFrame::getRigModel(PubSubName psn)
{
    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        return rd.getRigModel();
    }

    return "";

}



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


void DMButtonFrame::setCwMemType(int cwMemType, PubSubName psn)
{


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

    if (txKeyerType.isEmpty() || txKeyerType == keyerTypes[TxKeyerId::CW_RigControl])
    {
        updateFrameState();
    }

}



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


void DMButtonFrame::setCwEntryBoxVisible(bool visible)
{
    ui->cwEntry->setVisible(visible);
    ui->cwEntryLabel->setVisible(visible);
}

void DMButtonFrame::setAvailIndicatorVisible(bool visible)
{

    ui->availLabel->setVisible(visible);
    ui->availIndicator->setVisible(visible);
    ui->vmAvailIndicatorLine->setVisible(visible);

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
        ui->availIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);
    }
    else
    {
        ui->availIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);

    }

    if (txKeyerType == keyerTypes[TxKeyerId::RigControl])
    {
        ui->availIndicator->setToolTip(tr("Rig Voice Keyer Available"));
    }
    else if (txKeyerType == keyerTypes[TxKeyerId::CW_RigControl])
    {
        ui->availIndicator->setToolTip(tr("Rig CW Messages Available"));
    }
    else
    {
        ui->availIndicator->setToolTip(tr(""));
    }
}

void DMButtonFrame::setAvailIndicatorForRadioOnOff(PubSubName radName)
{
    if (txKeyerType == keyerTypes[TxKeyerId::CW_RigControl] || txKeyerType == keyerTypes[TxKeyerId::RigControl])
    {
        if (txKeyerType == keyerTypes[TxKeyerId::CW_RigControl])
        {
            setAvailIndicatorOnOff(isCwMemTypeAvail(radName));
        }
        else if (txKeyerType == keyerTypes[TxKeyerId::RigControl])
        {
            setAvailIndicatorOnOff(isVoiceMemAvail(radName));
        }
        else
        {
            setAvailIndicatorOnOff(false);
        }
    }

}

void DMButtonFrame::setRepeatIndicatorVisible(bool visible)
{
    ui->repeatIndicator->setVisible(visible);
    ui->repeatLabel->setVisible(visible);
}

void DMButtonFrame::setTXStatusVisible(bool visible)
{
    ui->txStatusIndicator->setVisible(visible);
    ui->txStatusLabel->setVisible(visible);
}

void DMButtonFrame::setRepeatIndicatorForMessageOnOff(bool state)
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


void DMButtonFrame::setRepeatIndicatorOnOff(bool on)
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

void DMButtonFrame::setKeyerIndicatorGroupBoxVisible(bool visible)
{
    ui->keyerIndicatorGroupBox->setVisible(visible);
}

void DMButtonFrame::setPttIndicatorGroupBoxVisible(bool visible)
{
    ui->pttIndicatorGroupBox->setVisible(visible);
}

void DMButtonFrame::setErrorMessageVisible(bool visible)
{
    ui->errorTitleLabel->setVisible(visible);
    ui->errorMeassageLabel->setVisible(visible);
}

void DMButtonFrame::displayErrorMessage(QString msg)
{
    setErrorMessageVisible(true);
    ui->errorMeassageLabel->setText(msg);
}

void DMButtonFrame::clearErrorMessage()
{
    ui->errorMeassageLabel->clear();
    setErrorMessageVisible(false);
}

void DMButtonFrame::setRadioPttState(bool state)
{
    if (txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
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
    txKeyer->setPip(ui->pipCb->isChecked());
}

void DMButtonFrame::setPttStatusIndicatorOnOff(bool on)
{
    if (on)
    {
        setMessagePlayingFlag(true);
        ui->txStatusIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);
        ui->txStatusIndicator->setToolTip(tr("TX On"));

    }
    else
    {
        setMessagePlayingFlag(false);
        ui->txStatusIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);
        ui->txStatusIndicator->setToolTip(tr("TX Off"));
    }

}

void DMButtonFrame::setPttEnabledIndicatorOnOff(bool on)
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

void DMButtonFrame::setEomTypeLabelsVisible(bool visible)
{
    ui->eomLabel->setVisible(visible);
    ui->eomText->setVisible(visible);
}

void DMButtonFrame::setEomLabelText(int selectedEomType)
{
    if (selectedEomType == TxKeyerCommon::KeyerEomTypes::CAT)
    {
        ui->eomText->setText("CAT");
    }
    else if (selectedEomType == TxKeyerCommon::KeyerEomTypes::Timer)
    {
        ui->eomText->setText("Timer");
    }
    else if (selectedEomType == TxKeyerCommon::KeyerEomTypes::InternalSoundCardVoiceKeyer)
    {
        ui->eomText->setText("EOF");
    }
    else if (selectedEomType == TxKeyerCommon::KeyerEomTypes::Eom_None)
    {
        ui->eomText->setText("None");
    }
}





bool DMButtonFrame::eventFilter(QObject *obj, QEvent *event)
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
    parseFKeyFile(fkeyFileName);

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
    if (c && c == ct)
    {
        if (isDataMode() && txKeyerType == keyerTypes[TxKeyerId::DigitalModes])
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

            auto &contestMap = allKeyConfigs[txKeyerType];
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
    if (txKeyerType == keyerTypes[TxKeyerId::RigControl] || txKeyerType == keyerTypes[TxKeyerId::CW_RigControl])
    {
        rigKey = selectedRadio.key();
    }

    auto &contestMap = allKeyConfigs[txKeyerType];
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
            ui->FButtonFrame->setEnabled(true);

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


void DMButtonFrame::populateFksetCombo(QString txKeyerName, QString currentName)
{
    ignoreFkComboSignal = true;

    nameList.clear();
    ui->fkeysetCombo->clear();

    nameList = getContestNamesForKeyerType(txKeyerName);
    ui->fkeysetCombo->addItems(nameList);
    ui->fkeysetCombo->setCurrentText(currentName);

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
    if (!allKeyConfigs.contains(txKeyerType))
        return radioList;

    auto &contestMap = allKeyConfigs[txKeyerType];

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

void DMButtonFrame::parseFKeyFile(QString fname)
{
    allKeyConfigs.clear();

    QFile lf(fname);

    if (!lf.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QString ebuff = QString("Failed to open Function Key file %1").arg(fname);
        MinosParameters::getMinosParameters()->mshowMessage(ebuff);
        return;
    }

    QString s = lf.readAll();
    bool retval = parseFKeyString(s);

    if (retval == false)
    {
        mShowMessage(tr("Invalid or missing FKey definitions"), this);
    }
    else
    {
        // Validate the loaded configuration
        ValidationResult validation = validateKeyConfigs(allKeyConfigs);

        if (!validation.isValid)
        {
            QString errorMsg = tr("Configuration file has errors:\n\n");
            errorMsg += validation.errors.join("\n");

            if (!validation.warnings.isEmpty())
            {
                errorMsg += tr("\n\nWarnings:\n");
                errorMsg += validation.warnings.join("\n");
            }

            mShowMessage(errorMsg, this);
            // Optionally: clear allKeyConfigs or attempt repair
        }
        else if (!validation.warnings.isEmpty())
        {
            // Log warnings but continue
            qWarning() << "Configuration loaded with warnings:";
            for (const QString &warn : validation.warnings)
            {
                qWarning() << "  " << warn;
            }
        }

        populateFksetCombo(txKeyerType, currentName);
    }
}

bool DMButtonFrame::parseFKeyArray(const QJsonArray &array, KeySet &dest)
{
    dest.clear();  // Clear existing data before parsing

    for (const QJsonValue &val : array) {
        if (!val.isObject()) {
            // Skip if not an object
            continue;
        }

        QJsonObject obj = val.toObject();

        KeyVal p;
        p.setFk(obj.value("key").toString());
        p.setKtop(obj.value("label").toString());
        p.setKval(obj.value("message").toString());
        p.setMsgDur(obj.value("messageDuration").toInt());
        p.setMsgDurEnable(obj.value("messageDurEnable").toBool());
        p.setRigVoiceMemNum(obj.value("rigVoiceMemNum").toInt());
        p.setRptEnable(obj.value("repeatEnable").toBool());
        p.setRptDur(obj.value("repeatDuration").toInt());

        dest.append(p);
    }

    return true;
}




bool DMButtonFrame::parseFKeyString(QString &s)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(s.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << err.errorString();
        return false;
    }

    if (!doc.isObject()) {
        qWarning() << "Expected top-level JSON object";
        return false;
    }

    QJsonObject rootObj = doc.object();
    if (!rootObj.contains("KeyerConfig")) {
        qWarning() << "Missing KeyerConfig in JSON";
        return false;
    }
    QJsonObject keyerConfig = rootObj["KeyerConfig"].toObject();



    QStringList keyerTypes = keyerConfig.keys();
    for (const QString &keyerType : std::as_const(keyerTypes))
    {
        QJsonObject contestMap = keyerConfig[keyerType].toObject();
        QStringList contests = contestMap.keys();

        for (const QString &contest : std::as_const(contests))
        {
            QJsonObject rigMap = contestMap[contest].toObject();
            QStringList rigModels = rigMap.keys();

            for (const QString &rigModel : std::as_const(rigModels))
            {
                QJsonObject section = rigMap[rigModel].toObject();

                ContestSection &cs = allKeyConfigs[keyerType][contest][rigModel];
                if (section.contains("Run"))
                {
                    parseFKeyArray(section["Run"].toArray(), cs.run);
                }
                if (section.contains("SandP"))
                {
                    parseFKeyArray(section["SandP"].toArray(), cs.sp);
                }
                if (!nameList.contains(contest))
                    nameList.append(contest);
            }
        }
    }

    clearAllDirtyFlags();  // Clear all dirty flags after loading
                           // as they have not been changed yet.

    return true;
}

void DMButtonFrame::rewriteFKeyFile()
{
    QJsonDocument json;
    QJsonObject keyerConfigObj;  // Top-level "KeyerConfig" object

    for (auto keyerIt = allKeyConfigs.constBegin(); keyerIt != allKeyConfigs.constEnd(); ++keyerIt)
    {
        const QString &keyerType = keyerIt.key();
        const auto &contestMap = keyerIt.value();

        QJsonObject contestObj;
        for (auto contestIt = contestMap.constBegin(); contestIt != contestMap.constEnd(); ++contestIt)
        {
            const QString &contestName = contestIt.key();
            const auto &rigMap = contestIt.value();

            QJsonObject rigModelObj;
            for (auto rigIt = rigMap.constBegin(); rigIt != rigMap.constEnd(); ++rigIt)
            {
                const QString &rigModel = rigIt.key();
                const ContestSection &section = rigIt.value();

                QJsonArray runArray;
                for (const KeyVal &k : section.run)
                {
                    QJsonObject obj;
                    obj["key"] = k.fk();
                    obj["label"] = k.ktop();
                    obj["message"] = k.kval();
                    obj["messageDuration"] = k.msgDur();
                    obj["messageDurEnable"] = k.msgDurEnable();
                    obj["rigVoiceMemNum"] = k.rigVoiceMemNum();
                    obj["repeatEnable"] = k.rptEnable();
                    obj["repeatDuration"] = k.rptDur();
                    runArray.append(obj);
                }

                QJsonArray spArray;
                for (const KeyVal &k : section.sp)
                {
                    QJsonObject obj;
                    obj["key"] = k.fk();
                    obj["label"] = k.ktop();
                    obj["message"] = k.kval();
                    obj["messageDuration"] = k.msgDur();
                    obj["messageDurEnable"] = k.msgDurEnable();
                    obj["rigVoiceMemNum"] = k.rigVoiceMemNum();
                    obj["repeatEnable"] = k.rptEnable();
                    obj["repeatDuration"] = k.rptDur();
                    spArray.append(obj);
                }

                QJsonObject sectionObj;
                sectionObj["Run"] = runArray;
                sectionObj["SandP"] = spArray;

                rigModelObj[rigModel] = sectionObj;
            }

            contestObj[contestName] = rigModelObj;
        }

        keyerConfigObj[keyerType] = contestObj;
    }

    QJsonObject rootObj;
    rootObj["KeyerConfig"] = keyerConfigObj;

    json.setObject(rootObj);

    QByteArray s = json.toJson(QJsonDocument::Indented);

    QFile jf(fkeyFileName);
    if (!jf.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        logMessage("Failed to open " + fkeyFileName);
        return;
    }
    jf.write(s);
    jf.close();
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
                    result.addError(QString("Missing Run section for rig '%1' in contest '%2', keyer '%3'")
                                        .arg(rigName, contestName, keyerType));
                }
                else if (section.run.size() != 12)
                {
                    result.addError(QString("Invalid Run size (%1, expected 12) for rig '%2' in contest '%3', keyer '%4'")
                                        .arg(section.run.size()).arg(rigName, contestName, keyerType));
                }

                // Validate S&P KeySet
                if (section.sp.isEmpty())
                {
                    result.addError(QString("Missing S&P section for rig '%1' in contest '%2', keyer '%3'")
                                        .arg(rigName, contestName, keyerType));
                }
                else if (section.sp.size() != 12)
                {
                    result.addError(QString("Invalid S&P size (%1, expected 12) for rig '%2' in contest '%3', keyer '%4'")
                                        .arg(section.sp.size()).arg(rigName, contestName, keyerType));
                }

                // Validate KeyVal data integrity
                for (int i = 0; i < section.run.size(); ++i)
                {
                    const KeyVal &k = section.run[i];
                    if (k.fk().isEmpty())
                    {
                        result.addWarning(QString("Run[%1]: Empty key for rig '%2', contest '%3'")
                                              .arg(i).arg(rigName, contestName));
                    }
                }

                for (int i = 0; i < section.sp.size(); ++i)
                {
                    const KeyVal &k = section.sp[i];
                    if (k.fk().isEmpty())
                    {
                        result.addWarning(QString("S&P[%1]: Empty key for rig '%2', contest '%3'")
                                              .arg(i).arg(rigName, contestName));
                    }
                }
            }
        }
    }

    return result;
}


bool DMButtonFrame::checkRadioExists(QString radioName, bool &radioExists)
{

    bool ok = false;

    if (radioName.isEmpty())
    {

        radioExists = false;
        return false;
    }

    if (allKeyConfigs.contains(txKeyerType))
    {
        const ContestMap &contestMap = allKeyConfigs.value(txKeyerType);

        ok = true;

        if (contestMap.contains(currentName))
        {
            const RigMap &rigMap = contestMap.value(currentName);

            if (rigMap.contains(radioName))
            {

                radioExists = true;
            }
            else
            {

                radioExists = false;
            }
        }
    }

    return ok;
}



void DMButtonFrame::on_stopButton_clicked()
{
    if (txKeyerType == keyerTypes[TxKeyerId::None])
    {
        return;
    }
    else if (txKeyerType != keyerTypes[TxKeyerId::DigitalModes])
    {

        logMessage("Digimode - send stop transmission to sender app");
        RPCGeneralClient rpc(rpcConstants::DMStopTransmit);
        rpc.queueCall( dataSender );
    }
    else if (txKeyerType == keyerTypes[TxKeyerId::CW_RigControl]
        || txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
    {
        txKeyer->stopCwMsg();

        if (curMode != savedMode && txKeyer->getSetCwModeAndRestoreFlag())       // restore mode?
        {
            sendModeToRadio(savedMode);
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

void DMButtonFrame::on_editButton_clicked()
{
    // bring up default file editor on "fkeyFileName"

    //   QDesktopServices::openUrl(QUrl::fromLocalFile(fkeyFileName));

    // Use built in fKey editor on "fkeyFileName"

    KeyerMap nfk = allKeyConfigs;


    if (txKeyerType == keyerTypes[TxKeyerId::RigControl] || txKeyerType == keyerTypes[TxKeyerId::CW_RigControl])
    {

        listOfRadios = LogContainer->sendDM->rigs();

        getVoiceCwMemSupportedRadios(listOfRadios, listOfRadioSupportKeyer);
        mapUniqueNames(listOfRadioSupportKeyer, radioMap);
    }

    DMKeysEditDlg jed(this, fkeyFileName, currentName, nfk, txKeyerType, selectedRadio, listOfRadioSupportKeyer);

    if (jed.exec() == QDialog::Accepted)
    {
        allKeyConfigs = nfk;
        // and we have to regenerate the JSON file
        rewriteFKeyFile();
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
    if (ignoreFkComboSignal)
        return;

    currentName = arg1;

    if (txKeyerType == keyerTypes[TxKeyerId::RigControl])
    {
        ct->rigControlCurrentFKeySetContest.setValue(currentName);

    }
    else  if (txKeyerType == keyerTypes[TxKeyerId::CW_RigControl])
    {
        ct->cwRigControlCurrentFKeySetContest.setValue(currentName);

    }
    else  if (txKeyerType == keyerTypes[TxKeyerId::SerialControl])
    {
        ct->serialControlCurrentFKeySetContest.setValue(currentName);
    }
    else  if (txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
    {
        ct->pcCwKeyerCurrentFKeySetContest.setValue(currentName);
    }
    else  if (txKeyerType == keyerTypes[TxKeyerId::DigitalModes])
    {
        ct->digitalModesCurrentFKeySetContest.setValue(currentName);
    }
    else  if (txKeyerType == keyerTypes[TxKeyerId::InternalVoiceKeyer])
    {
        ct->internalVoiceKeyerCurrentFKeySetContest.setValue(currentName);
    }
    else  if (txKeyerType == keyerTypes[TxKeyerId::ExternalVoiceKeyer])
    {
        ct->externalVoiceKeyerCurrentFKeySetContest.setValue(currentName);
    }



    ct->commonSave(false);
}

void DMButtonFrame::setCwMessagePlayingVisible(bool visible)
{
    ui->cwMesssagePlayingLabel->setVisible(visible);
    ui->cwMessagePlayingDisplay->setVisible(visible);
}
void DMButtonFrame::clearCwMessageDisplay()
{
    ui->cwMessagePlayingDisplay->clear();
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



void DMButtonFrame::sendModeToRadio(const QString m)
{

}


void DMButtonFrame::setMode(const QString m)
{

}


void DMButtonFrame::setPcCwKeyerComport(QString comportStr)
{
    if (txKeyer && txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
    {

    }
}
void DMButtonFrame::setPcCwKeyerConnectionState(QString stateStr)
{
    logMessage(QString("PcCwKeyerConnection state = %1").arg(stateStr));
    if (txKeyer && txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
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
void DMButtonFrame::setPcCwKeyerErrorMsg(QString errorMsg)
{
    logMessage(QString("PcCwKeyerConnection error message = %1").arg(errorMsg));
    if (txKeyer && txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
    {

    }
}
void DMButtonFrame::setPcCwKeyerPttEnabled(QString enabled)
{
    logMessage(QString("PcCwKeyer Ptt enabled = %1").arg(enabled));

    if (txKeyer && txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
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
void DMButtonFrame::setPcCwKeyerTxOnState(QString state)
{
    logMessage(QString("TX State received = %1").arg(state));

    if (txKeyer && txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
    {
        if (state == "On")
        {
            setPttStatusIndicatorOnOff(true);
        }
        else
        {
            setPttStatusIndicatorOnOff(false);
            clearCwMessageDisplay();
            onMsgDurTimerTimeout();
        }
    }
}
void DMButtonFrame::setPcCwKeyerCurrentWpm(QString wpm)
{
    logMessage(QString("Current WPM from PcCwKeyer = %1").arg(wpm));
    if (wpm.isEmpty())
    {
        return;
    }

    if (txKeyer && txKeyerType == keyerTypes[TxKeyerId::PcCwKeyer])
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

