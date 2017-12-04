/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2017
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////

#include <QSignalMapper>

#include "logger_pch.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "SendRPCDM.h"
#include "runbuttondialog.h"
#include "BandList.h"
#include "rigutils.h"
#include "rigcontrolframe.h"
#include "ui_rigcontrolframe.h"

#define MODE_ERROR "<font color='Red'>Mode Error</font>"


static QKeySequence runButShortCut[] {
    QKeySequence(Qt::CTRL + Qt::Key_BracketLeft),
    QKeySequence(Qt::CTRL + Qt::Key_BracketRight)

};

static QKeySequence runButShiftShortCut[] {
    QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_BracketLeft),
    QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_BracketRight)

};



RigControlFrame::RigControlFrame(QWidget *parent):
    QFrame(parent)
    , ui(new Ui::RigControlFrame)
    , curFreq(memDefData::DEFAULT_FREQ)
    , curMode(memDefData::DEFAULT_MODE)
    , radioName("NoRadio")
    , radioState("None")
    , radioLoaded(false)
    , freqEditOn(false)
    , radioConnected(false)
    , radioError(false)


{

    ui->setupUi(this);

    ui->freqInput->installEventFilter(this  );
    initRigFrame(parent);

    initRunMemoryButton();



    mgmLabelVisible(false);

    ui->txvertStat->setVisible(false);
    ui->TxVertLabel->setVisible(false);

    // init memory button data before radio connection
    setRadioName(radioName);

    freqEditShortKey = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F), parent);
    connect(freqEditShortKey, SIGNAL(activated()), this, SLOT(freqEditSelected()));

    freqPlusShortCut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_U), parent);
    connect(freqPlusShortCut, SIGNAL(activated()), this, SLOT(freqPlus_ShortCut()));
    connect(ui->freqUp, SIGNAL(clicked(bool)), this, SLOT(freqPlusShortCut_clicked(bool)));

    freqNegShortCut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_D), parent);
    connect(freqNegShortCut, SIGNAL(activated()), this, SLOT(freqNeg_ShortCut()));
    connect(ui->freqDown, SIGNAL(clicked(bool)), this, SLOT(freqNegShortCut_clicked(bool)));


    connect(&MinosLoggerEvents::mle, SIGNAL(FontChanged()), this, SLOT(on_FontChanged()), Qt::QueuedConnection);

    on_FontChanged();
    traceMsg(QString("RigControlFrame Started"));
}

RigControlFrame::~RigControlFrame()
{
    delete ui;

}
void RigControlFrame::on_FontChanged()
{
    QFont cf = QApplication::font();
    qreal fs = cf.pointSizeF();
    int fsi = fs * 1.25;
    cf.setPointSize(fsi);
    cf.setBold(true);
    ui->freqInput->setFont(cf);
}

void RigControlFrame::setContest(BaseContestLog *c)
{
    ct = dynamic_cast<LoggerContestLog *>( c);

    if (ct)
    {
        loadMemories();                 // mem and run should be independant of the radio!
    }
}

void RigControlFrame::initRigFrame(QWidget * /*parent*/)
{

    ui->modelbl->setText(" ");

    //connect(ui->freqInput, SIGNAL(lostFocus()), this, SLOT(exitFreqEdit()));
    connect(ui->freqInput, SIGNAL(freqEditReturn()), this, SLOT(returnChangeRadioFreq()));
    connect(ui->freqInput, SIGNAL(newFreq()), this, SLOT(changeRadioFreq()));

    // when no radio is connected
    connect(this, SIGNAL(noRadioSendFreq(QString)), this, SLOT(noRadioSetFreq(QString)));
    connect(this, SIGNAL(noRadioSendMode(QString)), this, SLOT(noRadioSetMode(QString)));

    for (int i = 0; i < bandSelData::bandNames.count(); i++)
    {
        ui->bandSelCombo->addItem(bandSelData::bandNames[i]);
    }

    //connect(ui->bandSelCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(radioBandFreq(int)));
    connect(ui->bandSelCombo, SIGNAL(activated(int)), this, SLOT(radioBandFreq(int)));



    if (!isRadioLoaded())
    {
        ui->modelbl->setVisible(false);
    }
}



