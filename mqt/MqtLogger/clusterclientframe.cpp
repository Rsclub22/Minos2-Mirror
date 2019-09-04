////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Cluster Client
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
///
//
//
/////////////////////////////////////////////////////////////////////////////

#include "base_pch.h"

#include "MinosRPC.h"
#include "MinosLoggerEvents.h"
#include "clusterclientframe.h"
#include "contest.h"
#include "ContestApp.h"
#include "cutils.h"
#include "rigmemcommondata.h"
#include "rotatorcommon.h"
#include "htmldelegate.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "ui_clusterclientframe.h"


#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>



ClusterClientFrame::ClusterClientFrame(QWidget *parent):
    QFrame(parent),
    ui(new Ui::ClusterClientFrame),
    purgeTimer(nullptr),
    timeToLive(0),
    purgeSpotFlag(false),
    holdUpdateFlag(false),
    contestBand(-1),
    contestMode(-1),
    isProtected(false)
{

    ui->setupUi(this);

    ui->clusterSplitter->setStretchFactor(0, 2);
    ui->clusterSplitter->setStretchFactor(1, 1);

    restoreSplitters();

    trace(QString("ClusterClientFrame Starting"));

    int lcf;
    MinosParameters::getMinosParameters() ->getIntDisplayProfile(edpListCompression, lcf);
    dxDelegate = QSharedPointer<HtmlDelegate>(new HtmlDelegate(1.0, lcf/100.0)) ;
    searchDelegate = QSharedPointer<HtmlDelegate>(new HtmlDelegate(1.0, lcf/100.0)) ;
    callsignDelegate = QSharedPointer<HtmlDelegate>(new HtmlDelegate(1.0, lcf/100.0)) ;
    locatorDelegate = QSharedPointer<HtmlDelegate>(new HtmlDelegate(1.0, lcf/100.0)) ;

    this->setMouseTracking(true);
    mouseInFrameTimer = new QTimer(this);
    connect (mouseInFrameTimer, SIGNAL(timeout()), this, SLOT(mouseTimerCheckNewSpots()));

    filterSetup = new ClusterClientFilterDialog(this);


    purgeTimer = new QTimer(this);

    checkNewSpotsTimer = new QTimer(this);
    connect (checkNewSpotsTimer, SIGNAL(timeout()), this, SLOT(checkNewSpots()));

    spotQueue.clear();

    checkNewFilters = new QTimer(this);
    connect (checkNewFilters, SIGNAL(timeout()), this, SLOT(checkSavedFilters()));

    connect (ClusterClientServer::getClusterClientServer(), SIGNAL(ClusterServerList(QVector<ClusterServer>)), this, SLOT(clusterClientServerList(QVector<ClusterServer>)));
    connect (ClusterClientServer::getClusterClientServer(), SIGNAL(dxSpot(QVector<QString>)), this, SLOT(dxSpots(QVector<QString>)));

    connect (purgeTimer, SIGNAL(timeout()), this, SLOT(purgeSpots()));


    connect(&MinosLoggerEvents::mle, SIGNAL(FontChanged()), this, SLOT(on_FontChanged()), Qt::QueuedConnection);

    connect (ui->filtersBut, SIGNAL(clicked()), this, SLOT(filterButtonSelected()));


    spotsMenu = new QMenu(ui->actionsButton);

    ui->actionsButton->setFocusPolicy(Qt::NoFocus);
    actionInObject = new MouseInObject(this, this);
    spotsMenu->installEventFilter(actionInObject);

    freqAction = new QAction("Set &Freq", this);
    bearingAction = new QAction("Set &Bearing", this);
    logAction = new QAction("Send &Log", this);
    memoryAction = new QAction("Send &Memory", this);
    clearSpotAction = new QAction("Clear &Spot", this);
    clearAllSpotsAction = new QAction("Clear &All Spots", this);
    memoryActionOveride = new QAction("Force &Send Memory", this);

    spotsMenu->addAction(freqAction);
    spotsMenu->addAction(bearingAction);
    spotsMenu->addAction(logAction);
    spotsMenu->addAction(memoryAction);
    spotsMenu->addAction(clearSpotAction);
    spotsMenu->addAction(clearAllSpotsAction);
    spotsMenu->addAction(memoryActionOveride);

    ui->actionsButton->setMenu(spotsMenu);
    connect(spotsMenu, SIGNAL(aboutToShow()), this, SLOT(onMenuShow()));

    connect( freqAction, SIGNAL( triggered() ), this, SLOT(on_freqActionSelected()) );
    connect( bearingAction, SIGNAL( triggered() ), this, SLOT(bearingActionSelected()) );
    connect( logAction, SIGNAL( triggered() ), this, SLOT(logActionSelected()) );
    connect( memoryAction, SIGNAL( triggered() ), this, SLOT(memoryActionSelected()) );
    connect( clearSpotAction, SIGNAL( triggered() ), this, SLOT(clearSpotActionSelected()) );
    connect( clearAllSpotsAction, SIGNAL( triggered() ), this, SLOT(clearAllSpotsActionSelected()) );
    connect( memoryActionOveride, SIGNAL( triggered() ), this, SLOT(memoryActionOverideSelected()) );

    //connect(&MinosLoggerEvents::mle, SIGNAL(AfterLogContactToCluster(BaseContestLog *, Callsign, Locator)), this, SLOT(delayed_afterLogContact(BaseContestLog *, Callsign, Locator)), Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, SIGNAL(AfterLogContactToCluster(BaseContestLog *, Callsign, QString)), this, SLOT(on_AfterLogContact(BaseContestLog *, Callsign, QString)));

    ui->searchLineEdit->setValidator(&ucValidator);
    connect(ui->searchLineEdit, SIGNAL(editingFinished()), this, SLOT(onSearchEditingFinished()));

    dxSpotDataModel = new DxSpotDataModel();
    dxSpotDataModel->delegate = dxDelegate;


    on_FontChanged();

    setupDXSpotView();

    setupSearchSpotView();

    setupCallsignSpotView();

    setupLocatorSpotView();


    filterProxyModelList.append(dxSpotProxyModel);
    filterProxyModelList.append(searchSortProxyModel);
    filterProxyModelList.append(callSignProxyModel);
    filterProxyModelList.append(locatorProxyModel);

    spotViewList.append(dxSpotView);
    spotViewList.append(searchView);
    spotViewList.append(callSignView);
    spotViewList.append(locatorView);

    setAllTabsColor(CLUSTER_TAB_NOT_SELECT_COLOR);
    ui->dxSpotTab->setTabColor(ui->dxSpotTab->currentIndex(), CLUSTER_TAB_SELECT_COLOR);


    connect(ui->dxSpotTab, SIGNAL(currentChanged(int)), this, SLOT(onSpotTabChanged(int)));
    restoreLocatorViewColumns();

    // all initial restores have been done

    connect(&MinosLoggerEvents::mle, SIGNAL(doColumnChanges(BaseContestLog*)), this, SLOT(on_doColumnChanges(BaseContestLog*)));
    connect(&MinosLoggerEvents::mle, SIGNAL(doSplitterChanges(BaseContestLog*)), this, SLOT(on_doSplitterChanges(BaseContestLog*)));


    connect(filterSetup, SIGNAL(filtersChanged(bool, bool, bool, bool)), this, SLOT(filtersChanged(bool, bool, bool, bool)));

    purgeTimer->start(PURGE_TIME);

    handleClusterStatusMessage(LogContainer->clusterConnectStatus);

    newSpotIndToggle(false);
    newCallsignSpotIndToggle(false);
    newLocatorSpotIndToggle(false);
    checkNewFilters->start(CHECK_NEWFILTERS_DURATION);
    checkNewSpotsTimer->start(CHECKSPOTS_DURATION);



}


