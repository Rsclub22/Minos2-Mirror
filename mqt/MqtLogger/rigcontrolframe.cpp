/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2017 - 2019
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2019
//
//
//
/////////////////////////////////////////////////////////////////////////////

#include "base_pch.h"
#include <math.h>
#include "ContestApp.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "SendRPCDM.h"
#include "runbuttondialog.h"
#include "BandList.h"
#include "rigutils.h"
#include "LoggerContest.h"
#include "rigcontrolframe.h"
#include "volumeslider.h"
#include "freqlineedit.h"
#include "ritlineedit.h"
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

const QString RIT_BUTTON_ON_STYLE = QString("background-color: Sandybrown ;\n");
const QString RIT_BUTTON_OFF_STYLE = QString("background-color: Gainsboro ;\n");


RigControlFrame::RigControlFrame(QWidget *parent):
    QFrame(parent),
    ui(new Ui::RigControlFrame),
    ct(nullptr),
    radioLoaded(false),
    radioConnected(false),
    radioError(false),
    freqEditOn(false),
    curFreq(memDefData::DEFAULT_FREQ),
    curMode(""),
    ritEnable(false),
    ritOn(false),
    ritEditOn(false),
    radioState("None"),
    onContestPageChangedFlag(false)

{
    ui->setupUi(this);

    // allow capture of events from these widgets
    ui->freqInput->installEventFilter(this  );
    ui->RitEdit->installEventFilter(this  );

    initRigFrame(parent);

    initRunMemoryButton();
    initTuneMemoryButton();

    showRitButOff();

    mgmLabelVisible(false);

    setRadioTxVertEnabled(false);
    setRitEnableState(false);
    setRadioVolumeState(false);



    bool tpm;
    TContestApp::getContestApp() ->displayBundle.getBoolProfile( edpShowTPM, tpm );

    ui->tpmBox->setVisible(tpm);


    // init memory button data before radio connection
    setRadioName(radioName, "");

    freqEditShortKey = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F), parent);
    connect(freqEditShortKey, SIGNAL(activated()), this, SLOT(freqEditSelected()));

    freqPlusShortCut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_U), parent);
    connect(freqPlusShortCut, SIGNAL(activated()), this, SLOT(freqPlus_ShortCut()));
    connect(ui->freqUp, SIGNAL(clicked(bool)), this, SLOT(freqPlusShortCut_clicked(bool)));

    freqNegShortCut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_D), parent);
    connect(freqNegShortCut, SIGNAL(activated()), this, SLOT(freqNeg_ShortCut()));
    connect(ui->freqDown, SIGNAL(clicked(bool)), this, SLOT(freqNegShortCut_clicked(bool)));

    // rit key shortcuts
    ritOnOffShortCut = new QShortcut(QKeySequence("Ctrl+o"), parent);
    connect(ritOnOffShortCut, SIGNAL(activated()), this, SLOT(ritButtonSelected()));
    ritClearShortCut = new QShortcut(QKeySequence("Ctrl+k"), parent);
    connect(ritClearShortCut, SIGNAL(activated()), this, SLOT(ritClearShortCutSelected()));
    ritFreqEditShortCut = new QShortcut(QKeySequence("Ctrl+i"), parent);
    connect(ritFreqEditShortCut, SIGNAL(activated()), this, SLOT(ritFreqEditShortCutInFocus()));


    connect(&MinosLoggerEvents::mle, SIGNAL(FontChanged()), this, SLOT(on_FontChanged()), Qt::QueuedConnection);

    on_FontChanged();

    traceMsg(QString("RigControlFrame Started"));

    // now check if we have the details to launch a radio selection


    if (listOfRadios.isEmpty() && LogContainer->sendDM->getRigCache()->getRigListCount() > 0)
    {
        listOfRadios = LogContainer->sendDM->rigs();
    }

    if (allRadioDetails.isEmpty())
    {
        if (LogContainer->sendDM->getRigCache()->getRigDetailCount() == listOfRadios.count())
        {
            QVector<PubSubName> rigList = LogContainer->sendDM->getRigCache()->getRigList();
            foreach (PubSubName psn, rigList)
            {
                RigDetails& selDetail = LogContainer->sendDM->getRigCache()->getDetails(psn);
                setTransVertOffset(selDetail.transverterOffset().getValue(), psn);
                setTransVertSwitch(selDetail.transverterSwitch().getValue(), psn);
                setTransVertEnabled(selDetail.transverterEnabled().getValue(), psn);
                setTransVertStatus(selDetail.transverterStatus().getValue(), psn);
                setVolumeStatus(selDetail.volumeStatus().getValue(), psn);
                setRitEnableStatus(selDetail.ritEnableStatus().getValue(), psn);
                setBandList(selDetail.bandList().getValue(), psn);
            }
        }
    }


    // start timer to wait for bandlist and rigdetails to launch
    launchRadioSelectTimer = new QTimer(this);
    launchRadioSelectCount = 5;     // wait five seconds
    connect(launchRadioSelectTimer, SIGNAL(timeout()), this, SLOT(checkRigDetailsAvail()));
    launchRadioSelectTimer->start(1000);



}