void RigControlFrame::setRadioLoaded()
{
    traceMsg(QString("Set Radio Loaded"));
    radioLoaded = true;
    ui->modelbl->setVisible(true);
}

bool RigControlFrame::isRadioLoaded()
{
    return radioLoaded;
}

void RigControlFrame::noRadioSetFreq(QString f)
{
    traceMsg(QString("No Radio SetFreq = %1").arg(f));
    setFreq(f);
}

void RigControlFrame::setFreq(QString f)
{
    traceMsg(QString("Set Freq = %1").arg(f));
    QString freq = f;

    if (freq.count() >= 4)
    {
        if (!freqEditOn)
        {
            ui->freqInput->setInputMask(maskData::freqMask[freq.count() - 4]);
            ui->freqInput->setText(freq);
        }
        curFreq = freq;

    }
    // an error here

}


void RigControlFrame::changeRadioFreq()
{
    traceMsg(QString("Change Radio Freq"));

    QString newfreq = ui->freqInput->text().trimmed().remove('.');
    double f = convertStrToFreq(newfreq);
    if (f != -1.0)
    {
        if (f != 0)
        {
            newfreq.remove( QRegExp("^[0]*")); //remove periods and leading zeros
        }

        if (newfreq != lastFreq)
        {
            lastFreq = newfreq;
            if (checkValidFreq(lastFreq))
            {
                if (lastFreq.count() >=4)
                {
                    if (isRadioLoaded())
                    {
                        if (radioConnected && !radioError)
                        {
                            sendFreq(lastFreq);
                        }
                        else if (!radioConnected && radioName.trimmed() == "No Radio")
                        {
                            noRadioSendOutFreq(lastFreq);
                        }
                    }

                }

            }
            else
            {
                ui->freqInput->setText(QString("%1 %2 %3").arg("<font color='Red'>").arg(lastFreq).arg("</font>"));
            }
        }
    }
}


bool RigControlFrame::checkValidFreq(QString freq)
{
    bool ok = false;
    BandList &blist = BandList::getBandList();
    BandInfo bi;
    bool bandOK = false;
    QString sfreq = freq.trimmed();

    double dfreq = sfreq.toDouble(&ok);

    if (ok)
    {
        bandOK = blist.findBand(dfreq, bi);
    }
    return bandOK;
}

void RigControlFrame::returnChangeRadioFreq()
{
    changeRadioFreq();
    exitFreqEdit();
}

void RigControlFrame::radioBandFreq(int index)
{
    traceMsg(QString("Radio Band Freq Index = %1").arg(QString::number(index)));
    QString f = bandSelData::bandFreq[index];
    if (index > 0 && index < bandSelData::bandFreq.count())
    {

        if (f != curFreq)
        {
            //ui->freqInput->setInputMask(maskData::freqMask[bandSelData::bandMaskIdx[index]]);
            //ui->freqInput->setText(bandSelData::freqDialZero[index]);
            if (isRadioLoaded())
            {
                if (radioConnected && !radioError)
                {
                    sendFreq(f);
                }
                else if (!radioConnected && radioName.trimmed() == "No Radio")
                {
                     noRadioSendOutFreq(f);
                }

            }
       }

    }
    traceMsg(QString("Freq the same or index out of range"));
}

void RigControlFrame::sendFreq(QString f)
{


    bool ok = false;

    double df = f.toDouble(&ok);
    if (ok && df > 0.0)
    {

     emit sendFreqControl(f);
    }
}




void RigControlFrame::noRadioSendOutFreq(QString f)
{
    traceMsg(QString("No Radio Send Freq to rigcontrolframe and qsologframe = %1").arg(f));
    // update rigframe
    emit noRadioSendFreq(f);
    // update logger
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    tslf->on_NoRadioSetFreq(f);
}


