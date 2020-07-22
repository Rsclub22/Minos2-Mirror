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


const QString RIT_BUTTON_ON_STYLE = QString("background-color: Sandybrown ;\n");
const QString RIT_BUTTON_OFF_STYLE = QString("background-color: Gainsboro ;\n");

const QStringList USB_TUNING_STEPS = {"5 KHz", "10 KHz", "15 KHz", "20 Khz"};
const QStringList MGM_TUNING_STEPS = {"100 Hz", "200 Hz", "300 Hz", "400 Hz", "500 Hz"};
const QStringList CW_TUNING_STEPS = {"100 Hz", "200 Hz", "300 Hz", "400 Hz", "500 Hz"};
const QStringList FM_TUNING_STEPS = {"5 KHz", "10 KHz", "12.5 KHz", "25 KHz"};
const int USB_DEFAULT_STEP = 0;
const int MGM_DEFAULT_STEP = 4;
const int CW_DEFAULT_STEP = 4;
const int FM_DEFAULT_STEP = 2;

RigControlFrame::RigControlFrame(QWidget *parent):
    QFrame(parent),
    ui(new Ui::RigControlFrame),
    ct(nullptr),
    radioLoaded(false),
    radioConnected(false),
    radioError(false),
    freqEditOn(false),
    curFreq(ZEROFREQ),
    lastFreq(ZEROFREQ),
    disconnectFreq(ZEROFREQ),
    curFStepButtonsFreq(0),
    curMode(""),
    ritEnable(false),
    ritOn(false),
    ritEditOn(false),
    maxRitFreq(MAX_RITFREQ),
    radioState("None"),
    onContestPageChangedFlag(false),
    ignorePreviousFreqFlag(false),
    ignorePresetFreqFlag(false)

{
    ui->setupUi(this);

    // allow capture of events from these widgets
    ui->freqInput->installEventFilter(this  );
    ui->RitEdit->installEventFilter(this  );

    initRigFrame(parent);

    showRitButOff();

    mgmLabelVisible(false);

    setRadioTxVertEnabled(false);
    setRitEnableState(false);
    setRadioVolumeState(false);

    // init memory button data before radio connection
    setRadioName(radioName, "");

    freqEditShortKey = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F), parent);
    connect(freqEditShortKey, SIGNAL(activated()), this, SLOT(freqEditSelected()));

    connect(ui->freqStepCombo, SIGNAL(currentIndexChanged(const QString)), this, SLOT(freqStepComboChanged(const QString)));

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


    operatingFreq = new CheckOperatingFreq();
    if (operatingFreq->loadFile("./Configuration/operating_frequencies.json"))
    {
        trace(QString("RigControl Frame: Operating frequency bandplan loaded OK"));
        operatingFreqPlanOk = true;
    }
    else
    {
        trace(QString("RigControl Frame: Operating frequency bandplan failed to load"));
        operatingFreqPlanOk = false;
    }

    freqDisplayPalette = new QPalette();       // to change colour when tuning

    setRitMaxKHzFreq(MAX_RITFREQ); // initial maxRitFreq

    // start timer to wait for bandlist and rigdetails to launch
    launchRadioSelectTimer = new QTimer(this);
    launchRadioSelectCount = 5;     // wait five seconds
    connect(launchRadioSelectTimer, SIGNAL(timeout()), this, SLOT(checkRigDetailsAvail()));
    launchRadioSelectTimer->start(1000);


//    trace(QString("constructor: launchRadioSelectTimer 1000 count is %1 contest n/a").arg(launchRadioSelectCount));


}

