#include "tlogcontainer.h"
#include "delayedaction.h"
#include "SendRPCDM.h"
#include "KeyerJson.h"
#include "cutils.h"
#include "txvmbuttonsframe.h"
#include "ui_txvmbuttonsframe.h"


const int NO_VM_BUTTON_ON = -1;

const QStringList vmButtonShortCutKeys = {
                                    "Shift+F1", "Shift+F2",
                                    "Shift+F3", "Shift+F4",
                                    "Shift+F5", "Shift+F6",
                                    "Shift+F7", "Shift+F8"

                                    };
const QString vmStopButtonShortCutKey = "Shift+F10";


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

    initTxVmButton();

    setPttStatusIndicatorOnOff(false);
}

TxVmButtonsFrame::~TxVmButtonsFrame()
{
    delete ui;

    for(const auto &b: qAsConst(txVmButtonMap))
    {
        delete b;
    }
}




void TxVmButtonsFrame::initTxVmButton()
{
    voiceMemButtonList << ui->vmToolButton1 << ui->vmToolButton2 << ui->vmToolButton3 << ui->vmToolButton4
                       << ui->vmToolButton5 << ui->vmToolButton6 << ui->vmToolButton7 << ui->vmToolButton8;

    for (int i = 0; i < voiceMemButtonList.count(); i++)
    {
        txVmButtonMap[i] = new TxVoiceMemButton(voiceMemButtonList[i], this, i);

    }

    stopButtonShortcut = new QShortcut(QKeySequence(vmStopButtonShortCutKey), ui->vmStopPb);
    connect(stopButtonShortcut, &QShortcut::activated, this, &TxVmButtonsFrame::onVmStopClicked);

    connect(ui->vmSetupPb, &QPushButton::clicked, this, &TxVmButtonsFrame::onVmSetupClicked);
    connect(ui->vmStopPb, &QPushButton::clicked, this, &TxVmButtonsFrame::onVmStopClicked);

    clearButtonLabels();

    setVoiceNumMemButtonsVisible(VOICEKEYER_MAX_NUMBUTTONS);


    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH + VOICEKEYER_COMMON_PARAMS_FILENAME;
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup(VOICEKEYER_COMMON_PARAMS_GROUPNAME);

    QString voiceKeyerName = config.value("KeyerName").toString();

    voiceKeyerFactory->populateComboKeyerList(ui->voiceKeyerSelect, voiceKeyerName);
    connect(ui->voiceKeyerSelect, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TxVmButtonsFrame::onVoiceKeyerSelect);

    trace(QString("start keyer name = %1").arg(ui->voiceKeyerSelect->currentText()));

    setAvailIndicatorVisible(false);
    setRepeatIndicatorVisible(false);

    config.endGroup();
}


void TxVmButtonsFrame::setVoiceNumMemButtonsVisible(int num)
{
    for (int i = 0; i < voiceMemButtonList.count(); i++)
    {
        voiceMemButtonList[i]->setVisible(false);
    }

    for (int i = 0; i < num; i++)
    {
        voiceMemButtonList[i]->setVisible(true);
    }
}