void RigControlFrame::noRadioSendOutMode(QString m)
{
    traceMsg(QString("No Radio Send Mode to to rigcontrolframe and qsologframe = %1").arg(m));
    QString mode = m + ": "; //create mode message, mgm mode is space
    // update rigframe
    emit noRadioSendMode(mode);
    // update logger
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    tslf->on_NoRadioSetMode(mode);

}

void RigControlFrame::on_ContestPageChanged(QString freq, QString mode)
{
    QStringList modelist = mode.split(':');  // unpack mode
    QString sMode;

    if (modelist.count() != 2)
    {
        return;
    }
    //if (modelist[0] == hamlibData::MGM)
    //{
    //    sMode = modelist[1];
    //}
    //else
   // {
        sMode = modelist[0];
   // }
    // contest paged changed - send freq and mode to synch
    if (curFreq == memDefData::DEFAULT_FREQ)
    {
        sendFreq(freq);
    }
    else if (curFreq != freq)
    {
        sendFreq(curFreq);
    }

    if (curMode == memDefData::DEFAULT_MODE && curMode != sMode)
    {
        sendModeToRadio(curMode);
    }
    else if (curMode != sMode)
    {
        sendModeToRadio(curMode);
    }



}

bool RigControlFrame::eventFilter(QObject *obj, QEvent *event)
{
   Q_UNUSED(obj)


   if (event->type() == QEvent::FocusIn)
      freqLineEditInFocus();
   else if (event->type() == QEvent::FocusOut)
      exitFreqEdit();

   return false;
}

void RigControlFrame::exitFreqEdit()
{
    traceMsg(QString("Exit Edit Freq"));
    freqEditOn = false;
    setFreq(curFreq);
    freqLineEditFrameColour(false);
    ui->freqInput->clearFocus();
}




void RigControlFrame::freqEditSelected()
{
    traceMsg(QString("Freq Edit Selected"));
    ui->freqInput->setFocus();
    int len = ui->freqInput->text().length();
    if (len > 5)
    {
       ui->freqInput->setCursorPosition(len - 5);
    }


}


// this is the routine called from read memory

void RigControlFrame::transferDetails(memoryData::memData &m)
{
    traceMsg(QString("Memory Read: Memory Freq = %1, CurFreq = %2, Mode = %3, CurMode = %4").arg(m.freq).arg(curFreq).arg(m.mode).arg(curMode));
    if (isRadioLoaded())
    {
        if (radioConnected && !radioError)
        {
            ui->freqInput->clearFocus();
            //if (m.freq.remove('.') != curFreq.remove('.'))
            if (m.freq != curFreq)
            {
                traceMsg(QString("Memory Read: Send Freq"));
                sendFreq(m.freq);
            }

            if (m.mode != curMode)
            {
                traceMsg(QString("Memory Read: Send Mode"));
                sendModeToRadio(m.mode);
            }

        }
        else if (!radioConnected && radioName.trimmed() == "No Radio")
        {
            noRadioSendOutFreq(m.freq);
            noRadioSendOutMode(m.mode);
        }
    }

}


void RigControlFrame::getDetails(memoryData::memData &logData)
{
    // get contest information
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    ScreenContact sc = tslf->getScreenEntry();

    logData.callsign = sc.cs.fullCall.getValue();
    logData.freq = curFreq;
    logData.locator = sc.loc.loc.getValue();
    logData.mode = curMode;

    QStringList dt = dtg( true ).getIsoDTG().split('T');
    logData.time = dt[1];
    // time now, other formats
    // are available QString qth = sc.extraText;

    //logData.bearing = sc.bearing;

    logData.bearing = tslf->getBearingFrmQSOLog();
    // load log data into memory
}


