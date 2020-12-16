#include "LoggerContest.h"
#include "rotatorcommon.h"
#include "runbuttondialog.h"
#include "rigutils.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"

#include "runbuttonsframe.h"
#include "ui_runbuttonsframe.h"

const int RUN_TOLERANCE = 300; // Hz

const QString RUN_BUTTON_ON_FREQ_STYLE = QString("background-color: orange ; color:black ; border-style: outset; border-width: 1px; border-color: black;\n");
const QString RUN_BUTTON_OFF_FREQ_STYLE = QString("background-color: yellow ; color:black ; border-style: outset; border-width: 1px; border-color: black;\n");
const QString RUN_BUTTON_OFF_STYLE = QString("background-color: Gainsboro ; color:black ; border-style: outset; border-width: 1px; border-color: black;\n");

const int NO_RUN_BUTTON_ON = -1;
const int RUN_BUTTON_1_ON = 0;
const int RUN_BUTTON_2_ON = 1;

const int CHECK_RUN_FREQ_POLLTIME = 1000;

static QKeySequence runButShortCut[] {
    QKeySequence(Qt::CTRL + Qt::Key_BracketLeft),
    QKeySequence(Qt::CTRL + Qt::Key_BracketRight)

};

static QKeySequence runButShiftShortCut[] {
    QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_BracketLeft),
    QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_BracketRight)

};

RunButtonsFrame::RunButtonsFrame(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::RunButtonsFrame),
    runButtonOnNum(NO_RUN_BUTTON_ON)
{
    ui->setupUi(this);

    initRunMemoryButton();

    chkRunFreqTimer = new QTimer(this);
    connect(chkRunFreqTimer, SIGNAL(timeout()), this, SLOT(on_radioFreqCheckTimer()));
}