void TxVmButtonsFrame::onVmSetupClicked()
{
    if (voiceKeyerType != keyerTypes[VoiceKeyerId::None])
    {
        int oldnb = txVoiceKeyer->numButtons;
        if (txVoiceKeyer->setup(voiceKeyerFactory, txVoiceKeyer->numButtons) == QDialog::Accepted)
        {
            if (txVoiceKeyer->numButtons != oldnb)
            {
                setVoiceNumMemButtonsVisible(txVoiceKeyer->numButtons);
            }
        }
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
            /*
            if (voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl])
            {
                if (!isVoiceMemAvail(selectedRadio))
                {
                    trace(QString("[Voice Keyer]  Voice memory not available for this radio"));
                    return;
                }

            }
            else if ( voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl])
            {
                if (!isCwMemTypeAvail(selectedRadio))
                {
                    trace(QString("[Voice Keyer] CW memory not available for this radio"));
                    return;
                }
            }
            */


            txVoiceKeyer = QSharedPointer<VoiceKeyerBase>(voiceKeyerFactory->createVoiceKeyer(voiceCap.getVmIdNum()));
            if (txVoiceKeyer)
            {
                trace(QString("[Voice Keyer] Voice Keyer type selected = %1").arg(voiceCap.getKeyerName()));

                connect(txVoiceKeyer.data(), &VoiceKeyerBase::remoteConfigChanged, this, &TxVmButtonsFrame::onRemoteConfigChanged, Qt::UniqueConnection);
                connect(txVoiceKeyer.data(), &VoiceKeyerBase::remoteKeyerStopped, this, &TxVmButtonsFrame::onRemoteKeyerStopped, Qt::UniqueConnection);
                connect(txVoiceKeyer.data(), &VoiceKeyerBase::remoteKeyerStarted, this, &TxVmButtonsFrame::onRemoteKeyerStarted, Qt::UniqueConnection);

                txVoiceKeyer->voiceKeyerInit(txVoiceKeyer->numButtons);

               for (int i = 0; i < voiceMemButtonList.count(); i++)
               {
                   VoiceKeyerParams vmData;
                   if (vmData.getType().isEmpty())
                   {
                       vmData.setType(voiceKeyerType);
                   }

                   txVoiceKeyer->readVmButtonParams(i, vmData);
                   vmKeyParamList.append(vmData);
                   setRunButtonText(i, vmData.getVmName());
               }

               setVoiceNumMemButtonsVisible(txVoiceKeyer->numButtons);


               ui->vmSetupPb->setVisible(txVoiceKeyer->hasSetup());
               ui->pipCb->setVisible(txVoiceKeyer->hasPip());

               if ( voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl] || voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl])
               {
                   setFrameWidgetsState();
               }
            }
        }
    }
}
void TxVmButtonsFrame::onExtConnectTimer()
{
    QString voiceKeyerName = ui->voiceKeyerSelect->currentText();

    notifyComboChange = false;
    voiceKeyerFactory->populateComboKeyerList(ui->voiceKeyerSelect, voiceKeyerName);
    notifyComboChange = true;

    VoiceKeyerCapabilities voiceCap = voiceKeyerFactory->supportedVoiceKeyers()->value(voiceKeyerName);
    voiceKeyerType = voiceCap.getKeyerType();
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
    else
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
    qDebug() << "keyer select name = " << ui->voiceKeyerSelect->currentText();

    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH + VOICEKEYER_COMMON_PARAMS_FILENAME;
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup(VOICEKEYER_COMMON_PARAMS_GROUPNAME);

    config.setValue("KeyerName", voiceKeyerName);

    config.endGroup();

    txVoiceKeyer.clear();

    createKeyer(voiceKeyerName);

    if (txVoiceKeyer == nullptr)
    {
       txVoiceKeyer = nullptr;
       clearButtonLabels();
       vmKeyParamList.clear();
       setVoiceNumMemButtonsVisible(0);
       if (voiceKeyerType == keyerTypes[ VoiceKeyerId::ExternalVoiceKeyer])
       {
           ui->noExtKeyerLabel->setText(HtmlFontColour(Qt::red) +  tr("To use the external keyer mqtKeyer must be running and connected"));
           extKeyerConnectTimer->start(1000);
       }
       voiceKeyerType = keyerTypes[VoiceKeyerId::None];

       setAvailIndicatorVisible(false);
       setRepeatIndicatorVisible(false);

       ui->vmSetupPb->setVisible(false);
       ui->pipCb->setVisible(false);

    }
    else
    {
        ui->noExtKeyerLabel->clear();
    }



}


void TxVmButtonsFrame::setFrameWidgetsState()
{
    if (voiceKeyerType == keyerTypes[ VoiceKeyerId::RigControl] || voiceKeyerType == keyerTypes[ VoiceKeyerId::CW_RigControl])
    {
        setAvailIndicatorVisible(true);
        setAvailIndicatorForRadioOnOff(selectedRadio);
        setRepeatIndicatorVisible(true);
    }
    else
    {
        setAvailIndicatorVisible(false);
        setRepeatIndicatorVisible(false);
        return;
    }

    if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl])
    {
        txVoiceKeyer->setCwMemType(getCwMemType(selectedRadio));
        if (getCwMemType(selectedRadio) == hamlibData::CW_MEMORY_TYPES::YAESU_MEM_RECALL)
        {
            // no stop with CW Memory Recall
            ui->vmStopPb->setVisible(false);

        }
        else
        {
            ui->vmStopPb->setVisible(true);
        }
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
    if (voiceKeyerType == keyerTypes[VoiceKeyerId::None])
    {
        return;
    }

    VoiceKeyerParams vmData;
    vmData.setType(voiceKeyerType);

    if (txVoiceKeyer)
    {
        txVoiceKeyer->readVmButtonParams(buttonNumber, vmData);
        if (vmData.getType().isEmpty())    // in case read data is empty
        {
            vmData.setType(voiceKeyerType);
        }
        vmData.setVkBase(txVoiceKeyer);
        trace(QString("[voiceMemSetup] edit selected button no = %1").arg(buttonNumber));

        QString title1 = "";
        if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl])
        {
            title1 = tr("Rig CW Message");
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
    if (voiceKeyerType == keyerTypes[VoiceKeyerId::None])
    {
        trace(QString("TxVmButtonsFrame - readActionSelected No Keyer Selected "));
        return;
    }


    if (voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl] && !isVoiceMemAvail(selectedRadio))
    {
        trace(QString("TxVmButtonsFrame - readActionSelected rigControl Voice Keyer Selected, but not available for this radio "));
        return;
    }
    else if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl] && !isCwMemTypeAvail(selectedRadio))
    {
        trace(QString("TxVmButtonsFrame - readActionSelected rigControl CW Message Keyer Selected, but not available for this radio "));
        return;
    }

    VoiceKeyerParams vmData;
    vmData.setType(voiceKeyerType);
    txVoiceKeyer->readVmButtonParams(buttonNumber, vmData);
    setRepeatIndicatorOnOff(vmData.getVmRepeatFlag());


    if (buttonNumSent != NO_VM_BUTTON_ON)
    {
        onVmStopClicked();

    }

    startVMMsg(buttonNumber);
}