ClusterClientFrame::~ClusterClientFrame()
{
    delete ui;
    delete dxSpotDataModel;

    foreach(auto m, filterProxyModelList)
    {
        delete m;
    }
    delete actionInObject;

}




void ClusterClientFrame::delayed_afterLogContact(BaseContestLog *c, Callsign cs, QString loc)
{
    // delay the search of the spots until the contact logging should have finished
    // and the screen been redrawn, or a lot of spots slows things down too much

    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);

    connect(timer, &QTimer::timeout, [=]()
    {
        // NB a lambda function
        on_AfterLogContact(c, cs, loc);
        timer->deleteLater();
    }
    );

    timer->start(50);
}



void ClusterClientFrame::setupDXSpotView()
{
    dxSpotView = new QTableView();
    dxSpotView->setFocusPolicy(Qt::NoFocus);

    dxSpotProxyModel = new DxSpotSortFilterProxyModel(filterSetup);
    dxSpotProxyModel->setSourceModel(dxSpotDataModel);
    dxSpotProxyModel->sort(RXTIME_COL_NUM, Qt::DescendingOrder);
    //dxSpotProxyModel->setDynamicSortFilter(true);

    ui->dxSpotTab->addTab(dxSpotView, "DX Spots");
    //dxSpotView = ui->dxSpotView;
    dxSpotView->setModel(dxSpotProxyModel);
    dxSpotView->setAlternatingRowColors(true);
    dxSpotView->setSelectionBehavior( QAbstractItemView::SelectRows );
    dxSpotView->setSelectionMode( QAbstractItemView::SingleSelection );
    //dxSpotView->setSelectionBehavior(QAbstractItemView::SelectItems);
    dxSpotView->verticalHeader()->setDefaultSectionSize(10);
    dxSpotView->verticalHeader()->setMinimumSectionSize(10);

    dxSpotView->setItemDelegate( dxDelegate.data());

    QHeaderView *spotVerticalHeader = dxSpotView->verticalHeader();

    connect(dxSpotView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onDxSpotViewClicked(const QModelIndex &)));
    connect(spotVerticalHeader, SIGNAL(sectionClicked(int)), this, SLOT(onDXSpotVertHeaderClicked(int)));


    restoreDxSpotViewColumns();
    dxSpotView->horizontalHeader()->setStretchLastSection(true);

    connect( dxSpotView->horizontalHeader(), SIGNAL(sectionResized(int, int , int)), this, SLOT( on_dxSpotViewSectionResized(int, int , int)));

    spotVerticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);


    dxSpotView->setColumnHidden(DXMODEMASK_COL_NUM, true);
    dxSpotView->setColumnHidden(DXSPOT_CALL_WORKED_COL_NUM, true);
    dxSpotView->setColumnHidden(DXLOC_WORKED_COL_NUM, true);
    dxSpotView->setColumnHidden(DXSPOT_TO_MEMORY_FLAG_COL_NUM, true);
    dxSpotView->setColumnHidden(RXTIME_COL_NUM, true);
    //dxSpotView->setColumnHidden(DXSPOT_MODE_COL_NUM, true);
    dxSpotView->setColumnHidden(DXSPOT_PROP_MODE_COL_NUM, true);
    dxSpotView->setColumnHidden(DXBANDMASK_COL_NUM, true);

}


void ClusterClientFrame::setupSearchSpotView()
{
    searchView = new QTableView();
    searchView->setFocusPolicy(Qt::NoFocus);

    searchSortProxyModel = new SearchSortFilterProxyModel(filterSetup);
    searchSortProxyModel->setSourceModel(dxSpotDataModel);
    searchSortProxyModel->sort(RXTIME_COL_NUM, Qt::DescendingOrder);

    ui->dxSpotTab->addTab(searchView, "Search Spots");
    searchView->setModel(searchSortProxyModel);
    searchView->setAlternatingRowColors(true);
    searchView->setSelectionMode( QAbstractItemView::SingleSelection );
    searchView->setSelectionBehavior(QAbstractItemView::SelectItems);
    //dxSpotView->setSelectionMode( QAbstractItemView::NoSelection );

    searchView->setItemDelegate( searchDelegate.data());
    searchView->verticalHeader()->setDefaultSectionSize(10);
    searchView->verticalHeader()->setMinimumSectionSize(10);

    QHeaderView *searchVerticalHeader = searchView->verticalHeader();
    connect(searchView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onSearchSpotViewClicked(const QModelIndex &)));
    connect(searchVerticalHeader, SIGNAL(sectionClicked(int)), this, SLOT(onSearchSpotVertHeaderClicked(int)));

    restoreSearchViewColumns();
    searchView->horizontalHeader()->setStretchLastSection(true);
    connect( searchView->horizontalHeader(), SIGNAL(sectionResized(int, int , int)), this, SLOT( on_searchViewSectionResized(int, int , int)));
    searchVerticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);


    searchView->setColumnHidden(DXMODEMASK_COL_NUM, true);
    searchView->setColumnHidden(DXSPOT_CALL_WORKED_COL_NUM, true);
    searchView->setColumnHidden(DXLOC_WORKED_COL_NUM, true);
    searchView->setColumnHidden(DXSPOT_TO_MEMORY_FLAG_COL_NUM, true);
    searchView->setColumnHidden(RXTIME_COL_NUM, true);
    //searchView->setColumnHidden(DXSPOT_MODE_COL_NUM, true);
    searchView->setColumnHidden(DXSPOT_PROP_MODE_COL_NUM, true);
    searchView->setColumnHidden(DXBANDMASK_COL_NUM, true);




}




