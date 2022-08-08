#include "contest.h"
#include "MinosLoggerEvents.h"
#include "ScreenContact.h"
#include "MatchThread.h"
#include "cutils.h"
#include "MonitoredLog.h"
#include "MonitoringFrame.h"
#include "MonitorMain.h"
#include "ui_MonitorMain.h"

MonitorMain *monitorMain = nullptr;

//=============================================================================================

MonitorMain::MonitorMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MonitorMain)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    monitorMain = this;

    createCloseEvent();
#ifdef Q_OS_ANDROID
    splitterHandleWidth = 20;
#else
    splitterHandleWidth = 6;
#endif
    QSettings settings;
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    MultLists::getMultLists(); // make sure everything is loaded

    treeModel = new MonitorTreeModel();
    ui->monitorTree->setModel(treeModel);
    ui->monitorTree->header()->show();

    monitorTimer = new QTimer();

    connect(monitorTimer, &QTimer::timeout, this, &MonitorMain::on_monitorTimeout);

    monitorTimer->start(100);


    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName(), true);

    MinosConfig *config = MinosConfig::getMinosConfig();
    localRouterName = config->getThisRouterName();


    connect(rpc, &MinosRPC::routerCall, this, &MonitorMain::on_routerCall);
    connect(rpc, &MinosRPC::notify, this, &MonitorMain::on_notify);
    connect(rpc, &MinosRPC::provider, this, &MonitorMain::on_provider);

    QStringList sv = {rpcConstants::monitorLogCategory};
    rpc->findProviders(rpcConstants::LoggerCategory, sv);

    QByteArray state;

    state = settings.value("MonitorSplitter/state").toByteArray();
    if (state.size())
    {
        ui->monitorSplitter->restoreState(state);
    }
    else
    {
        QList<int> split{200, 600};
        ui->monitorSplitter->setSizes(split);
    }

    state = settings.value("MainSplitter/state").toByteArray();
    if (state.size())
        ui->mainSplitter->restoreState(state);

    state = settings.value("SearchSplitter/state").toByteArray();
    if (state.size())
        ui->searchSplitter->restoreState(state);

    ui->monitorSplitter->setHandleWidth(splitterHandleWidth);
    ui->mainSplitter->setHandleWidth(splitterHandleWidth);
    ui->searchSplitter->setHandleWidth(splitterHandleWidth);

    ui->contestPageControl->setContextMenuPolicy( Qt::CustomContextMenu );

    closeMonitoredLog = newAction(tr("Close tab"), &TabPopup, &MonitorMain::on_closeMonitoredLog);
    newAction( "Cancel", &TabPopup, &MonitorMain::CancelClick );

    ui->callsignEdit->setValidator(&ucValidator);
    ui->locEdit->setValidator(&ucValidator);
    ui->exchangeEdit->setValidator(&ucValidator);
    ui->callsignEdit->installEventFilter(this);
    ui->locEdit->installEventFilter(this);
    ui->exchangeEdit->installEventFilter(this);


    TMatchThread::InitialiseMatchThread();
    ui->thisMatchFrame->initialise();
    ui->thisMatchFrame->setBaseName("Monitor");
    ui->thisMatchFrame->restoreColumns();
    ui->otherMatchFrame->initialise();
    ui->otherMatchFrame->setBaseName("Monitor");
    ui->otherMatchFrame->restoreColumns();

    ui->callsignEdit->setFocus();

    readPersistedLogs();
}

MonitorMain::~MonitorMain()
{
    delete ui;
    stationList.clear();
    delete MultLists::getMultLists();
}
void MonitorMain::closeEvent(QCloseEvent *event)
{
    // and tidy up all loose ends
    TMatchThread::FinishMatchThread();

    QWidget::closeEvent(event);
}
void MonitorMain::moveEvent(QMoveEvent * event)
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    QWidget::moveEvent(event);
}
void MonitorMain::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    QWidget::resizeEvent(event);
}
void MonitorMain::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("geometry", saveGeometry());
    }
}
bool MonitorMain::eventFilter(QObject * /*obj*/, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *ke = dynamic_cast<QKeyEvent *>(event);
        int Key = ke->key();

        if (Key == Qt::Key_Escape)
        {
            ui->callsignEdit->clear();
            ui->locEdit->clear();
            ui->exchangeEdit->clear();
            ui->callsignEdit->setFocus();
            return true;
        }
    }
    return false;
}

void MonitorMain::readPersistedLogs()
{
    inReadPersistedLogs = true;

    // read in the persistence file

    inReadPersistedLogs =false;
}

