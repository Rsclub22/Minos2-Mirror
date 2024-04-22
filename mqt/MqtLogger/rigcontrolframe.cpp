/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2017 - 2020
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2019
//
//
//
/////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include "ContestApp.h"
#include "MShowMessageDlg.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "SendRPCDM.h"
#include "BandList.h"
#include "rigutils.h"
#include "LoggerContest.h"
#include "volumeslider.h"
#include "freqlineedit.h"
#include "ritlineedit.h"
#include "delayedaction.h"
#include "MTrace.h"

#include "rigcontrolframe.h"
#include "ui_rigcontrolframe.h"

RigControlFrame::RigControlFrame(TSingleLogFrame *parent):
    QFrame(parent),
    ui(new Ui::RigControlFrame),
    ct(nullptr),
    tslf(parent),
    radioConnected(false),
    radioError(false),
    freqEditOn(false),
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
    traceMsg(QString("RigControlFrame Started"));

    // allow capture of events from these widgets
    ui->freqInput->installEventFilter(this  );
    ui->RitEdit->installEventFilter(this  );

    rigFrameStartFlag = true;

    initRigFrame(parent);

    showRitButOff();

    mgmLabelVisible(false);

    setRadioTxVertEnabled(false);
    setRitEnableState(false);
    setRadioVolumeState(false);

    BandList::getBandList().loadAllBands(bands);

    freqEditShortKey = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), parent);
    connect(freqEditShortKey, &QShortcut::activated, this, &RigControlFrame::freqEditSelected);

    connect(ui->freqStepCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &RigControlFrame::freqStepComboChanged);

    freqPlusShortCut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_U), parent);
    connect(freqPlusShortCut, &QShortcut::activated, this, &RigControlFrame::freqPlus_ShortCut);
    connect(ui->freqUp, &QToolButton::clicked, this, &RigControlFrame::freqPlusShortCut_clicked);

    freqNegShortCut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_D), parent);
    connect(freqNegShortCut, &QShortcut::activated, this, &RigControlFrame::freqNeg_ShortCut);
    connect(ui->freqDown, &QToolButton::clicked, this, &RigControlFrame::freqNegShortCut_clicked);

    // rit key shortcuts
    ritOnOffShortCut = new QShortcut(QKeySequence("Ctrl+o"), parent);
    connect(ritOnOffShortCut, &QShortcut::activated, this, &RigControlFrame::ritButtonSelected);
    ritClearShortCut = new QShortcut(QKeySequence("Ctrl+k"), parent);
    connect(ritClearShortCut, &QShortcut::activated, this, &RigControlFrame::ritClearShortCutSelected);
    ritFreqEditShortCut = new QShortcut(QKeySequence("Ctrl+i"), parent);
    connect(ritFreqEditShortCut, &QShortcut::activated, this, &RigControlFrame::ritFreqEditShortCutInFocus);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::FontChanged, this, &RigControlFrame::on_FontChanged, Qt::QueuedConnection);
    on_FontChanged();

    operatingFreq = new CheckOperatingFreq();
    if (operatingFreq->loadExclusionsFromBandList())
    {
        traceMsg(QString("RigControl Frame: Operating frequency bandplan loaded OK"));
        operatingFreqPlanOk = true;
    }
    else
    {
        traceMsg(QString("RigControl Frame: Operating frequency bandplan failed to load"));
        operatingFreqPlanOk = false;
    }

    freqDisplayPalette = new QPalette();       // to change colour when tuning

    setRitMaxKHzFreq(MAX_RITFREQ); // initial maxRitFreq

    traceMsg(QString("Frame Init Complete - Start timer to wait for rig and contest details"));
    launchRadioSelectTimer = new QTimer(this);
    launchRadioSelectCount = 10;     // wait five seconds
    connect(launchRadioSelectTimer, &QTimer::timeout, this, [=](){checkRigDetailsAvail();});
    launchRadioSelectTimer->start(1000);

    checkFreqContestBandTimer = new QTimer(this);
    connect(checkFreqContestBandTimer, &QTimer::timeout, this, [=](){onCheckContestBandMatch();});
    checkFreqContestBandTimer->start(CHECK_FREQ_MATCH_CONTEST_BAND_TIMEOUT);

    connect(LogContainer->sendDM, &TSendDM::setRadioLoaded, this, &RigControlFrame::on_RadioLoaded);

    setRigControlPanelTitle();

    dumpLoggerRadioOptionSettings();

    ui->resetBandFreqFrame->hide();

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::QSOMargins, this, &RigControlFrame::on_QSOMargins);
    on_QSOMargins();
}