void ClusterClientFrame::setupCallsignSpotView()
{
    callSignView = new QTableView();
    callSignView->setFocusPolicy(Qt::NoFocus);

    callSignProxyModel = new CallsignSortFilterProxyModel(filterSetup);
    callSignProxyModel->setSourceModel(dxSpotDataModel);
    callSignProxyModel->sort(RXTIME_COL_NUM, Qt::DescendingOrder);

    ui->dxSpotTab->addTab(callSignView, "Callsign Spots");

    callSignView->setModel(callSignProxyModel);
    callSignView->setAlternatingRowColors(true);
    callSignView->setSelectionMode( QAbstractItemView::SingleSelection );
    callSignView->setSelectionBehavior(QAbstractItemView::SelectItems);

    callSignView->setItemDelegate( callsignDelegate.data());

    callSignView->verticalHeader()->setDefaultSectionSize(10);
    callSignView->verticalHeader()->setMinimumSectionSize(10);


    QHeaderView *callSignVerticalHeader = callSignView->verticalHeader();
    connect(callSignView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onCallsignSpotViewClicked(const QModelIndex &)));
    connect(callSignVerticalHeader, SIGNAL(sectionClicked(int)), this, SLOT(onCallsignSpotVertHeaderClicked(int)));


    restoreCallsignViewColumns();
    callSignView->horizontalHeader()->setStretchLastSection(true);
    connect( callSignView->horizontalHeader(), SIGNAL(sectionResized(int, int , int)), this, SLOT( on_callsignViewSectionResized(int, int , int)));

    callSignVerticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);

    callSignView->setColumnHidden(DXMODEMASK_COL_NUM, true);
    callSignView->setColumnHidden(DXSPOT_CALL_WORKED_COL_NUM, true);
    callSignView->setColumnHidden(DXLOC_WORKED_COL_NUM, true);
    callSignView->setColumnHidden(DXSPOT_TO_MEMORY_FLAG_COL_NUM, true);
    callSignView->setColumnHidden(RXTIME_COL_NUM, true);
    //callSignView->setColumnHidden(DXSPOT_MODE_COL_NUM, true);
    callSignView->setColumnHidden(DXSPOT_PROP_MODE_COL_NUM, true);
    callSignView->setColumnHidden(DXBANDMASK_COL_NUM, true);




}

void ClusterClientFrame::setupLocatorSpotView()
{
    locatorView = new QTableView();
    locatorView->setFocusPolicy(Qt::NoFocus);

    locatorProxyModel = new LocatorSortFilterProxyModel(filterSetup);
    locatorProxyModel->setSourceModel(dxSpotDataModel);
    locatorProxyModel->sort(RXTIME_COL_NUM, Qt::DescendingOrder);

    ui->dxSpotTab->addTab(locatorView, "Locator Spots");
    locatorView->setModel(locatorProxyModel);
    locatorView->setAlternatingRowColors(true);
    locatorView->setSelectionMode( QAbstractItemView::SingleSelection );
    locatorView->setSelectionBehavior(QAbstractItemView::SelectItems);
    //dxSpotView->setSelectionMode( QAbstractItemView::NoSelection );

    locatorView->setItemDelegate( locatorDelegate.data());

    QHeaderView *locatorViewVerticalHeader = locatorView->verticalHeader();
    locatorView->verticalHeader()->setDefaultSectionSize(10);
    locatorView->verticalHeader()->setMinimumSectionSize(10);


    connect(locatorView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onLocatorSpotViewClicked(const QModelIndex &)));
    connect(locatorViewVerticalHeader, SIGNAL(sectionClicked(int)), this, SLOT(onLocatorSpotVertHeaderClicked(int)));

    restoreLocatorViewColumns();
    locatorView->horizontalHeader()->setStretchLastSection(true);
    connect( locatorView->horizontalHeader(), SIGNAL(sectionResized(int, int , int)), this, SLOT( on_locatorViewSectionResized(int, int , int)));
    locatorViewVerticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);

    locatorView->setColumnHidden(DXMODEMASK_COL_NUM, true);
    locatorView->setColumnHidden(DXSPOT_CALL_WORKED_COL_NUM, true);
    locatorView->setColumnHidden(DXLOC_WORKED_COL_NUM, true);
    locatorView->setColumnHidden(DXSPOT_TO_MEMORY_FLAG_COL_NUM, true);
    locatorView->setColumnHidden(RXTIME_COL_NUM, true);
    //locatorView->setColumnHidden(DXSPOT_MODE_COL_NUM, true);
    locatorView->setColumnHidden(DXSPOT_PROP_MODE_COL_NUM, true);
    locatorView->setColumnHidden(DXBANDMASK_COL_NUM, true);


}



void ClusterClientFrame::filterButtonSelected()
{
    filterSetup->copyBandFiltersToDialog();
    filterSetup->copyModeFiltersToDialog();
    filterSetup->copyCallsignFilterListToListWidget();
    filterSetup->copyLocatorFilterListToListWidget();
    filterSetup-> setTabCurrentIndex(filterSetup->getTabCurrentIndex());
    filterSetup->exec();

}

void ClusterClientFrame::filtersChanged(bool bandfilterChanged, bool modefilterChanged,  bool callsignfilterChanged, bool locatorfilterChanged)
{
    //update views..
    if (bandfilterChanged)
    {
        dxSpotProxyModel->setFilterRegExp("");
    }
    else if (modefilterChanged)
    {

    }
    else if (callsignfilterChanged)
    {
        callSignProxyModel->setFilterRegExp("");
    }
    else if (locatorfilterChanged)
    {
        locatorProxyModel->setFilterRegExp("");
    }

}

void ClusterClientFrame::on_FontChanged()
{
    QFont cf = QApplication::font();
    //ui->StationList->setFont(cf);
}


void ClusterClientFrame::setAllTabsColor(QColor c)
{
    for (int i = 0; i < ui->dxSpotTab->count(); i++)
    {
        ui->dxSpotTab->setTabColor(i, c);
    }
}


void ClusterClientFrame::onSpotTabChanged(int index)
{
    if (index == -1)
    {
        return;
    }
    else
    {
        setAllTabsColor(CLUSTER_TAB_NOT_SELECT_COLOR);
        ui->dxSpotTab->setTabColor(index, CLUSTER_TAB_SELECT_COLOR);
    }
}



void ClusterClientFrame::handleClickedItems(DxSpotSortFilterProxyModel* spotProxyModel, const QModelIndex &index)
{
    if (index.column() == FREQ_COL_NUM)
    {
        QString freq = spotProxyModel->data(index, DataStoredRole).toString();
        sendFreqToRig(freq);

    }
    else if (index.column() == DXSPOT_CALL_COL_NUM )
    {
        // transfer spot details to qsolog

        MinosLoggerEvents::SendSpotToLog(getSpotDataToMemoryVariable(spotProxyModel, index.row()));
    }
    else if (index.column() == DXBRG_COL_NUM)
    {
        QString brg = spotProxyModel->data(index, DataStoredRole).toString();
        QString loc = spotProxyModel->data(spotProxyModel->index(index.row(), DXLOC_COL_NUM), DataStoredRole).toString();
        if (loc.count() < 6)
        {
            brg = brg.append(SHORTLOCATOR_IDENTIFIER);
        }
        sendBrgToRot(brg);
    }
}

void ClusterClientFrame::handleVertHeaderClickedItems(DxSpotSortFilterProxyModel* spotProxyModel, int row)
{
    // check if spot has been sent to memory
    if (!spotProxyModel->data(spotProxyModel->index(row, DXSPOT_TO_MEMORY_FLAG_COL_NUM), DataStoredRole).toBool())
    {
        sendSpotToMemory(spotProxyModel, row);

    }
}


