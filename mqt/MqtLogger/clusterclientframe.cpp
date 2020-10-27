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
#include "delayedaction.h"


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

    ui->clusterClientFrameTitle->setText(tr("Cluster"));
    statusIndicatorToggle(false);

    ui->clusterSplitter->setStretchFactor(0, 2);
    ui->clusterSplitter->setStretchFactor(1, 1);

    restoreSplitters();

    traceMsg(QString("Starting"));

    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpClusterTraceDebug, traceDebugFlag );


    int lcf;
    MinosParameters::getMinosParameters() ->getIntDisplayProfile(edpListCompression, lcf);
    dxDelegate = QSharedPointer<HtmlDelegate>(new HtmlDelegate(1.0, lcf/100.0)) ;
    searchDelegate = QSharedPointer<HtmlDelegate>(new HtmlDelegate(1.0, lcf/100.0)) ;
    callsignDelegate = QSharedPointer<HtmlDelegate>(new HtmlDelegate(1.0, lcf/100.0)) ;
    locatorDelegate = QSharedPointer<HtmlDelegate>(new HtmlDelegate(1.0, lcf/100.0)) ;

    this->setMouseTracking(true);
    mouseInFrameTimer = new QTimer(this);
    connect (mouseInFrameTimer, SIGNAL(timeout()), this, SLOT(mouseTimerCheckNewSpots()));

    purgeTimer = new QTimer(this);

    checkNewSpotsTimer = new QTimer(this);
    connect (checkNewSpotsTimer, SIGNAL(timeout()), this, SLOT(checkNewSpots()));

    spotQueue.clear();

    checkNewFilters = new QTimer(this);
    connect (checkNewFilters, SIGNAL(timeout()), this, SLOT(checkSavedFilters()));

    connect (ClusterClientServer::getClusterClientServer(), SIGNAL(ClusterServerList(QVector<ClusterServer>)), this, SLOT(clusterClientServerList(QVector<ClusterServer>)));
    connect (ClusterClientServer::getClusterClientServer(), SIGNAL(dxSpot(QVector<ClusterMessage>)), this, SLOT(dxSpots(QVector<ClusterMessage>)));

    connect (purgeTimer, SIGNAL(timeout()), this, SLOT(purgeSpots()));


    connect(&MinosLoggerEvents::mle, SIGNAL(FontChanged()), this, SLOT(on_FontChanged()), Qt::QueuedConnection);

    connect (ui->filtersBut, SIGNAL(clicked()), this, SLOT(filterButtonSelected()));


    spotsMenu = new QMenu(ui->actionsButton);

    ui->actionsButton->setFocusPolicy(Qt::NoFocus);
    actionInObject = new MouseInObject(this, this);
    spotsMenu->installEventFilter(actionInObject);

    freqAction = new QAction(tr("Set &Freq"), this);
    bearingAction = new QAction(tr("Set &Bearing"), this);
    logAction = new QAction(tr("Send &Log"), this);
    memoryAction = new QAction(tr("Send &Memory"), this);
    clearSpotAction = new QAction(tr("Clear &Spot"), this);
    clearAllSpotsAction = new QAction(tr("Clear &All Spots"), this);
    //memoryActionOveride = new QAction(tr("Force &Send Memory"), this);

    spotsMenu->addAction(freqAction);
    spotsMenu->addAction(bearingAction);
    spotsMenu->addAction(logAction);
    spotsMenu->addAction(memoryAction);
    spotsMenu->addAction(clearSpotAction);
    spotsMenu->addAction(clearAllSpotsAction);
    //spotsMenu->addAction(memoryActionOveride);

    ui->actionsButton->setMenu(spotsMenu);
    connect(spotsMenu, SIGNAL(aboutToShow()), this, SLOT(onMenuShow()));

    connect( freqAction, SIGNAL( triggered() ), this, SLOT(on_freqActionSelected()) );
    connect( bearingAction, SIGNAL( triggered() ), this, SLOT(bearingActionSelected()) );
    connect( logAction, SIGNAL( triggered() ), this, SLOT(logActionSelected()) );
    connect( memoryAction, SIGNAL( triggered() ), this, SLOT(memoryActionSelected()) );
    connect( clearSpotAction, SIGNAL( triggered() ), this, SLOT(clearSpotActionSelected()) );
    connect( clearAllSpotsAction, SIGNAL( triggered() ), this, SLOT(clearAllSpotsActionSelected()) );
    //connect( memoryActionOveride, SIGNAL( triggered() ), this, SLOT(memoryActionOverideSelected()) );

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


    //connect(filterSetup, SIGNAL(filtersChanged(bool, bool, bool, bool)), this, SLOT(filtersChanged(bool, bool, bool, bool)));

    connect(ui->unworkedCallsignsChkBox, SIGNAL(stateChanged(int)), this, SLOT(on_unworkedCallsignsCheckBox(int)));
    connect(ui->unworkedLocChkBox, SIGNAL(stateChanged(int)), this, SLOT(on_unworkedLocCheckBox(int)));

    purgeTimer->start(PURGE_TIME);

    newSpotIndToggle(false);
    newCallsignSpotIndToggle(false);
    newLocatorSpotIndToggle(false);
    checkNewFilters->start(CHECK_NEWFILTERS_DURATION);
    checkNewSpotsTimer->start(CHECKSPOTS_DURATION);

    //setClusterServerState(LogContainer->getClusterServerState()); // need tslf?

    //QShortcut *shortcut = new QShortcut(QKeySequence("Ctrl+a"), parent);
    //QObject::connect(shortcut, SIGNAL(activated()), this, SLOT(onMenuShow()));
    connect(ui->resendSpotsTestPb, SIGNAL(pressed()), this, SLOT(on_pushbuttonPressed()));


    if (!isProtected)
    {
        QTimer::singleShot(2000, this, SLOT(requestSpots()));

    }


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