RigControlFrame::~RigControlFrame()
{
    delete ui;

    delete operatingFreq;
    delete freqDisplayPalette;
}
void RigControlFrame::on_QSOMargins()
{
    int ls;
    int cml;
    int cmt;
    int cmr;
    int cmb;

    TContestApp::getContestApp() ->getIntDisplayProfile(edpls, ls);
    TContestApp::getContestApp() ->getIntDisplayProfile(edpcml, cml);
    TContestApp::getContestApp() ->getIntDisplayProfile(edpcmt, cmt);
    TContestApp::getContestApp() ->getIntDisplayProfile(edpcmr, cmr);
    TContestApp::getContestApp() ->getIntDisplayProfile(edpcmb, cmb);

    adjustMargins(layout(), ls, cml, cmt, cmr, cmb);
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
QString RigControlFrame::getSelectedRadio()
{
    return ui->radioNameSel->currentData().toString();
}

bool RigControlFrame::setSelectedRadio(QString s)
{
   QStringList radios;
   int c = ui->radioNameSel->findData(s);
   if (c >= 0)
   {
        ui->radioNameSel->setCurrentIndex(c);
   }
   else
   {
       for (int i = 0; i < ui->radioNameSel->count(); i++)
       {
            radios.append(ui->radioNameSel->itemData(i).toString());
       }
   }
   return c >= 0;
}

void RigControlFrame::checkRigDetailsAvail()
{
    launchRadioSelectCount--;
    traceMsg(QString("Checking RigDetails are available Count = %1").arg(launchRadioSelectCount));
    if (launchRadioSelectCount == 0)
    {
        // timed out waiting for rigdetails
        launchRadioSelectTimer->stop();
        traceMsg(QString("checkRigDetailsAvail: Timed out waiting for rigdetails"));
        rigFrameStartFlag = false;
        return;
    }

    if (ct &&  ct->isReadOnly())
    {
        traceMsg(QString("checkRigDetailsAvail: contest protected or not current contest count is %1 contest %2").arg(launchRadioSelectCount).arg(ct->uuid));
        //launchRadioSelectTimer->stop();
        rigFrameStartFlag = false;
        launchRadioSelectCount = 1; // stop the sequence - rig won't be connecting
        return;

    }

    else if (ct && !ct->isReadOnly() )
    {

        if (!ct->radioName.getValue().toString().isEmpty() )
        {
            traceMsg(QString("checkRigDetailsAvail: contest radio name is %1").arg(ct->radioName.getValue().toString()));
            traceMsg(QString("checkRigDetailsAvail: number of radios in allradioDetails = %1").arg(allRadioDetails.count()));

            if (ui->radioNameSel->findData(ct->radioName.getValue().toString()) != -1)
            {
               traceMsg(QString("checkRigDetailsAvail: radioName in allRadioDetails = %1").arg(ct->radioName.getValue().toString()));
               if (allRadioDetails.contains(ct->radioName.getValue().toString())  && allRadioDetails[ct->radioName.getValue().toString()].getBandListCount() > 0)
               {

                   launchRadioSelectTimer->stop();
                   traceMsg(QString("checkRigDetailsAvail: radio data now available - stop timer"));
                   traceMsg(QString("checkRigDetailsAvail: select radio %1, mode %2").arg(ct->radioName.getValue().toString(), ct->currentMode.getValue()));

                   setRadioName(ct->radioName.getValue().toString(), true);


               }
               else
               {
                   traceMsg(QString("checkRigDetailsAvail: bandlist empty"));
               }
            }
            else
            {
                traceMsg(QString("checkRigDetailsAvail: contest radio missing from radioName Select = %1").arg(ct->radioName.getValue().toString()));
            }

        }
        else
        {
            traceMsg(QString("checkRigDetailsAvail: contest radioname is missing = %1").arg(ct->radioName.getValue().toString()));
        }


    }
    else
    {
        traceMsg(QString("checkRigDetailsAvail: contest nullptr"));
    }
}

void RigControlFrame::setContest(BaseContestLog *c)
{
    ct = dynamic_cast<LoggerContestLog *>( c);

    if (ct)
    {
        contestBand = ct->currentBand.getValue();

        setRadioList();
    }
    else
    {
        if (launchRadioSelectTimer)
        {
            launchRadioSelectTimer->stop();
        }
    }
}

void RigControlFrame::initRigFrame(QWidget * /*parent*/)
{

    ui->modelbl->setText(" ");

    // main freq tuning
    connect(ui->freqInput, &FreqLineEdit::freqEditReturn, this, &RigControlFrame::returnChangeRadioFreq);
    connect(ui->freqInput, &FreqLineEdit::newFreq, this, &RigControlFrame::changeMainRadioFreq);

    // rit freq tuning
    connect(ui->RitButton, &QToolButton::clicked, this, &RigControlFrame::ritButtonSelected);
    connect(ui->RitEdit, &RitLineEdit::newFreq, this, &RigControlFrame::changeRitRadioFreq);
    connect(ui->RitClear, &QToolButton::clicked, this, &RigControlFrame::ritClearButtonSelected);

    // from cluster frame
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::FreqToRig, this, &RigControlFrame::clusterUpdateRigFreq);

    // volume control updates to radio
    connect(ui->volumeSlider, &VolumeSlider::sendVolumeRadio, this, &RigControlFrame::sendVolumeRadio);

    // when no radio is connected
    connect(this, &RigControlFrame::noRadioSendFreq, this, &RigControlFrame::noRadioSetFreq);
    connect(this,&RigControlFrame::noRadioSendMode, this, &RigControlFrame::noRadioSetMode);

    connect(this, &RigControlFrame::radioSwitchCompleted, this, &RigControlFrame::setRadioSwitchCompleted);

    setVolControlVisible(false);

    if (!isRadioLoaded())
    {
        ui->modelbl->setVisible(false);
    }

}

void RigControlFrame::clusterUpdateRigFreq(Frequency freq)
{
    ui->freqInput->clearFocus();
    sendRigFreq(freq);
}

void RigControlFrame::on_radioNameSel_activated(int /*index*/)
{
    // radio combo selected
    radioName = getSelectedRadio();

    traceMsg(QString("on radioNameSel activated: radioName - %1 requested ***").arg(ui->radioNameSel->currentText()));
    setRadioName(radioName, true);       // set true here as we want to act like start and use preset freq, except if a last freq is available
}

void RigControlFrame::setTransVertOffset(double offset, PubSubName psn)
{
    traceMsg(QString("set transvertOffset %1 for radio %2").arg(QString::number(offset), psn.toString()));

    RadioDetails &rd = allRadioDetails[psn];
    rd.setTransVerterOffset(offset);
}

void RigControlFrame::setTransVertSwitch(int switchNum, PubSubName psn)
{
    traceMsg(QString("set transvertSwitch Number = %1 for radio %2").arg(QString::number(switchNum), psn.toString()));

    RadioDetails &rd = allRadioDetails[psn];
    rd.setTransVertSwitch(switchNum);
}

void RigControlFrame::setTransVertEnabled(bool status, PubSubName psn)
{
    traceMsg(QString("set transvertEnabled = %1 for radio %2").arg(status ? "True" : "False", psn.toString()));

    RadioDetails &rd = allRadioDetails[psn];
    rd.setTransVertEnabled(status);
    if (psn == selRadioName && ct && !ct->isReadOnly() && ct == TContestApp::getContestApp() ->getCurrentContest())
    {
        setRadioTxVertEnabled(status);
        selRadioDetails.setTransVertEnabled(status);
    }
}

void RigControlFrame::setTransVertStatus(bool status, PubSubName psn)
{

    traceMsg(QString("set transvertStatus = %1 for radio %2").arg(status ? "True" : "False", psn.toString()));

    RadioDetails &rd = allRadioDetails[psn];
    rd.setTransVertStatus(status);

    if (psn == selRadioName && ct && !ct->isReadOnly() && ct == TContestApp::getContestApp() ->getCurrentContest())
    {
        setRadioTxVertStatus(status);
        selRadioDetails.setTransVertStatus(status);
    }
}

void RigControlFrame::setVolumeStatus(bool status, PubSubName psn)
{
    traceMsg(QString("set volumeStatus = %1 for radio %2").arg((status ? "True" : "False"), psn.toString()));

    RadioDetails &rd = allRadioDetails[psn];
    rd.setVolumeStatus(status);

    if (psn == selRadioName && ct && !ct->isReadOnly() && ct == TContestApp::getContestApp() ->getCurrentContest())
    {
        setRadioVolumeState(status);
        selRadioDetails.setVolumeStatus(status);
    }
}

void RigControlFrame::setRitEnableStatus(bool status, PubSubName psn)
{
    traceMsg(QString("set RitEnableStatus = %1 for radio %2").arg(status ? "True" : "False", psn.toString()));

    RadioDetails &rd = allRadioDetails[psn];
    rd.setRitEnableStatus(status);

    if (psn == selRadioName && ct && !ct->isReadOnly() && ct == TContestApp::getContestApp() ->getCurrentContest())
    {
        setRitEnableState(status);
        selRadioDetails.setRitEnableStatus(status);
    }
}