void ClusterClientFrame::onDxSpotViewClicked(const QModelIndex &index)
{

    handleClickedItems(dxSpotProxyModel, index);
}


void ClusterClientFrame::onDXSpotVertHeaderClicked(int row)
{
    handleVertHeaderClickedItems(dxSpotProxyModel, row);
}

void ClusterClientFrame::onSearchSpotViewClicked(const QModelIndex &index)
{
    handleClickedItems(searchSortProxyModel, index);
}

void ClusterClientFrame::onSearchSpotVertHeaderClicked(int row)
{
    handleVertHeaderClickedItems(searchSortProxyModel, row);
}

void ClusterClientFrame::onCallsignSpotViewClicked(const QModelIndex &index)
{
    handleClickedItems(callSignProxyModel, index);
}

void ClusterClientFrame::onCallsignSpotVertHeaderClicked(int row)
{
    handleVertHeaderClickedItems(callSignProxyModel, row);
}

void ClusterClientFrame::onLocatorSpotViewClicked(const QModelIndex &index)
{
    handleClickedItems(locatorProxyModel, index);
}

void ClusterClientFrame::onLocatorSpotVertHeaderClicked(int row)
{
    handleVertHeaderClickedItems(locatorProxyModel, row);
}



void ClusterClientFrame::sendFreqToRig(QString freq)
{
    QString f = freq.remove('.');
    MinosLoggerEvents::SendFreqStrToRig(f);
}


void ClusterClientFrame::sendBrgToRot(QString brg)
{
    trace(QString("Send Bearing to Rot = %1").arg(brg));
    if (!brg.isEmpty())
    {
       MinosLoggerEvents::SendSpotBrgStrToRot(brg);
    }

}


//---------------------------------------------------------------------------
void ClusterClientFrame::clusterClientServerList(QVector<ClusterServer> serverList)
{
    //ui->StationList->clear();
    for ( QVector<ClusterServer>::iterator i = serverList.begin(); i != serverList.end(); i++ )
    {
        QString state = clusterStateIndicator[(*i).state] + " " + (*i).app + "\r\n";
        trace(QString("clusterClientServerList - state = %1").arg(state));
        //ui->StationList->addItem( state );
    }
}

void ClusterClientFrame::dxSpots(QVector<QString> spotMsg)
{
    // if contest is protected ignore
    if (isProtected)
    {
        return;
    }

    //get spot Message from queue
    for (int i = 0; i < spotMsg.count(); i++)
    {
        QString msg = spotMsg[i];

        // check to see if this is a non spot message
        if (msg.contains(CLUSTER_STATUS))
        {

            LogContainer->clusterConnectStatus = msg;       // save for new clusterClientFrames
            handleClusterStatusMessage(msg);

        }
        else if (msg.contains(DXSPOT))
        {
            spotQueue += spotMsg[i];
        }


    }


    if (!purgeSpotFlag && !holdUpdateFlag)     // do nothing while purging spots
    {
        handleDxSpots(spotQueue);
    }

 }


void ClusterClientFrame::handleDxSpots(QVector<QString> &spotQueue)
{
    int sqsize = spotQueue.count();
    for (int i = sqsize -1 ; i > -1; i--)
    {
       addDxSpotToTable(spotQueue[i]);
       trace("syncSpots " + spotQueue[i]);
    }


    spotQueue.clear();
}


void ClusterClientFrame::addDxSpotToTable(const QString spot)
{

    QDateTime spotDateTime = QDateTime::currentDateTimeUtc();
    QStringList sl = spot.split(DXSPOT);
    if (sl.count() == 2)
    {
        QStringList spotlist = sl[1].split(':', QString::KeepEmptyParts);

        if (spotlist.count() == TTLVALUE +1)
        {
            // get time to live value
            if (spotlist[TTLVALUE] == "0")
            {
                timeToLive = 0;  // timeToLive is off
            }
            else
            {
                bool ok = false;
                int ttl = spotlist[TTLVALUE].toInt(&ok);
                if (ok)
                {
                    if (ttl >= MIN_TTL && ttl <= MAX_TTL)
                    {
                        timeToLive = ttl * 60; // seconds
                    }
                }
            }

            //*********************************************************

            //timeToLive = 120; // for testing.....

            //*******************************************************


            // check to see if call or locator worked
            bool callWorked = false;
            bool locWorked = false;

            if (spotlist[DXBANDMASK].toInt() == contestBand) // if contestband matches spotband
            {
                checkSpotWorked(spotlist[DXCALL], spotlist[DXLOCATOR], &callWorked, &locWorked);
            }



            double dist = 0;
            int brg = 0;
            QString distance;
            QString bearing;
            if (!spotlist[DXLOCATOR].isEmpty())
            {
                calcSpotDistanceBearing(spotlist[DXLOCATOR], &dist, &brg);
                distance = QString::number(static_cast< int> ( dist));
                bearing =  QString::number(brg);
            }

            spotDateTime = getSpotDateTime(spotlist[SPOTDATE], spotlist[SPOTTIME]);
            qint64 rxTime = spotDateTime.toMSecsSinceEpoch()/1000;

            dxSpotDataModel->rowData = new SpotData(rxTime, spotlist[SPOTTIME],
                                                    spotlist[DXFREQ], spotlist[DXBANDSTR], spotlist[DXBANDMASK],
                                                    spotlist[DXMODESTR], spotlist[DXMODEMASK],
                                                    spotlist[DXCALL],
                                                    callWorked, spotlist[DXLOCATOR],
                                                    locWorked, distance,
                                                    bearing, spotlist[SPOTCALL],
                                                    spotlist[SPOTLOCATOR], spotlist[DXPROPMODE], spotlist[SPOTCOMMENT]);

            dxSpotDataModel->insertRows(dxSpotDataModel->rowCount(), 1);

       }
    }

}


void ClusterClientFrame::checkSpotWorked(QString &callsign, QString &locator, bool* callWorked, bool* locatorWorked)
{
    bool callfound = false;
    bool locfound = false;
    if (ct && !ct->isProtected())
    {

        Callsign mcs(callsign);
        mcs.validate();
        for ( LogIterator i = ct->ctList.begin(); i != ct->ctList.end(); i++ )
        {
            unsigned short cf = (*i).wt->contactFlags.getValue();
            if ( cf & ( LOCAL_COMMENT | COMMENT_ONLY | DONT_PRINT ) )
            {
                continue;
            }

            if (!callfound)
            {
            if ((*i).wt->cs == mcs)
            {
                *callWorked = true;
                    callfound = true;

            }
            }

            if (!locator.isEmpty())
            {
                QString loc = locator.mid(0,4);
                if ((*i).wt->loc.loc.getValue().mid(0,4) == loc)
                {
                    *locatorWorked = true;
                    locfound = true;

        }
            }

            if (callfound && locfound)
            {
                return;
            }

        }

    }



 /*
        if (!locator.isEmpty())
        {
            QString loc = locator.mid(0,4);

            for ( LogIterator i = ct->ctList.begin(); i != ct->ctList.end(); i++ )
            {
                if ((*i).wt->loc.loc.getValue().mid(0,4) == loc)
                {
                    *locatorWorked = true;

                }
            }
        }
*/


}


