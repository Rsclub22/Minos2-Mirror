#include "LoggerContest.h"
#include "runbuttonsframe.h"
#include "rotatorcommon.h"
#include "runbuttondialog.h"
#include "rigutils.h"
#include "ui_runbuttonsframe.h"

const int RUN_TOLERANCE = 5; //run freq tolerance = 500Hz

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
    runButtonOnFlag(false),
    radioOffRunFreq(false)
{
    ui->setupUi(this);

    initRunMemoryButton();

    chkRunFreqTimer = new QTimer(this);
    connect(chkRunFreqTimer, SIGNAL(timeout()), this, SLOT(on_ChkRunFreq()));
}

RunButtonsFrame::~RunButtonsFrame()
{
    delete ui;

    foreach(auto b, runButtonMap)
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
    connect(rc, SIGNAL(setFreqDisplay(QString, bool)), this, SLOT(setFreqDisplay(QString, bool)), Qt::UniqueConnection);
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
void RunButtonsFrame::setFreq(QString /*freq*/)
{
    on_ChkRunFreq();
    setRunButtonText(RUN_BUTTON_1_ON);
    setRunButtonText(RUN_BUTTON_2_ON);

    if (!chkRadioFreqOnRunFreq())
    {
        runButtonMap[RUN_BUTTON_1_ON]->returnFrequency.clear();
        runButtonMap[RUN_BUTTON_2_ON]->returnFrequency.clear();
    }
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

    runButtonMap[buttonNumber]->returnFrequency = rigControl->getCurFreq();
    runButtonMap[otherButton(buttonNumber)]->returnFrequency.clear();

    runButReadActSel(buttonNumber);
    runButtonMap[buttonNumber]->showButtonOnOff(false);
}

void RunButtonsFrame::setRunFreq(int buttonNumber)
{
    // run button not active
    // make this one active

    QString oldfreq = rigControl->getCurFreq();

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
    runButtonOnFlag = true;
    oldRadioOffRunFreq = false;
    chkRunFreqTimer->start(CHECK_RUN_FREQ_POLLTIME);
    runButtonOnNum = buttonNumber;
    emit sendRunOnFlag(curRunFreq, runButtonOnFlag);
}

void RunButtonsFrame::runModeOff(int buttonNumber)
{
    // on run freq - return to prior frequency

    QString rfreq = runButtonMap[buttonNumber]->returnFrequency;

    if (!rfreq.isEmpty())
    {
        // go to restore freq
        runButtonMap[buttonNumber]->returnFrequency.clear();
        runButtonMap[otherButton(buttonNumber)]->returnFrequency.clear();

        rigControl->sendFreq( rfreq);

        // on run Freq, turn off runmode
        runButtonMap[buttonNumber]->showButtonOnOff(false);
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
    runButtonMap[RUN_BUTTON_1_ON]->showButtonOnOff(false);
    runButtonMap[RUN_BUTTON_2_ON]->showButtonOnOff(false);
    runButReadActSel(buttonNumber);
    runButtonOnFlag = true;
    chkRunFreqTimer->start(CHECK_RUN_FREQ_POLLTIME);
    runButtonOnNum = buttonNumber;
    emit sendRunOnFlag(curRunFreq, runButtonOnFlag);
}

void RunButtonsFrame::runButActivated(int buttonNumber)
{
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
            // NO_RUN_BUTTON_ON
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
            if (m.freq.remove('.') != rigControl->getCurFreq().remove('.'))
            {
                rigControl->sendFreq(m.freq);
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
        runButActivated(buttonNumber);

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
    if (buttonNumber == runButtonOnNum && runButtonOnFlag)
    {
        runButtonMap[buttonNumber]->showButtonOnOff(false);
        runButtonOnFlag = false;
        runButtonOnNum = NO_RUN_BUTTON_ON;
        chkRunFreqTimer->stop();

        emit sendRunOnFlag(curRunFreq, runButtonOnFlag);
    }

}

void RunButtonsFrame::loadRunButtonLabels()
{
    for (int i = 0; i < runButData::NUM_RUNBUTTONS; i++)
    {
        runButtonUpdate(i);
    }
}


void RunButtonsFrame::setRunButtonText(int buttonNumber)
{
    memoryData::memData m = getRunMemoryData(buttonNumber);
    QString sc = ((buttonNumber == 0)?QString(" [ "):QString( " ] "));

    QString runText = "R" + QString::number(buttonNumber + 1) + "(" + sc + ") " + "." + extractKhz(m.freq) + "   ";
    QString restoreText;
    if (!runButtonMap[buttonNumber]->returnFrequency.isEmpty())
    {
        restoreText = "\n" + tr("Restore .%1    ").arg(extractKhz(runButtonMap[buttonNumber]->returnFrequency) );
    }
    runButtonMap[buttonNumber]->memButton->setText(runText + restoreText);
    QString tTipStr = tr("Freq: ") + convertFreqStrDisp(m.freq) + "\n"
            + tr("Mode: ") + m.mode + "\n";
    runButtonMap[buttonNumber]->memButton->setToolTip(tTipStr);
}

void RunButtonsFrame::runButtonUpdate(int buttonNumber)
{
    setRunButtonText(buttonNumber);

    if (buttonNumber == runButtonOnNum && runButtonOnFlag)
    {
        if (runButtonMap[buttonNumber]->memButton->text().contains(QChar('*')))
        {
            // cleared the active run freq - turn run off
            runButtonMap[buttonNumber]->showButtonOnOff(false);
            runButtonOnFlag = false;
            runButtonOnNum = NO_RUN_BUTTON_ON;
            chkRunFreqTimer->stop();
            emit sendRunOnFlag(curRunFreq, runButtonOnFlag);
        }
        else
        {
            // update run freq
            memoryData::memData m = getRunMemoryData(buttonNumber);
            if (m.freq.remove('.') != rigControl->getCurFreq().remove('.'))
            {
                rigControl->sendFreq(m.freq);
            }
            curRunFreq = m.freq.remove('.');
            emit sendRunOnFlag(curRunFreq, runButtonOnFlag);
        }
    }

}
void RunButtonsFrame::setFreqDisplay(QString f, bool legalFreq)
{
    QString freq;
    if (!legalFreq)
    {
        freq = HtmlFontColour(Qt::red);
    }
    ui->freqDisplay->setText(freq + convertFreqStrDisp(f));
}

void RunButtonsFrame::on_ChkRunFreq()
{
    if (runButtonOnFlag)
    {
        if (curRunFreq.toLongLong() != 0)
        {
            if (!chkRadioFreqOnRunFreq())
            {
                radioOffRunFreq = true;
                if (runButtonOnNum >= 0 && runButtonOnNum < NUM_RUNBUTTONS)
                {
                    runButtonMap[runButtonOnNum]->showRunToolButtonOffFreq();
                }

                if (oldRadioOffRunFreq != radioOffRunFreq)
                {
                    oldRadioOffRunFreq = radioOffRunFreq;
                    emit sendRunOffFreqFlag(curRunFreq, radioOffRunFreq);
                }
             }

            else if (chkRadioFreqOnRunFreq())
            {
                // back on a run freq

                radioOffRunFreq = false;
                if (runButtonOnNum >= 0 && runButtonOnNum < NUM_RUNBUTTONS)
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
}

bool RunButtonsFrame::chkRadioFreqOnRunFreq()
{
    qint64 curRunF = curRunFreq.toLongLong() / 100;
    qint64 curF = rigControl->getCurFreq().toLongLong() / 100;

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
    rigControlFrame = rcf;

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
//    delete memButton;
}

void RunMemoryButton::memoryShortCutSelected()
{
//    rigControlFrame->memoryShortCutSelected(memNo);
    memButton->showMenu();
    //emit lostFocus();
}
void RunMemoryButton::readActionSelected()
{
    rigControlFrame->runButReadActSel(memNo);
}
void RunMemoryButton::editActionSelected()
{
    rigControlFrame->runButEditActSel(memNo);
}
void RunMemoryButton::writeActionSelected()
{
    rigControlFrame->runButWriteActSel(memNo);
}
void RunMemoryButton::clearActionSelected()
{
    emit clearActionSelected(memNo);
}

void RunMemoryButton::runOffActionSelected()
{
    rigControlFrame->runButOffActionSelected(memNo);
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
