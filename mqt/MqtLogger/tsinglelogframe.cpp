#include "logger_pch.h"

#include "MatchThread.h"
#include "BandList.h"
#include "tqsoeditdlg.h"
#include "tentryoptionsform.h"
#include "tsinglelogframe.h"
#include "ui_tsinglelogframe.h"

#include "SendRPCDM.h"
#include "tlogcontainer.h"
#include "focuswatcher.h"
#include "htmldelegate.h"
#include "enqdlg.h"
#include "MatchTreesFrame.h"

TSingleLogFrame::TSingleLogFrame(QWidget *parent, BaseContestLog * contest) :
    QFrame(parent),
    ui(new Ui::TSingleLogFrame),
    contest( contest ),
    splittersChanged(false),
    curFreq( 0 ), oldFreq( 0 ),
    freqUpDateCnt(0), modeUpDateCnt(0),
    lastStanzaCount( 0 ),
    rotatorLoaded(false),
    bandMapLoaded(false),
    keyerLoaded(false),
    radioLoaded(false)

{
    ui->setupUi(this);

    ui->FKHRigControlFrame->setContest(contest);
    ui->FKHRotControlFrame->setContest(contest);

    ui->GJVQSOLogFrame->setAsEdit(false, "Log");

    ui->matchTreesFrame->setBaseName("Log");

    ui->ControlSplitter->setVisible(false);

#ifdef Q_OS_ANDROID
    splitterHandleWidth = 20;
#else
    splitterHandleWidth = 6;
#endif

    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );

    sendDM = new TSendDM( this, ct );

    qsoModel.initialise(contest);
    ui->QSOTable->setModel(&qsoModel);
    ui->QSOTable->setItemDelegate( new HtmlDelegate );

    ui->QSOTable->resizeColumnsToContents();
    ui->QSOTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    restoreColumns();

    ui->GJVQSOLogFrame->initialise( contest );

    connect(&MinosLoggerEvents::mle, SIGNAL(ContestPageChanged()), this, SLOT(on_ContestPageChanged()));

    //connect(&MinosLoggerEvents::mle, SIGNAL(BandMapPressed()), this, SLOT(on_BandMapPressed()));



    connect(&MinosLoggerEvents::mle, SIGNAL(TimerDistribution()), this, SLOT(NextContactDetailsTimerTimer()));
    connect(&MinosLoggerEvents::mle, SIGNAL(TimerDistribution()), this, SLOT(PublishTimerTimer()));
    connect(&MinosLoggerEvents::mle, SIGNAL(TimerDistribution()), this, SLOT(HideTimerTimer()));
    connect(&MinosLoggerEvents::mle, SIGNAL(MakeEntry(BaseContestLog*)), this, SLOT(on_MakeEntry(BaseContestLog*)));
    connect(&MinosLoggerEvents::mle, SIGNAL(AfterSelectContact(QSharedPointer<BaseContact>, BaseContestLog *)), this, SLOT(on_AfterSelectContact(QSharedPointer<BaseContact>, BaseContestLog *)));
    connect(&MinosLoggerEvents::mle, SIGNAL(AfterLogContact(BaseContestLog *)), this, SLOT(on_AfterLogContact(BaseContestLog *)));

    doNextContactDetailsOnLeftClick( true);  // but the sizes are zero...

    getSplitters();

    connect(&MinosLoggerEvents::mle, SIGNAL(LogColumnsChanged()), this, SLOT(onLogColumnsChanged()));
    connect(&MinosLoggerEvents::mle, SIGNAL(SplittersChanged()), this, SLOT(onSplittersChanged()));
    connect(&MinosLoggerEvents::mle, SIGNAL(NextContactDetailsOnLeft()), this, SLOT(on_NextContactDetailsOnLeft()));
    connect(&MinosLoggerEvents::mle, SIGNAL(NextUnfilled(BaseContestLog*)), this, SLOT(on_NextUnfilled(BaseContestLog*)));
    connect(&MinosLoggerEvents::mle, SIGNAL(GoToSerial(BaseContestLog*)), this, SLOT(on_GoToSerial(BaseContestLog*)));

    connect(ui->GJVQSOLogFrame, SIGNAL(xferPressed()), this, SLOT(on_XferPressed()));
    connect(ui->matchTreesFrame, SIGNAL(xferPressed()), this, SLOT(on_XferPressed()));
    connect(ui->matchTreesFrame, SIGNAL(editContact(QSharedPointer<BaseContact>)), this, SLOT(EditContact(QSharedPointer<BaseContact>)));
    connect(ui->matchTreesFrame, SIGNAL(setXferEnabled(bool)), ui->GJVQSOLogFrame, SLOT(setXferEnabled(bool)));


    // BandMap Updates

    connect(sendDM, SIGNAL(setBandMapLoaded()), this, SLOT(on_BandMapLoaded()));


    // RigControl Updates
    // From rig controller
    connect(sendDM, SIGNAL(setRadioLoaded()), this, SLOT(on_RadioLoaded()));
    connect(sendDM, SIGNAL(setMode(QString)), this, SLOT(on_SetMode(QString)));
    connect(sendDM, SIGNAL(setFreq(QString)), this, SLOT(on_SetFreq(QString)));
    connect(sendDM, SIGNAL(setRadioName(QString)), this, SLOT(on_SetRadioName(QString)));
    connect(sendDM, SIGNAL(setRadioState(QString)), this, SLOT(on_SetRadioState(QString)));
    connect(sendDM, SIGNAL(setRadioTxVertStatus(QString)), this, SLOT(on_SetRadioTxVertState(QString)));


    // To rig controller
    connect(ui->FKHRigControlFrame, SIGNAL(sendFreqControl(QString)), this, SLOT(sendRadioFreq(QString)));
    connect(ui->FKHRigControlFrame, SIGNAL(sendModeToControl(QString)), this, SLOT(sendRadioMode(QString)));
    connect(ui->GJVQSOLogFrame, SIGNAL(sendModeControl(QString)), this , SLOT(sendRadioMode(QString)));

    // Rotator updates
    // From rotator controller
    connect(sendDM, SIGNAL(RotatorLoaded()), this, SLOT(on_RotatorLoaded()));
    connect(sendDM, SIGNAL(RotatorState(QString)), this, SLOT(on_RotatorState(QString)));
    connect(sendDM, SIGNAL(RotatorBearing(QString)), this, SLOT(on_RotatorBearing(QString)));
    connect(sendDM, SIGNAL(RotatorMaxAzimuth(QString)), this, SLOT(on_RotatorMaxAzimuth(QString)));
    connect(sendDM, SIGNAL(RotatorMinAzimuth(QString)), this, SLOT(on_RotatorMinAzimuth(QString)));
    connect(sendDM, SIGNAL(RotatorAntennaName(QString)), this, SLOT(on_RotatorAntennaName(QString)));

    // To rotator controller
    connect(ui->FKHRotControlFrame, SIGNAL(sendRotator(rpcConstants::RotateDirection , int  )),
            this, SLOT(sendRotator(rpcConstants::RotateDirection , int  )));


    connect(sendDM, SIGNAL(setKeyerLoaded()), this, SLOT(on_KeyerLoaded()));


    connect( ui->QSOTable->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( on_sectionResized(int, int , int)));

    connect(LogContainer, SIGNAL(sendKeyerPlay( int )), this, SLOT(sendKeyerPlay(int)));
    connect(LogContainer, SIGNAL(sendKeyerRecord( int)), this, SLOT(sendKeyerRecord(int)));
    connect(LogContainer, SIGNAL(sendKeyerTone()), this, SLOT(sendKeyerTone()));
    connect(LogContainer, SIGNAL(sendKeyerTwoTone()), this, SLOT(sendKeyerTwoTone()));
    connect(LogContainer, SIGNAL(sendKeyerStop()), this, SLOT(sendKeyerStop()));