void ClusterClientFrame::calcSpotDistanceBearing(const QString& _locator, double* distance, int* bearing)
{
    bool locValid = true;
    QString locator = _locator;
    double latitude;
    double longitude;
    double dist;
    int brg = 0;

    if (ct && !locator.isEmpty())
    {
        if (locator.count() == 4)
        {
            locator.append("MM");
        }

        int locValres = lonlat( locator, longitude, latitude, MinosParameters::getMinosParameters() ->getAllowLoc4() );
        if ( ( locValres ) != LOC_OK )
        {
            locValid = false;
        }
        if (locValid)
        {
            ct->disbeara(longitude, latitude, dist, brg);
            *distance = dist;
            *bearing = brg;
        }

    }


}

void ClusterClientFrame::on_dxSpotViewSectionResized(int, int , int)
{
    QSettings settings;
    QByteArray state;

    state = dxSpotView->horizontalHeader()->saveState();
    settings.setValue("ClusterClientFilter/dxSpotView/state", state);
    MinosLoggerEvents::SendColumnsChanged();
}
void ClusterClientFrame::on_callsignViewSectionResized(int, int , int)
{
    QSettings settings;
    QByteArray state;

    state = callSignView->horizontalHeader()->saveState();
    settings.setValue("ClusterClientFilter/callSignView/state", state);
    MinosLoggerEvents::SendColumnsChanged();
}
void ClusterClientFrame::on_locatorViewSectionResized(int, int , int)
{
    QSettings settings;
    QByteArray state;

    state = locatorView->horizontalHeader()->saveState();
    settings.setValue("ClusterClientFilter/locatorView/state", state);
    MinosLoggerEvents::SendColumnsChanged();
}

void ClusterClientFrame::on_searchViewSectionResized(int, int , int)
{
    QSettings settings;
    QByteArray state;

    state = searchView->horizontalHeader()->saveState();
    settings.setValue("ClusterClientFilter/searchView/state", state);
    MinosLoggerEvents::SendColumnsChanged();

}

void ClusterClientFrame::restoreDxSpotViewColumns()
{
    QSettings settings;
    QByteArray state;

    state = settings.value("ClusterClientFilter/dxSpotView/state").toByteArray();
    dxSpotView->horizontalHeader()->restoreState(state);
    MinosLoggerEvents::SendColumnsChanged();

}


void ClusterClientFrame::restoreCallsignViewColumns()
{
    QSettings settings;
    QByteArray state;

    state = settings.value("ClusterClientFilter/callSignView/state").toByteArray();
    callSignView->horizontalHeader()->restoreState(state);
    MinosLoggerEvents::SendColumnsChanged();

}


void ClusterClientFrame::restoreLocatorViewColumns()
{
    QSettings settings;
    QByteArray state;

    state = settings.value("ClusterClientFilter/locatorView/state").toByteArray();
    locatorView->horizontalHeader()->restoreState(state);
}
void ClusterClientFrame::restoreSearchViewColumns()
{
    QSettings settings;
    QByteArray state;

    state = settings.value("ClusterClientFilter/searchView/state").toByteArray();
    searchView->horizontalHeader()->restoreState(state);
}

void ClusterClientFrame::restoreColumns()
{
    restoreCallsignViewColumns();
    restoreLocatorViewColumns();
    restoreSearchViewColumns();
    restoreDxSpotViewColumns();
}
void ClusterClientFrame::on_clusterSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    QByteArray state = ui->clusterSplitter->saveState();
    QSettings settings;
    settings.setValue("Splitters/ClusterClientFrame/state/", state);

    MinosLoggerEvents::SendSplittersChanged();
}

void ClusterClientFrame::restoreSplitters()
{
    QSettings settings;
    QByteArray state;

    state = settings.value("Splitters/ClusterClientFrame/state/").toByteArray();
    ui->clusterSplitter->restoreState(state);
}
void ClusterClientFrame::on_doColumnChanges(BaseContestLog *b)
{
    if (b == ct)
    {
        restoreColumns();
    }
}
void ClusterClientFrame::on_doSplitterChanges(BaseContestLog *b)
{
    if (b == ct)
    {
        restoreSplitters();
    }
}

void ClusterClientFrame::setContest(BaseContestLog *c)
{
    ct = c;
    LoggerContestLog* contest = dynamic_cast<LoggerContestLog *>( ct);

    // set the contest in the filter dialog
    filterSetup->setContest(c);

    if (ct != nullptr)
    {
        contestUuid = ct->uuid;
        trace(QString("Cluster ClientFrame Set Contest: contest uuid =  ContestUuid = %1").arg(contestUuid));
        contestBandStr = ct->band.getValue();
        contestBand = getBandOffSet(contestBandStr);
        contestModeStr = ct->currentMode.getValue();
        contestMode = getModeOffSet(contestModeStr);
        if (!contest->clusterFilterSettingsExist)       // have settings been saved before?
        {
            // no, save current band filter for this contest
            filterSetup->setBandFilter(contestBand);    // set cluster filter to current band - can be overidden
            filterSetup->saveClusterFilterToContest();
        }

        if (ct && ct == TContestApp::getContestApp() ->getCurrentContest())
        {
            if (!ct->isProtected())
            {
                isProtected = false;
            }
            else
            {
                isProtected = true;
            }
        }
    }


}


int ClusterClientFrame::getBandOffSet(QString contestBandStr)
{
    int i = 0;
    while(i != clusterBands.count())
    {
        if (contestBandStr == clusterBands[i])
        {
            return i;
        }
        i++;
    }

    return -1;
}

int ClusterClientFrame::getModeOffSet(QString contestModeStr)
{
    int i = 0;
    while(i != clusterModes.count())
    {
        if (contestModeStr == clusterModes[i])
        {
            return i;
        }
        i++;
    }

    return -1;
}

void ClusterClientFrame::purgeSpots()
{

    if (timeToLive > 0 && !holdUpdateFlag /*&& (ct && ct == TContestApp::getContestApp()->getCurrentContest())*/)      // don't purge spots if == 0 and holdupdateflag is on
    {
        if (dxSpotDataModel->rowCount() > 0)
        {
           purgeSpotFlag = true;
           int idx = dxSpotDataModel->rowCount() - 1;
           while (idx >= 0 && dxSpotDataModel->rowCount() > 0)
           {
               if (spotTimedOut(dxSpotDataModel->data(dxSpotDataModel->index(idx, RXTIME_COL_NUM), DataStoredRole).toLongLong(), timeToLive))
               {
                     dxSpotDataModel->removeRows(idx, 1, QModelIndex());
               }
               idx--;
           }
           purgeSpotFlag = false;
        }
    }

}







/********* Action Menu **********************************/

void ClusterClientFrame::onMenuShow()
{
    //int buttonNumber = getSelectedLine();

    //freqAction->setEnabled(buttonNumber >= 0);
    //bearingAction->setEnabled(buttonNumber >= 0);
    //logAction->setEnabled(buttonNumber >= 0);
    //memoryAction->setEnabled(buttonNumber >= 0);
    //clearSpotsAction->setEnabled(buttonNumber >= 0);
}