RigControlFrame::~RigControlFrame()
{
    delete ui;

}
void RigControlFrame::on_FontChanged()
{
    QFont cf = QApplication::font();
    qreal fs = cf.pointSizeF();
    int fsi = static_cast<int>(fs * 1.25);
    cf.setPointSize(fsi);
    cf.setBold(true);
    ui->freqInput->setFont(cf);
}


void RigControlFrame::checkRigDetailsAvail()
{

    launchRadioSelectCount--;
    if (launchRadioSelectCount == 0)
    {
        // timed out waiting for rigdetails
        launchRadioSelectTimer->stop();
        trace(QString("rigControlFrame: Timed out waiting for rigdetails"));
        return;
    }
    else if (ct && ( ct->isProtected() || ct != TContestApp::getContestApp() ->getCurrentContest()))
    {
        launchRadioSelectTimer->stop();
        return;
    }

    if (ct && !ct->isProtected() && ct == TContestApp::getContestApp() ->getCurrentContest())
    {
        if (!ct->radioName.getValue().toString().isEmpty() )
        {
            if (allRadioDetails.contains(ct->radioName.getValue().toString() ))
            {
               if (allRadioDetails[ct->radioName.getValue().toString()].getBandListCount() > 0)
               {
                   launchRadioSelectTimer->stop();
                   trace(QString("rigControlFrame: start select radio %1, mode %2").arg(ct->radioName.getValue().toString()).arg(ct->currentMode.getValue()));
                   setRadioName(ct->radioName.getValue().toString(), ct->currentMode.getValue());
               }
            }

        }

    }


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

    // main freq tuning
    //connect(ui->freqInput, SIGNAL(lostFocus()), this, SLOT(exitFreqEdit()));
    connect(ui->freqInput, SIGNAL(freqEditReturn()), this, SLOT(returnChangeRadioFreq()));
    connect(ui->freqInput, SIGNAL(newFreq()), this, SLOT(changeMainRadioFreq()));

    // rit freq tuning
    connect(ui->RitButton, SIGNAL(clicked(bool)), this, SLOT(ritButtonSelected()));
    connect(ui->RitEdit, SIGNAL(newFreq(int)), this, SLOT(changeRitRadioFreq(int)));
    connect(ui->RitClear, SIGNAL(clicked(bool)), this, SLOT(ritClearButtonSelected(bool)));

    // from cluster frame
    connect(&MinosLoggerEvents::mle, SIGNAL(FreqStrToRig(QString)), this, SLOT(clusterUpdateRigFreq(QString)));

    // volume control updates to radio
    connect(ui->volumeSlider, SIGNAL(sendVolumeRadio(int)), this, SLOT(sendVolumeRadio(int)));

    // when no radio is connected
    connect(this, SIGNAL(noRadioSendFreq(QString)), this, SLOT(noRadioSetFreq(QString)));
    connect(this, SIGNAL(noRadioSendMode(QString)), this, SLOT(noRadioSetMode(QString)));


    connect(ui->bandSelCombo, SIGNAL(activated(int)), this, SLOT(radioBandFreq(int)));

    //connect(this, SIGNAL(newBandList()), this, SLOT(setRadioFreq()));

    setVolControlVisible(false);

    if (!isRadioLoaded())
    {
        ui->modelbl->setVisible(false);
    }

}


void RigControlFrame::clusterUpdateRigFreq(QString freq)
{
    ui->freqInput->clearFocus();
    sendFreq(freq);
}




void RigControlFrame::on_radioNameSel_activated(const QString &arg1)
{
    // radio combo selected
    radioName = arg1;

    trace(QString("on radioNameSel activated: radioName - %1 requested").arg(arg1));
    setRadioName(arg1, ct->currentMode.getValue());

}


void RigControlFrame::setTransVertOffset(double offset, PubSubName psn)
{
    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        rd.setTransVerterOffset(offset);
        allRadioDetails[psn] = rd;
    }
    else
    {
        rd.setTransVerterOffset(offset);
        allRadioDetails[psn] = rd;
    }
}