void ClusterClientFrame::on_pushbuttonPressed()
{
    if (ct  && contestBand != -1)
    {
        MinosLoggerEvents::SendRequestResendSpotsToClusterServer(resendFrameId::CLUSTER_CLIENT, RESEND_ALL_SPOTS, contestBand, ct->uuid);
    }
}


void ClusterClientFrame::requestSpots()
{

    if (ct && contestBand != -1)
    {

        MinosLoggerEvents::SendRequestResendSpotsToClusterServer(resendFrameId::CLUSTER_CLIENT, RESEND_ALL_SPOTS, contestBand, ct->uuid);
    }
}




void ClusterClientFrame::delayed_afterLogContact(BaseContestLog *c, Callsign cs, QString loc)
{
    // delay the search of the spots until the contact logging should have finished
    // and the screen been redrawn, or a lot of spots slows things down too much

    delayedAction(this, [=]()
    {
        // NB a lambda function
        on_AfterLogContact(c, cs, loc);
    }
    , 50
    );
}



void ClusterClientFrame::setupDXSpotView()
{
    dxSpotView = new QTableView();
    dxSpotView->setFocusPolicy(Qt::NoFocus);

    dxSpotProxyModel = new DxSpotSortFilterProxyModel(&filterSettings);
    dxSpotProxyModel->traceDebugFlag = traceDebugFlag;
    dxSpotProxyModel->setSourceModel(dxSpotDataModel);
    dxSpotProxyModel->sort(RXTIME_COL_NUM, Qt::DescendingOrder);
    //dxSpotProxyModel->setDynamicSortFilter(true);

    ui->dxSpotTab->addTab(dxSpotView, tr("DX Spots"));
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

    spotVerticalHeader->setSectionResizeMode(QHeaderView::Interactive);

    // show these columns
    dxSpotView->setColumnHidden(TIME_COL_NUM, false);
    dxSpotView->setColumnHidden(FREQ_COL_NUM, false);
    dxSpotView->setColumnHidden(DXSPOT_CALL_COL_NUM, false);
    dxSpotView->setColumnHidden(DXSPOT_MODE_COL_NUM, false);
    dxSpotView->setColumnHidden(DXLOC_COL_NUM, false);
    dxSpotView->setColumnHidden(DXDIST_COL_NUM, false);
    dxSpotView->setColumnHidden(DXBRG_COL_NUM, false);
    dxSpotView->setColumnHidden(SPOTTER_CALL_COL_NUM, false);
    dxSpotView->setColumnHidden(SPOTTER_LOC_COL_NUM, false);
    dxSpotView->setColumnHidden(COMMENT_COL_NUM, false);

    // hide these columns
    dxSpotView->setColumnHidden(DXMODEMASK_COL_NUM, true);
    dxSpotView->setColumnHidden(DXSPOT_CALL_WORKED_COL_NUM, true);
    dxSpotView->setColumnHidden(DXLOC_WORKED_COL_NUM, true);
    dxSpotView->setColumnHidden(DXLOC_FROM_NODE_FLAG_COL_NUM, true);
    dxSpotView->setColumnHidden(DXSPOT_TO_MEMORY_FLAG_COL_NUM, true);
    dxSpotView->setColumnHidden(RXTIME_COL_NUM, true);
    //dxSpotView->setColumnHidden(DXSPOT_MODE_COL_NUM, true);
    dxSpotView->setColumnHidden(DXSPOT_PROP_MODE_COL_NUM, true);
    dxSpotView->setColumnHidden(DXBANDMASK_COL_NUM, true);
    dxSpotView->setColumnHidden(DATE_COL_NUM, true);
    dxSpotView->setColumnHidden(DXBANDSTR_COL_NUM, true);
}