void ClusterClientFrame::on_freqActionSelected()
{
    int curTab = ui->dxSpotTab->currentIndex();
    {
        if (filterProxyModelList[curTab]->rowCount() > 0)
        {
            int currentRow = spotViewList[curTab]->currentIndex().row();
            QString freq = filterProxyModelList[curTab]->data(filterProxyModelList[curTab]->index(currentRow, FREQ_COL_NUM), DataStoredRole).toString();
            sendFreqToRig(freq);
        }
    }

}




void ClusterClientFrame::bearingActionSelected()
{
    int curTab = ui->dxSpotTab->currentIndex();
    {
        if (filterProxyModelList[curTab]->rowCount() > 0)
        {
            int currentRow = spotViewList[curTab]->currentIndex().row();
            QString brg = filterProxyModelList[curTab]->data(filterProxyModelList[curTab]->index(currentRow, DXBRG_COL_NUM), DataStoredRole).toString();
            QString loc = filterProxyModelList[curTab]->data(filterProxyModelList[curTab]->index(currentRow, DXLOC_COL_NUM), DataStoredRole).toString();
            if (loc.count() < 6)
            {
                brg = brg.append(SHORTLOCATOR_IDENTIFIER);

            }
            sendBrgToRot(brg);
        }
    }
}



void ClusterClientFrame::logActionSelected()
{
    int curTab = ui->dxSpotTab->currentIndex();
    if (filterProxyModelList[curTab]->rowCount() > 0)
    {
        int currentRow = spotViewList[curTab]->currentIndex().row();
        // transfer spot details to qsolog

        MinosLoggerEvents::SendSpotToLog(getSpotDataToMemoryVariable(filterProxyModelList[curTab], currentRow));
    }
}


void ClusterClientFrame::memoryActionSelected()
{
    int curTab = ui->dxSpotTab->currentIndex();

    if (filterProxyModelList[curTab]->rowCount() > 0)
    {
        int currentRow = spotViewList[curTab]->currentIndex().row();
        if (currentRow >= 0 && currentRow < filterProxyModelList[curTab]->rowCount())
        {
            // check if spot has been sent to memory
            if (!filterProxyModelList[curTab]->data(filterProxyModelList[curTab]->index(currentRow, DXSPOT_TO_MEMORY_FLAG_COL_NUM), DataStoredRole).toBool())
            {
                sendSpotToMemory(filterProxyModelList[curTab], currentRow);
            }

        }

    }
}

// this sends spot to memory if it has allready been sent
void ClusterClientFrame::memoryActionOverideSelected()
{
    int curTab = ui->dxSpotTab->currentIndex();

    if (filterProxyModelList[curTab]->rowCount() > 0)
    {
        int currentRow = spotViewList[curTab]->currentIndex().row();
        if (currentRow >= 0 && currentRow < filterProxyModelList[curTab]->rowCount())
        {
            // check if spot has been sent to memory
            if (filterProxyModelList[curTab]->data(filterProxyModelList[curTab]->index(currentRow, DXSPOT_TO_MEMORY_FLAG_COL_NUM), DataStoredRole).toBool())
            {
                sendSpotToMemory(filterProxyModelList[curTab], currentRow);
            }

        }

    }
}


void ClusterClientFrame::sendSpotToMemory(DxSpotSortFilterProxyModel* spotProxyModel, int row)
{

    memoryData::memData spotData = getSpotDataToMemoryVariable(spotProxyModel, row);

    MinosLoggerEvents::SendSpotToMemory(spotData);
    spotProxyModel->setData(spotProxyModel->index(row, DXSPOT_TO_MEMORY_FLAG_COL_NUM), BOOL_YES, DataStoredRole);

}

memoryData::memData ClusterClientFrame::getSpotDataToMemoryVariable(DxSpotSortFilterProxyModel* spotProxyModel, int row)
{
    memoryData::memData spotData;
    spotData.callsign = spotProxyModel->data(spotProxyModel->index(row, DXSPOT_CALL_COL_NUM), DataStoredRole).toString();
    spotData.time = spotProxyModel->data(spotProxyModel->index(row, TIME_COL_NUM), DataStoredRole).toString();
    spotData.freq = spotProxyModel->data(spotProxyModel->index(row, FREQ_COL_NUM), DataStoredRole).toString().remove('.');
    spotData.mode = memDefData::DEFAULT_MODE;
    spotData.locator = spotProxyModel->data(spotProxyModel->index(row, DXLOC_COL_NUM), DataStoredRole).toString();
    spotData.bearing = spotProxyModel->data(spotProxyModel->index(row, DXBRG_COL_NUM), DataStoredRole).toString().toInt();
    return spotData;
}

void ClusterClientFrame::clearSpotActionSelected()
{
    int row;

    int curTab = ui->dxSpotTab->currentIndex();

    if (filterProxyModelList[curTab]->rowCount() > 0)
    {
        row = spotViewList[curTab]->currentIndex().row();
        if (row >= 0 && row < filterProxyModelList[curTab]->rowCount())
        {
            int ret = QMessageBox::warning(this, tr("Cluster"),
                                           tr("Please confirm you want to delete this spot?"),
                                           QMessageBox::Yes | QMessageBox::No);

            if (ret == QMessageBox::Yes)
            {
                // map to source row
                row = filterProxyModelList[curTab]->mapToSource(spotViewList[curTab]->currentIndex()).row();
                dxSpotDataModel->removeRows(row, 1, QModelIndex());

            }
        }


    }

}



void ClusterClientFrame::clearAllSpotsActionSelected()
{

    int curTab = ui->dxSpotTab->currentIndex();

    if (filterProxyModelList[curTab]->rowCount() > 0)
    {
        int ret = QMessageBox::warning(this, tr("Cluster"),
                                       tr("Please confirm you want to delete all the spots in the %1 tab?").arg(ui->dxSpotTab->tabText(curTab)),
                                       QMessageBox::Yes | QMessageBox::No);

        if (ret == QMessageBox::Yes)
        {
            filterProxyModelList[curTab]->removeRows(0, filterProxyModelList[curTab]->rowCount(), QModelIndex());
        }
    }
}

void ClusterClientFrame::onSearchEditingFinished()
{
    static QString oldEntry = "";

    if (ui->searchLineEdit->text() != oldEntry)
    {
        oldEntry = ui->searchLineEdit->text();

        if (ui->dxSpotTab->currentIndex() != SEARCH_TAB)
        {
            ui->dxSpotTab->setCurrentIndex(SEARCH_TAB);
        }

        if (ui->searchLineEdit->text().trimmed().isEmpty())
        {
            searchSortProxyModel->searchParameter = "";
            searchSortProxyModel->setFilterRegExp("");
        }
        else
        {
            searchSortProxyModel->searchParameter = ui->searchLineEdit->text().trimmed();
            //ui->searchLineEdit->selectAll();
            searchSortProxyModel->setFilterRegExp("");
        }

        ui->searchLineEdit->setFocus();
    }

}


