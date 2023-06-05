#include <QTimer>
#include "regsettings.h"
#include "AppStartup.h"
#include "MonitorTreeModel.h"
#include "contest.h"
#include "MinosLoggerEvents.h"
#include "ScreenContact.h"
#include "MatchThread.h"
#include "cutils.h"
#include "remotelogs.h"
#include "MonitoredLog.h"
#include "MonitoringFrame.h"
#include "LogEvents.h"
#include "MTrace.h"
#include "MonitorMain.h"
#include "fileutils.h"
#include "MinosRPC.h"
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
    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    MultLists::getMultLists(); // make sure everything is loaded

    monitorTimer = new QTimer();

    connect(monitorTimer, &QTimer::timeout, this, &MonitorMain::on_monitorTimeout);

    monitorTimer->start(100);

    /*MinosRPC *rpc =*/ MinosRPC::getMinosRPC(getAppStartupName(), true);

    iniName = "./Configuration/" + getAppStartupName() + ".ini";
    RemoteLogs::setSettingsFile(iniName);


    connect(RemoteLogs::getRemoteLogs(), &RemoteLogs::newMonitoredLog, this, &MonitorMain::onNewLog);
    connect(RemoteLogs::getRemoteLogs(), &RemoteLogs::logAutoStarted, this, &MonitorMain::onLogStarted);
    connect(ui->monitorTree, &MonitoredLogs::logStarted, this, &MonitorMain::onLogStarted);
    connect(ui->monitorTree, &MonitoredLogs::logClosed, this, &MonitorMain::onLogClosed);
    connect(ui->monitorTree->getLogTree(), &QTreeView::clicked, this, &MonitorMain::onMonitorTree_clicked);

    QByteArray state;

    state = settings.getSettings().value("MonitorSplitter/state").toByteArray();
    if (state.size())
    {
        ui->monitorSplitter->restoreState(state);
    }
    else
    {
        QList<int> split{200, 600};
        ui->monitorSplitter->setSizes(split);
    }

    state = settings.getSettings().value("MainSplitter/state").toByteArray();
    if (state.size())
        ui->mainSplitter->restoreState(state);

    state = settings.getSettings().value("SearchSplitter/state").toByteArray();
    if (state.size())
        ui->searchSplitter->restoreState(state);

    ui->monitorSplitter->setHandleWidth(splitterHandleWidth);
    ui->mainSplitter->setHandleWidth(splitterHandleWidth);
    ui->searchSplitter->setHandleWidth(splitterHandleWidth);

    ui->contestPageControl->setContextMenuPolicy( Qt::CustomContextMenu );

    closeMonitoredLog = newAction(tr("Close tab"), &TabPopup, &MonitorMain::on_closeMonitoredLog);

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

    QSettings isettings(iniName, QSettings::IniFormat);

    QSOGrid = isettings.value("showQSOGrid", true).toBool();
    ui->showGridcb->setChecked(QSOGrid);
    QSOLines = isettings.value("showQSOLines", true).toBool();
    ui->showLinescb->setChecked(QSOLines);

    mapShowSpots = isettings.value("mapShowSpots", true).toBool();
    ui->mapShowSpots->setChecked(mapShowSpots);
    clusterDistanceLimit = isettings.value("clusterDistanceLimit", 0).toInt();
    ui->clusterDistanceLimit->setValue(clusterDistanceLimit);
}

MonitorMain::~MonitorMain()
{
    delete ui;
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
    RegSettings settings;
    settings.getSettings().setValue("geometry", saveGeometry());
    QWidget::moveEvent(event);
}
void MonitorMain::resizeEvent(QResizeEvent * event)
{
    RegSettings settings;
    settings.getSettings().setValue("geometry", saveGeometry());
    QWidget::resizeEvent(event);
}
void MonitorMain::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        RegSettings settings;
        settings.getSettings().setValue("geometry", saveGeometry());
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

void MonitorMain::on_callsignEdit_textChanged(const QString &/*arg1*/)
{
    searchChanged();
}

void MonitorMain::on_monitorSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    QByteArray state = ui->monitorSplitter->saveState();
    RegSettings settings;
    settings.getSettings().setValue("MonitorSplitter/state", state);
}
void MonitorMain::on_mainSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    QByteArray state = ui->mainSplitter->saveState();
    RegSettings settings;
    settings.getSettings().setValue("MainSplitter/state", state);
}

void MonitorMain::on_searchSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    QByteArray state = ui->searchSplitter->saveState();
    RegSettings settings;
    settings.getSettings().setValue("SearchSplitter/state", state);
}