void TxVmButtonsFrame::startVMMsg(int buttonNumber)
{
    buttonNumSent = buttonNumber;

    if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl])
    {
        VoiceKeyerParams vmData;
        vmData.setType(voiceKeyerType);
        txVoiceKeyer->readVmButtonParams(buttonNumber, vmData);

        txVoiceKeyer->sendCwMsg(vmData.getVmCwMessage());
    }
    else
    {
        txVoiceKeyer->sendMsgNum(buttonNumSent);
    }



    int msgDur = vmKeyParamList[buttonNumber].getVmDuration() * 1000;
    if (msgDur > 0)
    {
        msgDurTimer->start(msgDur);
    }
    txVmButtonMap[buttonNumber]->showButtonOnOff(true);

}


void TxVmButtonsFrame::onVmStopClicked()
{
    if (voiceKeyerType == keyerTypes[VoiceKeyerId::None] || buttonNumSent == NO_VM_BUTTON_ON)
    {
        return;
    }



    if (voiceKeyerType == keyerTypes[VoiceKeyerId::CW_RigControl])
    {
        txVoiceKeyer->stopCwMsg();
    }
    else
    {
        txVoiceKeyer->stopMsg();
    }

    msgDurTimer->stop();
    repeatPauseTimer->stop();
    txVmButtonMap[buttonNumSent]->showButtonOnOff(false);
    setRepeatIndicatorOnOff(false);
    buttonNumSent = NO_VM_BUTTON_ON;
}

