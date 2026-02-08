#include "ContestApp.h"
#include "LoggerContest.h"
#include "rotatorcommon.h"
#include "runbuttondialog.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "SendRPCDM.h"
#include "MTrace.h"

#include "runbuttonsframe.h"
#include "ui_runbuttonsframe.h"

const int RUN_TOLERANCE = 300; // Hz

const char * RUN_BUTTON_ON_FREQ_STYLE = "background-color: orange ; color:black ; border-style: outset; border-width: 1px; border-color: black;\n";
const char * RUN_BUTTON_OFF_FREQ_STYLE = "background-color: yellow ; color:black ; border-style: outset; border-width: 1px; border-color: black;\n";
const char * RUN_BUTTON_OFF_STYLE = "background-color: Gainsboro ; color:black ; border-style: outset; border-width: 1px; border-color: black;\n";

const int CHECK_RUN_FREQ_POLLTIME = 1000;

QKeySequence RunMemoryButton::runButShortCut[] {
    QKeySequence(Qt::CTRL | Qt::Key_BracketLeft),
    QKeySequence(Qt::CTRL | Qt::Key_BracketRight)

};

QKeySequence RunMemoryButton::runButShiftShortCut[] {
    QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_BracketLeft),
    QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_BracketRight)

};
RunmodeController::RunmodeController()
{
    chkRunFreqTimer = new QTimer(this);
    connect(chkRunFreqTimer, &QTimer::timeout, this, &RunmodeController::on_radioFreqCheckTimer);

    chkRunFreqTimer->start(CHECK_RUN_FREQ_POLLTIME);    // usually once a second

}
RunmodeController::~RunmodeController()
{}

void RunmodeController::on_radioFreqCheckTimer()
{
    // should be a "belt and braces" check on frequency
    // moved from run frequency

    if (rigControl)
    {
        curRadioFreq = rigControl->getCurFreq();
        chkRunFreq();
    }
}
void RunmodeController::setFreq(Frequency freq)
{
    // either freq from rig control,
    // or from a pre-emptive response to our sending a new frequency

    curRadioFreq = freq;

    chkRunFreq();
}

void RunmodeController::chkRunFreq()
{
    if (runButtonOnNum >= 0)
    {
        // if current freq is sufficiently far from the run freq, turn run off
        // we should probably ose the band map frequency offset rules

        if (!curRunFreq.isClear())
        {
            bool oldRadioOffRunFreq = radioOffRunFreq;

            radioOffRunFreq = !chkRadioFreqOnRunFreq();

            if (radioOffRunFreq)
            {
                rbf->runButtonMap[runButtonOnNum]->showRunToolButtonOffFreq();
            }
            else
            {
                rbf->runButtonMap[runButtonOnNum]->showRunToolButtonOnFreq();
            }

           if (oldRadioOffRunFreq!= radioOffRunFreq)
            {
                TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
                tslf->sendRunOffFreqFlag(curRunFreq, radioOffRunFreq);
            }
        }
    }
}
bool RunmodeController::chkRadioFreqOnRunFreq()
{
    qint64 curRunF = curRunFreq;
    qint64 curF = curRadioFreq;

    if (curRunF != 0)
    {
        if ((curF >= (curRunF - RUN_TOLERANCE)) && (curF <= (curRunF + RUN_TOLERANCE)))
        {
            return true;
        }
    }

    return false;
}
void RunmodeController::setRunModeOff()
{
    runButtonOnNum = NO_RUN_BUTTON_ON;

    chkRunFreqTimer->setInterval(CHECK_RUN_FREQ_POLLTIME);  // restart check timer

    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    tslf->sendRunOnFlag(curRunFreq, curRunMode, false);
}

void RunmodeController::setRunMode(int buttonNumber, memoryData::memData m)
{
    runButtonOnNum = buttonNumber;

    rigControl->clearFreqInputFocus();
    if (m.freq != rigControl->getCurFreq())
    {
        rigControl->sendRigFreq(m.freq);
        // pre-empt us being told; if it doesn't happen, we will get told later
        setFreq(m.freq);

    }

    curRunFreq = m.freq;
    curRunMode = m.mode;

    if (m.mode != rigControl->curMode)
    {
        rigControl->sendModeToRadio(m.mode);
    }
    bool rit;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpCQRit, rit );
    if (rit)
    {
        rigControl->ritButtonOn();
    }
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    tslf->sendRunOnFlag(curRunFreq, curRunMode, true);
}