void RigControlFrame::noRadioSetMode(QString m)
{
    QString mode = m;
    QStringList ml = m.split(':');
    if (ml.count() != 2)
    {
        return;
    }

    if (ml[0] == hamlibData::MGM)
    {
        ml[1] = "N/A";
        mode = ml[0] + ml[1];
    }

    setMode(mode);

}




void RigControlFrame::setMode(QString m)
{

    traceMsg(QString("Set Mode = %1").arg(m));
    QStringList mode = m.split(':');
    if (mode.length() == 2 )
    {
        for (int i = 0; i < hamlibData::supModeList.count(); i++)
        {
                if (mode[0] == hamlibData::supModeList[i])
                {
                    ui->modelbl->setText(mode[0]);
                    curMode = mode[0];
                    if (mode[0] == hamlibData::MGM)
                    {
                        mgmLabelVisible(true);
                        ui->mgmLbl->setText(mode[1]);
                    }
                    else
                    {
                       mgmLabelVisible(false);
                    }
                   return;
                }
        }


        // mode not supported by minos
        ui->modelbl->setText(MODE_ERROR);
    }


}


void RigControlFrame::sendModeToRadio(QString m)
{
    traceMsg(QString("Send Mode to Radio = %1").arg(m));
    emit sendModeToControl(QString("%1").arg(m));

}



void RigControlFrame::setRadioName(QString n)
{
    traceMsg(QString("Set RadioName = %1").arg(n));
    QStringList rNameList = n.split(':');
    if (rNameList.count() != 2)
    {
        return;
    }
    rigAppName = rNameList[0];
    radioName = rNameList[1];
    if (rigAppName == radioName)
    {
        ui->radioName->setText(radioName);
    }
    else
    {
       ui->radioName->setText(n);
    }

}



void RigControlFrame::loadMemories()
{
    loadRunButtonLabels();
}


void RigControlFrame::setRadioState(QString s)
{
    traceMsg(QString("Set RadioState = %1").arg(s));

    if (s != "")
    {
        if (s == RIG_STATUS_CONNECTED)
        {
            radioConnected = true;
        }
        else if (s == RIG_STATUS_DISCONNECTED)
        {
           radioConnected = false;
           radioError = false;
        }
        else if (s == RIG_STATUS_ERROR)
        {
           radioError = true;
        }

        ui->rigState->setText(s);
        radioState = s;
    }
}


void RigControlFrame::setRadioTxVertState(QString s)
{
    if (s == TXVERT_ON)
    {
        ui->txvertStat->setVisible(true);
        ui->TxVertLabel->setVisible(true);
        ui->txvertStat->setText("On");
    }
    else
    {
        ui->txvertStat->setVisible(false);
        ui->TxVertLabel->setVisible(false);
    }


}


bool RigControlFrame::checkRadioState()
{
    if (radioLoaded && radioConnected && !radioError)
    {
        return true;
    }

    return false;
}



int RigControlFrame::calcMinosMode(QString mode)
{
    int iMode = -1;
    for (int i = 0; i < hamlibData::supModeList.count(); i++ )
    {
        if (mode == hamlibData::supModeList[i])
        {
            iMode = i;
            return iMode;
        }
    }
    return iMode;
}



void RigControlFrame::freqLineEditInFocus()
{
    traceMsg(QString("Freq LineEdit in Focus"));
    freqEditOn = true;
    ui->freqInput->setReadOnly(false);
    freqLineEditFrameColour(true);
}





void RigControlFrame::freqLineEditBkgnd(bool status)
{

    QPalette palette;

    if (status)
    {
        palette.setColor(QPalette::Base,Qt::yellow);
    }
    else
    {
        palette.setColor(QPalette::Base,Qt::white);
    }

    ui->freqInput->setPalette(palette);

}


void RigControlFrame::freqLineEditFrameColour(bool status)
{
    int curPos = ui->freqInput->cursorPosition();
    if (status)
    {
        ui->freqInput->setStyleSheet("border: 1px solid red");
        // restore cursor selection
        ui->freqInput->setSelection(curPos, 1);
    }
    else
    {
        ui->freqInput->setStyleSheet("border: 1px solid black");

    }

}

