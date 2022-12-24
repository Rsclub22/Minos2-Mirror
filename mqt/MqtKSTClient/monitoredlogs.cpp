#include "AppStartup.h"
#include "MTrace.h"
#include "MonitoredLog.h"
#include "monitoredlogs.h"
#include "MonitorTreeModel.h"
#include "ui_monitoredlogs.h"

MonitoredLogs::MonitoredLogs(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MonitoredLogs)
{
    ui->setupUi(this);

    treeModel = new MonitorTreeModel();
    ui->monitorTree->setModel(treeModel);
    ui->monitorTree->header()->show();

    monitorTimer = new QTimer();

    connect(monitorTimer, &QTimer::timeout, this, &MonitoredLogs::on_monitorTimeout);

    monitorTimer->start(100);

    /*MinosRPC *rpc =*/ MinosRPC::getMinosRPC(getAppStartupName(), true);
    remoteLogs = new RemoteLogs;

    connect(remoteLogs, &RemoteLogs::syncNeeded, this, &MonitoredLogs::onSyncNeeded);
    connect(remoteLogs, &RemoteLogs::newMonitoredLog, this, &MonitoredLogs::onNewLog);


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
             //MonitoringFrame *cttab = findContestPage( (*l)->getContest() );
             //closeTab(cttab);
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
        ui->monitorTree->setFirstColumnSpanned( i, QModelIndex(), true );
      }
      ui->monitorTree->expandAll();
   }
}
void MonitoredLogs::on_monitorTree_doubleClicked(const QModelIndex &index)
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
       }
    }
}

//=================================================================
// callback slots from RemoteLogs

void MonitoredLogs::onSyncNeeded()
{
    syncstat = true;
}

void MonitoredLogs::onNewLog(MonitoredLog *ml)
{
    connect(ml, &MonitoredLog::newStanzas, this, &MonitoredLogs::onNewStanzas);
    connect(ml, &MonitoredLog::newLastContact, this, &MonitoredLogs::onNewLastContact);
    connect(ml, &MonitoredLog::contactChanged, this, &MonitoredLogs::onContactChanged);
}
//---------------------------------------------------------------------------
// callback slots from RPC in MonitoredLog
void MonitoredLogs::onNewStanzas(MonitoredLog *l)
{
    trace("OnNewStanzas");
//    MonitoringFrame *frame = l->getFrame();
//    if (frame)
//    {
//        frame->newStanzas = true;
//    }
}
void MonitoredLogs::onNewLastContact(MonitoredLog *l)
{
    trace("onNewLastContact");
//    MonitoringFrame *frame = l->getFrame();
//    if (l->getContest()->lastInserted >= 0)
//    {
//        QSharedPointer<BaseContact> bct = l->getContest()->pcontactAt(l->getContest()->lastInserted);
//        frame->qsoModel.insertRows(l->getContest()->lastInserted, 1, QModelIndex());
//        l->getContest()->lastInserted = -1;

//        frame->on_AfterLogContact(l->getContest(), bct);
//    }
}
void MonitoredLogs::onContactChanged(MonitoredLog *l)
{
    trace("onContactChanged");
//    // change to a contact; we need a full rescan to understand it
//    MonitoringFrame *frame = l->getFrame();
//    frame->qsoModel.changeRow(l->getContest()->lastInserted);
//    frame->rescanNeeded = true;
}


void MonitoredLogs::on_OKButton_clicked()
{
    hide();
}