void MonitorMain::writePersistedLogs()
{
    if (!inReadPersistedLogs)
    {
        for ( auto const &s: qAsConst(stationList) )
        {
            for ( auto const &l: qAsConst(s->slotList) )
            {
            }
        }
    }
}
void MonitorMain::on_callsignEdit_textChanged(const QString &/*arg1*/)
{
    searchChanged();
}

void MonitorMain::on_monitorSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    QByteArray state = ui->monitorSplitter->saveState();
    QSettings settings;
    settings.setValue("MonitorSplitter/state", state);
}
void MonitorMain::on_mainSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    QByteArray state = ui->mainSplitter->saveState();
    QSettings settings;
    settings.setValue("MainSplitter/state", state);
}

void MonitorMain::on_searchSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    QByteArray state = ui->searchSplitter->saveState();
    QSettings settings;
    settings.setValue("SearchSplitter/state", state);
}

void MonitorMain::closeTab(MonitoringFrame *cttab)
{
    for ( auto const &s: qAsConst(stationList) )
    {
        for ( auto const &l: qAsConst(s->slotList) )
        {
            if (l->getFrame() == cttab)
            {
                // take it out of the slot list and close it
                // and we need to redo the list
                //treeModel->clear();
                l->setEnabled(false);
                l->setFrame(nullptr);
                ui->contestPageControl->removeTab(ui->contestPageControl->indexOf(cttab));
                delete cttab;
                return;
            }
        }
    }

}

int MonitorMain::getContestSlotCount()
{
    return ui->contestPageControl->count();
}

BaseContestLog *MonitorMain::getContestSlot(int s)
{
    QWidget *tw = ui->contestPageControl->widget(s);
    MonitoringFrame *f = dynamic_cast<MonitoringFrame *>(tw);
    if (f)
    {
        return f->getContest();
    }
    return nullptr;
}

BaseContestLog *MonitorMain::getCurrentContest()
{
    MonitoringFrame *mf = findCurrentLogFrame();
    if (mf)
        return mf->getContest();

    return nullptr;
}
void MonitorMain::on_contestPageControl_customContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = ui->contestPageControl->mapToGlobal( pos );

    closeMonitoredLog->setEnabled(findCurrentLogFrame() != nullptr);

    TabPopup.popup( globalPos );
}
QAction *MonitorMain::newAction(const QString &text, QMenu *m, void (MonitorMain::*slotparam)() )
{
    QAction * newAct = new QAction( text, this );
    m->addAction( newAct );
    connect( newAct, &QAction::triggered , this, slotparam );
    return newAct;
}
void MonitorMain::on_closeMonitoredLog()
{
    closeTab(findCurrentLogFrame());
}
void MonitorMain::CancelClick()
{
    // do nothing...
}
//---------------------------------------------------------------------------
void MonitorMain::on_provider(Provider provider, QString /*cat*/)
{
    stationList[provider] = new MonitoredStation;
    syncstat = true;
}