void RigControlFrame::setRitMaxKHzFreq(int maxRitFreq, PubSubName psn)
{
    traceMsg(QString("set RitMaxKHzFreq = %1 for radio %2").arg(maxRitFreq).arg(psn.toString()));

    RadioDetails &rd = allRadioDetails[psn];
    rd.setRitMaxKHzFreq(maxRitFreq);

    if (psn == selRadioName && ct && !ct->isReadOnly() && ct == TContestApp::getContestApp() ->getCurrentContest())
    {
        setRitMaxKHzFreq(maxRitFreq);
        selRadioDetails.setRitMaxKHzFreq(maxRitFreq);
    }
}

void RigControlFrame::setBandList(QString s,PubSubName psn)
{
    traceMsg(QString("set BandList = %1 for radio %2").arg(s, psn.toString()));

    RadioDetails &rd = allRadioDetails[psn];
    rd.setBandList(s);

    // update bandlist combo if current radio and current contest
    if (psn.toString().toLower() == radioName.toLower() && ct && !ct->isReadOnly())
    {
        createSupportedBandList(s);
    }
}

void RigControlFrame::on_RadioLoaded()
{
    traceMsg(QString("%1 Set Radio Loaded").arg(ct?ct->uuid:""));
    ui->modelbl->setVisible(true);
}

bool RigControlFrame::isRadioLoaded()
{
    return LogContainer->sendDM->isRadioLoaded();
}

void RigControlFrame::noRadioSetFreq(Frequency f)
{
    traceMsg(QString("No Radio SetFreq = %1").arg(f.traceStr()));
    setFreq(f);
}

void RigControlFrame::setFreq(Frequency freq)
{
    traceMsg(QString("Set Freq: = %1").arg(freq.traceStr()));


    traceMsg(QString("setFreq: lastFreq = %1, setFreq = %2").arg(lastFreq.traceStr(), freq.traceStr()));

    if (lastFreq != freq)
    {
        traceMsg(QString("setFreq: update lastFreq = %1, setFreq = %2").arg(lastFreq.traceStr(), freq.traceStr()));
        lastFreq = freq;

        // check freq matches contest band
        delayedAction(this, [=]()
        {
           // NB a lambda function
           onCheckContestBandMatch();
        }, CHECK_CONTEST_FREQ_MATCH_TIMEOUT);


    }
    checkContestBandMatch(freq);        // to show error on panel
    displayFreqOnFreqEditDisplay(freq);
    tslf->bandSwitchFrame->selectButtonGroupAndActiveBand(freq);
    tslf->bandSwitchFrame->setPreviousFreq(curMode, freq);
    curFreq = freq;
    emit setFreqDisplay(freq, legalFreq);
}


void RigControlFrame::displayFreqOnFreqEditDisplay(const Frequency &freq)
{

    if (checkFreqOK(freq))  // prevent a crash with invalid freq
    {

        if (!freqEditOn)
        {
            traceMsg(QString("displayFreqOnFreqEditDisplay: Freq = %1").arg(freq.traceStr()));
            QString sf = freq.str();
            ui->freqInput->setInputMask(maskData::freqMask[sf.size() - 4]);
            setFreqTextLegalColour(freq, curMode);
            ui->freqInput->setLineText(sf);

        }
    }
}


// from rigcontrol

void RigControlFrame::setRitFreq(ShortFreq freq)
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