void MonitorMain::closeTab(MonitoringFrame *cttab)
{
    for ( auto const &s: qAsConst(RemoteLogs::getRemoteLogs()->stationList) )
    {
        for ( auto const &l: qAsConst(s->slotList) )
        {
            if (l->getFrame() == cttab)
            {
                // take it out of the slot list and close it
                // and we need to redo the list

                RemoteLogs::getRemoteLogs()->closeLog(l.data());

                ui->contestPageControl->removeTab(ui->contestPageControl->indexOf(cttab));
                delete cttab;
                //syncstat = true;
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
void MonitorMain::onLogStarted(QSharedPointer<MonitoredLog> ml)
{
    trace("MonitorMain::onLogStarted");
    addSlot( ml );
//    sel->setLog(ml);
}
void MonitorMain::onLogClosed(QSharedPointer<MonitoredLog> l)
{
    MonitoringFrame *cttab = l->getFrame();
    if (cttab)
    {
       closeTab(cttab);
    }
}
//---------------------------------------------------------------------------
// callback slots from RPC in MonitoredLog
void MonitorMain::onNewStanzas(MonitoredLog *l)
{
    trace("OnNewStanzas");
    MonitoringFrame *frame = l->getFrame();
    if (frame)
    {
        frame->newStanzas = true;
    }
}
void MonitorMain::onNewLastContact(MonitoredLog *l)
{
    trace("onNewLastContact");
    MonitoringFrame *frame = l->getFrame();
    if (frame && l->getContest()->lastInserted >= 0)
    {
        QSharedPointer<BaseContact> bct = l->getContest()->pcontactAt(l->getContest()->lastInserted);
        frame->qsoModel.insertRows(l->getContest()->lastInserted, 1, QModelIndex());
        l->getContest()->lastInserted = -1;

        frame->on_AfterLogContact(l->getContest(), bct);
    }
}
void MonitorMain::onContactChanged(MonitoredLog *l)
{
    trace("onContactChanged");
    // change to a contact; we need a full rescan to understand it
    int lc = l->getContest()->lastInserted;
    if (lc >= 0)
    {
        MonitoringFrame *frame = l->getFrame();
        frame->qsoModel.changeRow(lc);
        frame->rescanNeeded = true;
    }
}
//=================================================================
// callback slots from RemoteLogs

void MonitorMain::onNewLog(QSharedPointer<MonitoredLog> ml)
{
    connect(ml.data(), &MonitoredLog::newStanzas, this, &MonitorMain::onNewStanzas);
    connect(ml.data(), &MonitoredLog::newLastContact, this, &MonitorMain::onNewLastContact);
    connect(ml.data(), &MonitoredLog::contactChanged, this, &MonitorMain::onContactChanged);
}
void MonitorMain::addSlot(  QSharedPointer< MonitoredLog>ct )
{
    trace("MonitorMain::addSlot");
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
}

MonitoringFrame *MonitorMain::findCurrentLogFrame()
{
    QWidget *w = ui->contestPageControl->currentWidget();
    MonitoringFrame *f = dynamic_cast<MonitoringFrame *>(w);
    return f;
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
    // we need addSlot( ml ); for any newly started log
    RemoteLogs::getRemoteLogs()->testAutoStart();
}
void MonitorMain::onMonitorTree_clicked(const QModelIndex &index)
{
    // select the correct tab
    TreeNode * sel = static_cast< TreeNode *>(index.internalPointer());

    if (!sel)
    {
       return;
    }
    if ( sel->GetNodeType() != entLog )
    {
       // station
    }
    else if (sel->getLog())
    {
        MonitoringFrame *mf = sel->getLog()->getFrame();
        int pc = ui->contestPageControl->count();
        for ( int i = 0; i < pc; i++ )
        {
            QWidget *tw = ui->contestPageControl->widget(i);
            MonitoringFrame *f = dynamic_cast<MonitoringFrame *>(tw);
            if (f == mf)
            {
                ui->contestPageControl->setCurrentWidget(f);
                break;
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



void MonitorMain::on_showGridcb_stateChanged(int /*arg1*/)
{
    QSettings settings(iniName, QSettings::IniFormat);
    QSOGrid = ui->showGridcb->isChecked();
    settings.setValue("showQSOGrid", QSOGrid);
    MinosLoggerEvents::SendRedrawQSOMap(QSOGrid, QSOLines, mapShowSpots, clusterDistanceLimit);
}


void MonitorMain::on_showLinescb_stateChanged(int /*arg1*/)
{
    QSettings settings(iniName, QSettings::IniFormat);
    QSOLines = ui->showLinescb->isChecked();
    settings.setValue("showQSOLines", QSOLines);
    MinosLoggerEvents::SendRedrawQSOMap(QSOGrid, QSOLines, mapShowSpots, clusterDistanceLimit);
}


void MonitorMain::on_mapShowSpots_stateChanged(int /*arg1*/)
{
    QSettings settings(iniName, QSettings::IniFormat);
    mapShowSpots = ui->mapShowSpots->isChecked();
    settings.setValue("mapShowSpots", mapShowSpots);
    MinosLoggerEvents::SendRedrawQSOMap(QSOGrid, QSOLines, mapShowSpots, clusterDistanceLimit);
}


void MonitorMain::on_clusterDistanceLimit_valueChanged(int /*arg1*/)
{
    QSettings settings(iniName, QSettings::IniFormat);
    clusterDistanceLimit = ui->clusterDistanceLimit->value();
    settings.setValue("clusterDistanceLimit", clusterDistanceLimit);
    MinosLoggerEvents::SendRedrawQSOMap(QSOGrid, QSOLines, mapShowSpots, clusterDistanceLimit);
}