RunButtonsFrame::~RunButtonsFrame()
{
    delete ui;

    for(auto const &b: runButtonMap)
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

void RunButtonsFrame::setRigControl(RigControlFrame *rc)
{
    rigControl = rc;
    // This shouldn't need to be a unique connection
    connect(rc, SIGNAL(setFreqDisplay(Frequency, bool)), this, SLOT(setFreqDisplay(Frequency, bool)), Qt::UniqueConnection);
    connect(rc, SIGNAL(radioIsConnected(bool)), this, SLOT(radioIsConnected(bool)));

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
    curRadioFreq = freq;
    // rig frequency changed signal
    chkRunFreq();

    if (!chkRadioFreqOnRunFreq())
    {
        runButtonMap[RUN_BUTTON_1_ON]->returnFrequency.clear();
        runButtonMap[RUN_BUTTON_2_ON]->returnFrequency.clear();
    }
    setRunButtonText(RUN_BUTTON_1_ON);
    setRunButtonText(RUN_BUTTON_2_ON);
}
//********************** Run Buttons *******************************

void RunButtonsFrame::initRunMemoryButton()
{
    memoryData::memData m;
    runButtonMap[0] = new RunMemoryButton(ui->RunButton1, this, 0);
    connect( runButtonMap[0], SIGNAL( clearActionSelected(int)) , this, SLOT(runButClearActSel(int)), Qt::QueuedConnection );
    connect( runButtonMap[0], SIGNAL( buttonActivated(int)) , this, SLOT(runButActivated(int)), Qt::QueuedConnection );

    runButtonMap[1] = new RunMemoryButton(ui->RunButton2, this, 1);
    connect( runButtonMap[1], SIGNAL( clearActionSelected(int)) , this, SLOT(runButClearActSel(int)), Qt::QueuedConnection );
    connect( runButtonMap[1], SIGNAL( buttonActivated(int)) , this, SLOT(runButActivated(int)), Qt::QueuedConnection );

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

    memoryData::memData m = getRunMemoryData(buttonNumber);
    Frequency curFreq = rigControl->getCurFreq();

    if (m.freq == curFreq)
    {
        runButtonMap[buttonNumber]->returnFrequency.clear();
    }
    else
    {
        runButtonMap[buttonNumber]->returnFrequency = curFreq;
    }
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

    if (rigControl->getCurFreq() != oldfreq)
    {
        runButtonMap[buttonNumber]->returnFrequency = oldfreq;
        runButtonMap[otherButton(buttonNumber)]->returnFrequency.clear();
    }
    else
    {
        runButtonMap[RUN_BUTTON_1_ON]->returnFrequency.clear();
        runButtonMap[RUN_BUTTON_2_ON]->returnFrequency.clear();
    }

    runButtonMap[buttonNumber]->showButtonOnOff(true);
    oldRadioOffRunFreq = false;
    runButtonOnNum = buttonNumber;

    chkRunFreqTimer->setInterval(CHECK_RUN_FREQ_POLLTIME);
    chkRunFreqTimer->start();

    emit sendRunOnFlag(curRunFreq, true);
}

void RunButtonsFrame::runModeOff(int buttonNumber)
{
    // on run freq - return to prior frequency

    Frequency rfreq = runButtonMap[buttonNumber]->returnFrequency;

    if (!rfreq.isClear())
    {
        // go to restore freq
        runButtonMap[buttonNumber]->returnFrequency.clear();
        runButtonMap[otherButton(buttonNumber)]->returnFrequency.clear();
        setRunButtonText(buttonNumber);


        rigControl->sendRigFreq( rfreq);
        curRadioFreq = rfreq;
        chkRunFreq();
        chkRunFreqTimer->setInterval(CHECK_RUN_FREQ_POLLTIME);  // reset the interval

        // on run Freq, gone to restore freq
       // runButtonMap[buttonNumber]->showButtonOnOff(true);
    }
    else
    {
        runButOffActionSelected(buttonNumber);
    }


//    runButtonOnFlag = false;
//    oldRadioOffRunFreq = false;
//    runButtonOnNum = NO_RUN_BUTTON_ON;
//    chkRunFreqTimer->stop();
//    emit sendRunOnFlag(curRunFreq, runButtonOnFlag);
}

void RunButtonsFrame::switchRunButton(int buttonNumber)
{
    if (radioOffRunFreq)
    {
        runButtonMap[buttonNumber]->returnFrequency = rigControl->getCurFreq();
        runButtonMap[otherButton(buttonNumber)]->returnFrequency.clear();
    }
    else
    {
        runButtonMap[RUN_BUTTON_1_ON]->returnFrequency.clear();
        runButtonMap[RUN_BUTTON_2_ON]->returnFrequency.clear();
    }
    // other run button frequency active, so switch over
    runButtonMap[buttonNumber]->showButtonOnOff(true);
    runButtonMap[otherButton(buttonNumber)]->showButtonOnOff(false);
    runButReadActSel(buttonNumber);

    // This will usually happen when the frequency comes back changed
    // BUT if the frequency is too close, that doesn't happen
    setRunButtonText(RUN_BUTTON_1_ON);
    setRunButtonText(RUN_BUTTON_2_ON);

    chkRunFreqTimer->start(CHECK_RUN_FREQ_POLLTIME);
    runButtonOnNum = buttonNumber;
    emit sendRunOnFlag(curRunFreq, true);

    chkRunFreq();
}

void RunButtonsFrame::runButActivated(int buttonNumber)
{
    // button pressed starts here
    trace(QString("RunButtonsFrame::runButActivated buttonNumber %1 radioOffRunFreq %2 runButtonOnNum %3")
          .arg(buttonNumber).arg(radioOffRunFreq).arg(runButtonOnNum));

    if (buttonNumber != RUN_BUTTON_1_ON && buttonNumber != RUN_BUTTON_2_ON)
        return;

    if (!runButtonMap[buttonNumber]->memButton->text().contains(QChar('*')))
    {
        // "*" in the text means that no frequency has been set (yet)
        // no freq set - press has no effect
        if (runButtonOnNum == NO_RUN_BUTTON_ON)
        {
            // neither run button active
            setRunFreq(buttonNumber);
        }
        else if (runButtonOnNum == buttonNumber)
        {
            if(radioOffRunFreq)
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
}
void RunButtonsFrame::runButReadActSel(int buttonNumber)
{
    trace(QString("Run Button Read Selected = %1").arg(QString::number(buttonNumber + 1)));
    memoryData::memData m = getRunMemoryData(buttonNumber);
    if (rigControl->isRadioLoaded())
    {
        if (rigControl->radioConnected && !rigControl->radioError)
        {
            rigControl->clearFreqInputFocus();
            if (m.freq != rigControl->getCurFreq())
            {
                rigControl->sendRigFreq(m.freq);
                 // pre-empt us being told; if it doesn't happen, we will get told later
                curRadioFreq = m.freq;
                TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
                tslf->sCurFreq = m.freq;
                chkRunFreqTimer->setInterval(CHECK_RUN_FREQ_POLLTIME);  // reset the interval

            }

            curRunFreq = m.freq;

            if (m.mode != rigControl->curMode)
            {
                rigControl->sendModeToRadio(m.mode);
            }

        }
        else if (!rigControl->radioConnected && rigControl->radioName.trimmed().isEmpty())
        {
            rigControl->noRadioSendOutFreq(m.freq);
        }
    }
}



void RunButtonsFrame::runButWriteActSel(int buttonNumber)
{
    trace(QString("Memory Write Selected %1 = ").arg(QString::number(buttonNumber + 1)));
    memoryData::memData runData;
    runData.callsign = tr("Run") + QString::number(buttonNumber + 1);
    runData.freq = rigControl->getCurFreq();
    runData.locator = "";
    runData.mode = rigControl->curMode;
    runData.bearing = COMPASS_ERROR;
    runData.time = "00:00";
    // load run data into run memory

    RunButtonDialog runDialog(this);
    runDialog.setWindowTitle(tr("Run %1 - New").arg(QString::number(buttonNumber + 1)));
    runDialog.setLogData(&runData, buttonNumber);
    if (runDialog.exec() == QDialog::Accepted)
    {
        setRunMemoryData(buttonNumber, runData);
        runButtonUpdate(buttonNumber);
        runButActivated(buttonNumber);  // simulate button press

    }

}

void RunButtonsFrame::runButEditActSel(int buttonNumber)
{
    memoryData::memData runData = getRunMemoryData(buttonNumber);

    trace(QString("Run Button Edit Selected = %1").arg(QString::number(buttonNumber + 1)));
    RunButtonDialog runDialog(this);
    runDialog.setWindowTitle(tr("Run %1 - Edit").arg(QString::number(buttonNumber + 1)));
    runDialog.setLogData(&runData, buttonNumber);

    if (runDialog.exec() == QDialog::Accepted)
    {
        setRunMemoryData(buttonNumber, runData);
        runButtonUpdate(buttonNumber);
    }
}

void RunButtonsFrame::runButClearActSel(int buttonNumber)
{
    trace(QString("Run Button Clear Selected = %1").arg(QString::number(buttonNumber + 1)));

    memoryData::memData m;
    setRunMemoryData(buttonNumber, m);
    runButtonMap[buttonNumber]->returnFrequency.clear();
    runButtonUpdate(buttonNumber);
}


void RunButtonsFrame::runButOffActionSelected(int buttonNumber)
{
    if (buttonNumber == runButtonOnNum)
    {
        runButtonMap[buttonNumber]->showButtonOnOff(false);
        runButtonOnNum = NO_RUN_BUTTON_ON;
        chkRunFreqTimer->stop();

        emit sendRunOnFlag(curRunFreq, false);
    }

}

void RunButtonsFrame::loadRunButtonLabels()
{
    runButtonUpdate(RUN_BUTTON_1_ON);
    runButtonUpdate(RUN_BUTTON_2_ON);
}


void RunButtonsFrame::setRunButtonText(int buttonNumber)
{
    memoryData::memData m = getRunMemoryData(buttonNumber);
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
}

void RunButtonsFrame::runButtonUpdate(int buttonNumber)
{
    setRunButtonText(buttonNumber);

    if (buttonNumber == runButtonOnNum)
    {
        if (runButtonMap[buttonNumber]->memButton->text().contains(QChar('*')))
        {
            // cleared the active run freq - turn run off
            runButtonMap[buttonNumber]->showButtonOnOff(false);
            runButtonOnNum = NO_RUN_BUTTON_ON;
            chkRunFreqTimer->stop();
            emit sendRunOnFlag(curRunFreq, false);
        }
        else
        {
            // update run freq
            memoryData::memData m = getRunMemoryData(buttonNumber);
            if (m.freq != rigControl->getCurFreq())
            {
                rigControl->sendRigFreq(m.freq);
                // pre-empt us being told; if it doesn't happen, we will get told later
                curRadioFreq = m.freq;
                chkRunFreqTimer->setInterval(CHECK_RUN_FREQ_POLLTIME);  // reset the interval
            }
            curRunFreq = m.freq;
            emit sendRunOnFlag(curRunFreq, true);
        }
    }

}
void RunButtonsFrame::setFreqDisplay(Frequency f, bool legalFreq)
{
    QString freqCol;
    if (!legalFreq)
    {
        freqCol = HtmlFontColour(Qt::red);
    }
    ui->freqDisplay->setText(freqCol + f.convertFreqStrDisp());
}

void RunButtonsFrame::on_radioFreqCheckTimer()
{
    curRadioFreq = rigControl->getCurFreq();
    chkRunFreq();
}
void RunButtonsFrame::chkRunFreq()
{
    if (runButtonOnNum >= 0)
    {
        if (!curRunFreq.isClear())
        {
            radioOffRunFreq = !chkRadioFreqOnRunFreq();

            if (radioOffRunFreq)
            {
                runButtonMap[runButtonOnNum]->showRunToolButtonOffFreq();
            }
            else
            {
                runButtonMap[runButtonOnNum]->showRunToolButtonOnFreq();
            }

            if (oldRadioOffRunFreq != radioOffRunFreq)
            {
                oldRadioOffRunFreq = radioOffRunFreq;
                emit sendRunOffFreqFlag(curRunFreq, radioOffRunFreq);
            }
        }
    }
}

bool RunButtonsFrame::chkRadioFreqOnRunFreq()
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

memoryData::memData RunButtonsFrame::getRunMemoryData(int memoryNumber)
{
    memoryData::memData m;

    if (ct != nullptr)
    {
        if (ct->runMemories.size() > memoryNumber)
        {
           m = ct->runMemories[memoryNumber].getValue();

        }
    }

    return m;
}
void RunButtonsFrame::setRunMemoryData(int memoryNumber, memoryData::memData m)
{
    ct->saveRunMemory(memoryNumber, m);

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
    readAction = new QAction(tr("&Read"), memButton);
    writeAction = new QAction(tr("&New"),memButton);
    editAction = new QAction(tr("&Edit"), memButton);
    clearAction = new QAction(tr("&Clear"),memButton);
    memoryMenu->addAction(runOffAction);
    memoryMenu->addAction(readAction);
    memoryMenu->addAction(writeAction);
    memoryMenu->addAction(editAction);
    memoryMenu->addAction(clearAction);
    memButton->setMenu(memoryMenu);

    //connect(shortKey, SIGNAL(activated()), this, SLOT(readActionSelected()));
    //connect( readAction, SIGNAL( triggered() ), this, SLOT(readActionSelected()) );
    //connect(memButton, SIGNAL(clicked(bool)), this, SLOT(readActionSelected()));
    connect( readAction, SIGNAL( triggered() ), this, SLOT(buttonSelected()) );
    connect(memButton, SIGNAL(clicked(bool)), this, SLOT(buttonSelected()));
    connect(shortKey, SIGNAL(activated()), this, SLOT(buttonSelected()));
    connect(shiftShortKey, SIGNAL(activated()), this, SLOT(memoryShortCutSelected()));
    connect( writeAction, SIGNAL( triggered() ), this, SLOT(writeActionSelected()) );
    connect( editAction, SIGNAL( triggered() ), this, SLOT(editActionSelected()) );
    connect( clearAction, SIGNAL( triggered() ), this, SLOT(clearActionSelected()) );
    connect( runOffAction, SIGNAL( triggered() ), this, SLOT(runOffActionSelected()) );

}
RunMemoryButton::~RunMemoryButton()
{

}

void RunMemoryButton::memoryShortCutSelected()
{
//    rigControlFrame->memoryShortCutSelected(memNo);
    memButton->showMenu();
    //emit lostFocus();
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
    emit clearActionSelected(memNo);
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