/*
    connect(ui->FKHBandMapFrame, SIGNAL(sendBandMap( QString, QString, QString, QString, QString )),
            this, SLOT(sendBandMap(QString,QString,QString,QString,QString)));
*/
    ui->logFrameSplitter->setStretchFactor(0, 1);
    ui->logFrameSplitter->setStretchFactor(1, 0);
    ui->logFrameSplitter->setStretchFactor(2, 0);
    ui->logFrameSplitter->setStretchFactor(3, 1);
    ui->logFrameSplitter->setChildrenCollapsible(false);
}

TSingleLogFrame::~TSingleLogFrame()
{
    delete ui;
    delete sendDM;

    ui = nullptr;
    contest = nullptr;
}
void TSingleLogFrame::keyPressEvent( QKeyEvent* event )
{
    ui->GJVQSOLogFrame->doKeyPressEvent(event);
}
QString TSingleLogFrame::makeEntry( bool saveMinos )
{
   LoggerContestLog * ct = dynamic_cast<LoggerContestLog *>( contest );
   if ( !ct )
   {
      return "";
   }
   TEntryOptionsForm EntryDlg( this, ct, saveMinos  );
   if ( saveMinos )
   {
      EntryDlg.setWindowTitle("Save imported log as a .minos file");
   }
   if ( EntryDlg.exec() == QDialog::Accepted )
   {
      ct->commonSave( false );
      QString expName = EntryDlg.doFileSave( );
      return expName;
   }
   return "";
}
BaseContestLog * TSingleLogFrame::getContest()
{
   return contest;
}

