#include "base_pch.h"
#include "contest.h"
#include "MonitoredLog.h"
#include "MonitoringFrame.h"
#include "MonitorMain.h"
#include "ui_MonitorMain.h"
//=============================================================================================
int TreeNode::find( const TreeNode *t ) const
{
    int i = 0;
    for ( auto v : nodes )
    {
        if ( v == t )
            return i;
        i++;
    }
    return 0;
}

TreeNode *TreeNode::parent()
{
    return parentItem;
}

TreeNode *TreeNode::child( int number )
{
    return nodes[ number ];
}

int TreeNode::childCount() const
{
    return nodes.size();
}
int TreeNode::childNumber() const
{
    if ( parentItem )
    {
        return parentItem->find( this );
    }
    return 0;
}
void TreeNode::clear()
{
    for ( QVector<TreeNode *>::iterator tn = nodes.begin(); tn != nodes.end(); tn++ )
    {
        delete ( *tn );
    }
    nodes.clear();
}
QString RootTreeNode::data(int /*column*/)
{
    return Name();
}
QString ServerTreeNode::data(int column)
{
    if (column == 0)
        return Name();
    return "";
}
static QStringList stateList =
{
   "P",
   "R",
   "NC"
};
QString LogTreeNode::data(int column)
{
    if (column == 1)
        return Name();

    if (column == 0)
    {
        QString state;
        if (mlog->getFrame())
            state = "Monitoring";
        return state;
    }
    return "";
}
MonitorTreeModel::MonitorTreeModel()
        : QAbstractItemModel( nullptr ), rootData( nullptr )
{}
MonitorTreeModel::~MonitorTreeModel()
{
    delete rootData;
}
void MonitorTreeModel::clear()
{
    beginResetModel();

    delete rootData;
    rootData = nullptr;

    endResetModel();
}
void MonitorTreeModel::setRoot(  TreeNode *root )
{
    beginResetModel();
    delete rootData;
    rootData = root;
    // And we probably need to tell the view that everything has changed
    endResetModel();
}

int MonitorTreeModel::columnCount( const QModelIndex & parent  ) const
{
    TreeNode *parentItem = getItem( parent );
    if (parentItem && parentItem->GetNodeType() == entServer)
        return 2;

    return 2;
}

QVariant MonitorTreeModel::data( const QModelIndex &index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();

    if ( role != Qt::DisplayRole && role != Qt::EditRole )
        return QVariant();

    TreeNode *item = getItem( index );

    return item->data( index.column() );
}

QVariant MonitorTreeModel::headerData( int section, Qt::Orientation orientation,
                     int role ) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        QString cell;
        switch (section)
        {
        case 1:
            cell = "Contest Name";
            break;

        case 0:
            cell = "State";
            break;

        default:
            break;
        }

        return cell;
    }
    return QVariant();
}

QModelIndex MonitorTreeModel::index( int row, int column, const QModelIndex &parent ) const
{
    if ( parent.isValid() && parent.column() != 0 )
        return QModelIndex();

    TreeNode *parentItem = getItem( parent );

    if ( parentItem && row < parentItem->childCount() )
    {
        TreeNode * childItem = parentItem->child( row );
        if ( childItem )
            return createIndex( row, column, childItem );
    }
    return QModelIndex();
}
QModelIndex MonitorTreeModel::parent( const QModelIndex &index ) const
{
    if ( !index.isValid() )
        return QModelIndex();

    TreeNode *childItem = getItem( index );
    TreeNode *parentItem = childItem->parent();

    if ( parentItem == rootData )
        return QModelIndex();

    return createIndex( parentItem->childNumber(), 0, parentItem );
}
int MonitorTreeModel::rowCount( const QModelIndex &parent ) const
{
    TreeNode * parentItem = getItem( parent );

    if ( parentItem )
        return parentItem->childCount();

    return 0;
}
TreeNode *MonitorTreeModel::getItem( const QModelIndex &index ) const
{
    if ( index.isValid() )
    {
        TreeNode * item = static_cast<TreeNode *>( index.internalPointer() );
        if ( item )
            return item;
    }
    return rootData;
}


//=============================================================================================