//=========================================================================
RunButtonsFrame::RunButtonsFrame(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::RunButtonsFrame)
{
    ui->setupUi(this);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::SandPChanged, this, &RunButtonsFrame::sandPChanged);

    rmc.linkRunButtonFrame(this);

    initRunMemoryButton();

    setRunButtonsFrameTitle();

}

RunButtonsFrame::~RunButtonsFrame()
{
    delete ui;

    for(auto const &b: QASCONST(runButtonMap))
    {
        delete b;
    }

}
void RunButtonsFrame::setContest(BaseContestLog *c)
{
    ct = dynamic_cast<LoggerContestLog *>( c);

    if (ct)
    {
        loadRunButtonLabels();
    }
}
void RunButtonsFrame::sandPChanged(bool s)
{
    if (!s)
    {
        rmc.radioOffRunFreq =  true;
        setCallFreq();
    }
    if (rmc.runButtonOnNum > NO_RUN_BUTTON_ON)
    {
        // manual switch to/from run mode
        if (s)  // switch to S&P
        {
            runButtonMap[rmc.runButtonOnNum]->returnFrequency.clear();
            runModeOff(rmc.runButtonOnNum);
        }
        else
        {
            setRunButtonActive(rmc.runButtonOnNum);
        }
    }
}
void RunButtonsFrame::setRigControl(RigControlFrame *rc)
{
    rigControl = rc;
    rmc.rigControl = rc;
    // This shouldn't need to be a unique connection
    connect(rc, &RigControlFrame::setFreqDisplay, this, &RunButtonsFrame::setFreqDisplay, Qt::UniqueConnection);
    connect(rc, &RigControlFrame::radioIsConnected, this, &RunButtonsFrame::radioIsConnected);

}
void RunButtonsFrame::radioIsConnected(bool on)
{
    if (on)
    {
        ui->radioStatusIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);
        ui->radioStatusIndicator->setToolTip(tr("Connected"));

    }
    else
    {
       ui->radioStatusIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);
       ui->radioStatusIndicator->setToolTip(tr("Disconnected"));
    }

}
void RunButtonsFrame::setFreq(Frequency freq)
{
    if (ct)
    {
        QSharedPointer<BandInfo> bandChanged = ct->checkBandChange(freq, rmc.curRadioFreq);
        if (bandChanged)
        {
            // we need to switch the run button mapping
            trace(QString("RunButtonsFrame::setFreq runButtonsFrame band changed"));
            runButOffActionSelected(RUN_BUTTON_1_ON);
            runButOffActionSelected(RUN_BUTTON_2_ON);
            runButtonMap[RUN_BUTTON_1_ON]->returnFrequency.clear();
            runButtonMap[RUN_BUTTON_2_ON]->returnFrequency.clear();
        }
        rmc.setFreq(freq);

        if (!rmc.chkRadioFreqOnRunFreq())
        {
            trace(QString("RunButtonsFrame::setFreq returnFrequency clear both"));
            runButtonMap[RUN_BUTTON_1_ON]->returnFrequency.clear();
            runButtonMap[RUN_BUTTON_2_ON]->returnFrequency.clear();
        }
        setRunButtonText(RUN_BUTTON_1_ON);
        setRunButtonText(RUN_BUTTON_2_ON);
    }
}

void RunButtonsFrame::initRunMemoryButton()
{
    runButtonMap[0] = new RunMemoryButton(ui->RunButton1, this, 0);
    connect( runButtonMap[0], &RunMemoryButton::clearActionActivated , this, &RunButtonsFrame::runButClearActSel, Qt::QueuedConnection );
    connect( runButtonMap[0], &RunMemoryButton::buttonActivated, this, &RunButtonsFrame::runButActivated, Qt::QueuedConnection );

    runButtonMap[1] = new RunMemoryButton(ui->RunButton2, this, 1);
    connect( runButtonMap[1], &RunMemoryButton::clearActionActivated, this, &RunButtonsFrame::runButClearActSel, Qt::QueuedConnection );
    connect( runButtonMap[1], &RunMemoryButton::buttonActivated, this, &RunButtonsFrame::runButActivated, Qt::QueuedConnection );

}
int RunButtonsFrame::otherButton(int buttonNumber)
{
    return buttonNumber^1;
}
void RunButtonsFrame::setRunButtonActive(int buttonNumber)
{
    // a run button is active; but we are not on a run frequency
    // select run freq for this button

    // first, save the current frequency so we can return to it

    memoryData::memData m = getCtRunMemoryData(buttonNumber);
    Frequency curFreq = rigControl->getCurFreq();

    qint64 offset = curFreq - m.freq;

    if (std::abs(offset) < 1000)
    {
        trace(QString(" RunButtonsFrame::setRunButtonActive returnFrequency clear %1").arg(buttonNumber));
        runButtonMap[buttonNumber]->returnFrequency.clear();
    }
    else
    {
        trace(QString(" RunButtonsFrame::setRunButtonActive returnFrequency set %1 to %2").arg(buttonNumber).arg(curFreq.traceStr()));
        runButtonMap[buttonNumber]->returnFrequency = curFreq;
    }
    trace(QString(" RunButtonsFrame::setRunButtonActive returnFrequency clear other"));
    runButtonMap[otherButton(buttonNumber)]->returnFrequency.clear();

    runButReadActSel(buttonNumber);
    setRunButtonText(buttonNumber);
    runButtonMap[buttonNumber]->showButtonOnOff(true);
}