void MonitorMain::on_notify(AnalysePubSubNotify an, const QString from )
{
    // pubsub notify
    trace( "Notify callback from " + from + ( !an.getOK() ? ":Error" : ":Normal" ) );

    if ( an.getOK() )
    {
        PublishState state = an.getState();
        QString key = an.getKey();          // key is minos file name
        QString value = an.getValue();      // value is stanzacount;[band] name;start time;end time

        if ( an.getCategory() == rpcConstants::monitorLogCategory )
        {
            QString router = an.getRouter();
            if ( router.size() == 0 )
            {
                // it is for us...
                router = localRouterName;
            }

            QString logval = router + " : " + key ;
            trace( "ContestLog " + logval + " " + value );

            MonitoredStation *stat = stationList[Provider(an)];

            QVector< QSharedPointer<MonitoredLog> >::iterator log = std::find_if( stat->slotList.begin(), stat->slotList.end(), MonitoredLogCmp( key ) );
            if (state == psPublished)
            {
                QStringList args = value.split(";");
                if ( log == stat->slotList.end() )
                {
                    QSharedPointer<MonitoredLog> ml(new MonitoredLog());
                    ml->initialise( router, key );


                    if (args.count() >= 1)
                    {
                        trace(QString("args 0 %1 ").arg(args[0]));
                        ml->setExpectedStanzaCount( args[0].toInt() );
                    }
                    if (args.count() >= 2)
                    {
                        trace(QString("args 1 %2").arg(args[1]));
                        ml->setDisplayName(args[1]);
                    }
                    else
                        ml->setDisplayName(key);

                    ml->setState(state);
                    stat->slotList.push_back( ml );
                    syncstat = true;

                }
                else
                {
                    if (args.count() >= 1)
                    {
                        trace(QString("args 0 %1 ").arg(args[0]));
                        (*log)->setExpectedStanzaCount( args[0].toInt() );
                    }
                    (*log)->setState(state);
                }
            }
            else
            {
                if ( log != stat->slotList.end() )
                {
                    (*log)->setState(state);
                }
            }
        }
    }
}
//---------------------------------------------------------------------------
void MonitorMain::on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from )
{
    trace( "logger router callback from " + from + ( err ? ":Error" : ":Normal" ) );
    if ( !err )
    {
        // This will return stanza id, pubname, and stanza content
        QString call = mro->getMethodName();
        if (call == rpcConstants::loggerStanzaResponse)
        {

            QSharedPointer<RPCParam> psLogName;
            QSharedPointer<RPCParam> psStanzaData;
            QSharedPointer<RPCParam> psStanza;
            QSharedPointer<RPCParam> psResult;
            RPCArgs *args = mro->getCallArgs();
            if ( args->getStructArgMember( 0, "LogName", psLogName )
                 && args->getStructArgMember( 0, "LoggerResult", psResult )
                 && args->getStructArgMember( 0, "Stanza", psStanza )
                 && args->getStructArgMember( 0, "StanzaData", psStanzaData )
                 )
            {
                QString logName;
                QString stanzaData;
                bool result;
                int stanza;

                if ( psLogName->getString( logName ) && psStanzaData->getString( stanzaData )
                     && psStanza->getInt( stanza ) && psResult->getBoolean( result )
                     )
                {
                    trace( "Name " + logName + " stanza " + QString::number( stanza ) );
                    // Find the matching MonitoredLog and send the stanza their for processing

                    QStringList sl = from.split('@');
                    if (sl.count() != 2)
                    {
                        return;
                    }
                    QString ss = sl[1] + "/" + sl[0] + "/xxx";
                    Provider p(ss);

                    MonitoredStation *s = stationList[p];

                    for ( auto const &l: qAsConst(s->slotList) )
                    {
                        if (l && l->getPublishedName() == logName )
                        {
                            trace( "||" + stanzaData + "||" );
                            l ->processLogStanza( stanza, stanzaData );
                            return ;
                        }
                    }
                }
            }
        }
    }
}

void MonitorMain::syncStations()
{
   // Here we want to subscribe to the loggers of the notified stations
   // Shouldn't matter if we end up doing it twice

   // Strictly I suppose we could ignore ourselves, but normally we won't run
   // a full monitor on a logging computer - we need a module that can
   // manage single stations for that.

   // And this module must make use of that single station monitor!

   // Probably it is basically the "MonitoredStation" class as a model, with a viewer
   // and maybe a controller...

  if ( syncstat )
   {
      syncstat = false;

      TreeNode *root = new RootTreeNode(this);
      for ( auto s = stationList.begin(); s != stationList.end(); s++ )
      {
          TreeNode *snode = new RouterTreeNode(root, s.key().app + "@" + s.key().routerName);
          for ( auto const &l: qAsConst(s.value()->slotList) )
          {
              /*TreeNode *lnode =*/ new LogTreeNode(snode, l);
          }
      }
      treeModel->setRoot(root);
      int rc = treeModel->rowCount();
      for(int i = 0; i < rc; i++)
      {
        ui->monitorTree->setFirstColumnSpanned( i, QModelIndex(), true );
      }
      ui->monitorTree->expandAll();
   }
}
void MonitorMain::addSlot(  QSharedPointer< MonitoredLog>ct )
{
   static int namegen = 0;
   QString baseFName = ExtractFileName( ct->getPublishedName() );

   MonitoringFrame *f = new MonitoringFrame( this );
   f->setObjectName( QString( "LogFrame" ) + QString::number(namegen++));

   f->initialise( ct->getContest() );
   ct->setFrame( f );

   int tno = ui->contestPageControl->addTab(f, baseFName);
   ui->contestPageControl->setCurrentWidget(ui->contestPageControl->widget(tno));
   ui->contestPageControl->setTabToolTip(tno, ct->getPublishedName());
   f->showQSOs();
   f->setFocusPolicy(Qt::NoFocus);

//   QSettings

   // we need to save the list of filenames, so that when we see them again we
   // can re-open them when we are restarted
}