void RigControlFrame::freqPlus_ShortCut()
{
    freqPlusShortCut_clicked(true);
}

void RigControlFrame::freqNeg_ShortCut()
{
    freqNegShortCut_clicked(true);
}


void RigControlFrame::freqPlusShortCut_clicked(bool /*click*/)
{

    freqPlusMinusButton(5000.0);

}



void RigControlFrame::freqNegShortCut_clicked(bool /*click*/)
{

    freqPlusMinusButton(-5000.0);

}


void RigControlFrame::freqPlusMinusButton(double f)
{

    if (isRadioLoaded())
    {
        QString freq = calcNewFreq(f);
        if (freq != "")
        {
           ui->freqInput->setText(freq);

           if (radioConnected && !radioError)
           {

               emit sendFreqControl(freq);
           }
           else if (!radioConnected && radioName.trimmed() == "No Radio")
           {
               noRadioSendOutFreq(freq);
           }
        }
    }

}


QString RigControlFrame::calcNewFreq(double incFreq)
{

    BandList &blist = BandList::getBandList();
    BandInfo bi;
    bool ok = false;
    bool bandOk = false;
    QString sfreq = "";
    sfreq = curFreq.trimmed().remove('.');

    double freq = sfreq.toDouble(&ok);
    if (ok)
    {
        freq += incFreq;
        bandOk = blist.findBand(freq, bi);
        if (!bandOk)
        {
            freq -= incFreq;
        }
        else
        {
            sfreq = convertFreqToStr(freq);
            trace(QString("CalcNewFreq: Freq  = %1").arg(sfreq));

        }
    }

    return sfreq;

}




void RigControlFrame::mgmLabelVisible(bool state)
{
    ui->mgmbreak->setVisible(state);
    ui->mgmLbl->setVisible(state);
}

void RigControlFrame::traceMsg(QString msg)
{
    trace(QString("RigcontrolFrame: %1 - %2 ").arg(radioName).arg(msg));
}








//********************** Run Buttons *******************************


void RigControlFrame::initRunMemoryButton()
{
    memoryData::memData m;
    runButtonMap[0] = new RunMemoryButton(ui->RunButton1, this, 0);
    connect( runButtonMap[0], SIGNAL( clearActionSelected(int)) , this, SLOT(runButClearActSel(int)), Qt::QueuedConnection );

    runButtonMap[1] = new RunMemoryButton(ui->RunButton2, this, 1);
    connect( runButtonMap[1], SIGNAL( clearActionSelected(int)) , this, SLOT(runButClearActSel(int)), Qt::QueuedConnection );

}


void RigControlFrame::runButReadActSel(int buttonNumber)
{
    traceMsg(QString("Run Button Read Selected = %1").arg(QString::number(buttonNumber + 1)));
    memoryData::memData m = getRunMemoryData(buttonNumber);
    if (isRadioLoaded())
    {
        if (radioConnected && !radioError)
        {
            ui->freqInput->clearFocus();
            if (m.freq.remove('.') != curFreq.remove('.'))
            {

                sendFreq(m.freq);
            }


            if (m.mode != curMode)
            {
                sendModeToRadio(m.mode);
            }

        }
        else if (!radioConnected && radioName.trimmed() == "No Radio")
        {
            noRadioSendOutFreq(m.freq);
        }
    }
}



void RigControlFrame::runButWriteActSel(int buttonNumber)
{
    traceMsg(QString("Memory Write Selected %1 = ").arg(QString::number(buttonNumber + 1)));
    memoryData::memData runData;
    runData.callsign = "Run" + QString::number(buttonNumber + 1);
    runData.freq = curFreq;
    runData.locator = "";
    runData.mode = curMode;
    runData.bearing = COMPASS_ERROR;
    runData.time = "00:00";
    // load run data into run memory

    RunButtonDialog runDialog(this);
    runDialog.setWindowTitle(QString("Run %1 - Write").arg(QString::number(buttonNumber + 1)));
    runDialog.setLogData(&runData, buttonNumber);
    if (runDialog.exec() == QDialog::Accepted)
    {
        setRunMemoryData(buttonNumber, runData);
        runButtonUpdate(buttonNumber);
    }

}