void RigControlFrame::changeRitRadioFreq(ShortFreq freq)
{
    traceMsg(QString("Change Rit Freq = %1").arg(convertRitFreqToStr(freq, ritKHzFlag)));
    if (ritEnable /*&& ritOn*/)
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

    if (ritEnable /*&& ritOn*/)
    {
        int pos = ui->RitEdit->cursorPosition();
        changeRitRadioFreq(ShortFreq());  // turns off rit in hamlib
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

void RigControlFrame::logRadioSettingsChanged(QSharedPointer<RadioSettingsDialogChangeFlag> /*logRadioSettingsFlags*/)
{
    // update preset freqs
    tslf->bandSwitchFrame->readPresetFreqsFromIni(bands);

    setRigControlPanelTitle();

    dumpLoggerRadioOptionSettings();
}

void RigControlFrame::changeMainRadioFreq()
{
    traceMsg(QString("Change Main Radio Freq"));


    QString newFreqStr = ui->freqInput->text();
    Frequency newFreq(newFreqStr.trimmed().remove('.'));

    // check legal freq
    setFreqTextLegalColour(newFreq, curMode);

    if (!newFreq.isClear())
    {
        if (!newFreq.isClear())
        {
            static QRegularExpression qre("^[0]*");
            newFreqStr.remove( qre); //remove periods and leading zeros
        }

        if (newFreq != lastFreq)
        {

            if (checkValidFreq(newFreq))
            {
                lastFreq = newFreq;
                if (lastFreq.str().size() >= 4)
                {
                    if (isRadioLoaded())
                    {
                        if (radioConnected && !radioError)
                        {
                            sendRigFreq(lastFreq);
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
                traceMsg("changeMainRadioFreq " + lastFreq.traceStr());
                QString f = HtmlFontColour(Qt::red) + lastFreq.str();
                ui->freqInput->setLineText(f);
                emit setFreqDisplay(f, lastFreq);
            }
        }
    }
}


bool RigControlFrame::checkValidFreq(Frequency freq)
{
    BandList &blist = BandList::getBandList();
    QSharedPointer<BandInfo>  bi;
    bool bandOK = false;
    bandOK = blist.findBand(freq, bi);
    return bandOK;
}

void RigControlFrame::returnChangeRadioFreq()
{
    changeMainRadioFreq();
    exitFreqEdit();
}

void RigControlFrame::radioBandFreq(Frequency f)
{
    setRadioBandWarning("");

    if (f != curFreq)
    {

        if (isRadioLoaded())
        {
            if (radioConnected && !radioError)
            {
                sendRigFreq(f);
            }
            else if (!radioConnected && radioName.trimmed() == "No Radio")
            {
                noRadioSendOutFreq(f);
            }

        }
    }
    else
    {
        traceMsg(QString("RigContFrame: Freq the same or index out of range"));
    }
}

void RigControlFrame::sendRigFreq(Frequency f)
{
    emit sendFreqControl(f);
}

void RigControlFrame::onRadioBandChange(QString band)
{
    traceMsg(QString("band change requested %1").arg(band));
    emit sendBandToRigControl(band);
}


void RigControlFrame::noRadioSendOutFreq(Frequency f)
{
    traceMsg(QString("No Radio Send Freq to rigcontrolframe and qsologframe = %1").arg(f.traceStr()));
    // update rigframe
    emit noRadioSendFreq(f);
    // update logger
    tslf->on_NoRadioSetFreq(f);
}


void RigControlFrame::noRadioSendOutMode(QString m)
{
    traceMsg(QString("No Radio Send Mode to to rigcontrolframe and qsologframe = %1").arg(m));
    QString mode = m + ": "; //create mode message, mgm mode is space
    // update rigframe
    emit noRadioSendMode(mode);
    // update logger
    tslf->on_NoRadioSetMode(mode);

}


void RigControlFrame::on_ContestPageChanged()
{

    if (ct && !ct->isReadOnly() && ct == TContestApp::getContestApp() ->getCurrentContest())
    {


        QString radNam = ct->radioName.getValue().toString();
        //QString mode = ct->currentMode.getValue();

        onContestPageChangedFlag = true;


        sendFreq.clear();


        traceMsg(QString("on_ContestPageChanged: radio = %1, uuid = %2").arg(radNam, ct->uuid));
        setRadioName(radNam, false);



    }
}

void RigControlFrame::rigChangedFromDetails()
{
    if (ct && !ct->isReadOnly() && ct == TContestApp::getContestApp() ->getCurrentContest())
    {
        QString radNam = ct->radioName.getValue().toString();

        traceMsg(QString("rigChangedFromDetails: radio = %1, uuid = %2").arg(radNam, ct->uuid));
        setRadioName(radNam, true);
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

Frequency RigControlFrame::getCurFreq() const
{
    return curFreq;
}

void RigControlFrame::exitFreqEdit()
{
    traceMsg(QString("Exit Edit Freq"));
    freqEditOn = false;

    freqLineEditFrameColour(false);
    Frequency freq( ui->freqInput->text().remove('.'));
    if (freq != curFreq)
    {
        // up date display to current radio freq
        QString sf = curFreq.str();
        ui->freqInput->setInputMask(maskData::freqMask[sf.size() - 4]);
        ui->freqInput->setLineText(sf);
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
    traceMsg(QString("Memory Read: Memory Freq = %1, CurFreq = %2, Mode = %3, CurMode = %4")
             .arg(m.freq.traceStr(), curFreq.traceStr(), m.mode, curMode));
    if (isRadioLoaded())
    {
        Frequency mfreq = m.freq;
        if (m.mode.compare( hamlibData::RY, Qt::CaseInsensitive ) == 0)
        {
             mfreq = mfreq - Frequency(RTTY_MARK_OFFSET);
        }
        if (m.mode.compare( hamlibData::PSK, Qt::CaseInsensitive ) == 0)
        {
            mfreq = mfreq + Frequency(BPSK_OFFSET);
        }

        if (radioConnected && !radioError)
        {
            ui->freqInput->clearFocus();
            //if (m.freq.remove('.') != curFreq.remove('.'))
            if (!m.freq.isClear() &&m.freq != curFreq)
            {
                traceMsg(QString("Memory Read: Send Freq"));
                sendRigFreq(m.freq);
            }

            if (!m.mode.isEmpty() && m.mode != curMode)
            {
                traceMsg(QString("Memory Read: Send Mode"));
                sendModeToRadio(m.mode);
            }

        }
        else if (!radioConnected && radioName.trimmed().isEmpty())
        {
            if (!m.freq.isClear())
                noRadioSendOutFreq(m.freq);
            if (!m.mode.isEmpty())
                noRadioSendOutMode(m.mode);
        }
    }

}


void RigControlFrame::getDetails(memoryData::memData &logData)
{
    // get contest information
    ScreenContact &sc = tslf->getScreenEntry();

    logData.callsign = sc.cs.getFullCall();
    logData.freq = curFreq;
    logData.locator = sc.loc.getLoc();
    if (curMode.isEmpty())
    {
        logData.mode = sc.mode.getValue();
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

    if (sc.loc.getLoc().isEmpty())
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

void RigControlFrame::xferFromKST(QString freq)
{
    if (!freq.isEmpty())
    {
        Frequency f(freq);
        setFreq(f);
    }
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
    if (mode == hamlibData::PH)
    {
        ui->freqStepCombo->clear();
        ui->freqStepCombo->addItems(PH_TUNING_STEPS);
        ui->freqStepCombo->setCurrentIndex(PH_DEFAULT_STEP);
    }
    if (mode == hamlibData::USB)
    {
        ui->freqStepCombo->clear();
        ui->freqStepCombo->addItems(USB_TUNING_STEPS);
        ui->freqStepCombo->setCurrentIndex(USB_DEFAULT_STEP);
    }
    else if (mode == hamlibData::FM)
    {
        ui->freqStepCombo->clear();
        ui->freqStepCombo->addItems(FM_TUNING_STEPS);
        ui->freqStepCombo->setCurrentIndex(FM_DEFAULT_STEP);
    }
    else if (mode == hamlibData::RY)
    {
        ui->freqStepCombo->clear();
        ui->freqStepCombo->addItems(RY_TUNING_STEPS);
        ui->freqStepCombo->setCurrentIndex(RY_DEFAULT_STEP);
    }
    else if (mode == hamlibData::PSK)
    {
        ui->freqStepCombo->clear();
        ui->freqStepCombo->addItems(PSK_TUNING_STEPS);
        ui->freqStepCombo->setCurrentIndex(PSK_DEFAULT_STEP);
    }
    else if (mode == hamlibData::CW)
    {
        ui->freqStepCombo->clear();
        ui->freqStepCombo->addItems(CW_TUNING_STEPS);
        ui->freqStepCombo->setCurrentIndex(CW_DEFAULT_STEP);
    }
    else if (mode == hamlibData::MGM)
    {
        ui->freqStepCombo->clear();
        ui->freqStepCombo->addItems(MGM_TUNING_STEPS);
        ui->freqStepCombo->setCurrentIndex(MGM_DEFAULT_STEP);
    }


    curFStepButtonsFreq = getStepFreqFromComboText(ui->freqStepCombo->currentText());

}

void RigControlFrame::freqStepComboChanged(int step)
{
    curFStepButtonsFreq = getStepFreqFromComboText(ui->freqStepCombo->itemText(step));

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
        if (curMode == hamlibData::USB || curMode == hamlibData::LSB || curMode == hamlibData::FM || curMode == hamlibData::PH)
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
    if (ct)
    {
        traceMsg(QString("Set Mode = %1").arg(m));
        QStringList mode = m.split(':');
        if (mode.length() == 2 )
        {
            if (ct->modeList.getValue().contains(mode[0]))
            {
                ui->modelbl->setText(mode[0]);
                curMode = mode[0];
                tslf->bandSwitchFrame->setMode(curMode);
                if (mode[0] == hamlibData::MGM
                        || mode[0] == hamlibData::RY
                        || mode[0] == hamlibData::PSK
                        )
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


            // mode not supported by minos
            ui->modelbl->setText(HtmlFontColour(Qt::red) + "Mode Error");
        }

    }
}


void RigControlFrame::sendModeToRadio(QString m)
{
    traceMsg(QString("Send Mode to Radio = %1").arg(m));
    emit sendModeToControl(QString("%1").arg(m));

}


void RigControlFrame::setRadioName(QString radNam, bool fromStartRigControl)
{



    traceMsg(QString("setRadioName: Set RadioName = %1, contest = %2, contestChangedFlag = %3, rigFrameStartFlag = %4")
             .arg(radNam, ct ? ct->uuid : "",
             onContestPageChangedFlag ? "True" : "False", rigFrameStartFlag ? "True" : "False"));



    if (ct && !ct->isReadOnly() /*&& ct == TContestApp::getContestApp() ->getCurrentContest()*/)
    {
            radioName = radNam;

            selRadioName = PubSubName(radioName);
            sendVmButtonFrameSelectedRadio(selRadioName);

            traceMsg(QString("setRadioName:: set radiocombosel to radioName  %1").arg(radioName));
            traceMsg(QString("setRadioName:: Looking for radio in combo sel"));


            // set Radio Sel Combo
            int index = ui->radioNameSel->findData(radioName);
            if (index >= 0)
            {
                traceMsg(QString("found radio, set radioComboSel to index %1").arg(index));
                ui->radioNameSel->setCurrentIndex(index);
            }
            else
            {
                traceMsg(QString("setRadioName: Can't find %1 in radioNameSel").arg(radioName));
                onContestPageChangedFlag = false;
                //rigFrameStartFlag = false;
                return;
            }


            setRadioTxVertEnabled(false);
            setRitEnableState(false);
            setRadioVolumeState(false);


            if (radioName.isEmpty())
            {
                traceMsg(QString("setRadioName: radioName is empty clear radio in rigcontrol"));
                emit selectRadio(radioName, contestBand, Frequency(), "");  // send radio and mode.
                selRadioDetails = RadioDetails();
                createSupportedBandList(selRadioDetails.getBandList());
            }
            else if (allRadioDetails.contains(selRadioName))
            {
                selRadioDetails = allRadioDetails[selRadioName];
                traceMsg(QString("setRadioName:: Select Radio - radio details for %1 in allRadioDetails").arg(radioName));
                createSupportedBandList(selRadioDetails.getBandList());

                // get freq to send
                setRadioFreq(sendFreq,  fromStartRigControl);
                traceMsg(QString("setRadioName: set contest band limits for band = %1").arg(contestBand));
                setContestBandLimits(contestBand);


                QString contestMode = ct->currentMode.getValue();
                bool restoreModeFlag = false;
                TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpContestChangeRestoreContestMode, restoreModeFlag );

                if (fromStartRigControl)
                {
                    rigFrameStartFlag = false;
                    onContestPageChangedFlag = false;
                    traceMsg(QString("setRadioName: start contest frame radioName = %1, freq = %2, mode = %3, contest = %4")
                             .arg(radioName, sendFreq.traceStr(), contestMode, ct->uuid));
                    emit selectRadio(radioName, contestBand, sendFreq, contestMode.remove(':'));
                }
                else if (onContestPageChangedFlag && !rigFrameStartFlag)
                {
                    onContestPageChangedFlag = false;

                    traceMsg(QString("setRadioName: onContestPageChanged flag set and rigFrameStartFlag clear"));

                    if (!restoreModeFlag)
                    {
                        traceMsg(QString("setRadioName: restoreModeFlag clear"));
                        traceMsg(QString("setRadioName: onContestPageChangedFlag = %1, restoreModeFlag = %2").arg(onContestPageChangedFlag ? "true" : "false", restoreModeFlag ? "true" : "false"));

                        QString m = curMode;

                        traceMsg(QString("setRadioName: SavedCurMode = %1").arg(curMode));
                        if (m.contains(':') &&
                                (m.contains(hamlibData::MGM)
                                || m.contains(hamlibData::RY)
                                || m.contains(hamlibData::PSK))
                                )
                        {
                            QStringList ml = m.split(':');
                            if (ml.count() == 2)
                            {
                                m = ml[0].remove(':').trimmed();
                            }

                        }

                        if (m.isEmpty())
                        {
                            m = contestMode;
                            traceMsg(QString("setRadioName: saved mode is empty - revert to contest mode = %1").arg(m));

                        }
                        traceMsg(QString("setRadioName: sending radioName = %1, mode = %2").arg(radioName, m));
                        emit selectRadio(radioName, contestBand, sendFreq, m.remove(':'));  // send radio and previous mode.
                    }
                    else
                    {
                        traceMsg(QString("setRadioName: restoreModeFlag set"));
                        traceMsg(QString("setRadioName: onContestPageChange = %1, radioName = %2, mode = %3").arg(onContestPageChangedFlag ? "true" : "false", radioName, contestMode));
                        emit selectRadio(radioName, contestBand, sendFreq, contestMode);
                    }
                }

            }
            else if (!radioName.isEmpty() && isRadioLoaded())
            {

                    trace(QString("setRadioName:: Select Radio for %1 , Radio wasn't found = %1").arg(radioName));
                    setRadioBandWarning(HtmlFontColour(Qt::red) + tr("Selected radio details were not found, please add radio or restart rigcontrol!"));
                    // clear selection in rigcontrol
                    emit selectRadio(radioName, contestBand, Frequency(), curMode);  // send radio and mode.
                    selRadioDetails = RadioDetails();
                    createSupportedBandList(selRadioDetails.getBandList());
            }



            //LogContainer->sendDM->notifyRigChanges();
    }
    else
    {
        traceMsg(QString("setRadioName:: No contest or protect, no radio selection"));
    }




}

// used to test freq received from rigcontrol in tsinglelogframe

Frequency RigControlFrame::getSendFreq()
{
    return sendFreq;
}

void RigControlFrame::setRadioFreq( Frequency &freqToSend, bool &fromStartRigControl)
{
    traceMsg(QString("setRadioFreq: enter function, fromStartRigControl = %1, onContestPageChangedFlag = %2, rigFrameStartFlag = %3, lastFreq = %4")
             .arg(fromStartRigControl ? "True" : "False", onContestPageChangedFlag ? "True" : "False",
             rigFrameStartFlag ? "True" : "False", lastFreq.traceStr()));

    if (selRadioDetails.getBandList().isEmpty())
    {
        setRadioBandWarning(HtmlFontColour(Qt::red) + tr("Radio Bandlist is empty!"));
        traceMsg(QString("setRadioFreq:: Radio Bandlist is empty!"));
        freqToSend = NO_BAND_SUPPORT;
        return;
    }


    if (listOfBands.isEmpty())
    {
        setRadioBandWarning(HtmlFontColour(Qt::red) + tr("Radio has no available bands"));
        traceMsg(QString("setRadioFreq:: Error No available bands!"));
        freqToSend = NO_BAND_SUPPORT;
        return;
    }

    setRadioBandWarning(QString(""));
    traceMsg(QString("setRadioFreq: list of bands for radio %1 is %2").arg(selRadioName.toString(), selRadioDetails.getBandList()));

    if (ct )
    {
       ignorePresetFreqFlag = readIgnorePresetFreqFlag();
       ignorePreviousFreqFlag = readIgnorePreviousFreqFlag();

       if (!fromStartRigControl /*&& !lastFreq.isClear()*/)
       {
           // frame has been running, so use lastFreq, except if ignorePreviousFreqFlag set
           if (!ignorePreviousFreqFlag)
           {
               traceMsg(QString("frame has been running, using lastFreq = %1").arg(lastFreq.traceStr()));
               freqToSend = lastFreq;
           }
           else
           {
               traceMsg(QString("frame has been running, ignore lastFreq"));

           }

           return;
       }
       else if (fromStartRigControl)   // it is a contest frame starting
       {

           traceMsg(QString("setRadioFreq: curFreq default - starting contest"));



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


           QString cb = ct->currentBand.getValue().trimmed();
           traceMsg(QString("setRadioFreq: contest band = %1").arg(cb));

           BandList &blist = BandList::getBandList();
           QSharedPointer<BandInfo>  bi;
           bool bandOK = blist.findBand(cb, bi);
           if (bandOK)
           {
               QString mode = ct->currentMode.getValue();
               if (mode == hamlibData::PH)
               {
                   Frequency modeTestFreq = bi->fLow;
                   if (modeTestFreq > Frequency(10000000)) // over 10MHz is USB
                   {
                       mode = "USB";
                   }
                   else
                   {
                       mode = "LSB";
                   }
               }

               Frequency freq = tslf->bandSwitchFrame->getPresetFreq(cb, mode);

               traceMsg(QString("setRadioFreq: set preset freq = %1").arg(freq.traceStr()));
               if (checkValidFreq(freq))
               {
                     traceMsg(QString("setRadioFreq: freq valid = %1, send freq to rigcontrol").arg(freq.traceStr()));
                     freqToSend = freq;
                     curFreq = freq;
                     lastFreq = freq;
                     traceMsg(QString("setRadioFreq: sendFreq = %1, curFreq = %2, lastFreq = %3")
                             .arg(freqToSend.traceStr(), curFreq.traceStr(), lastFreq.traceStr()));

               }
               else
               {
                     traceMsg(QString("setRadioFreq: freq not valid = %1").arg(freq.traceStr()));
                     freqToSend.clear();

               }


           }
           else
           {
               // warn no band for this radio
               setRadioBandWarning(HtmlFontColour(Qt::red) + tr("No %1 Band found for this radio!").arg(cb));
               traceMsg(QString("SsetRadioFreq: %1 Band not found on this radio").arg(cb));
               freqToSend = NO_BAND_SUPPORT;
           }



       }
       else if (onContestPageChangedFlag && !rigFrameStartFlag)
       {


           Frequency freq;
           traceMsg(QString("setRadioFreq: onContestPageChanged and not default curFreq"));

           if (ignorePreviousFreqFlag)
           {
               traceMsg(QString("setRadioFreq: ignorePreviousFreqflag set "));
               return;
           }
           else
           {

               freq = lastFreq;
               traceMsg(QString("setRadioFreq: set send freq to lastFreq = %1").arg(freq.traceStr()));

               if (checkValidFreq(freq))
               {
                   traceMsg(QString("setRadioFreq: freq valid = %1, send freq to rigcontrol").arg(freq.traceStr()));

                   //displayFreqOnFreqEditDisplay(freq);

                   freqToSend = freq;

                   return;
               }
               else
               {
                   traceMsg(QString("setRadioFreq: freq not valid = %1").arg(freq.traceStr()));
                   freqToSend.clear();
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

        Frequency freq = disconnectFreq;
        traceMsg(QString("restorRadioFreq: restore Freq for this contest"));

        if (!freq.isClear())
        {
            disconnectFreq.clear();
            QString cb = ct->currentBand.getValue().trimmed();
            int err = tslf->bandSwitchFrame->selectButtonGroupAndActiveBand(cb);
            if (err >= 0 )
            {

                if (checkValidFreq(freq))
                {
                  traceMsg(QString("restoreRadioFreq: restoring this freq = %1").arg(freq.traceStr()));
                  sendRigFreq(freq);
                }
                else
                {
                   traceMsg(QString("restoreRadioFreq: freq %1 invalid").arg(curFreq.traceStr()));
                }

            }
            else
            {
                traceMsg(QString("restoreRadioFreq: error finding band =  %1, in bandsel combo").arg(cb));
            }

        }

    }
}

void RigControlFrame::onCheckContestBandMatch()
{
    checkContestBandMatch(curFreq);
}

bool RigControlFrame::checkContestBandMatch(const Frequency &freq)
{

    if (!contestBandFHigh.isClear() && !contestBandFLow.isClear() && radioConnected)
    {
        if (freq >= contestBandFLow && freq <= contestBandFHigh)
        {

            setRadioBandWarning("");
            freqLineEditBkgnd(false);
            ui->resetBandFreqFrame->hide();
            return true;
        }
        else
        {
            setRadioBandWarning(HtmlFontColour(Qt::red) + tr("Freq out of contest band"));
            freqLineEditBkgnd(true);
            ui->resetBandFreqFrame->show();
        }

    }

    return false;
}





void RigControlFrame::setContestBandLimits(QString band)
{
    BandList &blist = BandList::getBandList();

    for (int i = 0; i < blist.bandList.count(); i++)
    {
        if (band == blist.bandList[i]->uk)
        {
            contestBandFLow = blist.bandList[i]->fLow;
            contestBandFHigh = blist.bandList[i]->fHigh;
            return;
        }
    }

    contestBandFLow = 0;
    contestBandFHigh = 0;

}

void RigControlFrame::setContestBand(QString band)
{
    contestBand = band;
    setContestBandLimits(contestBand);
    tslf->bandSwitchFrame->setContestBand(contestBand);
}
void RigControlFrame::setRadioListFromTslf()
{
    setRadioList();

}

void RigControlFrame::setRadioList()
{
    if (ct && !ct->isReadOnly() /*&& ct == TContestApp::getContestApp() ->getCurrentContest()*/)
    {
        if (LogContainer->sendDM->rigs().count() > 0)
        {
            traceMsg(QString("setRadioList: list of radios in LogContainer = %1, list of radios in radio selector = %2")
                     .arg(LogContainer->sendDM->rigs().count()).arg(ui->radioNameSel->count()));
            QString currentSelRadioName = ui->radioNameSel->currentData().toString();
            traceMsg(QString("setRadioList: currentSelRadioName = %1").arg(currentSelRadioName));

            traceMsg(QString("setRadioList: Update list of radios and radioNameSel"));

            listOfRadios = LogContainer->sendDM->rigs();

            traceMsg(QString("setRadioList: add %1 radios to radioNameSel").arg(listOfRadios.count()));
            comboSetUniqueNames(listOfRadios, ui->radioNameSel);

            if (setSelectedRadio(radioName))
            {
                traceMsg(QString("setRadioList: set index back to radioName = %1").arg(radioName));
            }
            else
            {
                ui->radioNameSel->setCurrentIndex(0);
                traceMsg(QString("setRadioList: Can't find %1 in radioNameSel").arg(radioName));

            }

            launchRadioSelectCount = 5;     // wait five seconds
            launchRadioSelectTimer->start(1000);

        }
        else
        {
            traceMsg(QString("setRadioList: logcontainer rigs is empty"));
        }

    }


}







// create the active bands on selected radio

void RigControlFrame::createSupportedBandList(QString b)
{
    traceMsg(QString("createActiveBandList: %1").arg(b));
    if (!b.isEmpty())
    {
        listOfBands.clear();
        listOfBands = b.split(":");

        tslf->bandSwitchFrame->selectSupportedBands(listOfBands);

        // set combo to current contest band
        if (ct && !ct->isReadOnly())
        {
            QString contestBand = ct->currentBand.getValue();
            // is band in combo sel
            int retCode = tslf->bandSwitchFrame->setButtonOnOff(contestBand, true);

            if (retCode >= 0)
            {
                traceMsg(QString("createActveBandList: restore currentBand = %1").arg(contestBand));

            }
            else
            {
                traceMsg(QString("createActveBandList: couldn't find contestBand = %1 in select combo").arg(contestBand));

            }
        }

    }
}







void RigControlFrame::setRadioState(QString s)
{
    traceMsg(QString("Set RadioState = %1").arg(s));

    if (s == oldRigState)
        return;

    oldRigState = s;

    if (s != "")
    {

        if (s.contains(RIG_STATUS_ERROR))
        {
           radioError = true;
           QStringList sl = s.split(':');
           if (sl.count() == 2)
           {
               QString errMess = tr("Error: %1").arg(sl[1]);
               ui->rigState->setText(HtmlFontColour(Qt::red) + errMess);
               emit radioHasError(sl[1]);
               mShowMessage(errMess, this);
           }
        }
        else if (s == RIG_STATUS_CONNECTED)
        {
            radioConnected = true;
            ui->rigState->setText(tr("Connected"));
            int index = ui->radioNameSel->findData(radioName);
            if (index >= 0)
            {
                ui->radioNameSel->setCurrentIndex(index);

                //restoreRadioFreq();
                emit radioIsConnected(true);
                sendVmButtonFrameRadioConnected(true);
            }
            else
            {
                traceMsg(QString("setRadioState: Can't find %1 in radioNameSel").arg(radioName));
            }

        }
        else if (s == RIG_STATUS_DISCONNECTED)
        {
           radioConnected = false;
           radioError = false;

           ui->rigState->setText(tr("Disconnected"));

           //disconnectFreq = lastFreq;

           //ui->bandWarnLabel->setText("");

           if (ui->radioNameSel->currentText() == "")
           {
               curFreq.clear();
               QString sf = curFreq.str();
               ui->freqInput->setInputMask(maskData::freqMask[sf.size() - 4]);
               ui->freqInput->setLineText(sf);
               emit setFreqDisplay(curFreq, legalFreq);
           }
           emit radioIsConnected(false);
           emit radioDisconnected();
           sendVmButtonFrameRadioConnected(false);
        }
        else if (s == RIG_SWITCH_COMPLETED)
        {
            emit radioSwitchCompleted();
        }

        radioState = s;
    }
}


void RigControlFrame::setRadioSwitchCompleted()
{
    // now connected update display freq
    RigState rigSt = LogContainer->sendDM->getRigState(radioName);
    QString fStr = rigSt.getRadioFreq().str();
    traceMsg(QString("Radio State RadioSwitch Completed, radio freq = %1").arg(fStr));
    displayFreqOnFreqEditDisplay(fStr);


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
    txVertShown = s;
    ui->ritTxvertFrame->setVisible( txVertShown || ritEnable);
     ui->txVertFrame->setVisible(s);
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
    ritEnable = s;
    ui->ritTxvertFrame->setVisible( txVertShown || ritEnable);
    ui->RitFrame->setVisible(s);
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
    if (isRadioLoaded() && radioConnected && !radioError)
    {
        return true;
    }

    return false;
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
    if (status)
    {
        freqDisplayPalette->setColor(QPalette::Base,Qt::yellow);
    }
    else
    {
        freqDisplayPalette->setColor(QPalette::Base,Qt::white);
    }


    ui->freqInput->setPalette(*freqDisplayPalette);

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
    qint64 f = curFStepButtonsFreq;
    f *= -1;
    freqPlusMinusButton(Frequency(f));
}


void RigControlFrame::freqPlusMinusButton(Frequency f)
{

    if (isRadioLoaded())
    {
        Frequency freq = calcNewFreq(f);
        if (!freq.isClear())
        {

            setFreqTextLegalColour(freq, curMode);
            ui->freqInput->setLineText(freq.str());
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


Frequency RigControlFrame::calcNewFreq(Frequency incFreq)
{
    BandList &blist = BandList::getBandList();
    QSharedPointer<BandInfo>  bi;
    bool bandOk = false;

    Frequency freq(curFreq);
    if (freq.isOK())
    {
        freq = freq + Frequency(incFreq);
        bandOk = blist.findBand(freq, bi);
        if (!bandOk)
        {
            freq = freq - Frequency(incFreq);    // never used...
        }
        else
        {
            traceMsg(QString("CalcNewFreq: Freq  = %1").arg(freq.traceStr()));

        }
    }

    return freq;

}

void RigControlFrame::mgmLabelVisible(bool state)
{
    ui->mgmbreak->setVisible(state);
    ui->mgmLbl->setVisible(state);
}

void RigControlFrame::setFreqTextLegalColour(const Frequency _freq, QString mode)
{
    bool legalOperatingFreqFlag;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpContestTurnOffOperatingFreqColorRadioDial, legalOperatingFreqFlag );


    if (checkFreqIsLegal(_freq, mode))
    {

        freqDisplayPalette->setColor(QPalette::Text, Qt::black);
        ui->freqInput->setPalette(*freqDisplayPalette);


        legalFreq = true;
    }
    else
    {

        if (!legalOperatingFreqFlag)
        {
            freqDisplayPalette->setColor(QPalette::Text,Qt::red);
            ui->freqInput->setPalette(*freqDisplayPalette);
        }
        else
        {
            freqDisplayPalette->setColor(QPalette::Text, Qt::black);
            ui->freqInput->setPalette(*freqDisplayPalette);
        }

        legalFreq = false;
    }

}





bool RigControlFrame::checkFreqIsLegal(const Frequency &freq, const QString mode)
{
    BandList &blist = BandList::getBandList();
    QSharedPointer<BandInfo>  bi;
    bool bandOk = false;


    bandOk = blist.findBand(freq, bi);
    if (bandOk)
    {
        QString band = bi->uk;
        return isFreqLegal(freq, band, mode);
    }

    return false;  // out of band


}





// returns true if freq ok, false if it is not...it will return true and error to tracelog is mode or band
// is missing from operating freq file
bool RigControlFrame::isFreqLegal(const Frequency &freq, const QString band, const QString mode)
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
                    traceMsg(QString("RigControl Frame isFreqLegal: mode is missing from file - band %1, mode %2").arg(band, mode));
                    return true;
                case BAND_MISSING:
                    traceMsg(QString("Bandmap isFreqLegal: band is missing from file - band %1, mode %2").arg(band, mode));
                    return true;
            }
    }

    traceMsg(QString("RigControl Frame isFreqLegal: Operating Freq file not loaded"));
    return true;

}

bool RigControlFrame::checkFreqOK(const Frequency &freq)
{
    qint64 fInt64 = freq;

    if (fInt64 >=0 && fInt64 < 500000000000)
    {
        return true;
    }
    else
    {
        return false;
    }
}






void RigControlFrame::setRigControlPanelTitle()
{
    if (getRadioReadOnlyFlag())
    {
        ui->rigControlPanelTitle->setText("RigControl - Read Only");
    }
    else
    {
        ui->rigControlPanelTitle->setText("RigControl");
    }

}


bool RigControlFrame::getRadioReadOnlyFlag()
{

    bool state;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpContestRadioReadOnly, state );
    return state;
}

void RigControlFrame::traceMsg(QString msg)
{

    QString frameUuid;
    if (ct)
    {
        frameUuid = ct->uuid;
    }


    trace(QString("[RigcontrolFrame: %1, Uuid %2] %3 ").arg(radioName, frameUuid, msg));
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

void RigControlFrame::setVmButtonsFrame(TxVmButtonsFrame *txVmButtonsFrame_)
{
    if (txVmButtonsFrame)
    {
        txVmButtonsFrame = txVmButtonsFrame_;
    }
}

bool RigControlFrame::isVmButtonsFrameVisible()
{
    if (txVmButtonsFrame)
    {
       return txVmButtonsFrame->isVisible();
    }

    return false;
}

void RigControlFrame::sendVmButtonFrameSelectedRadio(PubSubName selectedRadio)
{

    if (txVmButtonsFrame)
    {
      txVmButtonsFrame->setSelectedRadio(selectedRadio);
    }

}

void RigControlFrame::sendVmButtonFrameRadioConnected(bool connected)
{
    if (txVmButtonsFrame)
    {
       txVmButtonsFrame->setRadioIsConnected(connected);
    }

}


void RigControlFrame::dumpLoggerRadioOptionSettings()
{
    bool state;
    traceMsg(QString("Logger Radio Options Current Settings"));
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpContestRadioReadOnly, state );
    traceMsg(QString("Radio Read Only = %1").arg(state ? "True" : "False"));

    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpContestStartIgnorePresetFreq, state );
    traceMsg(QString("Contest Start - Ignore Preset Frequency = %1").arg(state ? "True" : "False"));

    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpContestChangeIgnorePreviousFreq, state );
    traceMsg(QString("Contest Change - Ignore Previous Frequency = %1").arg(state ? "True" : "False"));

    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpContestChangeRestoreContestMode, state );
    traceMsg(QString("Contest Change - Restore Contest Mode = %1").arg(state ? "True" : "False"));

    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpContestTurnOffOperatingFreqColorRadioDial, state );
    traceMsg(QString("Turn Off Operating Freq Color Radio Dial = %1").arg(state ? "True" : "False"));

    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpCQRit, state );
    traceMsg(QString("CQ RIT = %1").arg(state ? "True" : "False"));
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



void RigControlFrame::on_resetBandFreqButton_clicked()
{
    //We want to select the frequency based on the contest band

    Frequency freqToSend;
    traceMsg(QString("on_resetBandFreqButton_clicked"));

    QString cb = ct->currentBand.getValue().trimmed();
    traceMsg(QString("setRadioFreq: contest band = %1").arg(cb));

    if (readIgnorePresetFreqFlag() || readIgnorePreviousFreqFlag())
    {
        traceMsg(QString("Ignore Preset Flag = %1 or Ignore Previous Freq Flag = %2, just send Band = %3")
                .arg(readIgnorePresetFreqFlag() ? "True" : "False"
                , readIgnorePreviousFreqFlag() ? "True" : "False"
                , cb));
        emit sendBandToRigControl(cb);
        return;
    }



    BandList &blist = BandList::getBandList();
    QSharedPointer<BandInfo>  bi;
    bool bandOK = blist.findBand(cb, bi);
    if (bandOK)
    {
        QString mode = ct->currentMode.getValue();
        if (mode == hamlibData::PH)
        {
            Frequency modeTestFreq = bi->fLow;
            if (modeTestFreq > Frequency(10000000)) // over 10MHz is USB
            {
                mode = "USB";
            }
            else
            {
                mode = "LSB";
            }
        }

        Frequency freq = tslf->bandSwitchFrame->getPresetFreq(cb, mode);

        traceMsg(QString("setRadioFreq: set preset freq = %1").arg(freq.traceStr()));
        if (checkValidFreq(freq))
        {
              traceMsg(QString("setRadioFreq: freq valid = %1, send freq to rigcontrol").arg(freq.traceStr()));
              freqToSend = freq;
              curFreq = freq;
              lastFreq = freq;
              traceMsg(QString("setRadioFreq: sendFreq = %1, curFreq = %2, lastFreq = %3")
                      .arg(sendFreq.traceStr(), curFreq.traceStr(), lastFreq.traceStr()));

        }
        else
        {
              traceMsg(QString("setRadioFreq: freq not valid = %1").arg(freq.traceStr()));
              freqToSend.clear();

        }
    }
    else
    {
        // warn no band for this radio
        setRadioBandWarning(HtmlFontColour(Qt::red) + tr("No %1 Band found for this radio!").arg(cb));
        traceMsg(QString("SsetRadioFreq: %1 Band not found on this radio").arg(cb));
        freqToSend = NO_BAND_SUPPORT;
    }
    sendRigFreq(freqToSend);
}