void RunButtonsFrame::setRunFreq(int buttonNumber)
{
    // run button not active
    // make this one active

    Frequency oldfreq = rigControl->getCurFreq();

    runButReadActSel(buttonNumber);

    qint64 offset = oldfreq - rigControl->getCurFreq();
    if (std::abs(offset) > 1000)
    {
        trace(QString("RunButtonsFrame::setRunFreq returnFrequency set %1 to %2").arg(buttonNumber).arg(oldfreq.traceStr()));
        runButtonMap[buttonNumber]->returnFrequency = oldfreq;
        trace(QString("RunButtonsFrame::setRunFreq returnFrequency clear other"));
        runButtonMap[otherButton(buttonNumber)]->returnFrequency.clear();
    }
    else
    {
        trace(QString("RunButtonsFrame::setRunFreq returnFrequency clear both"));
        runButtonMap[RUN_BUTTON_1_ON]->returnFrequency.clear();
        runButtonMap[RUN_BUTTON_2_ON]->returnFrequency.clear();
    }

    runButtonMap[buttonNumber]->showButtonOnOff(true);

    rmc.setRunMode(buttonNumber, getCtRunMemoryData(buttonNumber)
);
}

void RunButtonsFrame::runModeOff(int buttonNumber)
{
    // on run freq - return to prior frequency

    if (buttonNumber > NO_RUN_BUTTON_ON)
    {
        Frequency rfreq = runButtonMap[buttonNumber]->returnFrequency;

        trace(QString("RunButtonsFrame::runModeOff returnFrequency clear both"));
        runButtonMap[buttonNumber]->returnFrequency.clear();
        setRunButtonText(buttonNumber);

        runButtonMap[otherButton(buttonNumber)]->returnFrequency.clear();
        setRunButtonText(buttonNumber);

        if (!rfreq.isClear())
        {
            // go to restore freq

            rigControl->sendRigFreq( rfreq);
            rmc.setFreq(rfreq);

            rmc.chkRunFreqTimer->setInterval(CHECK_RUN_FREQ_POLLTIME);  // reset the interval
        }
        else
        {
            runButOffActionSelected(buttonNumber);
        }
    }
}

void RunButtonsFrame::switchRunButton(int buttonNumber)
{
    trace(QString("RunButtonsFrame::switchRunButton returnFrequency clear both"));
    runButtonMap[RUN_BUTTON_1_ON]->returnFrequency.clear();
    runButtonMap[RUN_BUTTON_2_ON]->returnFrequency.clear();

    // other run button frequency active, so switch over
    runButtonMap[buttonNumber]->showButtonOnOff(true);
    runButtonMap[otherButton(buttonNumber)]->showButtonOnOff(false);
    runButReadActSel(buttonNumber);

    // This will usually happen when the frequency comes back changed
    // BUT if the frequency is too close, that doesn't happen
    setRunButtonText(RUN_BUTTON_1_ON);
    setRunButtonText(RUN_BUTTON_2_ON);

    rmc.setRunMode(buttonNumber, getCtRunMemoryData(buttonNumber)
);
}