void RigControlFrame::runButEditActSel(int buttonNumber)
{
    memoryData::memData runData = getRunMemoryData(buttonNumber);

    traceMsg(QString("Run Button Edit Selected = %1").arg(QString::number(buttonNumber + 1)));
    RunButtonDialog runDialog(this);
    runDialog.setWindowTitle(QString("Run %1 - Edit").arg(QString::number(buttonNumber + 1)));
    runDialog.setLogData(&runData, buttonNumber);

    if (runDialog.exec() == QDialog::Accepted)
    {
        setRunMemoryData(buttonNumber, runData);
        runButtonUpdate(buttonNumber);
    }
}

void RigControlFrame::runButClearActSel(int buttonNumber)
{
    traceMsg(QString("Run Button Clear Selected = %1").arg(QString::number(buttonNumber + 1)));

    memoryData::memData m;
    setRunMemoryData(buttonNumber, m);
    runButtonUpdate(buttonNumber);
}
void RigControlFrame::loadRunButtonLabels()
{
    for (int i = 0; i < runButData::NUM_RUNBUTTONS; i++)
    {
        runButtonUpdate(i);
    }
}


void RigControlFrame::runButtonUpdate(int buttonNumber)
{
    memoryData::memData m = getRunMemoryData(buttonNumber);
    QString sc = ((buttonNumber == 0)?QString(" [ "):QString( " ] "));

    runButtonMap[buttonNumber]->memButton->setText("R" + QString::number(buttonNumber + 1) + "(" + sc + ") " + extractKhz(m.freq) + " ");
    QString tTipStr = "Freq: " + convertFreqStrDisp(m.freq) + "\n"
            + "Mode: " + m.mode + "\n";

    runButtonMap[buttonNumber]->memButton->setToolTip(tTipStr);
}

QString RigControlFrame::extractKhz(QString f)
{
    QString khz = "***";
    QString sf = f.remove('.');
    bool ok = false;
    double df = sf.toDouble(&ok);
    if (ok && df != 0.0)
    {
        if (f.contains('.'))
        {
            QStringList k = f.split('.');
            int i = k.length();
            if (i >=2)
            {
                return k[i-2];
            }
        }
        else
        {
            int i = f.length();
            if (i >= 6)
            {
                khz = f.mid(i - 6, 3);
                return khz;
            }
        }
    }


    return khz;


}




memoryData::memData RigControlFrame::getRunMemoryData(int memoryNumber)
{
    memoryData::memData m;

    if (ct->runMemories.size() > memoryNumber)
        m = ct->runMemories[memoryNumber].getValue();
    return m;
}
void RigControlFrame::setRunMemoryData(int memoryNumber, memoryData::memData m)
{
    ct->saveRunMemory(memoryNumber, m);
}


//*******************Run Memory Button *************************//