void ClusterClientFrame::setupSearchSpotView()
{
    searchView = new QTableView();
    searchView->setFocusPolicy(Qt::NoFocus);

    searchSortProxyModel = new SearchSortFilterProxyModel(&filterSettings);
    searchSortProxyModel->setSourceModel(dxSpotDataModel);
    searchSortProxyModel->sort(RXTIME_COL_NUM, Qt::DescendingOrder);

    ui->dxSpotTab->addTab(searchView, tr("Search Spots"));
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
    searchVerticalHeader->setSectionResizeMode(QHeaderView::Interactive);

    // show these columns
    searchView->setColumnHidden(TIME_COL_NUM, false);
    searchView->setColumnHidden(FREQ_COL_NUM, false);
    searchView->setColumnHidden(DXSPOT_CALL_COL_NUM, false);
    searchView->setColumnHidden(DXSPOT_MODE_COL_NUM, false);
    searchView->setColumnHidden(DXLOC_COL_NUM, false);
    searchView->setColumnHidden(DXDIST_COL_NUM, false);
    searchView->setColumnHidden(DXBRG_COL_NUM, false);
    searchView->setColumnHidden(SPOTTER_CALL_COL_NUM, false);
    searchView->setColumnHidden(SPOTTER_LOC_COL_NUM, false);
    searchView->setColumnHidden(COMMENT_COL_NUM, false);


    // hide these columns
    searchView->setColumnHidden(DXMODEMASK_COL_NUM, true);
    searchView->setColumnHidden(DXSPOT_CALL_WORKED_COL_NUM, true);
    searchView->setColumnHidden(DXLOC_WORKED_COL_NUM, true);
    searchView->setColumnHidden(DXLOC_FROM_NODE_FLAG_COL_NUM, true);
    searchView->setColumnHidden(DXSPOT_TO_MEMORY_FLAG_COL_NUM, true);
    searchView->setColumnHidden(RXTIME_COL_NUM, true);
    //searchView->setColumnHidden(DXSPOT_MODE_COL_NUM, true);
    searchView->setColumnHidden(DXSPOT_PROP_MODE_COL_NUM, true);
    searchView->setColumnHidden(DXBANDMASK_COL_NUM, true);
    searchView->setColumnHidden(DATE_COL_NUM, true);
    searchView->setColumnHidden(DXBANDSTR_COL_NUM, true);




}




void ClusterClientFrame::setupCallsignSpotView()
{
    callSignView = new QTableView();
    callSignView->setFocusPolicy(Qt::NoFocus);

    callSignProxyModel = new CallsignSortFilterProxyModel(&filterSettings);
    callSignProxyModel->setSourceModel(dxSpotDataModel);
    callSignProxyModel->sort(RXTIME_COL_NUM, Qt::DescendingOrder);

    ui->dxSpotTab->addTab(callSignView, tr("Callsign Spots"));

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

    callSignVerticalHeader->setSectionResizeMode(QHeaderView::Interactive);

    // show these columns
    callSignView->setColumnHidden(TIME_COL_NUM, false);
    callSignView->setColumnHidden(FREQ_COL_NUM, false);
    callSignView->setColumnHidden(DXSPOT_CALL_COL_NUM, false);
    callSignView->setColumnHidden(DXSPOT_MODE_COL_NUM, false);
    callSignView->setColumnHidden(DXLOC_COL_NUM, false);
    callSignView->setColumnHidden(DXDIST_COL_NUM, false);
    callSignView->setColumnHidden(DXBRG_COL_NUM, false);
    callSignView->setColumnHidden(SPOTTER_CALL_COL_NUM, false);
    callSignView->setColumnHidden(SPOTTER_LOC_COL_NUM, false);
    callSignView->setColumnHidden(COMMENT_COL_NUM, false);



    // hide these columns
    callSignView->setColumnHidden(DXMODEMASK_COL_NUM, true);
    callSignView->setColumnHidden(DXSPOT_CALL_WORKED_COL_NUM, true);
    callSignView->setColumnHidden(DXLOC_WORKED_COL_NUM, true);
    callSignView->setColumnHidden(DXLOC_FROM_NODE_FLAG_COL_NUM, true);
    callSignView->setColumnHidden(DXSPOT_TO_MEMORY_FLAG_COL_NUM, true);
    callSignView->setColumnHidden(RXTIME_COL_NUM, true);
    //callSignView->setColumnHidden(DXSPOT_MODE_COL_NUM, true);
    callSignView->setColumnHidden(DXSPOT_PROP_MODE_COL_NUM, true);
    callSignView->setColumnHidden(DXBANDMASK_COL_NUM, true);
    callSignView->setColumnHidden(DATE_COL_NUM, true);
    callSignView->setColumnHidden(DXBANDSTR_COL_NUM, true);



}