MonitoringFrame *MonitorMain::findCurrentLogFrame()
{
    QWidget *w = ui->contestPageControl->currentWidget();
    MonitoringFrame *f = dynamic_cast<MonitoringFrame *>(w);
    return f;
}
MonitoringFrame *MonitorMain::findContestPage( BaseContestLog *ct )
{
   // we need to find the embedded frame...
   if ( !ui->contestPageControl->count() || !ct )
      return nullptr;
   int pc = ui->contestPageControl->count();
   for ( int i = 0; i < pc; i++ )
   {
       QWidget *tw = ui->contestPageControl->widget(i);
       MonitoringFrame *f = dynamic_cast<MonitoringFrame *>(tw);
       if (f)
       {
           if (f->getContest() == ct)
               return f;
       }
   }
   return nullptr;
}

bool nolog( MonitoredLog *ip )
{
   if ( ip == nullptr )
      return true;
   else
      return false;
}

void MonitorMain::on_monitorTimeout()
{
    static bool closed = false;
    if ( !closed )
    {
       if ( checkCloseEvent() )
       {
          closed = true;
          close();
       }
    }
    for ( auto const &s: qAsConst(stationList) )
    {
       for ( auto &l: s->slotList )
       {
          if (l->getState() == psRevoked)
          {
             MonitoringFrame *cttab = findContestPage( l->getContest() );
             closeTab(cttab);
             // take it out of the slot list and close it
             // and we need to redo the list
             s->slotList.erase(&l);
             syncstat = true;
             break;             // as we have changed the list - don't continue

          }
          else
          {
             l->checkMonitor();
          }
       }
       if (syncstat)
       {
          syncStations();
       }
    }
    static int ticks = 0;
    if (ticks++ > 10)
    {
        MonitoringFrame *f = findCurrentLogFrame();
        if ( f )
        {
            f->on_monitorTimeout();
        }
        ticks = 0;
    }
}
void MonitorMain::on_monitorTree_doubleClicked(const QModelIndex &index)
{
    // apply double click to node MonitorTreeClickNode
    TreeNode * sel = static_cast< TreeNode *>(index.internalPointer());

    if (!sel)
    {
       return;
    }
    if ( sel->GetNodeType() != entLog )
    {
       // station
    }
    else
    {
        RouterTreeNode *tn = static_cast< RouterTreeNode *>(index.parent().internalPointer());
        QString pn = tn->data(0);

        QStringList sl = pn.split('@');
        if (sl.count() != 2)
        {
            return;
        }
        QString s = sl[1] + "/" + sl[0] + "/xxx";
        Provider p(s);
       MonitoredStation *ms = stationList[ p ];
       if (!ms)
       {
           return;
       }
       // log
       if (sel->childNumber() >= ms->slotList.count())
       {
           return;
       }
      QSharedPointer< MonitoredLog> ml = ms ->slotList[ sel->childNumber()];
      if (!ml)
      {
          return;
      }
      if ( !ml->enabled() )
      {
         ml->startMonitor();
         addSlot( ml );
         sel->setLog(ml);
         syncstat = true;
      }
      else
      {
         MonitoringFrame *cttab = findContestPage( ml->getContest() );
         if ( cttab )
         {
            ui->contestPageControl->setCurrentWidget(cttab);
         }
      }
    }
}

void MonitorMain::on_contestPageControl_tabCloseRequested(int index)
{
    // close tab index
    QWidget *w = ui->contestPageControl->widget(index);
    MonitoringFrame *f = dynamic_cast<MonitoringFrame *>(w);

    closeTab(f);
}

void MonitorMain::searchChanged()
{
    MonitoringFrame *mf = findCurrentLogFrame();
    if (!mf)
        return;
    BaseContestLog *bct = mf->getContest();
    if (!bct)
        return;

    screenContact.cs.setFullCall(ui->callsignEdit->text());
    screenContact.loc.setLoc(ui->locEdit->text());
    screenContact.extraText.setValue( ui->exchangeEdit->text().trimmed());

    MinosLoggerEvents::SendScreenContactChanged(&screenContact, bct, "Monitor");

}

void MonitorMain::on_locEdit_textChanged(const QString &/*arg1*/)
{
    searchChanged();
}

void MonitorMain::on_exchangeEdit_textChanged(const QString &/*arg1*/)
{
    searchChanged();
}

void MonitorMain::on_contestPageControl_currentChanged(int /*index*/)
{
    MonitoringFrame *mf = findCurrentLogFrame();
    if (!mf)
        return;
    BaseContestLog *bct = mf->getContest();
    if (!bct)
        return;

    ui->thisMatchFrame->setContest(bct);
    ui->otherMatchFrame->setContest(bct);

    searchChanged();
    ui->callsignEdit->setFocus();
}