RunMemoryButton::RunMemoryButton(QToolButton *b, RigControlFrame *rcf, int no)
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
    readAction = new QAction("&Read", memButton);
    writeAction = new QAction("&Write",memButton);
    editAction = new QAction("&Edit", memButton);
    clearAction = new QAction("&Clear",memButton);
    memoryMenu->addAction(readAction);
    memoryMenu->addAction(writeAction);
    memoryMenu->addAction(editAction);
    memoryMenu->addAction(clearAction);
    memButton->setMenu(memoryMenu);

    connect(shortKey, SIGNAL(activated()), this, SLOT(readActionSelected()));
    connect(shiftShortKey, SIGNAL(activated()), this, SLOT(memoryShortCutSelected()));
    connect(memButton, SIGNAL(clicked(bool)), this, SLOT(readActionSelected()));
    connect( readAction, SIGNAL( triggered() ), this, SLOT(readActionSelected()) );
    connect( writeAction, SIGNAL( triggered() ), this, SLOT(writeActionSelected()) );
    connect( editAction, SIGNAL( triggered() ), this, SLOT(editActionSelected()) );
    connect( clearAction, SIGNAL( triggered() ), this, SLOT(clearActionSelected()) );
}
RunMemoryButton::~RunMemoryButton()
{
//    delete memButton;
}
void RunMemoryButton::memoryUpdate()
{
    rigControlFrame->runButtonUpdate(memNo);
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




//*******************Freq Line Edit *************************//

FreqLineEdit::FreqLineEdit(QWidget *parent):
    QLineEdit(parent)
{

}


FreqLineEdit::~FreqLineEdit()
{


}


void FreqLineEdit::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta() / 8;
    int numTicks = numDegrees / 15;

    if (numTicks == 1)
    {
       changeFreq(true);
    }
    else
    {
        changeFreq(false);
    }

    event->accept();
}


void FreqLineEdit::keyPressEvent(QKeyEvent *event)
{

    if(event->key() == Qt::Key_Up)
    {
        changeFreq(true);
    }
    else if(event->key() == Qt::Key_Down)
    {
        changeFreq(false);

    }
    else if (event->key() == Qt::Key_Return)
    {
        emit freqEditReturn();
        return;
    }
    else
    {
        // default handler for event
        QLineEdit::keyPressEvent(event);
    }
}



void FreqLineEdit::changeFreq(bool direction)
{
    static const double tuningData[][14] =
                                        {
                                        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                        {10, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                        {100, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                        {1000, 0, 100, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                        {10000, 1000, 0, 100, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0},
                                        {100000, 10000, 1000, 0, 100, 10, 1, 0, 0, 0, 0, 0, 0, 0},
                                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                        {1000000, 0, 100000, 10000, 1000, 0, 100, 10, 1, 0, 0, 0, 0, 0},
                                        {10000000, 1000000, 0, 100000, 10000, 1000, 0, 100, 10, 1, 0, 0, 0, 0},
                                        {100000000, 10000000, 1000000, 0, 100000, 10000, 1000, 0, 100, 10, 1, 0, 0, 0},
                                        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                        {1000000000, 0, 100000000, 10000000, 1000000, 0, 100000, 10000, 1000, 0, 100, 10, 1, 0},
                                        {10000000000, 1000000000, 0, 100000000, 10000000, 1000000, 0, 100000, 10000, 1000, 0, 100, 10, 1}
                                        };


    BandList &blist = BandList::getBandList();
    BandInfo bi;
    bool bandOK = false;

    bool ok = false;
    QString sfreq = text();
    int sfreqLen = sfreq.length();
    int pos = cursorPosition();
    if (sfreqLen < 0 || sfreqLen > 14 || pos < 0 || pos > sfreqLen)
    {
        return;
    }
    const double tuneStep = tuningData[sfreqLen - 1][pos];


    sfreq = sfreq.trimmed().remove('.');


    double freq = sfreq.toDouble(&ok);

    if (ok)
    {
        if (direction)
        {
            freq += tuneStep;
            bandOK = blist.findBand(freq, bi);
            if (!bandOK)
            {
                freq -= tuneStep;
            }
        }
        else
        {
            freq -= tuneStep;
            bandOK = blist.findBand(freq, bi);
            if (!bandOK)
            {
                freq += tuneStep;
            }
        }


        sfreq = convertFreqToStr(freq);
        trace(QString("Change Freq: Freq Tuning = %1").arg(freq));
        if (bandOK)
        {
            setText(convertFreqStrDisp(sfreq));
            emit newFreq();
        }
        else
        {
            setText(QString("%1 %2 %3").arg("<font color='Red'>", convertFreqStrDisp(sfreq), "</font>"));
        }

        setCursorPosition(pos);
   }
}