void ClusterClientFrame::setupLocatorSpotView()
{
    locatorView = new QTableView();
    locatorView->setFocusPolicy(Qt::NoFocus);

    locatorProxyModel = new LocatorSortFilterProxyModel(&filterSettings);
    locatorProxyModel->setSourceModel(dxSpotDataModel);
    locatorProxyModel->sort(RXTIME_COL_NUM, Qt::DescendingOrder);

    ui->dxSpotTab->addTab(locatorView, tr("Locator Spots"));
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
    locatorViewVerticalHeader->setSectionResizeMode(QHeaderView::Interactive);


    // show these columns
    locatorView->setColumnHidden(TIME_COL_NUM, false);
    locatorView->setColumnHidden(FREQ_COL_NUM, false);
    locatorView->setColumnHidden(DXSPOT_CALL_COL_NUM, false);
    locatorView->setColumnHidden(DXSPOT_MODE_COL_NUM, false);
    locatorView->setColumnHidden(DXLOC_COL_NUM, false);
    locatorView->setColumnHidden(DXDIST_COL_NUM, false);
    locatorView->setColumnHidden(DXBRG_COL_NUM, false);
    locatorView->setColumnHidden(SPOTTER_CALL_COL_NUM, false);
    locatorView->setColumnHidden(SPOTTER_LOC_COL_NUM, false);
    locatorView->setColumnHidden(COMMENT_COL_NUM, false);




    // hide these columns
    locatorView->setColumnHidden(DXMODEMASK_COL_NUM, true);
    locatorView->setColumnHidden(DXSPOT_CALL_WORKED_COL_NUM, true);
    locatorView->setColumnHidden(DXLOC_WORKED_COL_NUM, true);
    locatorView->setColumnHidden(DXLOC_FROM_NODE_FLAG_COL_NUM, true);
    locatorView->setColumnHidden(DXSPOT_TO_MEMORY_FLAG_COL_NUM, true);
    locatorView->setColumnHidden(RXTIME_COL_NUM, true);
    locatorView->setColumnHidden(DXSPOT_PROP_MODE_COL_NUM, true);
    locatorView->setColumnHidden(DXBANDMASK_COL_NUM, true);
    locatorView->setColumnHidden(DATE_COL_NUM, true);
    locatorView->setColumnHidden(DXBANDSTR_COL_NUM, true);
}



void ClusterClientFrame::filterButtonSelected()
{
    ClusterClientFilterDialog *filterSetup = new ClusterClientFilterDialog(ct, filterSettings, this);
    //filterSetup-> setTabCurrentIndex(filterSetup->getTabCurrentIndex());

    filterSetup->exec();
    if (filterSetup->getSettingsChangedFlag())
    {
        filterSettings = filterSetup->getFilterSettings();
        //update views..
        if (filterSetup->getBandFilterChangedFlag()
         || filterSetup->getModeFilterChangedFlag()
         || filterSetup->getDistanceFilterChangedFlag()
         || filterSetup->getIgnoreDistChangedFlag()
         || filterSetup->getIgnoreEmptyDistChangedFlag())
        {
            dxSpotProxyModel->setFilterRegExp("");
        }
        else if (filterSetup->getCallsignFilerChangedFlag())
        {
            callSignProxyModel->setFilterRegExp("");
        }
        else if (filterSetup->getLocatorFilterChangedFlag())
        {
            locatorProxyModel->setFilterRegExp("");
        }

    }


    filterSetup->close();
    delete filterSetup;

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
        if (ui->dxSpotTab->currentIndex() == DXSPOT_TAB)
        {
            setUnworkedCheckboxesVisible(true);
        }
        else
        {
            setUnworkedCheckboxesVisible(false);
        }
    }
}



void ClusterClientFrame::handleClickedItems(DxSpotSortFilterProxyModel* spotProxyModel, const QModelIndex &index)
{
    if (index.column() == FREQ_COL_NUM)
    {
        Frequency freq = qvariant_cast<Frequency>(spotProxyModel->data(index, DataStoredRole));
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
        if (!brg.isEmpty())
        {
            if (loc.count() < 6)
            {
                brg = brg.append(SHORTLOCATOR_IDENTIFIER);
            }
            sendBrgToRot(brg);
        }

    }
}

void ClusterClientFrame::handleVertHeaderClickedItems(DxSpotSortFilterProxyModel* spotProxyModel, int row)
{
    // check if spot has been sent to memory
    //if (!spotProxyModel->data(spotProxyModel->index(row, DXSPOT_TO_MEMORY_FLAG_COL_NUM), DataStoredRole).toBool())
    //{
    sendSpotToMemory(spotProxyModel, row);

    //}
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



void ClusterClientFrame::sendFreqToRig(Frequency freq)
{
    MinosLoggerEvents::SendFreqToRig(freq);
}


void ClusterClientFrame::sendBrgToRot(QString brg)
{
    traceMsg(QString("Send Bearing to Rot = %1").arg(brg));
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
        QString state = QString(clusterStateList[(*i).state]) + " " + (*i).app + "\r\n";
        traceMsg(QString("clusterClientServerList - state = %1").arg(state));
        //ui->StationList->addItem( state );
    }
}

void ClusterClientFrame::dxSpots(QVector<ClusterMessage> spotMsg)
{
    // if contest is protected ignore
    if (!isProtected && ct)
    {
        //get spot Message from queue
        for (int i = 0; i < spotMsg.count(); i++)
        {
            ClusterMessage msg = spotMsg[i];
            traceMsg(QString("retrieve cluster spot from queue - spot = %1 for loggeruuid = %2, this contest uuid = %3, this frameId = %4").arg(msg.getMessage()).arg(msg.getLoggerUuid()).arg(ct->uuid).arg(msg.getFrameId()));

            // if loggerUuid is empty, message is for all frames
            if ((msg.getLoggerUuid().isEmpty() || msg.getLoggerUuid() == ct->uuid) && (msg.getFrameId() == resendFrameId::CLUSTER_CLIENT || msg.getFrameId() == resendFrameId::ALL_CLIENTS))
            {
                if (msg.getMessage().contains(DXSPOT) || msg.getMessage().contains(RESENTSPOT))
                {
                    traceMsg(QString("Spot for this loggeruuid = %1, add to queue").arg(ct->uuid));
                    spotQueue += msg.getMessage();
                }
            }

        }
    }
    else
    {
        // protected
        spotQueue.clear();
        return;
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
       //traceMsg("syncSpots " + spotQueue[i]);
    }


    spotQueue.clear();
}