RigControlFrame::~RigControlFrame()
{
    delete ui;

    delete operatingFreq;
    delete freqDisplayPalette;
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

void RigControlFrame::clearFreqInputFocus()
{
    ui->freqInput->clearFocus();
}

void RigControlFrame::checkRigDetailsAvail()
{

    launchRadioSelectCount--;
    if (launchRadioSelectCount == 0)
    {
//        trace(QString("checkRigDetailsAvail: launchRadioSelectTimer stopped count is %1 contest %2").arg(launchRadioSelectCount).arg(ct->uuid));
        // timed out waiting for rigdetails
        launchRadioSelectTimer->stop();
        trace(QString("rigControlFrame: Timed out waiting for rigdetails"));
        return;
    }
    else if (ct && ( ct->isProtected() || ct != TContestApp::getContestApp() ->getCurrentContest()))
    {
        //trace(QString("checkRigDetailsAvail: launchRadioSelectTimer stopped protected or not current contest count is %1 contest %2").arg(launchRadioSelectCount).arg(ct->uuid));
        //launchRadioSelectTimer->stop();
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
//                   trace(QString("checkRigDetailsAvail: launchRadioSelectTimer stopped rig found count is %1 contest %2").arg(launchRadioSelectCount).arg(ct->uuid));
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

    trace(QString("on radioNameSel activated: radioName - %1 requested ***").arg(arg1));
    setRadioName(arg1, ct->currentMode.getValue());



}


void RigControlFrame::setTransVertOffset(double offset, PubSubName psn)
{
    traceMsg(QString("set transvertOffset %1 for radio %2").arg(QString::number(offset)).arg(psn.toString()));
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
    traceMsg(QString("set transvertSwitch Number = %1 for radio %2").arg(QString::number(switchNum)).arg(psn.toString()));

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
    traceMsg(QString("set transvertEnabled = %1 for radio %2").arg(status ? "True" : "False").arg(psn.toString()));

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

    traceMsg(QString("set transvertStatus = %1 for radio %2").arg(status ? "True" : "False").arg(psn.toString()));

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
    traceMsg(QString("set volumeStatus = %1 for radio %2").arg(status ? "True" : "False").arg(psn.toString()));

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
    traceMsg(QString("set RitEnableStatus = %1 for radio %2").arg(status ? "True" : "False").arg(psn.toString()));

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

void RigControlFrame::setRitMaxKHzFreq(int maxRitFreq, PubSubName psn)
{
    traceMsg(QString("set RitMaxKHzFreq = %1 for radio %2").arg(maxRitFreq).arg(psn.toString()));
    RadioDetails rd;
    if (allRadioDetails.contains(psn))
    {
        rd = allRadioDetails[psn];
        rd.setRitMaxKHzFreq(maxRitFreq);
        allRadioDetails[psn] = rd;
    }
    else
    {
        rd.setRitMaxKHzFreq(maxRitFreq);
        allRadioDetails[psn] = rd;
    }

    if (psn == selRadioName && ct && !ct->isProtected() && ct == TContestApp::getContestApp() ->getCurrentContest())
    {
        setRitMaxKHzFreq(maxRitFreq);
        selRadioDetails.setRitMaxKHzFreq(maxRitFreq);
    }
}





void RigControlFrame::setBandList(QString s,PubSubName psn)
{
    traceMsg(QString("set BandList = %1 for radio %2").arg(s).arg(psn.toString()));


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

    if (lastFreq != freq)
    {
        lastFreq = freq;
    }
    if (freq.count() >= 4)
    {
        displayFreqOnFreqEditDisplay(freq);
        curFreq = freq;
        emit setFreqDisplay(freq, legalFreq);
    }
    // an error here?

}


void RigControlFrame::displayFreqOnFreqEditDisplay(QString freq)
{

    if (checkValidBand(freq))  // prevent a crash with invalid freq
    {
        if (!freqEditOn)
        {
            trace(QString("displayFreqOnFreqEditDisplay: Freq = %1").arg(freq));
            ui->freqInput->setInputMask(maskData::freqMask[freq.count() - 4]);
            setFreqTextLegalColour(freq, curMode);
            ui->freqInput->setText(freq);

        }
    }


}


// from rigcontrol

void RigControlFrame::setRitFreq(int freq)
{
    if (!ritEditOn)
    {
      ui->RitEdit->setText(convertRitFreqToStr(freq, ritKHzFlag));
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
    traceMsg(QString("Change Rit Freq = %1").arg(convertRitFreqToStr(freq, ritKHzFlag)));
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
        QString sfreq = convertRitFreqToStr(0, ritKHzFlag);       // set rit display to zero
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
    ui->RitButton->setText(tr("On"));
}

void RigControlFrame::showRitButOff()
{
    //ui->Rotate->setPalette(*blackText);
    ui->RitButton->setStyleSheet(RIT_BUTTON_OFF_STYLE);
    ui->RitButton->setText(tr("Off"));
}




void RigControlFrame::changeMainRadioFreq()
{
    traceMsg(QString("Change Main Radio Freq"));


    QString newFreqStr = ui->freqInput->text();
    QString newFreq = newFreqStr.trimmed().remove('.');

    // check legal freq
    setFreqTextLegalColour(newFreq, curMode);

    double f = convertStrToFreq(newFreq);


    if (f >= 0.0)
    {
        if (f > 0)
        {
            newFreq.remove( QRegExp("^[0]*")); //remove periods and leading zeros
        }

        if (newFreq != lastFreq)
        {
            lastFreq = newFreq;
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
                QString f =HtmlFontColour(Qt::red) + lastFreq;
                trace("changeMainRadioFreq " + f);
                ui->freqInput->setText(f);
                emit setFreqDisplay(f, legalFreq);
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

QString RigControlFrame::getCurFreq() const
{
    return curFreq;
}

void RigControlFrame::exitFreqEdit()
{
    traceMsg(QString("Exit Edit Freq"));
    freqEditOn = false;

    freqLineEditFrameColour(false);
    QString freq = ui->freqInput->text();
    if (freq.remove('.') != curFreq)
    {
        // up date display to current radio freq
        ui->freqInput->setInputMask(maskData::freqMask[curFreq.count() - 4]);
        ui->freqInput->setText(curFreq);
        emit setFreqDisplay(curFreq, legalFreq);
    }


    ui->freqInput->clearFocus();
    setFreqTextLegalColour(curFreq, curMode);
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


void RigControlFrame::setFreqStepCombo(QString mode)
{
    if (mode == "USB")
    {
        ui->freqStepCombo->clear();
        ui->freqStepCombo->addItems(USB_TUNING_STEPS);
        ui->freqStepCombo->setCurrentIndex(USB_DEFAULT_STEP);
    }
    else if (mode == "FM")
    {
        ui->freqStepCombo->clear();
        ui->freqStepCombo->addItems(FM_TUNING_STEPS);
        ui->freqStepCombo->setCurrentIndex(FM_DEFAULT_STEP);
    }
    else if (mode == "CW")
    {
        ui->freqStepCombo->clear();
        ui->freqStepCombo->addItems(CW_TUNING_STEPS);
        ui->freqStepCombo->setCurrentIndex(CW_DEFAULT_STEP);
    }
    else if (mode == "MGM")
    {
        ui->freqStepCombo->clear();
        ui->freqStepCombo->addItems(MGM_TUNING_STEPS);
        ui->freqStepCombo->setCurrentIndex(MGM_DEFAULT_STEP);
    }


    curFStepButtonsFreq = getStepFreqFromComboText(ui->freqStepCombo->currentText());

}

void RigControlFrame::freqStepComboChanged(const QString step)
{
    curFStepButtonsFreq = getStepFreqFromComboText(step);

}


double RigControlFrame::getStepFreqFromComboText(const QString step)
{

    double stepF = 0;
    QStringList sl = step.split(" ");

    if (sl.count() != 2)
    {
        stepF = 0;
    }
    else
    {
        if (curMode == "USB" || curMode == "FM")
        {
            stepF = sl[0].toDouble() * 1000;
        }
        else
        {
            stepF = sl[0].toDouble();
        }
    }

    return stepF;
}


void RigControlFrame::setMode(QString m)
{

    traceMsg(QString("Set Mode = %1").arg(m));
    QStringList mode = m.split(':');
    if (mode.length() == 2 )
    {
        for (int i = 0; i < supModeList.count(); i++)
        {
                if (mode[0] == supModeList[i])
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
                    setFreqStepCombo(curMode);
                   return;
                }
        }


        // mode not supported by minos
        ui->modelbl->setText(HtmlFontColour(Qt::red) + "Mode Error");
    }


}


void RigControlFrame::sendModeToRadio(QString m)
{
    traceMsg(QString("Send Mode to Radio = %1").arg(m));
    emit sendModeToControl(QString("%1").arg(m));

}


void RigControlFrame::setRadioName(QString radNam, QString mode)
{


    traceMsg(QString("setRadioName: Set RadioName = %1, mode = %2, contest = %3").arg(radNam).arg(mode).arg(ct ? ct->uuid : ""));


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
                TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();

                trace(QString("setRadioName: contest = %1, radioName = %2, contestMode = %3, savedMode = %4")
                      .arg(ct ? ct->uuid : "")
                      .arg(radioName)
                      .arg(mode)
                      .arg(tslf->sSavedCurMode));
                //qDebug() << "setRadioNAme " << "contest = " << ct->uuid << " contest mode = " << mode << " saved mode = " << tslf->sSavedCurMode;


                selRadioName = PubSubName(radioName);


                if (allRadioDetails.contains(selRadioName))
                {
                    selRadioDetails = allRadioDetails[selRadioName];
                    traceMsg(QString("setRadioName:: Select Radio - radio details for %1 in allRadioDetails").arg(radioName));
                    createActiveBandList(selRadioDetails.getBandList());

                    bool restoreModeFlag = false;
                    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpContestChangeRestoreContestMode, restoreModeFlag );



                    if (onContestPageChangedFlag && !restoreModeFlag)
                    {
                        traceMsg(QString("setRadioName: onContestPageChangedFlag = %1, restoreModeFlag = %2").arg(onContestPageChangedFlag ? "true" : "false").arg(restoreModeFlag ? "true" : "false"));

                        QString m = tslf->sSavedCurMode;

                        traceMsg(QString("setRadioName: SavedCurMode = %1").arg(tslf->sSavedCurMode));
                        if (m.contains(':') && m.contains("MGM"))
                        {
                            QStringList ml = m.split(':');
                            if (ml.count() == 2)
                            {
                                m = ml[0].remove(':').trimmed();
                            }

                        }

                        if (m.isEmpty())
                        {
                            m = mode;
                            traceMsg(QString("setRadioName: saved mode is empty - revert to contest mode = %1").arg(m));
                            //qDebug() << "saved mode is empty - revert to contest mode = " << m;
                        }
                        traceMsg(QString("setRadioName: sending radioName = %1, mode = %2").arg(radioName).arg(m));
                        //qDebug() << "setRadioNAme " << "contest = " << ct->uuid << " setting save mode = " << m;
                        emit selectRadio(radioName, m.remove(':'));  // send radio and previous mode.
                    }
                    else
                    {
                        traceMsg(QString("setRadioName: Not onContestPageChange = %1, radioName = %2, mode =%3").arg(onContestPageChangedFlag ? "true" : "false").arg(radioName).arg(mode));
                        //qDebug() << "setRadioNAme " << "contest = " << ct->uuid << " setting contest mode = " << mode;
                        emit selectRadio(radioName, mode.remove(':'));
                    }


                    setRadioFreq();


                }
                else
                {
                    if (!radioName.isEmpty() && isRadioLoaded())
                    {

                        trace(QString("setRadioName:: Select Radio for %1 , Bandlist is empty = %1").arg(radioName));
                        setRadioBandWarning(HtmlFontColour(Qt::red) + tr("Bandlist empty for this radio, please add a band or tranverter!"));
                        // clear selection in rigcontrol
                        emit selectRadio(radioName, mode);  // send radio and mode.
                        selRadioDetails = RadioDetails();
                        createActiveBandList(selRadioDetails.getBandList());
                    }
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
    traceMsg(QString("setRadioFreq: enter function"));

    if (selRadioDetails.getBandList().isEmpty())
    {
        setRadioBandWarning(HtmlFontColour(Qt::red) + tr("Radio Bandlist is empty!"));
        traceMsg(QString("setRadioFreq:: Radio Bandlist is empty!"));
        sendFreq(NO_BAND_SUPPORT);
        return;
    }


    if (listOfBands.isEmpty())
    {
        setRadioBandWarning(HtmlFontColour(Qt::red) + tr("Radio has no available bands"));
        traceMsg(QString("setRadioFreq:: Error No available bands!"));
        sendFreq(NO_BAND_SUPPORT);
        return;
    }

    setRadioBandWarning(QString(""));
    traceMsg(QString("setRadioFreq: list of bands for radio %1 is %2").arg(selRadioName.toString()).arg(selRadioDetails.getBandList()));

    if (ct == TContestApp::getContestApp() ->getCurrentContest())
    {


       if (onContestPageChangedFlag && curFreq == ZEROFREQ) // is it contest frame starting
       {
           onContestPageChangedFlag = false;

           traceMsg(QString("setRadioFreq: curFreq default - starting contest"));


           ignorePresetFreqFlag = readIgnorePresetFreqFlag();
           if (ignorePresetFreqFlag)
           {
               traceMsg(QString("setRadioFreq: ignoring preset freq"));
               return;
           }
           else
           {
               traceMsg(QString("setRadioFreq: using preset freq"));
           }

           //We want to select the frequency based on the contest band


           QString cb = ct->contestBands.getValue().trimmed();
           traceMsg(QString("setRadioFreq: contest band = %1").arg(cb));

           BandList &blist = BandList::getBandList();
           BandInfo bi;
           bool bandOK = blist.findBand(cb, bi);
           if (bandOK)
           {




               for (int i = 0; i < listOfBands.size(); i++)
               {
                   if (listOfBands[i].band == cb)
                   {
                       traceMsg(QString("setRadioFreq: found band %1 on radio, set band select").arg(cb));
                       ui->bandSelCombo->setCurrentIndex(i + 1);

                       QString freq = listOfBands[i].freq;

                       traceMsg(QString("setRadioFreq: set preset freq = %1").arg(freq));
                       if (checkValidFreq(freq))
                       {
                           traceMsg(QString("setRadioFreq: freq valid = %1, send freq to rigcontrol").arg(freq));
                           sendFreq(freq);

                       }
                       else
                       {
                           traceMsg(QString("setRadioFreq: freq not valid = %1").arg(freq));
                           //sendFreq(freq);

                       }

                       return;

                   }

               }
           }
           else
           {
               // warn no band for this radio
               setRadioBandWarning(HtmlFontColour(Qt::red) + tr("No %1 Band found for this radio!").arg(cb));
               traceMsg(QString("SsetRadioFreq: %1 Band not found on this radio").arg(cb));
               sendFreq(NO_BAND_SUPPORT);
           }



       }
       else if (onContestPageChangedFlag && curFreq != ZEROFREQ)
       {
           onContestPageChangedFlag = false;

           QString freq;
           traceMsg(QString("setRadioFreq: onContestPageChanged and not default curFreq"));
           TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
           ignorePreviousFreqFlag = readIgnorePreviousFreqFlag();
           if (ignorePreviousFreqFlag)
           {
               traceMsg(QString("setRadioFreq: ignorePreviousFreqflag set "));
               return;
           }
           else
           {
               freq = tslf->sSavedCurFreq;
               if (checkValidFreq(freq))
               {
                   traceMsg(QString("setRadioFreq: freq valid = %1, send freq to rigcontrol").arg(freq));

                   displayFreqOnFreqEditDisplay(freq);

                   sendFreq(freq);

                   return;
               }
               else
               {
                   traceMsg(QString("setRadioFreq: freq not valid = %1").arg(freq));
                   //sendFreq(freq);
               }
           }
       }
    }
    else
    {
        traceMsg(QString("setRadioFreq: contest isn't the current contest!"));

    }


}

// restore after rigcontrol restart

void RigControlFrame::restoreRadioFreq()
{
    if (ct == TContestApp::getContestApp() ->getCurrentContest())
    {

        QString freq = disconnectFreq;
        traceMsg(QString("restorRadioFreq: restore Freq for this contest"));
/*
        bool state;
        TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpContestChangeIgnorePreviousFreq, state );
        if (state)
        {
           traceMsg(QString("restoreRadioFreq: Ignore Previous Freq Set, skip restor freq"));
           return;
        }
*/
        if (freq != ZEROFREQ)
        {
            disconnectFreq = ZEROFREQ;
            QString cb = ct->contestBands.getValue().trimmed();
            int err = setBandSelComboIndex(cb);
            if (err >= 0 )
            {

                if (checkValidFreq(freq))
                {
                  traceMsg(QString("restoreRadioFreq: restoring this freq = %1").arg(freq));
                  sendFreq(freq);
                }
                else
                {
                   traceMsg(QString("restoreRadioFreq: freq %1 invalid").arg(curFreq));
                }

            }
            else
            {
                traceMsg(QString("restoreRadioFreq: error finding band =  %1, in bandsel combo").arg(cb));
            }

        }

    }



}


int RigControlFrame::setBandSelComboIndex(QString band)
{


    if (ui->bandSelCombo->count() > 0)
    {
        for (int i = 0; i < ui->bandSelCombo->count(); i++)
        {
            if (ui->bandSelCombo->itemText(i) == band)
            {
                ui->bandSelCombo->setCurrentIndex(i);
                return 0;
            }
        }
    }


    return -1; //error


}

void RigControlFrame::setRadioList()
{
    if (ct && !ct->isProtected() /*&& ct == TContestApp::getContestApp() ->getCurrentContest()*/)
    {
        listOfRadios = LogContainer->sendDM->rigs();

        ui->radioNameSel->clear();
        ui->radioNameSel->addItem("");
        ui->radioNameSel->addItems(listOfRadios);

        if  (ui->radioNameSel->count() > 0 && !launchRadioSelectTimer->isActive())
        {
            trace(QString("setRadioList: rigControl restart? reconnecting"));
            launchRadioSelectCount = 5;     // wait five seconds
            launchRadioSelectTimer->start(1000);
//            trace(QString("setRadioList: launchRadioSelectTimer restarted count is %1 contest %2").arg(launchRadioSelectCount).arg(ct->uuid));
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
    else
    {
        ui->bandSelCombo->clear();
    }
}







void RigControlFrame::setRadioState(QString s)
{
    traceMsg(QString("Set RadioState = %1").arg(s));

    if (s != "")
    {

        if (s.contains(RIG_STATUS_ERROR))
        {
           radioError = true;
           QStringList sl = s.split(':');
           if (sl.count() == 2)
           {
               ui->rigState->setText(HtmlFontColour(Qt::red) + tr("Error: %1").arg(sl[1]));
               emit radioHasError(sl[1]);
           }
        }
        else if (s == RIG_STATUS_CONNECTED)
        {
            radioConnected = true;
            ui->rigState->setText(tr("Connected"));
            int index = ui->radioNameSel->findText(radioName, Qt::MatchFixedString);
            if (index >= 0)
            {
                ui->radioNameSel->setCurrentIndex(index);
                restoreRadioFreq();
                emit radioIsConnected(true);
            }
            else
            {
                trace(QString("setRadioName: Can't find %1 in radioNameSel").arg(radioName));
            }

        }
        else if (s == RIG_STATUS_DISCONNECTED)
        {
           radioConnected = false;
           radioError = false;

           ui->rigState->setText(tr("Disconnected"));

           disconnectFreq = lastFreq;

           ui->bandWarnLabel->setText("");

           if (ui->radioNameSel->currentText() == "")
           {
               curFreq = ZEROFREQ;
               ui->freqInput->setInputMask(maskData::freqMask[curFreq.count() - 4]);
               ui->freqInput->setText(curFreq);
               emit setFreqDisplay(curFreq, legalFreq);
               ui->bandSelCombo->clear();
           }




           emit radioIsConnected(false);
           emit radioDisconnected();

        }





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

void RigControlFrame::setRitMaxKHzFreq(int maxRitFreq_)
{
    maxRitFreq = maxRitFreq_;
    if (maxRitFreq >= 10000)
    {
        ui->RitEdit->setTensKhz(true);
        ui->RitEdit->setInputMask("x99.99");
        ui->RitEdit->setText("+00.00");
        ritKHzFlag = true;

    }
    else
    {
        ui->RitEdit->setTensKhz(false);
        ui->RitEdit->setInputMask("x9.99");
        ui->RitEdit->setText("+0.00");
        ritKHzFlag = false;
    }

    ui->RitEdit->setMaxRit(maxRitFreq);
    ui->RitEdit->setMinRit(maxRitFreq * -1);
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
    for (int i = 0; i < supModeList.count(); i++ )
    {
        if (mode == supModeList[i])
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
    setFreqTextLegalColour(curFreq, curMode);
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
        ui->freqInput->setStyleSheet("border: 1px solid magenta");
        // restore cursor selection
        ui->freqInput->setSelection(curPos, 1);
    }
    else
    {
        ui->freqInput->setStyleSheet("border: 1px solid black");

    }

    //QString freq = ui->freqInput->text();
    //setFreqTextLegalColour(freq, curMode);

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

    freqPlusMinusButton(curFStepButtonsFreq);

}



void RigControlFrame::freqNegShortCut_clicked(bool /*click*/)
{

    freqPlusMinusButton(curFStepButtonsFreq * -1);

}


void RigControlFrame::freqPlusMinusButton(double f)
{

    if (isRadioLoaded())
    {
        QString freq = calcNewFreq(f);
        if (freq != "")
        {

            setFreqTextLegalColour(freq, curMode);
            ui->freqInput->setText(freq);
            emit setFreqDisplay(freq, legalFreq);

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

void RigControlFrame::setFreqTextLegalColour(const QString _freq, QString mode)
{
    QString freq = _freq;
    double f = freq.remove('.').toDouble();

    if (checkFreqIsLegal(f, mode))
    {

        freqDisplayPalette->setColor(QPalette::Text, Qt::black);
        ui->freqInput->setPalette(*freqDisplayPalette);
        legalFreq = true;
    }
    else
    {
        freqDisplayPalette->setColor(QPalette::Text,Qt::red);
        ui->freqInput->setPalette(*freqDisplayPalette);
        legalFreq = false;
    }

}





bool RigControlFrame::checkFreqIsLegal(const double freq, const QString mode)
{
    BandList &blist = BandList::getBandList();
    BandInfo bi;
    bool bandOk = false;


    bandOk = blist.findBand(freq, bi);
    QString band = bi.uk;
    if (bandOk)
    {
        return isFreqLegal(freq, band, mode);
    }

    return false;  // out of band


}





// returns true if freq ok, false if it is not...it will return true and error to tracelog is mode or band
// is missing from operating freq file
bool RigControlFrame::isFreqLegal(const double freq, const QString band, const QString mode)
{

    int retCode;
    if (operatingFreqPlanOk)
    {
            retCode =  operatingFreq->freqValid(band, mode, freq);
            switch (retCode)
            {
                case FREQ_NOT_OK:
                    return false;
                case FREQ_OK:
                    return true;
                case FREQ_NO_MATCH:
                    return false;
                case MODE_MISSING:
                    trace(QString("RigControl Frame isFreqLegal: mode is missing from file - band %1, mode %2").arg(band).arg(mode));
                    return true;
                case BAND_MISSING:
                    trace(QString("Bandmap isFreqLegal: band is missing from file - band %1, mode %2").arg(band).arg(mode));
                    return true;
            }
    }

    trace(QString("RigControl Frame isFreqLegal: Operating Freq file not loaded"));
    return true;

}

void RigControlFrame::closeContest()
{
//    // is this the last frame
//    if (LogContainer->getLogFrameCount() == 1 && radioConnected)
//    {
//        trace(QString("Last contest frame closing, close radio connection"));
//        setRadioName("", ct->currentMode.getValue());
//    }
}

void RigControlFrame::traceMsg(QString msg)
{
    trace(QString("RigcontrolFrame: %1 - %2 ").arg(radioName).arg(msg));
}



bool RigControlFrame::readIgnorePresetFreqFlag()
{

    bool state;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpContestStartIgnorePresetFreq, state );
    return state;
}



bool RigControlFrame::readIgnorePreviousFreqFlag()
{

    bool state;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpContestChangeIgnorePreviousFreq, state );
    return state;
}





//-----------------------------------------------------------------------------------

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