void TSingleLogFrame::closeContest()
{
    if ( TContestApp::getContestApp() )
    {
       RPCPubSub::publish( rpcConstants::monitorLogCategory, contest->publishedName, QString::number( 0 ), psRevoked );
       qsoModel.initialise(0);
       ui->matchTreesFrame->setContest(0);
       ui->stackedInfoFrame->setContest(0);
       ui->FKHRigControlFrame->setContest(0);
       ui->FKHRotControlFrame->setContest(0);
       TContestApp::getContestApp() ->closeFile( contest );
       ui->GJVQSOLogFrame->closeContest();
       contest = 0;
    }
}
void TSingleLogFrame::restoreColumns()
{
    QSettings settings;
    QByteArray state;

    state = settings.value("QSOTable/state").toByteArray();
    ui->QSOTable->horizontalHeader()->restoreState(state);

    ui->matchTreesFrame->restoreColumns();

    logColumnsChanged = false;

}

void TSingleLogFrame::showQSOs()
{

   NextContactDetailsTimerTimer( );

   restoreColumns();

   ui->GJVQSOLogFrame->clearCurrentField();
   ui->GJVQSOLogFrame->startNextEntry();

}
void TSingleLogFrame::on_ContestPageChanged ()
{
    if (!ui)
        return;

    if ( this != LogContainer->getCurrentLogFrame() )
    {

       ui->GJVQSOLogFrame->savePartial();  // we kill it on (re) entry, so not needed
       return ;
    }

    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( getContest() );
    TContestApp::getContestApp() ->setCurrentContest( ct );
    ui->matchTreesFrame->setContest(ct);
    ui->stackedInfoFrame->setContest(ct);

    if ( logColumnsChanged )
    {
       ui->GJVQSOLogFrame->killPartial();
       showQSOs();             // this does a restorePartial
       logColumnsChanged = false;
    }

    if (splittersChanged)
    {
       getSplitters();
    }

    refreshMults();

    ui->GJVQSOLogFrame->selectField(0);
    ui->GJVQSOLogFrame->logTabChanged();

//    MultDispFrame->setContest( contest );
    doNextContactDetailsOnLeftClick( false );
    MinosLoggerEvents::SendShowOperators();

    if ( this == LogContainer->getCurrentLogFrame() )
    {
        ui->FKHRigControlFrame->on_ContestPageChanged(sCurFreq, sCurMode);
    }

    updateQSODisplay();

}
void TSingleLogFrame::doNextContactDetailsOnLeftClick(bool keepSizes )
{
    QList<int> sizes = ui->CribSplitter->sizes();

    bool isOnLeft = (ui->CribSplitter->widget(0) == ui->CribSheet);

    bool conleft = LogContainer->isNextContactDetailsOnLeft();

    if (conleft)
    {
        ui->CribSplitter->insertWidget(0, ui->CribSheet);
    }
    else
    {
        ui->CribSplitter->insertWidget(1, ui->CribSheet);
    }

    bool nowOnLeft = (ui->CribSplitter->widget(0) == ui->CribSheet);
    if (isOnLeft != nowOnLeft)
    {
        //they have flipped
        std::reverse(sizes.begin(), sizes.end());
    }
    if (!keepSizes)
    {
        ui->CribSplitter->setSizes(sizes);
        on_CribSplitter_splitterMoved(0, 0);    // preserve the splitter position
    }
}
void TSingleLogFrame::NextContactDetailsTimerTimer( )
{
    if ( contest )
    {
        QString cb = contest->band.getValue().trimmed();
        BandList &blist = BandList::getBandList();
        BandInfo bi;
        bool bandOK = blist.findBand(cb, bi);
        if (bandOK)
        {
            cb = bi.uk;
        }
        if ( contest->isReadOnly() )
        {
            ui->NextContactDetailsLabel->setText( "<b><center><nobr><p><big>"
                                                  + cb + "</p><h1>"
                                                  + contest->mycall.fullCall.getValue() + "<br>"
                                                  + contest->myloc.loc.getValue() + "<br>"
                                                  + contest->location.getValue());
        }
        else
        {
            QString snBuff = QString("%1").arg( contest->maxSerial + 1, 3, 10, QChar('0') );
            QString locBuff;
            if (contest->location.getValue().size())
            {
                locBuff = "<br>" + contest->location.getValue();
            }
            ui->NextContactDetailsLabel->setText( "<b><center><nobr><p><big>"
                                                  + cb + "</p><h1>"
                                                  + contest->mycall.fullCall.getValue() + "<br>"
                                                  + snBuff + "<br>"
                                                  + contest->myloc.loc.getValue()
                                                  + locBuff);
        }
    }
}
void TSingleLogFrame::PublishTimerTimer(  )
{
   LoggerContestLog * ct = dynamic_cast<LoggerContestLog *>( contest );
   if ( ct && ct->isMinosFile() && !ct->isUnwriteable() && !ct->isProtected())
   {
      int stanzaCount = contest->getCtStanzaCount();
      if ( lastStanzaCount != stanzaCount )
      {
         // publish this contest details - what to use?
         // category LoggerContestLog
         // name filename(?)
         // value stanzaCount
         RPCPubSub::publish( rpcConstants::monitorLogCategory, contest->publishedName, QString::number( stanzaCount ), psPublished );
         lastStanzaCount = stanzaCount;
      }
   }
}
void TSingleLogFrame::HideTimerTimer(  )
{
    bool controlsLoaded = isBandMapLoaded() || isRadioLoaded() || isRotatorLoaded();

    ui->ControlSplitter->setVisible(controlsLoaded);

    if (controlsLoaded)
    {
        ui->FKHRigControlFrame->setVisible(isRadioLoaded());
        ui->FKHRotControlFrame->setVisible(isRotatorLoaded());
    }
}
void TSingleLogFrame::on_NextContactDetailsOnLeft()
{
    doNextContactDetailsOnLeftClick(false);
}
void TSingleLogFrame::updateQSODisplay()
{
   ui->GJVQSOLogFrame->updateQSODisplay();
}
void TSingleLogFrame::on_XferPressed()
{
   // transfer from current match
   if (!contest || contest->isReadOnly() )
      return ;

   MatchTreeItem *mi = ui->matchTreesFrame->getXferItem();

   transferDetails(mi);
}
//==============================================================================
void TSingleLogFrame::transferDetails(MatchTreeItem *MatchTreeIndex )
{
    if ( !contest  )
    {
       return ;
    }
   // needs to be transferred into QSOLogFrame.cpp
   QSharedPointer<MatchContact> mc = MatchTreeIndex->getMatchContact();

   if (mc)
   {
       QSharedPointer<BaseContact> bct = mc->getBaseContact();

       if ( bct )
       {
          BaseContestLog *matct = mc->getContactLog();
          ui->GJVQSOLogFrame->transferDetails( bct, matct );
       }
       else
       {
           ListContact *lct = mc->getListContact();
           if (lct)
           {
               ContactList *matct = mc->getContactList();
               ui->GJVQSOLogFrame->transferDetails( lct, matct );
           }
       }
   }
}
void TSingleLogFrame::transferDetails(memoryData::memData &m )
{
    if ( !contest  )
    {
       return ;
    }
    ui->GJVQSOLogFrame->transferDetails( m.callsign, m.locator );
    ui->FKHRigControlFrame->transferDetails(m);
}
void TSingleLogFrame::getDetails(memoryData::memData &m)
{
    ui->FKHRigControlFrame->getDetails(m);
}
//---------------------------------------------------------------------------