void RigControlFrame::setTransVertSwitch(int switchNum, PubSubName psn)
{
    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        rd.setTransVertSwitch(switchNum);
        allRadioDetails[psn] = rd;
    }
    else
    {
        rd.setTransVertSwitch(switchNum);
        allRadioDetails[psn] = rd;
    }
}

void RigControlFrame::setTransVertEnabled(bool status, PubSubName psn)
{
    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        rd.setTransVertEnabled(status);
        allRadioDetails[psn] = rd;
    }
    else
    {
        rd.setTransVertEnabled(status);
        allRadioDetails[psn] = rd;
    }
    if (psn == selRadioName && ct && !ct->isProtected() && ct == TContestApp::getContestApp() ->getCurrentContest())
    {
        setRadioTxVertEnabled(status);
        selRadioDetails.setTransVertEnabled(status);
    }
}





void RigControlFrame::setTransVertStatus(bool status, PubSubName psn)
{
    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        rd.setTransVertStatus(status);
        allRadioDetails[psn] = rd;
    }
    else
    {
        rd.setTransVertStatus(status);
        allRadioDetails[psn] = rd;
    }
    if (psn == selRadioName && ct && !ct->isProtected() && ct == TContestApp::getContestApp() ->getCurrentContest())
    {
        setRadioTxVertStatus(status);
        selRadioDetails.setTransVertStatus(status);
    }
}

void RigControlFrame::setVolumeStatus(bool status, PubSubName psn)
{
    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        rd.setVolumeStatus(status);
        allRadioDetails[psn] = rd;
    }
    else
    {
        rd.setVolumeStatus(status);
        allRadioDetails[psn] = rd;
    }
    if (psn == selRadioName && ct && !ct->isProtected() && ct == TContestApp::getContestApp() ->getCurrentContest())
    {
        setRadioVolumeState(status);
        selRadioDetails.setVolumeStatus(status);
    }
}

void RigControlFrame::setRitEnableStatus(bool status, PubSubName psn)
{
    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        rd.setRitEnableStatus(status);
        allRadioDetails[psn] = rd;
    }
    else
    {
        rd.setRitEnableStatus(status);
        allRadioDetails[psn] = rd;
    }

    if (psn == selRadioName && ct && !ct->isProtected() && ct == TContestApp::getContestApp() ->getCurrentContest())
    {
        setRitEnableState(status);
        selRadioDetails.setRitEnableStatus(status);
    }
}

void RigControlFrame::setBandList(QString s,PubSubName psn)
{
    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        rd.setBandList(s);
        allRadioDetails[psn] = rd;
    }
    else
    {
        rd.setBandList(s);
        allRadioDetails[psn] = rd;
    }

    // update bandlist combo if current radio and current contest
    if (psn.toString().toLower() == radioName.toLower() && ct && !ct->isProtected() && ct == TContestApp::getContestApp() ->getCurrentContest())
    {
        createActiveBandList(s);
    }
}