void ClusterClientFrame::addDxSpotToTable(const QString spot)
{

    traceMsg(QString("addDXSpotToTable: %1").arg(spot));
    QDateTime spotDateTime = QDateTime::currentDateTimeUtc();

    bool resentSpot = false;

    QStringList sl;
    if (spot.contains(DXSPOT))
    {
      sl = spot.split(DXSPOT);
    }
    else if (spot.contains(RESENTSPOT))
    {
        resentSpot = true;
        sl = spot.split(RESENTSPOT);
    }

    if (sl.count() == 2)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        QStringList spotlist = sl[1].split(':', Qt::KeepEmptyParts);
#else
        QStringList spotlist = sl[1].split(':', QString::KeepEmptyParts);
#endif

        if (spotlist.count() == TTLVALUE +1)
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

            bool dxLocFromNodeFlag = extractDxLocFromNodeFlag(spotlist[DXLOC_FROM_NODE_FLAG]);

            //spotDateTime = getSpotDateTime(spotlist[SPOTDATE], spotlist[SPOTTIME]);
            spotDateTime = QDateTime::fromString(spotlist[SPOTDATETIME], "yyyyMMMddHHmmss" );
            qint64 rxTime = spotDateTime.toMSecsSinceEpoch()/1000;

            ClusterSpotData* newSpot = new ClusterSpotData();

            newSpot->setRxTime(rxTime);
            newSpot->setSpotDateTime(spotDateTime);
            newSpot->setFreq(spotlist[DXFREQ]);
            newSpot->setBand(spotlist[DXBANDSTR]);
            newSpot->setBandMask(spotlist[DXBANDMASK]);
            newSpot->setMode(spotlist[DXMODESTR]);
            newSpot->setModeMask(spotlist[DXMODEMASK]);
            newSpot->setDxCall(spotlist[DXCALL]);
            newSpot->setDxCallWorked(callWorked);
            newSpot->setDxLocator(spotlist[DXLOCATOR]);
            newSpot->setDxLocatorIsFromNode(dxLocFromNodeFlag);
            newSpot->setDxLocatorWorked(locWorked);
            newSpot->setDxDist(distance);
            newSpot->setDxBrg(bearing);
            newSpot->setSpotterCall(spotlist[SPOTCALL]);
            newSpot->setSpotterLocator(spotlist[SPOTLOCATOR]);
            newSpot->setDxPropMode(spotlist[DXPROPMODE]);
            newSpot->setSpotComment(spotlist[SPOTCOMMENT]);

            dxSpotDataModel->rowData = newSpot;



            if (resentSpot)
            {
                if (checkspotExists(newSpot))
                {
                    return;     // spot exists in table
                }
            }

            //dxSpotDataModel->rowData = newSpot;

            dxSpotDataModel->insertRows(dxSpotDataModel->rowCount(), 1);
            traceMsg(QString("addDxSpotToTable: adding %1 to cluster data table").arg(spotlist[DXCALL]));
       }
    }

}


bool ClusterClientFrame::checkspotExists(ClusterSpotData *spotData)
{
    if (dxSpotDataModel->rowCount() == 0)
    {
        return false;
    }


    for (int i = 0; i < dxSpotDataModel->rowCount(); i++)
    {
        if (*dxSpotDataModel->getSpotData(i) == *spotData)
        {
            trace(QString("Spot Call = %1, already in display, skip").arg(spotData->getDxCall().realCall));
            return true;
        }
    }


    return false;
}



bool ClusterClientFrame::checkDbRowForMatch(QString incomingVal, int row, const int colNum)
{
    if (incomingVal == dxSpotDataModel->data(dxSpotDataModel->index(row, colNum,  QModelIndex()), DataStoredRole).toString())
    {
        return true;
    }

    return false;
}

bool ClusterClientFrame::checkDbRowForMatch(bool incomingVal, int row, const int colNum)
{
    if (incomingVal == dxSpotDataModel->data(dxSpotDataModel->index(row, colNum,  QModelIndex()), DataStoredRole).toBool())
    {
        return true;
    }

    return false;
}

bool ClusterClientFrame::checkDbRowForMatch(Frequency incomingVal, int row, const int colNum)
{

    if (incomingVal == qvariant_cast<Frequency>(dxSpotDataModel->data(dxSpotDataModel->index(row, colNum,  QModelIndex()), DataStoredRole)))
    {
        return true;
    }

    return false;
}

bool ClusterClientFrame::checkDbRowForMatch(qint64 incomingVal, int row, const int colNum)
{
    if (incomingVal == dxSpotDataModel->data(dxSpotDataModel->index(row, colNum,  QModelIndex()), DataStoredRole).toLongLong())
    {
        return true;
    }

    return false;
}