void TSingleLogFrame::QSOTreeSelectContact( QSharedPointer<BaseContact> lct )
{
   if (lct)
   {
      EditContact( lct );
   }
}
void TSingleLogFrame::on_QSOTable_doubleClicked(const QModelIndex &index)
{
    QSOTreeSelectContact(contest->pcontactAt( index.row() ));
}
void TSingleLogFrame::EditContact( QSharedPointer<BaseContact> lct )
{
   TQSOEditDlg qdlg( this, false );
   qdlg.selectContact( contest, lct );

   qdlg.exec();

   contest->scanContest();

   ui->GJVQSOLogFrame->refreshOps();
   refreshMults();
   ui->GJVQSOLogFrame->startNextEntry();

}
//---------------------------------------------------------------------------
ScreenContact &TSingleLogFrame::getScreenEntry()
{
    ui->GJVQSOLogFrame->getScreenEntry();

    ui->GJVQSOLogFrame->calcLoc();

    return ui->GJVQSOLogFrame->screenContact;
}
//---------------------------------------------------------------------------
int TSingleLogFrame::getBearingFrmQSOLog()
{
    return ui->FKHRotControlFrame->getAngle(ui->GJVQSOLogFrame->getBearing());
}
//---------------------------------------------------------------------------

void TSingleLogFrame::on_MakeEntry(BaseContestLog *ct)
{
    if (ct == contest)
    {
       makeEntry( false );
    }
}
void TSingleLogFrame::on_AfterSelectContact( QSharedPointer<BaseContact>lct, BaseContestLog *ct)
{
    if (ct == contest && !lct)
    {
        ui->QSOTable->scrollToBottom();
        int row = ui->QSOTable->model()->rowCount() - 1;
        if (row >= 0)
        {
            QModelIndex index = ui->QSOTable->model()->index( row, 0 );
            ui->QSOTable->setCurrentIndex(index);
        }
    }
}
void TSingleLogFrame::on_AfterLogContact( BaseContestLog *ct)
{
      if (ct == contest)
      {
         contest->scanContest();
         updateTrees();
         NextContactDetailsTimerTimer( );
      }
}
void TSingleLogFrame::refreshMults()
{
    ui->stackedInfoFrame->refreshMults();
}