void RunButtonsFrame::runButActivated(int buttonNumber)
{
    // button pressed starts here
    trace(QString("RunButtonsFrame::runButActivated buttonNumber %1 radioOffRunFreq %2 runButtonOnNum %3")
          .arg(buttonNumber).arg(rmc.radioOffRunFreq).arg(rmc.runButtonOnNum));

    if (buttonNumber != RUN_BUTTON_1_ON && buttonNumber != RUN_BUTTON_2_ON)
        return;

    if (!runButtonMap[buttonNumber]->memButton->text().contains(QChar('*')))
    {
        // "*" in the text means that no frequency has been set (yet)
        // no freq set - press has no effect
        if (rmc.runButtonOnNum == NO_RUN_BUTTON_ON)
        {
            // neither run button active
            setRunFreq(buttonNumber);
        }
        else if (rmc.runButtonOnNum == buttonNumber)
        {
            if(rmc.radioOffRunFreq)
            {
                // a run button is active; but we are not on a run frequency
                // select run freq for this button
                setRunButtonActive(buttonNumber);
            }
            else
            {
                // on run Freq, turn off runmode
                runModeOff(buttonNumber);
            }
        }
        else
        {
            // other run button frequency active
            switchRunButton(buttonNumber);
        }
    }
    else
    {
        // not set - go to "new" action
        runButWriteActSel(buttonNumber);
    }
    rmc.chkRunFreq();

}
void RunButtonsFrame::runButReadActSel(int buttonNumber)
{
    trace(QString("RunButtonsFrame::runButReadActSel Run Button Read Selected = %1").arg(QString::number(buttonNumber + 1)));
    memoryData::memData m = getCtRunMemoryData(buttonNumber);
    if (LogContainer->sendDM->isRadioLoaded())
    {
        if (rigControl->radioConnected && !rigControl->radioError)
        {
            rmc.setRunMode(buttonNumber, m);
        }
        else if (!rigControl->radioConnected && rigControl->radioName.trimmed().isEmpty())
        {
            rigControl->noRadioSendOutFreq(m.freq);
        }
    }
}



void RunButtonsFrame::runButWriteActSel(int buttonNumber)
{
    trace(QString("RunButtonsFrame::runButWriteActSel Run Button Write Selected = %1").arg(QString::number(buttonNumber + 1)));
    memoryData::memData runData;
    runData.callsign = tr("Run") + QString::number(buttonNumber + 1);
    runData.freq = rigControl->getCurFreq();
    runData.locator = "";
    runData.mode = rigControl->curMode;
    runData.bearing = COMPASS_ERROR;
    runData.time = "00:00";
    runData.memno = buttonNumber;

    // load run data into run memory

    RunButtonDialog runDialog(this);
    runDialog.setWindowTitle(tr("Run %1 - Set").arg(QString::number(buttonNumber + 1)));
    runDialog.setLogData(&runData, buttonNumber, ct);
    if (runDialog.exec() == QDialog::Accepted)
    {
        trace(QString("RunButtonsFrame::runButWriteActSel frequency returned = %1").arg(runData.freq.traceStr()));
        setCtRunMemoryData(buttonNumber, runData);
        runButtonUpdate(buttonNumber);
        runButActivated(buttonNumber);  // simulate button press

    }

}

void RunButtonsFrame::runButEditActSel(int buttonNumber)
{
    memoryData::memData runData = getCtRunMemoryData(buttonNumber);

    trace(QString("RunButtonsFrame::runButEditActSel Run Button Edit Selected = %1").arg(QString::number(buttonNumber + 1)));
    RunButtonDialog runDialog(this);
    runDialog.setWindowTitle(tr("Run %1 - Edit").arg(QString::number(buttonNumber + 1)));
    runDialog.setLogData(&runData, buttonNumber, ct);

    if (runDialog.exec() == QDialog::Accepted)
    {
        setCtRunMemoryData(buttonNumber, runData);
        runButtonUpdate(buttonNumber);
    }
}

void RunButtonsFrame::runButClearActSel(int buttonNumber)
{
    trace(QString("RunButtonsFrame::runButClearActSel Run Button Clear Selected = %1").arg(QString::number(buttonNumber + 1)));

    runButOffActionSelected(buttonNumber);

    memoryData::memData mr = getCtRunMemoryData(buttonNumber);

    clearCtRunMemoryData(buttonNumber, mr);
    runButtonMap[buttonNumber]->returnFrequency.clear();
    runButtonUpdate(buttonNumber);
}


void RunButtonsFrame::runButOffActionSelected(int buttonNumber)
{
    if (buttonNumber == rmc.runButtonOnNum)
    {
        runButtonMap[buttonNumber]->showButtonOnOff(false);
        rmc.setRunModeOff();
    }

}

void RunButtonsFrame::loadRunButtonLabels()
{
    runButtonUpdate(RUN_BUTTON_1_ON);
    runButtonUpdate(RUN_BUTTON_2_ON);
}