void ClusterClientFrame::checkSpotWorked(QString &callsign, QString &locator, bool* callWorked, bool* locatorWorked)
{
    bool callfound = false;
    bool locfound = false;
    if (ct && !ct->isReadOnly())
    {

        Callsign mcs;
        mcs.setFullCall(callsign);

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
                if ((*i).wt->loc.getLoc().mid(0,4) == loc)
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
    //filterSetup->setContest(c);

    if (ct != nullptr)
    {
        contestUuid = ct->uuid;
        traceMsg(QString("Set Contest: ContestUuid = %1").arg(contestUuid));
        contestBandStr = ct->contestBands.getValue();
        contestBand = getBandOffSet(contestBandStr);
        contestModeStr = ct->currentMode.getValue();
        contestMode = getModeOffSet(contestModeStr);

        ClusterFilterIdAndNames clustId;

        if (contestBand != -1)
        {
            if (!contest->clusterFilterSettingsExist)       // have band settings been saved before?
            {
                // no, save current band filter for this contest
                filterSettings.setBandFilter(true, contestBand);    // set cluster filter to current band - can be overidden

                if (contestModeStr == "MGM")       //  have mode settings been saved before?
                {
                    for (int m = 4; m < clusterModes.count(); m++)
                    {
                        //filterSetup->setModeFilter(true, m);  // set all the mgm modes in filter
                        *filterSettings.modeFilters[m] = true; // set all the mgm modes in filter
                    }
                }
                else
                {
                    // no, save current mode filter for this contest
                    //filterSetup->setModeFilter(true, contestMode);
                    *filterSettings.modeFilters[contestMode] = true;
                }

                //QSettings config(CLUSTER_FILTER_FILE, QSettings::IniFormat);
                //config.beginGroup("distanceFilter");

                for (int i = 0; i < filterSettings.distanceFilters.count(); i++)
                {

                    TContestApp::getContestApp() ->loggerBundle.getIntProfile( clustId.getDefaultFilterId(i), *filterSettings.distanceFilters[i]);

                }

                //config.endGroup();

                //filterSetup->saveClusterFilterToContest();  // save these settings
                contest->saveClusterFilter(filterSettings);
            }
            else
            {
                filterSettings = contest->getClusterFilter();
            }
        }
        else
        {
            traceMsg(QString("set Contest - ContestBand error %1").arg(contestBand));
        }







        if (ct && ct == TContestApp::getContestApp() ->getCurrentContest())
        {
            if (!ct->isReadOnly())
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
                   traceMsg(QString("purged spot = %1").arg(dxSpotDataModel->data(dxSpotDataModel->index(idx, DXSPOT_CALL_COL_NUM), DataStoredRole).toString()));
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
   //ui->actionsButton->showMenu();

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
            Frequency freq = qvariant_cast<Frequency>(filterProxyModelList[curTab]->data(filterProxyModelList[curTab]->index(currentRow, FREQ_COL_NUM), DataStoredRole));
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
            if (!brg.isEmpty())
            {
                QString loc = filterProxyModelList[curTab]->data(filterProxyModelList[curTab]->index(currentRow, DXLOC_COL_NUM), DataStoredRole).toString();
                if (loc.count() < 6)
                {
                    brg = brg.append(SHORTLOCATOR_IDENTIFIER);

                }
                sendBrgToRot(brg);
            }

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
            sendSpotToMemory(filterProxyModelList[curTab], currentRow);

        }

    }

}

/*
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
*/

void ClusterClientFrame::sendSpotToMemory(DxSpotSortFilterProxyModel* spotProxyModel, int row)
{

    memoryData::memData spotData = getSpotDataToMemoryVariable(spotProxyModel, row);

    MinosLoggerEvents::SendSpotToMemory(ct, spotData);
    spotProxyModel->setData(spotProxyModel->index(row, DXSPOT_TO_MEMORY_FLAG_COL_NUM), BOOL_YES, DataStoredRole);

}

memoryData::memData ClusterClientFrame::getSpotDataToMemoryVariable(DxSpotSortFilterProxyModel* spotProxyModel, int row)
{
    memoryData::memData spotData;
    spotData.callsign = spotProxyModel->data(spotProxyModel->index(row, DXSPOT_CALL_COL_NUM), DataStoredRole).toString();
    spotData.time = spotProxyModel->data(spotProxyModel->index(row, TIME_COL_NUM), DataStoredRole).toString();
    spotData.freq = qvariant_cast<Frequency>(spotProxyModel->data(spotProxyModel->index(row, FREQ_COL_NUM), DataStoredRole));

    spotData.mode = memDefData::DEFAULT_MODE;
    spotData.locator = spotProxyModel->data(spotProxyModel->index(row, DXLOC_COL_NUM), DataStoredRole).toString();
    spotData.bearing = spotProxyModel->data(spotProxyModel->index(row, DXBRG_COL_NUM), DataStoredRole).toString().toInt();
    spotData.fromBandmapOrMemory = true;
    spotData.dxLocFromNode = spotProxyModel->data(spotProxyModel->index(row, DXLOC_FROM_NODE_FLAG_COL_NUM), DataStoredRole).toBool();
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
                  dxSpotProxyModelUpdate();
              }
              else if (ui->dxSpotTab->currentIndex() == CALLSIGN_TAB)
              {
                  callSignProxyModelUpdate();
              }
              else if (ui->dxSpotTab->currentIndex() == LOCATOR_TAB)
              {
                  locatorProxyModelUpdate();
              }
              else if (ui->dxSpotTab->currentIndex() == SEARCH_TAB)
              {
                  searchProxyModelUpdate();
              }

          }

      }
}