void TSingleLogFrame::updateTrees()
{
   qsoModel.reset();
   refreshMults();
}
bool TSingleLogFrame::getStanza( unsigned int stanza, QString &stanzaData )
{
   return contest->getStanza( stanza, stanzaData );
}
void TSingleLogFrame::getSplitters()
{
    QSettings settings;
    QByteArray state;

    state = settings.value("logFrameSplitter/state").toByteArray();
    ui->logFrameSplitter->restoreState(state);
    ui->logFrameSplitter->setHandleWidth(splitterHandleWidth);

    // and reset some of the saved state
    ui->logFrameSplitter->setStretchFactor(0, 1);
    ui->logFrameSplitter->setStretchFactor(1, 0);
    ui->logFrameSplitter->setStretchFactor(2, 0);
    ui->logFrameSplitter->setStretchFactor(3, 1);
    ui->logFrameSplitter->setChildrenCollapsible(false);


    state = settings.value("CribSplitter/state").toByteArray();
    ui->CribSplitter->restoreState(state);
    ui->CribSplitter->setHandleWidth(splitterHandleWidth);

    state = settings.value("MultSplitter/state").toByteArray();
    ui->MultSplitter->restoreState(state);
    ui->MultSplitter->setHandleWidth(splitterHandleWidth);

    state = settings.value("ControlSplitter/state").toByteArray();
    ui->ControlSplitter->restoreState(state);
    ui->ControlSplitter->setHandleWidth(splitterHandleWidth);

    ui->matchTreesFrame->getSplitters();
}
void TSingleLogFrame::onSplittersChanged()
{
    splittersChanged = true;
}