void RunButtonsFrame::setRunButtonText(int buttonNumber)
{
    memoryData::memData m = getCtRunMemoryData(buttonNumber);
    QString sc = ((buttonNumber == 0)?QString(" [ "):QString( " ] "));

    QString runText = "R" + QString::number(buttonNumber + 1) + "(" + sc + ") " + "." + m.freq.extractKhz() + "   ";
    QString tTipStr = tr("Freq: ") + m.freq.convertFreqStrDisp() + "\n"
            + tr("Mode: ") + m.mode + "\n";
    QString restoreText;
    if (!runButtonMap[buttonNumber]->returnFrequency.isClear())
    {
        restoreText = "\n" + tr("Restore .%1    ").arg(runButtonMap[buttonNumber]->returnFrequency.extractKhz() );
        tTipStr += tr("Restore freq: ") + runButtonMap[buttonNumber]->returnFrequency.convertFreqStrDisp() + "\n";
    }
    runButtonMap[buttonNumber]->memButton->setText(runText + restoreText);

    runButtonMap[buttonNumber]->memButton->setToolTip(tTipStr);

    runButtonMap[buttonNumber]->memButton->repaint();

}

void RunButtonsFrame::runButtonUpdate(int buttonNumber)
{
    setRunButtonText(buttonNumber);

    if (buttonNumber == rmc.runButtonOnNum)
    {
        if (runButtonMap[buttonNumber]->memButton->text().contains(QChar('*')))
        {
            rmc.setRunModeOff();
        }
        else
        {
            rmc.setRunMode(buttonNumber, getCtRunMemoryData(buttonNumber));
        }
    }

}
void RunButtonsFrame::setFreqDisplay(Frequency f, bool legalFreq)
{
    bool legalOperatingFreqFlag;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpContestTurnOffOperatingFreqColorRadioDial, legalOperatingFreqFlag );

    QString freqCol;
    if (!legalFreq)
    {
        if (!legalOperatingFreqFlag)
        {
           freqCol = HtmlFontColour(Qt::red);
        }
    }
    ui->freqDisplay->setText(freqCol + f.convertFreqStrDisp());
}


memoryData::memData RunButtonsFrame::getCtRunMemoryData(int memoryNumber)
{
    memoryData::memData m;

    if (ct != nullptr)
    {
        if (ct->runMemories[ct->currentBand.getValue()].size() > memoryNumber)
        {
           m = ct->runMemories[ct->currentBand.getValue()][memoryNumber].getValue();

        }
    }

    return m;
}
void RunButtonsFrame::clearCtRunMemoryData(int memoryNumber, memoryData::memData m)
{
    ct->clearRunMemory(memoryNumber, m);

}
void RunButtonsFrame::setCtRunMemoryData(int memoryNumber, memoryData::memData m)
{
    ct->saveRunMemory(memoryNumber, m);

}
void RunButtonsFrame::setCallFreq()
{
    // triggered by clicking "run" radiobutton in QSOLogFrame
    // Intended to set run button at current frequency/mode
    // It chooses an unused button, or defaults to button 1

    // "*" in the text means that no frequency has been set (yet)
    // so choose the first with "*", or else button 1

    if (rmc.radioOffRunFreq)
    {
        int buttonNumber = rmc.runButtonOnNum;

        trace(QString("unButtonsFrame::setCallFreq b1 text %1 b2 text %2").arg(runButtonMap[RUN_BUTTON_1_ON]->memButton->text(), runButtonMap[RUN_BUTTON_2_ON]->memButton->text()));
        if (runButtonMap[RUN_BUTTON_1_ON]->memButton->text().contains(QChar('*')))
        {
            buttonNumber = RUN_BUTTON_1_ON;
        }
        else if (runButtonMap[RUN_BUTTON_2_ON]->memButton->text().contains(QChar('*')))
        {
            buttonNumber = RUN_BUTTON_2_ON;
        }
        if (buttonNumber == NO_RUN_BUTTON_ON)
        {
            buttonNumber = RUN_BUTTON_1_ON;
        }

        trace(QString("setCallFreq Selected = %1").arg(QString::number(buttonNumber + 1)));
        memoryData::memData runData;
        runData.callsign = tr("Run") + QString::number(buttonNumber + 1);
        runData.freq = rigControl->getCurFreq();
        runData.locator = "";
        runData.mode = rigControl->curMode;
        runData.bearing = COMPASS_ERROR;
        runData.time = "00:00";
        runData.memno = buttonNumber;

        // load run data into run memory

        setCtRunMemoryData(buttonNumber, runData);
        runButtonUpdate(buttonNumber);
        runButActivated(buttonNumber);  // simulate button press

    }
}