MonitorMain::MonitorMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MonitorMain)
{
    ui->setupUi(this);
    connect(&stdinReader, SIGNAL(stdinLine(QString)), this, SLOT(onStdInRead(QString)));
    stdinReader.start();

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

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

    connect(monitorTimer, SIGNAL(timeout()), this, SLOT(on_monitorTimeout()));

    monitorTimer->start(100);


    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName(), true);

    connect(rpc, SIGNAL(serverCall(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_serverCall(bool,QSharedPointer<MinosRPCObj>,QString)));
    connect(rpc, SIGNAL(notify(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_notify(bool,QSharedPointer<MinosRPCObj>,QString)));

    //rpc->subscribe(rpcConstants::StationCategory);
    rpc->subscribe(rpcConstants::LocalStationCategory);

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
    ui->monitorSplitter->setHandleWidth(splitterHandleWidth);
    ui->contestPageControl->setContextMenuPolicy( Qt::CustomContextMenu );

    closeMonitoredLog = newAction("Close tab", &TabPopup, SLOT(on_closeMonitoredLog()));
    newAction( "Cancel", &TabPopup, SLOT( CancelClick() ) );
}

MonitorMain::~MonitorMain()
{
    delete ui;
    for ( QVector<MonitoredStation *>::iterator i = stationList.begin(); i != stationList.end(); i++ )
    {
       for ( QVector< MonitoredLog *>::iterator j = ( *i ) ->slotList.begin(); j != ( *i ) ->slotList.end(); j++ )
       {
          delete ( *j );
       }
       ( *i ) ->slotList.clear();
       delete ( *i );
    }
    stationList.clear();
    delete MultLists::getMultLists();
}
void MonitorMain::closeEvent(QCloseEvent *event)
{
    // and tidy up all loose ends

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
void MonitorMain::onStdInRead(QString cmd)
{
    trace("Command read from stdin: " + cmd);
    if (cmd.indexOf("ShowServers", 0, Qt::CaseInsensitive) >= 0)
        setShowServers(true);
    if (cmd.indexOf("HideServers", 0, Qt::CaseInsensitive) >= 0)
        setShowServers(false);
}

void MonitorMain::on_monitorSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    QByteArray state = ui->monitorSplitter->saveState();
    QSettings settings;
    settings.setValue("MonitorSplitter/state", state);
}

void MonitorMain::on_closeButton_clicked()
{
    close();
}
void MonitorMain::logMessage( const QString &s )
{
   trace( s );
}
void MonitorMain::closeTab(MonitoringFrame *cttab)
{
    for ( QVector<MonitoredStation *>::iterator i = stationList.begin(); i != stationList.end(); i++ )
    {
        for ( QVector<MonitoredLog *>::iterator j = ( *i ) ->slotList.begin(); j != ( *i ) ->slotList.end(); j++ )
        {
            if ((*j)->getFrame() == cttab)
            {
                // take it out of the slot list and close it
                // and we need to redo the list
                //treeModel->clear();
                (*j)->setEnabled(false);
                (*j)->setFrame(nullptr);
                ui->contestPageControl->removeTab(ui->contestPageControl->indexOf(cttab));
                delete cttab;
                return;
            }
        }
    }

}
void MonitorMain::on_contestPageControl_customContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = ui->contestPageControl->mapToGlobal( pos );

    closeMonitoredLog->setEnabled(findCurrentLogFrame() != nullptr);

    TabPopup.popup( globalPos );
}
QAction *MonitorMain::newAction( const QString &text, QMenu *m, const char *atype )
{
    QAction * newAct = new QAction( text, this );
    m->addAction( newAct );
    connect( newAct, SIGNAL( triggered() ), this, atype );
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

void MonitorMain::on_notify(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from )
{
    // pubsub notify
    logMessage( "Notify callback from " + from + ( err ? ":Error" : ":Normal" ) );
    AnalysePubSubNotify an( err, mro );

    if ( an.getOK() )
    {
       PublishState state = an.getState();
       QString key = an.getKey();
       QString value = an.getValue();

       if ( an.getCategory() == rpcConstants::LocalStationCategory )
       {
          // This state better not be anything other than published!
          localServerName = an.getKey();
          RPCPubSub::subscribe( rpcConstants::StationCategory );  //want ALL keys - but do it once we know who WE are!
       }
       if ( an.getCategory() == rpcConstants::StationCategory )
       {
           RPCPubSub::subscribeRemote( key, rpcConstants::LoggerCategory );  //want ALL keys - but do it once we know who WE are!
       }
       if ( an.getCategory() == rpcConstants::LoggerCategory )
       {

           logMessage( "Station " + key + " " + value );
          QVector<MonitoredStation *>::iterator stat = std::find_if( stationList.begin(), stationList.end(), MonitoredStationCmp( key, an.getPublisherProgram() ) );

          if (state != psRevoked)
          {
             if ( stat == stationList.end() )
             {
                MonitoredStation * ms = new MonitoredStation();
                ms->stationName = key;
                ms->state = state;
                ms->publisher = an.getPublisherProgram();
                stationList.push_back( ms );
                RPCPubSub::subscribeRemote( key, rpcConstants::monitorLogCategory);
             }
             else
             {
                (*stat)->state = state;
             }
          }
          else
          {
             (*stat)->state = state;
          }
       }
       if ( an.getCategory() == rpcConstants::monitorLogCategory )
       {
          QString server = an.getServer();
          if ( server.size() == 0 )
          {
             // it is for us...
             server = localServerName;
          }

          QString logval = server + " : " + key ;
          logMessage( "ContestLog " + logval + " " + value );

          QVector<MonitoredStation *>::iterator stat = std::find_if( stationList.begin(), stationList.end(), MonitoredStationCmp( server, an.getPublisherProgram() ) );
          if ( stat != stationList.end() )
          {
             QVector< MonitoredLog *>::iterator log = std::find_if( ( *stat ) ->slotList.begin(), ( *stat ) ->slotList.end(), MonitoredLogCmp( key ) );
             if (state == psPublished)
             {
                if ( log == ( *stat ) ->slotList.end() )
                {
                   MonitoredLog * ml = new MonitoredLog();
                   ml->initialise( server, key );
                   ml->setExpectedStanzaCount( value.toInt() );
                   ml->setState(state);
                   ( *stat ) ->slotList.push_back( ml );
                   syncstat = true;

                }
                else
                {
                   ( *log ) ->setExpectedStanzaCount( value.toInt() );
                   (*log)->setState(state);
                }
             }
             else
             {
                if ( log != ( *stat ) ->slotList.end() )
                {
                   (*log)->setState(state);
                }
             }
          }
       }
    }
}
//---------------------------------------------------------------------------
void MonitorMain::on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from )
{
    logMessage( "logger server callback from " + from + ( err ? ":Error" : ":Normal" ) );
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
                    logMessage( "Name " + logName + " stanza " + QString::number( stanza ) );
                    // Find the matching MonitoredLog and send the stanza their for processing
                    for ( QVector<MonitoredStation *>::iterator i = stationList.begin(); i != stationList.end(); i++ )
                    {
                        // "from" is something like Logger@dev-station
                        // BUT it isn't necessarily Logger!
                        if ( ( *i ) ->publisher + "@" + (*i)->stationName == from )
                        {
                            for ( QVector<MonitoredLog *>::iterator j = ( *i ) ->slotList.begin(); j != ( *i ) ->slotList.end(); j++ )
                            {
                                if ((*j) && ( *j ) ->getPublishedName() == logName )
                                {
                                    logMessage( "||" + stanzaData + "||" );
                                    ( *j ) ->processLogStanza( stanza, stanzaData );
                                    return ;
                                }
                            }
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
      for ( QVector<MonitoredStation *>::iterator i = stationList.begin(); i != stationList.end(); i++ )
      {
          TreeNode *snode = new ServerTreeNode(root, (*i)->stationName);
          for ( QVector<MonitoredLog *>::iterator j = ( *i ) ->slotList.begin(); j != ( *i ) ->slotList.end(); j++ )
          {
              /*TreeNode *lnode =*/ new LogTreeNode(snode, (*j));
          }
      }
      treeModel->setRoot(root);
      int rc = treeModel->rowCount();
      for(int i = 0; i < rc; i++)
      ui->monitorTree->setFirstColumnSpanned( i, QModelIndex(), true );
      ui->monitorTree->expandAll();
   }
}
void MonitorMain::addSlot( MonitoredLog *ct )
{
   static int namegen = 0;
   QString baseFName = ExtractFileName( ct->getPublishedName() );

   MonitoringFrame *f = new MonitoringFrame( this );
   f->setObjectName( QString( "LogFrame" ) + QString::number(namegen++));

   int tno = ui->contestPageControl->addTab(f, baseFName);
   ui->contestPageControl->setCurrentWidget(ui->contestPageControl->widget(tno));
   ui->contestPageControl->setTabToolTip(tno, ct->getPublishedName());


   f->initialise( ct->getContest() );
   ct->setFrame( f );
   f->showQSOs();
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
    int icnt = 0;
    for ( QVector<MonitoredStation *>::iterator i = stationList.begin(); i != stationList.end(); i++ )
    {
        icnt++;
        int jcnt = 0;
       for ( QVector<MonitoredLog *>::iterator j = ( *i ) ->slotList.begin(); j != ( *i ) ->slotList.end(); j++ )
       {
           jcnt++;
          if ((*j)->getState() == psRevoked)
          {
             QWidget *cttab = findContestPage( (*j)->getContest() );
             if ( cttab )
             {
                delete cttab;
             }
             // take it out of the slot list and close it
             // and we need to redo the list
             delete (*j);
             (*j) = nullptr;
             (*i)->slotList.erase(j);
             syncstat = true;
             break;             // as we have changed the list - don't continue

          }
          else
          {
             ( *j ) ->checkMonitor();
          }
       }
       if (syncstat)
       {
          syncStations();
       }
    }
    MonitoringFrame *f = findCurrentLogFrame();
    if ( f )
    {
        f->getContest()->scanContest();
        f->setScore();
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
       MonitoredStation *ms = stationList[ sel->parent()->childNumber() ];
       // log
      MonitoredLog * ml = ms ->slotList[ sel->childNumber()];
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