void TSingleLogFrame::on_logFrameSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    QByteArray state = ui->logFrameSplitter->saveState();
    QSettings settings;
    settings.setValue("logFrameSplitter/state", state);
    MinosLoggerEvents::SendSplittersChanged();
}

void TSingleLogFrame::on_CribSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    QByteArray state = ui->CribSplitter->saveState();
    QSettings settings;
    settings.setValue("CribSplitter/state", state);
    MinosLoggerEvents::SendSplittersChanged();
}

void TSingleLogFrame::on_MultSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    QByteArray state = ui->MultSplitter->saveState();
    QSettings settings;
    settings.setValue("MultSplitter/state", state);
    MinosLoggerEvents::SendSplittersChanged();
}

void TSingleLogFrame::on_ControlSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    QByteArray state = ui->ControlSplitter->saveState();
    QSettings settings;
    settings.setValue("ControlSplitter/state", state);
    MinosLoggerEvents::SendSplittersChanged();
}
void TSingleLogFrame::on_sectionResized(int, int, int)
{
    QSettings settings;
    QByteArray state;

    state = ui->QSOTable->horizontalHeader()->saveState();
    settings.setValue("QSOTable/state", state);

    MinosLoggerEvents::SendLogColumnsChanged();
}
void TSingleLogFrame::onLogColumnsChanged()
{
    logColumnsChanged = true;
}
void TSingleLogFrame::goNextUnfilled()
{
   QSharedPointer<BaseContact> nuc = contest->findNextUnfilledContact( );
   if ( nuc )
   {
      TQSOEditDlg qdlg(this, true );
      qdlg.setContest( contest );
      qdlg.setFirstContact( nuc );
      qdlg.exec();
      contest->scanContest();
      refreshMults();
      ui->GJVQSOLogFrame->startNextEntry();
   }
   else
   {
      MinosParameters::getMinosParameters() ->mshowMessage( "No unfilled contacts" );
   }

}
void TSingleLogFrame::on_NextUnfilled(BaseContestLog *ct)
{
    if (ct == contest)
    {
       goNextUnfilled();
    }
}

void TSingleLogFrame::goSerial( )
{
    static int serial = 0;
    do
    {
       if ( serial == -1 )
          serial = 0;
       if ( !enquireDialog( this, "Please give serial wanted", serial ) )
          return ;
    }
    while ( serial == -1 );

    QSharedPointer<BaseContact> cfu;
    for ( LogIterator i = contest->ctList.begin(); i != contest->ctList.end(); i++ )
    {
        bool ok;
        int s = i->wt->serials.getValue().toInt(&ok );
       if ( ok && serial == s )
       {
          cfu = i->wt;
          break;
       }
    }

    if ( cfu )
    {
       EditContact( cfu );
    }
    else
       MinosParameters::getMinosParameters() ->mshowMessage( "Serial number " + QString::number( serial ) + " not found" );
}

void TSingleLogFrame::on_GoToSerial(BaseContestLog *ct)
{
    if (ct == contest)
    {
       goSerial();
    }
}

//---------------------------------------------------------------------------
void TSingleLogFrame::on_KeyerLoaded()
{
   keyerLoaded = true;
   ui->GJVQSOLogFrame->setKeyerLoaded();
}
bool TSingleLogFrame::isKeyerLoaded()
{
   return keyerLoaded;
}


void TSingleLogFrame::sendKeyerPlay( int fno )
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        sendDM->sendKeyerPlay(fno);
}

void TSingleLogFrame::sendKeyerRecord( int fno )
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        sendDM->sendKeyerRecord(fno);
}

void TSingleLogFrame::sendBandMap( QString freq, QString call, QString utc, QString loc, QString qth )
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        sendDM->sendBandMap(freq, call, utc, loc, qth);
}

void TSingleLogFrame::sendKeyerTone()
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        sendDM->sendKeyerTone();
}

void TSingleLogFrame::sendKeyerTwoTone()
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        sendDM->sendKeyerTwoTone();
}

void TSingleLogFrame::sendKeyerStop()
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        sendDM->sendKeyerStop();
}


//---------------------------------------------------------------------------

// Bandmap

void TSingleLogFrame::on_BandMapLoaded()
{
   bandMapLoaded = true;
   ui->GJVQSOLogFrame->setBandMapLoaded();
}