void TxVmButtonsFrame::writeActionSelected(int buttonNumber)
{
    if (voiceKeyerType == keyerTypes[VoiceKeyerId::None])
    {
        return;
    }
    VoiceKeyerParams vmData;
    vmData.clear();


    trace(QString("[voiceMemSetup] write selected button no = %1").arg(buttonNumber));


//    TxVmRigButtonDialog vmButtonDialog(this);

    QString title(tr("Voice Memory %1 - New").arg(buttonNumber + 1));
    vmData.setvmButtonNum(buttonNumber);
    vmData.setType(voiceKeyerType);
    vmData.setVkBase(txVoiceKeyer);

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

void TxVmButtonsFrame::clearActionSelected(int buttonNumber)
{
    if (voiceKeyerType == keyerTypes[VoiceKeyerId::None])
    {
        return;
    }

    VoiceKeyerParams vmData;
    vmData.clear();
    vmData.setvmButtonNum(buttonNumber);
    vmData.setType(voiceKeyerType);
    vmData.setVkBase(txVoiceKeyer);

    QMessageBox msgBox;
    msgBox.setText(tr("Are you sure you want to clear this button %1?").arg(buttonNumber));
    msgBox.setInformativeText(tr("Click save to clear, cancel to ignore"));
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    int ret = msgBox.exec();

    switch (ret)
    {
      case QMessageBox::Save:
        if (txVoiceKeyer)
        {
            txVoiceKeyer->saveVmButtonParams(vmData);
            setRunButtonText(buttonNumber, vmData.getVmName());
            vmKeyParamList[buttonNumber] = vmData;
        }
          break;

      case QMessageBox::Cancel:
          // Cancel was clicked
          break;
      default:
          // should never be reached
          break;
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

    int msgDur = vmKeyParamList[buttonNumSent].getVmDuration() * 1000;
    if (msgDur > 0)
    {
        msgDurTimer->start(msgDur);
    }
    txVmButtonMap[buttonNumSent]->showButtonOnOff(true);
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
    txVmButtonMap[buttonNumSent]->showButtonOnOff(false);
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
        if (vmKeyParamList[buttonNumSent].getVmRepeatFlag())
        {
            int repeatPauseDur = vmKeyParamList[buttonNumSent].getVmRepeatPauseDur() * 1000;
            repeatPauseTimer->start(repeatPauseDur);
        }
        else
        {
            txVmButtonMap[buttonNumSent]->showButtonOnOff(false);
            setRepeatIndicatorOnOff(false);
            buttonNumSent = NO_VM_BUTTON_ON;
        }
    }

    msgDurTimer->stop();


}



void TxVmButtonsFrame::onRepeatPauseTimerTimeout()
{
    if (buttonNumSent >= 0)
    {
       if (vmKeyParamList[buttonNumSent].getVmRepeatFlag())
       {
           startVMMsg(buttonNumSent);

       }
       else
       {
           onVmStopClicked();


       }
    }

    repeatPauseTimer->stop();


}



void TxVmButtonsFrame::setRadioIsConnected(bool connected)
{
    radioConnected = connected;
}

void TxVmButtonsFrame::setSelectedRadio(PubSubName selectedRadio_)
{
    if (selectedRadio != selectedRadio_)
    {
        selectedRadio = selectedRadio_;

        setFrameWidgetsState();


    }


}

void TxVmButtonsFrame::setPttEnabled(bool state, PubSubName psn)
{
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


}

void TxVmButtonsFrame::setPttType(int type, PubSubName psn)
{
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
}

void TxVmButtonsFrame::setVoiceMemAvail(bool avail, PubSubName psn)
{
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

    setFrameWidgetsState();
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

void TxVmButtonsFrame::setCwMemType(int cwMemType, PubSubName psn)
{
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

    setFrameWidgetsState();
}

bool TxVmButtonsFrame::isCwMemTypeAvail(PubSubName psn)
{
    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        if (rd.getCwMemType() == hamlibData::CW_MEMORY_TYPES::NONE)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    return false;
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

void TxVmButtonsFrame::setAvailIndicatorVisible(bool visible)
{

    ui->availLabel->setVisible(visible);
    ui->availIndicator->setVisible(visible);

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

void TxVmButtonsFrame::setRepeatIndicatorVisible(bool visible)
{
    ui->repeatIndicator->setVisible(visible);
    ui->repeatLabel->setVisible(visible);
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

void TxVmButtonsFrame::setPttState(bool state)
{
    if (pttState != state)
    {
        pttState = state;
        setPttStatusIndicatorOnOff(state);
        emit pttStatus(pttState);
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

    }
    else
    {
       ui->txStatusIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);
       ui->txStatusIndicator->setToolTip(tr("TX Off"));
    }

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

    shortKey = new QShortcut(QKeySequence(vmButtonShortCutKeys[memNo]), vmButton);

    readAction = new QAction(tr("&Read"), vmButton);
    newAction = new QAction(tr("&New"),vmButton);
    editAction = new QAction(tr("&Edit"), vmButton);
    clearAction = new QAction(tr("&Clear"), vmButton);
    vmMenu->addAction(readAction);
    vmMenu->addAction(newAction);
    vmMenu->addAction(editAction);
    vmMenu->addAction(clearAction);


    vmButton->setMenu(vmMenu);

    connect(shortKey, &QShortcut::activated, this, &TxVoiceMemButton::readActionSelected);
    connect(readAction, &QAction::triggered, this, &TxVoiceMemButton::readActionSelected);
    connect(vmButton, &QToolButton::clicked, this, &TxVoiceMemButton::readActionSelected);
    connect(vmButton, &QToolButton::clicked, this, &TxVoiceMemButton::buttonSelected);
    connect( newAction, &QAction::triggered, this, &TxVoiceMemButton::writeActionSelected);
    connect( editAction, &QAction::triggered, this, &TxVoiceMemButton::editActionSelected);
    connect(clearAction, &QAction::triggered, this, &TxVoiceMemButton::clearActionSelected);


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
    txVmButtonsFrame->readActionSelected(memNo);
}
void TxVoiceMemButton::editActionSelected()
{

    txVmButtonsFrame->editActionSelected(memNo);

}
void TxVoiceMemButton::writeActionSelected()
{
    txVmButtonsFrame->writeActionSelected(memNo);
}

void TxVoiceMemButton::clearActionSelected()
{
    txVmButtonsFrame->clearActionSelected(memNo);
}


void TxVoiceMemButton::buttonSelected()
{
    emit buttonActivated(memNo);
}

void TxVoiceMemButton::showButtonOnOff(bool state)
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