void RigControlFrame::setRadioLoaded()
{
    traceMsg(QString("%1 Set Radio Loaded").arg(ct?ct->uuid:""));
    radioLoaded = true;
    ui->modelbl->setVisible(true);

    //if (ct && !ct->isProtected())
   // {
        // this sets the radio on frame launch
    //    trace(QString("setRadioList:: setRadioName - radioName = %1, mode = %2").arg(ct->radioName.getValue().toString()).arg(ct->currentMode.getValue()));
   //     setRadioName(ct->radioName.getValue().toString(), ct->currentMode.getValue());
   // }

    setTpm(1);
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

void RigControlFrame::setFreq(QString freq)
{
    traceMsg(QString("Set Freq: = %1").arg(freq));

            if (freq == "0" && freq == "-1")
    {
        // this is force an update of freq, ignore
        traceMsg(QString("Force Freq Update Received - Ignore!"));
        return;
    }



//    if (tuneButtonMap[0]->freq.isEmpty() && tuneButtonMap[1]->freq.isEmpty())
//    {
//        tuneButtonMap[0]->freq = freq;
//        tuneButtonMap[1]->freq = freq;
//        curTuneButton = tuneButtonMap[0];
//        trace(QString("setFreq: curTuneButton initialised to button 1 freq %1").arg(freq));
//    }
//    else
    if (curTuneButton)
    {
        trace(QString("setFreq: curTuneButton %1 freq set to %2").arg(curTuneButton->memNo).arg(freq));
        curTuneButton->freq = freq;
    }

    updateTuneButtons();

    if (lastFreq != freq)
    {
        lastFreq = freq;
    }
    if (freq.count() >= 4)
    {
        if (!freqEditOn)
        {
            trace(QString("setFreq: Display Freq = %1").arg(freq));
            ui->freqInput->setInputMask(maskData::freqMask[freq.count() - 4]);
            ui->freqInput->setText(freq);
        }
        curFreq = freq;

    }
    // an error here?

}


// from rigcontrol

void RigControlFrame::setRitFreq(QString freq)
{
    if (!ritEditOn)
    {
      ui->RitEdit->setText(freq);
    }
}


void RigControlFrame::setRitRadioStatus(bool status)
{
    if (ritOn != status)
    {
        if (status)
        {
            ritButtonOn();

        }
        else
        {
            ritButtonOff();

        }
    }

}

// to rigcontrol

void RigControlFrame::changeRitRadioFreq(int freq)
{
    traceMsg(QString("Change Rit Freq = %1").arg(convertRitFreqToStr(freq)));
    if (ritEnable && ritOn)
    {
        emit sendRitFreq(freq);
    }


}


void RigControlFrame::ritClearShortCutSelected()
{
    ritClearButtonSelected(true);
}



void RigControlFrame::ritClearButtonSelected(bool /*state*/)
{

    if (ritEnable && ritOn)
    {
        int pos = ui->RitEdit->cursorPosition();
        changeRitRadioFreq(0);  // turns off rit in hamlib
        QString sfreq = convertRitFreqToStr(0);       // set rit display to zero
        ui->RitEdit->setText(sfreq);
        ui->RitEdit->setCursorPosition(pos);
    }
}

void RigControlFrame::ritButtonSelected()
{

    traceMsg(QString("Rit Button Pressed"));
    if (!ritOn)
    {
        ritButtonOn();
    }
    else
    {
       ritButtonOff();
    }

}


void RigControlFrame::ritButtonOn()
{
    traceMsg(QString("Rit Button On"));
    ritOn = true;
    //ui->RitEdit->setRitOnFlag(true);        // set flag to allow editing of RIT freq
    ui->RitEdit->setEnabled(true);
    showRitButOn();
    emit ritStatus(true);

}

void RigControlFrame::ritButtonOff()
{
    traceMsg(QString("Rit Button Off"));
    ui->RitEdit->clearFocus();
    ritOn = false;
    ritEditOn = false;
    ui->RitEdit->setEnabled(false);
    showRitButOff();
    emit ritStatus(false);


}



void RigControlFrame::showRitButOn()
{
    //ui->Rotate->setPalette(*redText);
    ui->RitButton->setStyleSheet(RIT_BUTTON_ON_STYLE);
    ui->RitButton->setText("On");
}

void RigControlFrame::showRitButOff()
{
    //ui->Rotate->setPalette(*blackText);
    ui->RitButton->setStyleSheet(RIT_BUTTON_OFF_STYLE);
    ui->RitButton->setText("Off");
}




void RigControlFrame::changeMainRadioFreq()
{
    traceMsg(QString("Change Main Radio Freq"));

    QString newfreq = ui->freqInput->text().trimmed().remove('.');
    double f = convertStrToFreq(newfreq);
    if (f >= 0.0)
    {
        if (f > 0)
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
                        else if (!radioConnected && radioName.trimmed().isEmpty())
                        {
                            noRadioSendOutFreq(lastFreq);
                        }
                    }

                }

            }
            else
            {
                QString f = QString("%1 %2 %3").arg("<font color='Red'>").arg(lastFreq).arg("</font>");
                trace("changeMainRadioFreq " + f);
                ui->freqInput->setText(f);
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
    changeMainRadioFreq();
    exitFreqEdit();
}