void ClusterClientFrame::on_AfterLogContact( BaseContestLog *c, Callsign cs, QString loc)
{
      bool worked = false;
      if (c && ct == c)
      {

          for (int spotNumber = 0; spotNumber < dxSpotDataModel->rowCount(); spotNumber++)
          {
              int bandMask = dxSpotDataModel->data(dxSpotDataModel->index(spotNumber, DXBANDMASK_COL_NUM,  QModelIndex()), DataStoredRole).toString().toInt();
              if (bandMask == contestBand)
              {
              QString callsign = dxSpotDataModel->data(dxSpotDataModel->index(spotNumber, DXSPOT_CALL_COL_NUM,  QModelIndex()), DataStoredRole).toString();
                  //bool callsignWkd = dxSpotDataModel->data(dxSpotDataModel->index(spotNumber, DXSPOT_CALL_WORKED_COL_NUM,  QModelIndex()), DataStoredRole).toBool();
                  //if (!callsignWkd)
                  //{
                      if (cs.realCall == callsign)
                      {
                          dxSpotDataModel->setData(dxSpotDataModel->index(spotNumber, DXSPOT_CALL_WORKED_COL_NUM,  QModelIndex()), BOOL_YES, DataStoredRole);
                          worked = true;
                      }
                  //}


              QString locator = dxSpotDataModel->data(dxSpotDataModel->index(spotNumber, DXLOC_COL_NUM,  QModelIndex()), DataStoredRole).toString();
                  //bool locatorWkd = dxSpotDataModel->data(dxSpotDataModel->index(spotNumber, DXLOC_WORKED_COL_NUM,  QModelIndex()), DataStoredRole).toBool();

                  //if(!locatorWkd)
                  //{
                      if (loc.mid(0,4) == locator.mid(0, 4) )
                      {
                          dxSpotDataModel->setData(dxSpotDataModel->index(spotNumber, DXLOC_WORKED_COL_NUM,  QModelIndex()), BOOL_YES, DataStoredRole);
                          worked = true;
                      }
                  //}
              }
          }

          if (worked)
          {
              // refresh views
              if (ui->dxSpotTab->currentIndex() == DXSPOT_TAB)
              {
                  dxSpotProxyModel->setDynamicSortFilter(false);
                  dxSpotProxyModel->sort(RXTIME_COL_NUM, Qt::DescendingOrder);
                  dxSpotProxyModel->setDynamicSortFilter(true);
              }
              else if (ui->dxSpotTab->currentIndex() == CALLSIGN_TAB)
              {
                  callSignProxyModel->setDynamicSortFilter(false);
                  callSignProxyModel->sort(RXTIME_COL_NUM, Qt::DescendingOrder);
                  callSignProxyModel->setDynamicSortFilter(true);
              }
              else if (ui->dxSpotTab->currentIndex() == LOCATOR_TAB)
              {
                  locatorProxyModel->setDynamicSortFilter(false);
                  locatorProxyModel->sort(RXTIME_COL_NUM, Qt::DescendingOrder);
                  locatorProxyModel->setDynamicSortFilter(true);
              }
              else if (ui->dxSpotTab->currentIndex() == SEARCH_TAB)
              {
                  searchSortProxyModel->setDynamicSortFilter(false);
                  searchSortProxyModel->sort(RXTIME_COL_NUM, Qt::DescendingOrder);
                  searchSortProxyModel->setDynamicSortFilter(true);
              }

          }

      }
}


void ClusterClientFrame::checkNewSpots()
{

    if (ct && !ct->isProtected() && ct == TContestApp::getContestApp()->getCurrentContest())
    {
        LoggerContestLog* contest = dynamic_cast<LoggerContestLog *>( ct);

        if (ui->dxSpotTab->currentIndex() == DXSPOT_TAB || dxSpotProxyModel->rowCount() == 0)
        {
            newSpotIndToggle(false);
            contest->lastSpotTabTime = QDateTime::currentDateTimeUtc();
        }
        else if (ui->dxSpotTab->currentIndex() == CALLSIGN_TAB || callSignProxyModel->rowCount() == 0)
        {
            newCallsignSpotIndToggle(false);
            contest->lastCallsignTabTime = QDateTime::currentDateTimeUtc();
        }
        else if (ui->dxSpotTab->currentIndex() == LOCATOR_TAB  || locatorProxyModel->rowCount() == 0)
        {
            newLocatorSpotIndToggle(false);
            contest->lastLocatorTabTime = QDateTime::currentDateTimeUtc();
        }


        if (ui->dxSpotTab->currentIndex() != DXSPOT_TAB)
        {
            if (dxSpotProxyModel->rowCount() == 0)
            {
                newSpotIndToggle(false);
                contest->lastSpotTabTime = QDateTime::currentDateTimeUtc();
            }
            else if (getNumberSpotsIndicator(contest->lastSpotTabTime, dxSpotProxyModel) > 0)
            {
                newSpotIndToggle(true);
            }
        }



        if (ui->dxSpotTab->currentIndex() != CALLSIGN_TAB)
        {
            if (callSignProxyModel->rowCount() == 0)
            {
                newCallsignSpotIndToggle(false);
                contest->lastCallsignTabTime = QDateTime::currentDateTimeUtc();
            }
            else if (getNumberSpotsIndicator(contest->lastCallsignTabTime, callSignProxyModel) > 0)
            {
                newCallsignSpotIndToggle(true);
            }
        }



        if (ui->dxSpotTab->currentIndex() != LOCATOR_TAB)
        {
            if (locatorProxyModel->rowCount() == 0)
            {
                newLocatorSpotIndToggle(false);
                contest->lastLocatorTabTime = QDateTime::currentDateTimeUtc();
            }
            else if (getNumberSpotsIndicator(contest->lastLocatorTabTime, locatorProxyModel) > 0)
            {
                newLocatorSpotIndToggle(true);
            }
        }
    }


}

int ClusterClientFrame::getNumberSpotsIndicator(const QDateTime& _lastTime, DxSpotSortFilterProxyModel* _spotProxyModel)
{
    int spotCount = 0;
    if (_spotProxyModel->rowCount() > 0)
    {
        qlonglong spotTime = 0;
        qlonglong lastTime = _lastTime.toMSecsSinceEpoch() / 1000;
        for (int i =0; i < _spotProxyModel->rowCount(); i++)
        {
            spotTime = _spotProxyModel->data(_spotProxyModel->index(i, RXTIME_COL_NUM), DataStoredRole).toLongLong();
            if (spotTime > lastTime)
            {
                spotCount++;
            }
        }
    }


    return spotCount;
}



void ClusterClientFrame::checkSavedFilters()
{
    // this looks for changed saved settings
    LoggerContestLog* contest = dynamic_cast<LoggerContestLog *>( ct);
    if (contest)
    {
        QString cUuuid = ct->uuid;
        ClusterClientFilterSettings cfs = contest->clusterFilterSettings.getValue();
        if (cfs != filterSetup->filterSettings)
        {
            filterSetup->filterSettings = cfs;
        }
    }
}