void ClusterClientFrame::dxSpotProxyModelUpdate()
{
    dxSpotProxyModel->setFilterRegExp("");
}

void ClusterClientFrame::callSignProxyModelUpdate()
{
   callSignProxyModel->setFilterRegExp("");
}

void ClusterClientFrame::locatorProxyModelUpdate()
{
    locatorProxyModel->setFilterRegExp("");
}

void ClusterClientFrame::searchProxyModelUpdate()
{
    searchSortProxyModel->setFilterRegExp("");
}



void ClusterClientFrame::checkNewSpots()
{

    if (ct && !ct->isReadOnly() && ct == TContestApp::getContestApp()->getCurrentContest())
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
        if (cfs != filterSettings)
        {
            filterSettings = cfs;
        }
    }
}


void ClusterClientFrame::setUnworkedCheckboxesVisible(bool state)
{
    setUnWorkedLocCheckBoxVisible(state);
    setUnWorkedCallsignsCheckBoxVisible(state);
}



void ClusterClientFrame::setUnWorkedLocCheckBoxVisible(bool state)
{
    ui->unworkedLocChkBox->setVisible(state);
}

void ClusterClientFrame::setUnWorkedCallsignsCheckBoxVisible(bool state)
{
    ui->unworkedCallsignsChkBox->setVisible(state);
}


void ClusterClientFrame::on_unworkedCallsignsCheckBox(int state)
{
    if (state == Qt::Checked)
    {
        dxSpotProxyModel->setUnworkedCallsignFlag(true);
    }
    else
    {
        dxSpotProxyModel->setUnworkedCallsignFlag(false);
    }

    dxSpotProxyModelUpdate();

}




void ClusterClientFrame::on_unworkedLocCheckBox(int state)
{
    if (state == Qt::Checked)
    {
        dxSpotProxyModel->setUnworkedLocFlag(true);
    }
    else
    {
        dxSpotProxyModel->setUnworkedLocFlag(false);
    }

    dxSpotProxyModelUpdate();

}

void ClusterClientFrame::setClusterServerState(QString stateMsg)
{
    QStringList s = stateMsg.split("<>");
    if (s.size() < 2)
        return;

    if (s[0].contains("Connected"))
    {
         statusIndicatorToggle(true);
         clusterServerConnected = true;

    }
    else
    {
         statusIndicatorToggle(false);
         clusterServerConnected = false;

    }


    if (clusterServerLoaded)
    {

        ui->statusIndicator->setToolTip(s[1]);
        traceMsg(QString("Cluster Status: %1").arg(stateMsg));
    }
    else
    {
        ui->statusIndicator->setToolTip("");
    }
}


void ClusterClientFrame::setClusterServerLoaded(bool loaded)
{
    clusterServerLoaded = loaded;
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
        setHoldUpdateFlag(true);
    }
    else if (event->type() == QEvent::Leave)
    {
        mouseInFrameTimer->stop();
        if (!spotQueue.isEmpty())
        {
            handleDxSpots(spotQueue);
        }
        setHoldUpdateFlag(false);

    }


    return QWidget::event(event);
}

static void setTextToLabel(QLabel *label, QString text1, QString col, QString text2)
{
    QFontMetrics metrix(label->font());
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    int width1 = metrix.horizontalAdvance(text1);
#else
    int width1 = metrix.width(text1);
#endif

    int width = label->width() - width1 - 2;
    QString clippedText;
    if (width > 0)
        clippedText = col + metrix.elidedText(text2, Qt::ElideRight, width);
    label->setText(text1 + clippedText);

    label->setToolTip(text1 + col + text2);
}
void ClusterClientFrame::setHoldUpdateFlag(bool state)
{
    QString clText = tr("Cluster");
    holdUpdateFlag = state;
    if (state)
    {
        setTextToLabel(ui->clusterClientFrameTitle, clText, HtmlFontColour(Qt::red), tr(" - Mouse in frame, updates paused"));

    }
    else
    {
        ui->clusterClientFrameTitle->setText(clText);
        ui->clusterClientFrameTitle->setToolTip(clText);  // not really needed, as if we hover we must be in frame
    }
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


void ClusterClientFrame::traceMsg(QString msg)
{
    trace(QString("[ClusterClientFrame] %1").arg(msg));
}


bool DxSpotSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
    bool match_band = matchBand(sourceRow);
    bool match_distance = matchDistance(sourceRow);
    bool match_mode = matchMode(sourceRow);
    bool match_WorkedLoc = matchWorkedLoc(sourceRow);
    bool match_WorkedCallsign = matchWorkedCallsign(sourceRow);
    bool matchFlag = match_band && match_distance && match_mode && match_WorkedLoc && match_WorkedCallsign;
    if (traceDebugFlag)
    {
        trace(QString("[ClusterClientFrame] filter - callsign = %1, matchBand = %2, matchDistance = %3, matchmode = %4, matchWorkedLoc = %5, matchWorkedCallsign = %6, matchFlag = %7")
            .arg(sourceModel()->data(sourceModel()->index(sourceRow, DXSPOT_CALL_COL_NUM), DataStoredRole).toString())
            .arg(match_band ? "True" : "False")
            .arg(match_distance ? "True" : "False")
            .arg(match_mode ? "True" : "False")
            .arg(match_WorkedLoc ? "True" : "False")
            .arg(match_WorkedCallsign ? "True" : "False")
            .arg(matchFlag ? "True" : "False"));

    }
    return matchFlag;
}