void RunButtonsFrame::logRadioSettingsChanged(QSharedPointer<RadioSettingsDialogChangeFlag> /*logRadioSettingsFlags*/)
{
    setRunButtonsFrameTitle();
}


void RunButtonsFrame::setRunButtonsFrameTitle()
{
    if (getRadioReadOnlyFlag())
    {
        setTitle("Run Frequencies - Read Only");
    }
    else
    {
        setTitle("Run Frequencies");
    }

}


bool RunButtonsFrame::getRadioReadOnlyFlag()
{

    bool state;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpContestRadioReadOnly, state );
    return state;
}


//*******************Run Memory Button *************************//


RunMemoryButton::RunMemoryButton(QToolButton *b, RunButtonsFrame *rcf, int no)
{
    memNo = no;
    runButtonsFrame = rcf;

    memButton = b;

    memoryMenu = new QMenu(memButton);

    memButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    memButton->setPopupMode(QToolButton::MenuButtonPopup);
    memButton->setFocusPolicy(Qt::NoFocus);
    memButton->setText(runButData::runButTitle[memNo]);

    shortKey = new QShortcut(QKeySequence(runButShortCut[memNo]), memButton);
    shiftShortKey = new QShortcut(QKeySequence(runButShiftShortCut[memNo]), memButton);

    runOffAction = new QAction(tr("&Off"), memButton);
    memoryMenu->addAction(runOffAction);
    connect( runOffAction, &QAction::triggered, this, &RunMemoryButton::runOffActionSelected);

//    readAction = new QAction(tr("&Read"), memButton);
//    memoryMenu->addAction(readAction);
//    connect( readAction, &QAction::triggered, this, &RunMemoryButton::buttonSelected);

//    writeAction = new QAction(tr("&New"),memButton);
    writeAction = new QAction(tr("&Set"),memButton);
    memoryMenu->addAction(writeAction);
    connect( writeAction, &QAction::triggered, this, &RunMemoryButton::writeActionSelected);

//    editAction = new QAction(tr("&Edit"), memButton);
//    memoryMenu->addAction(editAction);
//    connect( editAction, &QAction::triggered, this, &RunMemoryButton::editActionSelected);

    clearAction = new QAction(tr("&Clear"),memButton);
    memoryMenu->addAction(clearAction);
    connect( clearAction, &QAction::triggered, this, &RunMemoryButton::clearActionSelected);

    memButton->setMenu(memoryMenu);

    connect(memButton, &QToolButton::clicked, this, &RunMemoryButton::buttonSelected);
    connect(shortKey, &QShortcut::activated, this, &RunMemoryButton::buttonSelected);
    connect(shiftShortKey, &QShortcut::activated, this, &RunMemoryButton::memoryShortCutSelected);

}
RunMemoryButton::~RunMemoryButton()
{

}

void RunMemoryButton::memoryShortCutSelected()
{
    memButton->showMenu();
}
void RunMemoryButton::readActionSelected()
{
    runButtonsFrame->runButReadActSel(memNo);
}
void RunMemoryButton::editActionSelected()
{
    runButtonsFrame->runButEditActSel(memNo);
}
void RunMemoryButton::writeActionSelected()
{
    runButtonsFrame->runButWriteActSel(memNo);
}
void RunMemoryButton::clearActionSelected()
{
    emit clearActionActivated(memNo);
}

void RunMemoryButton::runOffActionSelected()
{
    runButtonsFrame->runButOffActionSelected(memNo);
}

void RunMemoryButton::buttonSelected()
{
    emit buttonActivated(memNo);
}

void RunMemoryButton::showButtonOnOff(bool state)
{
    if (state)
    {
        showRunToolButtonOnFreq();
    }
    else
    {
        memButton->setStyleSheet(RUN_BUTTON_OFF_STYLE);
    }
}


void RunMemoryButton::showRunToolButtonOffFreq()
{
    memButton->setStyleSheet(RUN_BUTTON_OFF_FREQ_STYLE);
    //memoryMenu->setStyleSheet(RUN_BUTTON_OFF_STYLE);
}

void RunMemoryButton::showRunToolButtonOnFreq()
{
    memButton->setStyleSheet(RUN_BUTTON_ON_FREQ_STYLE);
    // memoryMenu->setStyleSheet(RUN_BUTTON_OFF_STYLE);
}