void ClusterClientFrame::handleClusterStatusMessage(QString &msg)
{

    if (msg.contains("!Connected"))
    {
         statusIndicatorToggle(true);
    }
    else
    {
         statusIndicatorToggle(false);
    }

    QStringList sl;
    sl = msg.split(CLUSTER_STATUS);
    if (sl.count() ==  2)
    {
        QString statusMsg = QString("%1").arg(sl[1]);
        ui->statusIndicator->setToolTip(statusMsg);
        trace(QString("Cluster Status: %1").arg(statusMsg));
    }
    else
    {
        ui->statusIndicator->setToolTip("");
    }
}








void ClusterClientFrame::statusIndicatorToggle(bool on)
{
    if (on)
    {
        ui->statusIndicator->setStyleSheet(STATUS_INDICATOR_CONNECT_STYLE);
    }
    else
    {
       ui->statusIndicator->setStyleSheet(STATUS_INDICATOR_DISCONNECT_STYLE);
    }

}




/*
void ClusterClientFrame::queueIndToggle(bool on)
{

    if (on)
    {
        ui->queueIndicator->setStyleSheet(NEWSPOT_INDICATOR_ON_STYLE);
    }
    else
    {
       ui->queueIndicator->setStyleSheet(NEWSPOT_INDICATOR_OFF_STYLE);
    }

}

*/

void ClusterClientFrame::newSpotIndToggle(bool on)
{

    if (on)
    {
        ui->spotIndicator->setStyleSheet(NEWSPOT_INDICATOR_ON_STYLE);
    }
    else
    {
       ui->spotIndicator->setStyleSheet(NEWSPOT_INDICATOR_OFF_STYLE);
    }

}

void ClusterClientFrame::newCallsignSpotIndToggle(bool on)
{

    if (on)
    {
        ui->callsignIndicator->setStyleSheet(NEWSPOT_INDICATOR_ON_STYLE);
    }
    else
    {
       ui->callsignIndicator->setStyleSheet(NEWSPOT_INDICATOR_OFF_STYLE);
    }

}

void ClusterClientFrame::newLocatorSpotIndToggle(bool on)
{

    if (on)
    {
        ui->locatorIndicator->setStyleSheet(NEWSPOT_INDICATOR_ON_STYLE);
    }
    else
    {
       ui->locatorIndicator->setStyleSheet(NEWSPOT_INDICATOR_OFF_STYLE);
    }

}



bool ClusterClientFrame::event(QEvent *event)
{
    if (event->type() == QEvent::Enter)
    {
        holdUpdateFlag = true;
    }
    else if (event->type() == QEvent::Leave)
    {
        mouseInFrameTimer->stop();
        if (!spotQueue.isEmpty())
        {
            handleDxSpots(spotQueue);
        }
        holdUpdateFlag = false;

    }


    return QWidget::event(event);
}


void ClusterClientFrame::setHoldUpdateFlag(bool state)
{

    holdUpdateFlag = state;
}


bool ClusterClientFrame::isSpotQueueEmpty()
{
    return spotQueue.isEmpty();
}

void ClusterClientFrame::buttonHandleDxSpots()
{
    handleDxSpots(spotQueue);
}

void ClusterClientFrame::mouseMoveEvent(QMouseEvent *event)
{
    static QPoint mousePos = QPoint(0, 0);
    if (holdUpdateFlag)
    {
       mouseInFrameTimer->start(MOUSE_IN_FRAME_TIMEOUT);
       if (mousePos != event->pos())
       {
           mousePos = event->pos();
           mouseInFrameTimer->start(MOUSE_IN_FRAME_TIMEOUT);
       }
    }

}

void ClusterClientFrame::mouseTimerCheckNewSpots()
{
    if (holdUpdateFlag)
    {
        if (!spotQueue.isEmpty())
        {
            handleDxSpots(spotQueue);
        }
        mouseInFrameTimer->start(MOUSE_IN_FRAME_TIMEOUT);
    }
}


bool DxSpotSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
    return matchBand(sourceRow) && matchMode(sourceRow);

}

bool DxSpotSortFilterProxyModel::matchBand(int sourceRow) const
{
    bool ok = false;
    int bandMask = sourceModel()->data(sourceModel()->index(sourceRow, DXBANDMASK_COL_NUM), DataStoredRole).toString().toInt(&ok);

    if (ok && (bandMask >=0 && bandMask < NUMBANDS) )
    {
       return filterSetup->filterSettings.getBandFilter(bandMask);
    }
    else if (!ok && filterSetup->getEnableHFSpotsFlag())
    {
        return true;        // if DXBandMask is empty it must be a HF Spot
    }
    else
    {
        return false;
    }

}

bool DxSpotSortFilterProxyModel::matchMode(int sourceRow) const
{
    bool ok = false;
    int modeMask = sourceModel()->data(sourceModel()->index(sourceRow, DXMODEMASK_COL_NUM), DataStoredRole).toString().toInt(&ok);
    if (ok and modeMask >=0)
    {
        return filterSetup->filterSettings.getModeFilter(modeMask);
    }
    else
    {
        return false;
    }

}


bool SearchSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
    if (!searchParameter.isEmpty() && matchBand(sourceRow) && matchMode(sourceRow))
    {
        if (searchParameter.contains(SEARCH_LOC_EXP))
        {
            QString loc = sourceModel()->data(sourceModel()->index(sourceRow, DXLOC_COL_NUM), DataStoredRole).toString().mid(0,4);
            if (loc.contains(searchParameter, Qt::CaseInsensitive))
            {
                return true;
            }
        }
        else
        {
            Callsign spotCall(sourceModel()->data(sourceModel()->index(sourceRow, DXSPOT_CALL_COL_NUM), DataStoredRole).toString());
            spotCall.validate();
            if (spotCall.realCall.contains(searchParameter, Qt::CaseInsensitive))
            {
                return true;
            }
        }
    }


    return false;
}



bool CallsignSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{

    if (!filterSetup->getCallsignFilterList().empty())
    {
        if (matchBand(sourceRow)  && matchMode(sourceRow))
        {
            Callsign spotCall(sourceModel()->data(sourceModel()->index(sourceRow, DXSPOT_CALL_COL_NUM), DataStoredRole).toString());
            spotCall.validate();
            foreach (const QString &str, filterSetup->getCallsignFilterList())
            {
                if (spotCall.realCall.contains(str, Qt::CaseInsensitive))
                {
                    return true;
                }
            }
        }
    }

    return false;
}



bool LocatorSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
    if (!filterSetup->getLocatorFilterList().empty())
    {
        if (matchBand(sourceRow)  && matchMode(sourceRow))
        {
            QModelIndex index = sourceModel()->index(sourceRow, DXLOC_COL_NUM);
            QString locator = sourceModel()->data(index, DataStoredRole).toString().mid(0,4);
            if (locator != "")
            {
                if (filterSetup->getLocatorFilterList().contains(locator))
                {
                    return true;
                }

            }
        }
    }

    return false;
}