bool TSingleLogFrame::isBandMapLoaded()
{
   return bandMapLoaded;
}

//---------------------------------------------------------------------------

// RigControl

void TSingleLogFrame::on_SetMode(QString m)
{
    if (sCurMode != m)
    {
        sCurMode = m;
        if (modeUpDateCnt < 1)
        {
            modeUpDateCnt++;
            ui->FKHRigControlFrame->setMode(m);
            ui->GJVQSOLogFrame->modeSentFromRig(m);
        }
        else if ( this == LogContainer->getCurrentLogFrame() )
        {
            ui->FKHRigControlFrame->setMode(m);
            ui->GJVQSOLogFrame->modeSentFromRig(m);
        }
    }
}

void TSingleLogFrame::on_SetFreq(QString f)
{

    if (sCurFreq != f)
    {
        sCurFreq = f;
        if (freqUpDateCnt < 1)
        {
            freqUpDateCnt++;
            ui->FKHRigControlFrame->setFreq(f);
            ui->GJVQSOLogFrame->setFreq(f);
        }
        else if ( this == LogContainer->getCurrentLogFrame() )
        {
            ui->FKHRigControlFrame->setFreq(f);
            ui->GJVQSOLogFrame->setFreq(f);
        }
    }
}

void TSingleLogFrame::on_NoRadioSetFreq(QString f)
{
    ui->GJVQSOLogFrame->setFreq(f);
}

void TSingleLogFrame::on_NoRadioSetMode(QString m)
{
    ui->GJVQSOLogFrame->modeSentFromRig(m);
}

void TSingleLogFrame::on_RadioLoaded()
{
    ui->FKHRigControlFrame->setRadioLoaded();
    ui->GJVQSOLogFrame->setRadioLoaded();
}

bool TSingleLogFrame::isRadioLoaded()
{
   return ui->FKHRigControlFrame->isRadioLoaded();
}

void TSingleLogFrame::on_SetRadioName(QString n)
{
    ui->FKHRigControlFrame->setRadioName(n);
    ui->GJVQSOLogFrame->setRadioName(n);
}

void TSingleLogFrame::on_SetRadioState(QString s)
{
    ui->FKHRigControlFrame->setRadioState(s);
    ui->GJVQSOLogFrame->setRadioState(s);
}


void TSingleLogFrame::on_SetRadioTxVertState(QString s)
{
    ui->FKHRigControlFrame->setRadioTxVertState(s);
}


void TSingleLogFrame::sendRadioFreq(QString freq)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        sendDM->sendRigControlFreq(freq);
}

void TSingleLogFrame::sendRadioMode(QString mode)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        sendDM->sendRigControlMode(mode);
}


/*
void TSingleLogFrame::sendRadioPassBandState(int state)
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        sendDM->sendRigControlPassBandState(state);
}
*/

//---------------------------------------------------------------------------

// RotatorControl



void TSingleLogFrame::on_RotatorLoaded()
{
   rotatorLoaded = true;
   ui->FKHRotControlFrame->setRotatorLoaded();
   ui->GJVQSOLogFrame->setRotatorLoaded();
}

bool TSingleLogFrame::isRotatorLoaded()
{
   return rotatorLoaded;
}

void TSingleLogFrame::on_RotatorState(QString s)
{
    ui->FKHRotControlFrame->setRotatorState(s);
}

void TSingleLogFrame::on_RotatorBearing(QString s)
{
    ui->FKHRotControlFrame->setRotatorBearing(s);
    ui->GJVQSOLogFrame->setRotatorBearing(s);
}


void TSingleLogFrame::on_RotatorMaxAzimuth(QString s)
{
    ui->FKHRotControlFrame->setRotatorMaxAzimuth(s);
}

void TSingleLogFrame::on_RotatorMinAzimuth(QString s)
{
    ui->FKHRotControlFrame->setRotatorMinAzimuth(s);
}


void TSingleLogFrame::on_RotatorAntennaName(QString s)
{
    ui->FKHRotControlFrame->setRotatorAntennaName(s);
}


void TSingleLogFrame::sendRotator(rpcConstants::RotateDirection direction, int angle )
{
    if (contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        sendDM->sendRotator(direction, angle);
}