void RigControlFrame::radioBandFreq(int index)
{
    int idx = index -1;
    setRadioBandWarning("");
    if (idx >= 0 && idx < listOfBands.count())
    {
        QString f = listOfBands[idx].freq;
        if (f != curFreq)
        {

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
    else
    {
        traceMsg(QString("RigContFrame: Freq the same or index out of range"));
    }
}

void RigControlFrame::sendFreq(QString f)
{

    if (f != NO_BAND_SUPPORT)
    {
        bool ok = false;
        double df = f.toDouble(&ok);
        if (ok && df > 0.0)
        {
         emit sendFreqControl(f);
        }

    }
    else
    {
        // send no band support
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


void RigControlFrame::on_ContestPageChanged()
{

    if (ct && !ct->isProtected() && ct == TContestApp::getContestApp() ->getCurrentContest())
    {

        QString radNam = ct->radioName.getValue().toString();
        QString mode = ct->currentMode.getValue();

        onContestPageChangedFlag = true;
        setRadioName(radNam, mode);


    }
}


bool RigControlFrame::eventFilter(QObject *obj, QEvent *event)
{
   Q_UNUSED(obj)

   if (obj == ui->freqInput)
   {
       if (event->type() == QEvent::FocusIn)
          freqLineEditInFocus();
       else if (event->type() == QEvent::FocusOut)
          exitFreqEdit();
   }
   else if (obj == ui->RitEdit)
   {
       if (event->type() == QEvent::FocusIn)
          ritLineEditInFocus();
       else if (event->type() == QEvent::FocusOut)
          exitRitFreqEdit();
   }


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

void RigControlFrame::exitRitFreqEdit()
{
    traceMsg(QString("Exit Rit Edit Freq"));
    ritEditOn = false;
    ui->RitEdit->setRitOnFlag(false);        // set flag to allow prevent editing of RIT freq
    //setFreq(curFreq);
    ritFreqLineEditFrameColour(false);
    ui->RitEdit->clearFocus();
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
            if (!m.freq.isEmpty() &&m.freq != curFreq)
            {
                traceMsg(QString("Memory Read: Send Freq"));
                sendFreq(m.freq);
            }

            if (!m.mode.isEmpty() && m.mode != curMode)
            {
                traceMsg(QString("Memory Read: Send Mode"));
                sendModeToRadio(m.mode);
            }

        }
        else if (!radioConnected && radioName.trimmed().isEmpty())
        {
            if (!m.freq.isEmpty())
                noRadioSendOutFreq(m.freq);
            if (!m.mode.isEmpty())
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
    logData.locator = sc.loc.loc.getValue().trimmed();
    if (curMode.isEmpty())
    {
        logData.mode = sc.mode;
    }
    else
    {
        logData.mode = curMode;
    }

    QStringList dt = dtg( true ).getIsoDTG().split('T');
    logData.time = dt[1];
    // time now, other formats
    // are available QString qth = sc.extraText;

    //logData.bearing = sc.bearing;

    if (sc.loc.loc.getValue().trimmed().isEmpty())
    {
        logData.bearing = tslf->getCurrentBearing();
    }
    else
    {
        logData.bearing = tslf->getBearingFrmQSOLog();
    }
    // load log data into memory
}
void RigControlFrame::getRigDetails(memoryData::memData &m)
{
    m.freq = curFreq;
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


void RigControlFrame::setRadioName(QString radNam, QString mode)
{


    traceMsg(QString("setRadioName: Set RadioName = %1, mode = %2, contest = %3").arg(radNam).arg(mode).arg(ct?ct->uuid:""));


    if (ct && !ct->isProtected() && ct == TContestApp::getContestApp() ->getCurrentContest())
    {

            radioName = radNam;
            trace(QString("setRadioName:: update  radioName  %1").arg(radioName));
            curMode = mode;
            trace(QString("setRadioName:: update with new radio request - radioName  %1, mode %2").arg(radNam).arg(curMode));
            trace(QString("setRadioName:: Looking for radio"));
            int index = ui->radioNameSel->findText(radioName, Qt::MatchFixedString);
            if (index >= 0)
            {
                ui->radioNameSel->setCurrentIndex(index);
            }
            else
            {
                trace(QString("setRadioName: Can't find %1 in radioNameSel").arg(radioName));
            }

            trace(QString("setRadioName:: Select Radio = %1 Mode = %2 on rigcontrol").arg(radioName).arg(mode));

            setRadioTxVertEnabled(false);
            setRitEnableState(false);
            setRadioVolumeState(false);


            if (radioName.isEmpty())
            {
                emit selectRadio(radioName, mode);  // send radio and mode.
                selRadioDetails = RadioDetails();
                createActiveBandList(selRadioDetails.getBandList());
            }
            else
            {
                selRadioName = PubSubName(radioName);
                if (allRadioDetails.contains(selRadioName))
                {
                    trace(QString("setRadioName:: Select Radio = %1 Mode = %2 on rigcontrol").arg(radioName).arg(mode));
                    selRadioDetails = allRadioDetails[selRadioName];
                    createActiveBandList(selRadioDetails.getBandList());
                    emit selectRadio(radioName, mode);  // send radio and mode.

                    setRadioFreq();
                }


            }







    }
    else
    {
        trace(QString("setRadioName:: No contest or protect, no radio selection"));
    }

}



void RigControlFrame::setRadioFreq()
{

    if (selRadioDetails.getBandList().isEmpty())
    {
        setRadioBandWarning(QString("<font color='Red'>Error Receiving Bandlist!</font>"));
        trace(QString("setRadioFreq:: Error Receiving Bandlist!"));
        sendFreq(NO_BAND_SUPPORT);
        return;
    }

    //createActiveBandList(selRadioDetails.getBandList());

    if (listOfBands.isEmpty())
    {
        setRadioBandWarning(QString("<font color='Red'>Radio has no available bands</font>"));
        trace(QString("setRadioFreq:: Error No available bands!"));
        sendFreq(NO_BAND_SUPPORT);
        return;



    }

    setRadioBandWarning(QString(""));
    trace(QString("setRadioFreq: list of bands for radio %1 is %2").arg(selRadioName.toString()).arg(selRadioDetails.getBandList()));

    if (ct == TContestApp::getContestApp() ->getCurrentContest())
    {
       //And we want to select the frequency based on the contest band
       TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
       trace(QString("setRadioFreq: select frequency"));
       QString cb = ct->band.getValue().trimmed();

       BandList &blist = BandList::getBandList();
       BandInfo bi;
       bool bandOK = blist.findBand(cb, bi);
       if (bandOK)
       {

            for (int i = 0; i < listOfBands.size(); i++)
            {
                if (listOfBands[i].band == cb)
                {
                    trace(QString("setRadioFreq: found band %1 on radio, set band select").arg(cb));
                    ui->bandSelCombo->setCurrentIndex(i + 1);

                    QString freq;

                    if (onContestPageChangedFlag)
                    {
                        onContestPageChangedFlag = false;
                        freq = tslf->sSavedCurFreq;
                    }
                    else
                    {
                       freq = lastFreq;
                    }


                    trace(QString("setRadioFreq: found band %1 on radio, freq %2").arg(cb).arg(freq));


                    QRegExp re("\\d*");  // a digit (\d), zero or more times (*)
                    if (!re.exactMatch(freq))
                    {
                        freq = "0";
                    }

                    double cf = convertStrToFreq(freq);
                    QString cfstr;
                    // find band for current freq
                    for (int i = 0; i < blist.bandList.count(); i++)
                    {
                        if (cf >= blist.bandList[i].flow && cf <= blist.bandList[i].fhigh)
                        {
                            cfstr = blist.bandList[i].uk;
                            break;
                        }

                    }


                    if ((cf > bi.flow && cf < bi.fhigh) && (cb == cfstr))
                    {
                        sendFreq(freq);
                        trace(QString("setRadioFreq: Set previous freq = %1").arg(QString::number(cf)));
                        if (ui->freqInput->text().toInt() == 0) // if display is zero update display locally
                        {
                            setFreq(freq);
                        }
                    }
                    else
                    {
                        sendFreq(listOfBands[i].freq);
                        trace(QString("setRadioFreq: Set defaut freq = %1").arg(listOfBands[i].freq));
                    }

                    setRadioBandWarning("");

                    return;
                }
            }
            // warn no band for this radio
            setRadioBandWarning(QString("<font color='Red'>No %1 Band found for this radio!</font>").arg(cb));
            trace(QString("SsetRadioFreq: %1 Band not found on this radio").arg(cb));
            sendFreq(NO_BAND_SUPPORT);
        }

      }


}


void RigControlFrame::loadMemories()
{
    loadRunButtonLabels();
}



void RigControlFrame::setRadioList()
{
    if (ct && !ct->isProtected() && ct == TContestApp::getContestApp() ->getCurrentContest())
    {


        // test if rigcontrol has restarted
        // radioNameSel already has list of radios, it is rigcontrol restart
        if  (ui->radioNameSel->count() > 0 && !launchRadioSelectTimer->isActive())
        {
            trace(QString("setRadioList: rigControl restart? reconnecting"));
            launchRadioSelectCount = 5;     // wait five seconds
            launchRadioSelectTimer->start(1000);
        }


        listOfRadios = LogContainer->sendDM->rigs();

        ui->radioNameSel->clear();
        ui->radioNameSel->addItem("");
        ui->radioNameSel->addItems(listOfRadios);
        if (radioName != "")
        {
            ui->radioNameSel->setCurrentText(radioName);
        }


    }
}


// create the active bands on selected radio

void RigControlFrame::createActiveBandList(QString b)
{
    trace("createActiveBandList " + b);
    if (!b.isEmpty())
    {
        QString currentBand = ui->bandSelCombo->currentText();
        listOfBands.clear();
        QStringList lbf;
        QStringList lb;
        quickBandSelData d;
        // split into bands
        lbf = b.split(":");
        for (int i = 0; i < lbf.count(); i++)
        {
            QStringList s = lbf[i].split('-');
            lb.append(s[0]);
            d.band = s[0];
            d.freq = s[1];
            listOfBands.append(d);
        }
        ui->bandSelCombo->clear();
        ui->bandSelCombo->addItem("");
        ui->bandSelCombo->addItems(lb);
        ui->bandSelCombo->setCurrentText(currentBand); // restore for now


    }
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


           ui->bandWarnLabel->setText("");
           if (ui->radioNameSel->currentText() == "")
           {
               curFreq = "00000000000";
               ui->freqInput->setInputMask(maskData::freqMask[curFreq.count() - 4]);
               ui->freqInput->setText(curFreq);
               ui->bandSelCombo->clear();
           }





           emit radioDisconnected();

        }
        else if (s == RIG_STATUS_ERROR)
        {
           radioError = true;
        }

        ui->rigState->setText(s);
        radioState = s;
    }
}

// volume level from radio
void RigControlFrame::setVolume(int level)
{

    ui->volumeSlider->setVolume(level);
}

// volume level radio
void RigControlFrame::sendVolumeRadio(int level)
{
    emit sendVolumeToRadio(level);
}

void RigControlFrame::setVolControlVisible(bool value)
{
    ui->volumeSlider->setVisible(value);
    ui->volumeLabel->setVisible(value);
}

void RigControlFrame::setRadioVolumeState(bool state)
{
    setVolControlVisible(state);
}



void RigControlFrame::setTpm(int t)
{
    // tpm change received from rig control
    if (t > 0 && t <= tuneButData::NUM_TUNEBUTTONS)
    {
        trace(QString("%1 Current tune button set to button %2").arg(ct?ct->uuid:QString()).arg(t));
        curTuneButton = tuneButtonMap[t - 1];
    }
    else
    {
        trace(QString("%1 Current tune button set to null").arg(ct?ct->uuid:QString()));
        curTuneButton = nullptr;
    }
    updateTuneButtons();
}

void RigControlFrame::setRadioTxVertEnabled(bool s)
{
     ui->TxVertLabel->setVisible(s);
     ui->transvertIndicator->setVisible(s);
     transVertIndicatorOff();
}


void RigControlFrame::setRadioTxVertStatus(bool status)
{

    if(status)
    {
        transVertIndicatorOn();
    }
    else
    {
        transVertIndicatorOff();
    }
}


void RigControlFrame::transVertIndicatorOn()
{
    ui->transvertIndicator->setStyleSheet(SUP_RADIO_INDICATOR_TRANSVERT_ON_STYLE);
}

void RigControlFrame::transVertIndicatorOff()
{
    ui->transvertIndicator->setStyleSheet(SUP_RADIO_INDICATOR_RADIO_STYLE);
}


void RigControlFrame::setRadioBandWarning(QString s)
{
    ui->bandWarnLabel->setText(s);
}


void RigControlFrame::setRitEnableState(bool s)
{
    ui->RitButton->setVisible(s);
    ui->RitEdit->setVisible(s);
    ui->RitClear->setVisible(s);
    ui->RitGroupBox->setVisible(s);
    ritEnable = s;
    if (s)
    {
        ritButtonOff();
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



void RigControlFrame::ritFreqEditShortCutInFocus()
{
    ui->RitEdit->setFocus();
    ui->RitEdit->setCursorPosition(3);
}


void RigControlFrame::ritLineEditInFocus()
{
    traceMsg(QString("Rit LineEdit in Focus"));
    if (ritOn)
    {
        ritEditOn = true;
        ui->RitEdit->setReadOnly(false);
        ui->RitEdit->setRitOnFlag(true);        // prevent updates from rigcontrol
        ritFreqLineEditFrameColour(true);
    }
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


void RigControlFrame::ritFreqLineEditFrameColour(bool status)
{
    int curPos = ui->RitEdit->cursorPosition();
    if (status)
    {
        ui->RitEdit->setStyleSheet("border: 1px solid red");
        // restore cursor selection
        ui->RitEdit->setSelection(curPos, 1);
    }
    else
    {
        ui->RitEdit->setStyleSheet("border: 1px solid black");

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
           else if (!radioConnected && radioName.trimmed().isEmpty())
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
            freq -= incFreq;    // never used...
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


void RigControlFrame::closeContest()
{
    // is this the last frame
    if (LogContainer->getLogFrameCount() == 1 && radioConnected)
    {
        trace(QString("Last contest frame closing, close radio connection"));
        setRadioName("", ct->currentMode.getValue());
    }
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
        else if (!radioConnected && radioName.trimmed().isEmpty())
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

    runButtonMap[buttonNumber]->memButton->setText("R" + QString::number(buttonNumber + 1) + "(" + sc + ") " + "." + extractKhz(m.freq) + " ");
    QString tTipStr = "Freq: " + convertFreqStrDisp(m.freq) + "\n"
            + "Mode: " + m.mode + "\n";

    runButtonMap[buttonNumber]->memButton->setToolTip(tTipStr);
}
//********************** Tune point Buttons *******************************


void RigControlFrame::initTuneMemoryButton()
{
    tuneButtonMap[0] = new TuneMemoryButton(ui->TuneButton1, this, 0);
    tuneButtonMap[1] = new TuneMemoryButton(ui->TuneButton2, this, 1);

    updateTuneButtons();
}


void RigControlFrame::tuneButReadActSel(int buttonNumber)
{
    traceMsg(QString("Tune Button Read Selected = %1").arg(QString::number(buttonNumber + 1)));
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    curTuneButton = tuneButtonMap[buttonNumber];
    if (isRadioLoaded())
    {
        if (radioConnected && !radioError)
        {
            ui->freqInput->clearFocus();
            if (curTuneButton->freq.remove('.') != curFreq.remove('.'))
            {

                tslf->sendTpm(buttonNumber + 1, curTuneButton->freq);
            }
        }
    }
    updateTuneButtons();
}



void RigControlFrame::tuneButWriteActSel(int buttonNumber)
{
    traceMsg(QString("Tune Button Write Selected = %1").arg(QString::number(buttonNumber + 1)));
    curTuneButton = tuneButtonMap[buttonNumber];
    curTuneButton->freq = curFreq;
    updateTuneButtons();
}

void RigControlFrame::updateTuneButtons()
{
    for (int i = 0; i < tuneButData::NUM_TUNEBUTTONS; i++)
    {
        tuneButtonUpdate(i);
    }
}
void RigControlFrame::tuneButtonUpdate(int buttonNumber)
{
    QString star = "  ";
    if (curTuneButton == tuneButtonMap[buttonNumber])
        star = "*";
    tuneButtonMap[buttonNumber]->memButton->setText(star + "." + extractKhz(tuneButtonMap[buttonNumber]->freq) + star);
    QString tTipStr = "Freq: " + convertFreqStrDisp(tuneButtonMap[buttonNumber]->freq);

    tuneButtonMap[buttonNumber]->memButton->setToolTip(tTipStr);
}
//-----------------------------------------------------------------------------------
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

void RigControlFrame::checkConnection()
{
    QString loggerUuid = LogContainer->sendDM->getLoggerUuid();
    PubSubName rigSelected = LogContainer->sendDM->getSelectedRig(loggerUuid);
    if (rigSelected.isEmpty())
    {
        // clear the rig selection

        ui->radioNameSel->setCurrentText("");
        setRadioState(RIG_STATUS_DISCONNECTED);
    }
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
    writeAction = new QAction("&New",memButton);
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

//*******************Run Memory Button *************************//

TuneMemoryButton::TuneMemoryButton(QToolButton *b, RigControlFrame *rcf, int no)
{
    memNo = no;
    rigControlFrame = rcf;

    memButton = b;

    memoryMenu = new QMenu(memButton);

    memButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    memButton->setPopupMode(QToolButton::MenuButtonPopup);
    memButton->setFocusPolicy(Qt::NoFocus);

    readAction = new QAction("&Read", memButton);
    writeAction = new QAction("&Write",memButton);
    memoryMenu->addAction(readAction);
    memoryMenu->addAction(writeAction);
    memButton->setMenu(memoryMenu);

    connect(memButton, SIGNAL(clicked(bool)), this, SLOT(readActionSelected()));
    connect( readAction, SIGNAL( triggered() ), this, SLOT(readActionSelected()) );
    connect( writeAction, SIGNAL( triggered() ), this, SLOT(writeActionSelected()) );
}
TuneMemoryButton::~TuneMemoryButton()
{
//    delete memButton;
}

void TuneMemoryButton::readActionSelected()
{
    rigControlFrame->tuneButReadActSel(memNo);
}

void TuneMemoryButton::writeActionSelected()
{
    rigControlFrame->tuneButWriteActSel(memNo);
}
