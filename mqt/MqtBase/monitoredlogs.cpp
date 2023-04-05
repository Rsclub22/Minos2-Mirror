#include <QTimer>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QTreeView>
#include <QGroupBox>
#include <QCheckBox>
#include <QSettings>

#include "AppStartup.h"
#include "MTrace.h"
#include "MonitorTreeModel.h"
#include "MonitoredLog.h"
#include "cutils.h"
#include "monitoredlogs.h"
#include "remotelogs.h"
#include "ui_monitoredlogs.h"

MonitoredLogs::MonitoredLogs(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::MonitoredLogs)
{
    ui->setupUi(this);

    QVBoxLayout *vbl = new QVBoxLayout(this);

    logTree = new QTreeView(this);

    vbl->addWidget(logTree);
    treeModel = new MonitorTreeModel();
    logTree->setModel(treeModel);
    logTree->header()->show();

    autoStationsBox = new QGroupBox(this);
    autoStationsBox->setTitle(tr("Auto start stations"));
    vbl->addWidget(autoStationsBox);

    /*vbl = */new QVBoxLayout(autoStationsBox);

    monitorTimer = new QTimer();

    connect(monitorTimer, &QTimer::timeout, this, &MonitoredLogs::on_monitorTimeout);

    monitorTimer->start(100);

    /*MinosRPC *rpc =*/ MinosRPC::getMinosRPC(getAppStartupName(), true);

    connect(RemoteLogs::getRemoteLogs(), &RemoteLogs::syncNeeded, this, &MonitoredLogs::onSyncNeeded);

    connect(logTree, &QTreeView::doubleClicked, this, &MonitoredLogs::onMonitorTree_doubleClicked);
}

MonitoredLogs::~MonitoredLogs()
{
    delete ui;
}

void MonitoredLogs::on_monitorTimeout()
{

    for ( auto const &s: qAsConst(RemoteLogs::getRemoteLogs()->stationList) )
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

void MonitoredLogs::syncStations()
{
  if ( syncstat )
   {
      syncstat = false;

      QString fname = RemoteLogs::getSettingsFile();
      QSettings settings(fname, QSettings::IniFormat);
      QString autoSyncStations = settings.value("autoStations", "None").toString();
      QStringList autoStations;
      if (autoSyncStations != "None")
      {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
          autoStations = autoSyncStations.split(" ", Qt::SkipEmptyParts);
#else
          autoStations = autoSyncStations.split(" ", QString::SkipEmptyParts);
#endif
      }
      clearLayout(autoStationsBox->layout());

      QStringList allStations = autoStations;

      TreeNode *root = new RootTreeNode();
      for ( auto s = RemoteLogs::getRemoteLogs()->stationList.begin(); s != RemoteLogs::getRemoteLogs()->stationList.end(); s++ )
      {
          allStations.append((*s)->name);

          // clang complains that snode may leak - but it gets taken over by the tree
          TreeNode *snode = new RouterTreeNode(root, (*s)->name);
          for ( auto const &l: qAsConst(s.value()->slotList) )
          {
              /*TreeNode *lnode =*/ new LogTreeNode(snode, l);
          }
      }
      treeModel->setRoot(root);
      int rc = treeModel->rowCount();
      for(int i = 0; i < rc; i++)
      {
        logTree->setFirstColumnSpanned( i, QModelIndex(), true );
      }
      logTree->expandAll();

      allStations.sort();
      allStations.removeDuplicates();
      for (const auto &sn:qAsConst(allStations))
      {
          QCheckBox *scb = new QCheckBox(autoStationsBox);
          scb->setText(sn);
          autoStationsBox->layout()->addWidget(scb);

          if (autoSyncStations == "None")
          {
              MinosConfig *config = MinosConfig::getMinosConfig();
              QString localRouterName = config->getThisRouterName();

              if (sn.contains("@" + localRouterName))
              {
                  scb->setChecked(true);
              }
          }
          if (autoStations.contains(sn))
          {
                scb->setChecked(true);
          }
          connect(scb, &QCheckBox::stateChanged, this, &MonitoredLogs::autoSyncChanged);
      }
   }
}
void MonitoredLogs::autoSyncChanged(int)
{
    QStringList autoStations;
    for(int i = 0; i < autoStationsBox->layout()->count(); i++)
    {
        QLayoutItem *li = autoStationsBox->layout()->itemAt(i);
        QWidget *w = li->widget();

        QCheckBox *cb = dynamic_cast<QCheckBox *>(w);
        if (cb && cb->isChecked())
        {
            autoStations.append(cb->text());
        }

    }
    QString fname = RemoteLogs::getSettingsFile();
    QSettings settings(fname, QSettings::IniFormat);
    settings.setValue("autoStations", autoStations.join(';'));
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
        MonitoredStation *ms = RemoteLogs::getRemoteLogs()->stationList[ p ];
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
          syncstat = true;
          emit logStarted(ml);
       }
       else
       {
           ml->setEnabled(false);
           ml->setManualClose(true);
           syncstat = true;
           emit logClosed(ml);
       }
    }
}

//=================================================================
// callback slots from RemoteLogs

void MonitoredLogs::onSyncNeeded()
{
    syncstat = true;
}