bool DxSpotSortFilterProxyModel::matchBand(int sourceRow) const
{
    bool ok = false;
    int bandMask = sourceModel()->data(sourceModel()->index(sourceRow, DXBANDMASK_COL_NUM), DataStoredRole).toString().toInt(&ok);

    if (ok && (bandMask >=0 && bandMask < NUMBANDS) )
    {
       return filterSettings->getBandFilter(bandMask);
    }
    else if (!ok && false/*filterSetup->getEnableHFSpotsFlag()*/) // need to modify this for HF
    {
        return true;        // if DXBandMask is empty it must be a HF Spot
    }
    else
    {
        return false;
    }

}


bool DxSpotSortFilterProxyModel::matchDistance(int sourceRow) const
{
    bool ok = false;
    int bandMask = sourceModel()->data(sourceModel()->index(sourceRow, DXBANDMASK_COL_NUM), DataStoredRole).toString().toInt(&ok);

    if (ok && (bandMask >=0 && bandMask < NUMBANDS) )
    {
        if (!*filterSettings->ignoreDistanceFlags[bandMask])
        {


            QString distanceStr = sourceModel()->data(sourceModel()->index(sourceRow, DXDIST_COL_NUM), DataStoredRole).toString();
            if (distanceStr.isEmpty() && *filterSettings->ignoreEmptyDistanceFlags[bandMask])
            {
                return false;
            }

            int distance = distanceStr.toInt(&ok);
            if (ok)
            {
                return filterSettings->testDistanceFilter(distance, bandMask);
            }
        }
    }

    return true;

}

bool DxSpotSortFilterProxyModel::matchMode(int sourceRow) const
{
    bool ok = false;
    int modeMask = sourceModel()->data(sourceModel()->index(sourceRow, DXMODEMASK_COL_NUM), DataStoredRole).toString().toInt(&ok);
    if (ok && modeMask >=0)
    {
        return filterSettings->getModeFilter(modeMask);
    }
    else
    {
        return false;
    }

}

bool DxSpotSortFilterProxyModel::matchWorkedLoc(int sourceRow) const
{
    if (unWorkedLocFlag)
    {
        return !sourceModel()->data(sourceModel()->index(sourceRow, DXLOC_WORKED_COL_NUM), DataStoredRole).toBool();
    }
    else
    {
        return true;
    }
}


bool DxSpotSortFilterProxyModel::matchWorkedCallsign(int sourceRow) const
{
    if (unWorkedCallsignFlag)
    {
        return !sourceModel()->data(sourceModel()->index(sourceRow, DXSPOT_CALL_WORKED_COL_NUM), DataStoredRole).toBool();
    }
    else
    {
        return true;
    }
}


void DxSpotSortFilterProxyModel::setUnworkedLocFlag(bool state)
{
    unWorkedLocFlag = state;
}


void DxSpotSortFilterProxyModel::setUnworkedCallsignFlag(bool state)
{
    unWorkedCallsignFlag = state;
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
            Callsign spotCall;
            spotCall.setFullCall(sourceModel()->data(sourceModel()->index(sourceRow, DXSPOT_CALL_COL_NUM), DataStoredRole).toString());

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

    if (!filterSettings->unpackFilterList(filterSettings->callsignFilterList).empty())
    {
        if (matchBand(sourceRow)  && matchMode(sourceRow))
        {
            Callsign spotCall;
            spotCall.setFullCall(sourceModel()->data(sourceModel()->index(sourceRow, DXSPOT_CALL_COL_NUM), DataStoredRole).toString());

            foreach (const QString &str, filterSettings->unpackFilterList(filterSettings->callsignFilterList))
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
    if (!filterSettings->unpackFilterList(filterSettings->locatorFilterList).empty())
    {
        if (matchBand(sourceRow)  && matchMode(sourceRow))
        {
            QModelIndex index = sourceModel()->index(sourceRow, DXLOC_COL_NUM);
            QString locator = sourceModel()->data(index, DataStoredRole).toString().mid(0,4);
            if (locator != "")
            {
                if (filterSettings->unpackFilterList(filterSettings->locatorFilterList).contains(locator))
                {
                    return true;
                }

            }
        }
    }

    return false;
}
