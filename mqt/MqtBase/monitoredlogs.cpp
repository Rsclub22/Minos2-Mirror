#include <QTimer>
#include "AppStartup.h"
#include "MTrace.h"
#include "MonitorTreeModel.h"
#include "MonitoredLog.h"
#include "monitoredlogs.h"
#include "remotelogs.h"
#include "ui_monitoredlogs.h"

MonitoredLogs::MonitoredLogs(QWidget *parent) :
    QTreeView(parent),
    ui(new Ui::MonitoredLogs)
{
    ui->setupUi(this);

    treeModel = new MonitorTreeModel();
    setModel(treeModel);
    header()->show();

    monitorTimer = new QTimer();

    connect(monitorTimer, &QTimer::timeout, this, &MonitoredLogs::on_monitorTimeout);

    monitorTimer->start(100);

    /*MinosRPC *rpc =*/ MinosRPC::getMinosRPC(getAppStartupName(), true);
    remoteLogs = new RemoteLogs;

    connect(remoteLogs, &RemoteLogs::syncNeeded, this, &MonitoredLogs::onSyncNeeded);

    connect(this, &QTreeView::doubleClicked, this, &MonitoredLogs::onMonitorTree_doubleClicked);
}

MonitoredLogs::~MonitoredLogs()
{
    delete ui;
}

void MonitoredLogs::on_monitorTimeout()
{

    for ( auto const &s: qAsConst(remoteLogs->stationList) )
    {

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        for (QVector< QSharedPointer<MonitoredLog> >::const_iterator l = s->slotList.begin(); l != s->slotList.end(); l++)
#else
        for (QVector< QSharedPointer<MonitoredLog> >::iterator l = s->slotList.begin(); l != s->slotList.end(); l++)
#endif
       {
          if ((*l)->getState() == psRevoked)
          {
              emit logClosed(*l);
             // take it out of the slot list and close it
             // and we need to redo the list
             s->slotList.erase(l);
             syncstat = true;
             break;             // as we have changed the list - don't continue

          }
          else
          {
             (*l)->checkMonitor();
          }
       }
       if (syncstat)
       {
          syncStations();
       }
    }
}

RemoteLogs *MonitoredLogs::getRemoteLogs() const
{
    return remoteLogs;
}

void MonitoredLogs::syncStations()
{
  if ( syncstat )
   {
      syncstat = false;

      TreeNode *root = new RootTreeNode();
      for ( auto s = remoteLogs->stationList.begin(); s != remoteLogs->stationList.end(); s++ )
      {
          // clang complains that snode may leak - but if gets taken over by the tree
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
        setFirstColumnSpanned( i, QModelIndex(), true );
      }
      expandAll();
   }
}
void MonitoredLogs::onMonitorTree_doubleClicked(const QModelIndex &index)
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
        MonitoredStation *ms = remoteLogs->stationList[ p ];
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
          //addSlot( ml );
          //sel->setLog(ml);
          syncstat = true;
          emit logStarted(ml);
       }
    }
}

//=================================================================
// callback slots from RemoteLogs

void MonitoredLogs::onSyncNeeded()
{
    syncstat = true;
}
