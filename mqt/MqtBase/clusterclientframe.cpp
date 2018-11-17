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

#include "MinosRPC.h"
#include "MinosLoggerEvents.h"

#include "clusterclientframe.h"
#include "clustercommon.h"
#include "contest.h"
#include "cutils.h"
#include "base_pch.h"
#include "rigmemcommondata.h"
#include "ui_clusterclientframe.h"


ClusterClientFrame::ClusterClientFrame(QWidget *parent):
    QFrame(parent)
    , ui(new Ui::ClusterClientFrame)
    , purgeTimer(nullptr)
    , timeToLive(0)
    , purgeSpotFlag(false)
{

    ui->setupUi(this);
    filterSetup = new ClusterClientFilterDialog();

    purgeTimer = new QTimer(this);

    spotQueue.clear();

    connect (ClusterClientServer::getClusterClientServer(), SIGNAL(ClusterServerList(QVector<ClusterServer>)), this, SLOT(clusterClientServerList(QVector<ClusterServer>)));
    connect (ClusterClientServer::getClusterClientServer(), SIGNAL(dxSpot(QVector<QString>)), this, SLOT(dxSpots(QVector<QString>)));
    connect(&MinosLoggerEvents::mle, SIGNAL(FontChanged()), this, SLOT(on_FontChanged()), Qt::QueuedConnection);

    connect (ui->filtersBut, SIGNAL(clicked()), this, SLOT(filterButtonSelected()));
    connect (purgeTimer, SIGNAL(timeout()), this, SLOT(purgeSpots()));

    spotsMenu = new QMenu(ui->actionsButton);

    ui->actionsButton->setFocusPolicy(Qt::NoFocus);

    freqAction = new QAction("Set &Freq", this);
    bearingAction = new QAction("Set &Bearing", this);
    logAction = new QAction("Send &Log", this);
    memoryAction = new QAction("Send &Memory", this);
    clearSpotAction = new QAction("Clear &Spot", this);
    clearAllSpotsAction = new QAction("Clear &All Spots", this);

    spotsMenu->addAction(freqAction);
    spotsMenu->addAction(bearingAction);
    spotsMenu->addAction(logAction);
    spotsMenu->addAction(memoryAction);
    spotsMenu->addAction(clearSpotAction);
    spotsMenu->addAction(clearAllSpotsAction);

    ui->actionsButton->setMenu(spotsMenu);
    connect(spotsMenu, SIGNAL(aboutToShow()), this, SLOT(onMenuShow()));

    connect( freqAction, SIGNAL( triggered() ), this, SLOT(on_freqActionSelected()) );
    connect( bearingAction, SIGNAL( triggered() ), this, SLOT(bearingActionSelected()) );
    connect( logAction, SIGNAL( triggered() ), this, SLOT(logActionSelected()) );
    connect( memoryAction, SIGNAL( triggered() ), this, SLOT(memoryActionSelected()) );
    connect( clearSpotAction, SIGNAL( triggered() ), this, SLOT(clearSpotActionSelected()) );
    connect( clearAllSpotsAction, SIGNAL( triggered() ), this, SLOT(clearAllSpotsActionSelected()) );

    connect(&MinosLoggerEvents::mle, SIGNAL(AfterLogContact(BaseContestLog *)), this, SLOT(on_AfterLogContact(BaseContestLog *)), Qt::QueuedConnection);

    ui->searchLineEdit->setValidator(new UpperCaseValidator(true));
    connect(ui->searchLineEdit, SIGNAL(editingFinished()), this, SLOT(onSearchEditingFinished()));

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

    connect(filterSetup, SIGNAL(filtersChanged(int)), this, SLOT(filtersChanged(int)));

    purgeTimer->start(PURGE_TIME);


}


ClusterClientFrame::~ClusterClientFrame()
{
    delete ui;
}



void ClusterClientFrame::setupDXSpotView()
{
    dxSpotDataModel = new DxSpotDataModel();

    dxSpotProxyModel = new DxSpotSortFilterProxyModel(filterSetup);
    dxSpotProxyModel->setSourceModel(dxSpotDataModel);

    dxSpotView = new QTableView();
    ui->dxSpotTab->addTab(dxSpotView, "DX Spots");
    //dxSpotView = ui->dxSpotView;
    dxSpotView->setModel(dxSpotProxyModel);
    dxSpotView->setAlternatingRowColors(true);
    dxSpotView->setSelectionBehavior( QAbstractItemView::SelectRows );
    dxSpotView->setSelectionMode( QAbstractItemView::SingleSelection );
    //dxSpotView->setSelectionBehavior(QAbstractItemView::SelectItems);

    QHeaderView *spotVerticalHeader = dxSpotView->verticalHeader();
    spotVerticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    spotVerticalHeader->setDefaultSectionSize(18);



    //connect( dxSpotView->horizontalHeader(), SIGNAL(sectionResized(int, int , int)), this, SLOT( on_sectionResized(int, int , int)));
    connect(dxSpotView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onDxSpotViewClicked(const QModelIndex &)));
    connect(spotVerticalHeader, SIGNAL(sectionClicked(int)), this, SLOT(onDXSpotVertHeaderClicked(int)));

    dxSpotView->setColumnHidden(DXBANDMASK_COL_NUM, true);
    dxSpotView->setColumnHidden(MODEMASK_COL_NUM, true);
    dxSpotView->setColumnHidden(DXSPOT_CALL_WORKED_COL_NUM, true);
    dxSpotView->setColumnHidden(DXLOC_WORKED_COL_NUM, true);
    dxSpotView->setColumnHidden(DXSPOT_TO_MEMORY_FLAG_COL_NUM, true);

    dxSpotView->setColumnWidth(TIME_COL_NUM, TIME_COL_WIDTH);
    dxSpotView->setColumnWidth(FREQ_COL_NUM, FREQ_COL_WIDTH);
    dxSpotView->setColumnWidth(DXSPOT_CALL_COL_NUM, DXSPOT_CALL_COL_WIDTH);
    dxSpotView->setColumnWidth(DXSPOT_CALL_WORKED_COL_NUM, DXSPOT_CALL_WKD_COL_WIDTH);
    dxSpotView->setColumnWidth(DXLOC_COL_NUM, DXLOC_COL_WIDTH);
    dxSpotView->setColumnWidth(DXDIST_COL_NUM, DXDIST_COL_WIDTH);
    dxSpotView->setColumnWidth(DXBRG_COL_NUM, DXBRG_COL_WIDTH);
    dxSpotView->setColumnWidth(DXLOC_WORKED_COL_NUM, DXLOC_WKD_COL_WIDTH);
    dxSpotView->setColumnWidth(SPOT_CALL_COL_NUM, SPOT_CALL_COL_WIDTH);
    dxSpotView->setColumnWidth(SPOTLOC_COL_NUM, SPOTLOC_COL_WIDTH);
    dxSpotView->setColumnWidth(COMMENT_COL_NUM, COMMENT_COL_WIDTH);

    restoreDxSpotViewColumns();
}


void ClusterClientFrame::setupSearchSpotView()
{
    searchSortProxyModel = new SearchSortFilterProxyModel(filterSetup);
    searchSortProxyModel->setSourceModel(dxSpotDataModel);


    searchView = new QTableView();

    ui->dxSpotTab->addTab(searchView, "Search Spots");


    searchView->setModel(searchSortProxyModel);
    searchView->setAlternatingRowColors(true);
    searchView->setSelectionMode( QAbstractItemView::SingleSelection );
    searchView->setSelectionBehavior(QAbstractItemView::SelectItems);
    //dxSpotView->setSelectionMode( QAbstractItemView::NoSelection );

    QHeaderView *searchVerticalHeader = searchView->verticalHeader();
    searchVerticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    searchVerticalHeader->setDefaultSectionSize(18);


    //connect( callSignView->horizontalHeader(), SIGNAL(sectionResized(int, int , int)), this, SLOT( on_sectionResized(int, int , int)));
    connect(searchView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onSearchSpotViewClicked(const QModelIndex &)));
    connect(searchVerticalHeader, SIGNAL(sectionClicked(int)), this, SLOT(onSearchSpotVertHeaderClicked(int)));

    searchView->setColumnHidden(DXBANDMASK_COL_NUM, true);
    searchView->setColumnHidden(MODEMASK_COL_NUM, true);
    searchView->setColumnHidden(DXSPOT_CALL_WORKED_COL_NUM, true);
    searchView->setColumnHidden(DXLOC_WORKED_COL_NUM, true);
    searchView->setColumnWidth(TIME_COL_NUM, TIME_COL_WIDTH);
    searchView->setColumnWidth(FREQ_COL_NUM, FREQ_COL_WIDTH);
    searchView->setColumnWidth(DXSPOT_CALL_COL_NUM, DXSPOT_CALL_COL_WIDTH);
    searchView->setColumnWidth(DXSPOT_CALL_WORKED_COL_NUM, DXSPOT_CALL_WKD_COL_WIDTH);
    searchView->setColumnWidth(DXLOC_COL_NUM, DXLOC_COL_WIDTH);
    searchView->setColumnWidth(DXLOC_WORKED_COL_NUM, DXLOC_WKD_COL_WIDTH);
    searchView->setColumnWidth(DXDIST_COL_NUM, DXDIST_COL_WIDTH);
    searchView->setColumnWidth(DXBRG_COL_NUM, DXBRG_COL_WIDTH);
    searchView->setColumnWidth(SPOT_CALL_COL_NUM, SPOT_CALL_COL_WIDTH);
    searchView->setColumnWidth(SPOTLOC_COL_NUM, SPOTLOC_COL_WIDTH);
    searchView->setColumnWidth(COMMENT_COL_NUM, COMMENT_COL_WIDTH);
}




void ClusterClientFrame::setupCallsignSpotView()
{
    callSignProxyModel = new CallsignSortFilterProxyModel(filterSetup);
    callSignProxyModel->setSourceModel(dxSpotDataModel);


    callSignView = new QTableView();

    ui->dxSpotTab->addTab(callSignView, "Callsign Spots");


    callSignView->setModel(callSignProxyModel);
    callSignView->setAlternatingRowColors(true);
    callSignView->setSelectionMode( QAbstractItemView::SingleSelection );
    callSignView->setSelectionBehavior(QAbstractItemView::SelectItems);
    //dxSpotView->setSelectionMode( QAbstractItemView::NoSelection );

    QHeaderView *callSignVerticalHeader = callSignView->verticalHeader();
    callSignVerticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    callSignVerticalHeader->setDefaultSectionSize(18);


    //connect( callSignView->horizontalHeader(), SIGNAL(sectionResized(int, int , int)), this, SLOT( on_sectionResized(int, int , int)));
    connect(callSignView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onCallsignSpotViewClicked(const QModelIndex &)));
    connect(callSignVerticalHeader, SIGNAL(sectionClicked(int)), this, SLOT(onCallsignSpotVertHeaderClicked(int)));

    callSignView->setColumnHidden(DXBANDMASK_COL_NUM, true);
    callSignView->setColumnHidden(MODEMASK_COL_NUM, true);
    callSignView->setColumnHidden(DXSPOT_CALL_WORKED_COL_NUM, true);
    callSignView->setColumnHidden(DXLOC_WORKED_COL_NUM, true);
    callSignView->setColumnWidth(TIME_COL_NUM, TIME_COL_WIDTH);
    callSignView->setColumnWidth(FREQ_COL_NUM, FREQ_COL_WIDTH);
    callSignView->setColumnWidth(DXSPOT_CALL_COL_NUM, DXSPOT_CALL_COL_WIDTH);
    callSignView->setColumnWidth(DXSPOT_CALL_WORKED_COL_NUM, DXSPOT_CALL_WKD_COL_WIDTH);
    callSignView->setColumnWidth(DXLOC_COL_NUM, DXLOC_COL_WIDTH);
    callSignView->setColumnWidth(DXDIST_COL_NUM, DXDIST_COL_WIDTH);
    callSignView->setColumnWidth(DXBRG_COL_NUM, DXBRG_COL_WIDTH);
    callSignView->setColumnWidth(DXLOC_WORKED_COL_NUM, DXLOC_WKD_COL_WIDTH);
    callSignView->setColumnWidth(SPOT_CALL_COL_NUM, SPOT_CALL_COL_WIDTH);
    callSignView->setColumnWidth(SPOTLOC_COL_NUM, SPOTLOC_COL_WIDTH);
    callSignView->setColumnWidth(COMMENT_COL_NUM, COMMENT_COL_WIDTH);

    restoreCallsignViewColumns();
}

void ClusterClientFrame::setupLocatorSpotView()
{
    locatorProxyModel = new LocatorSortFilterProxyModel(filterSetup);
    locatorProxyModel->setSourceModel(dxSpotDataModel);

    locatorView = new QTableView();
    ui->dxSpotTab->addTab(locatorView, "Locator Spots");

    locatorView->setModel(locatorProxyModel);
    locatorView->setAlternatingRowColors(true);
    locatorView->setSelectionMode( QAbstractItemView::SingleSelection );
    locatorView->setSelectionBehavior(QAbstractItemView::SelectItems);
    //dxSpotView->setSelectionMode( QAbstractItemView::NoSelection );

    QHeaderView *locatorViewVerticalHeader = locatorView->verticalHeader();
    locatorViewVerticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    locatorViewVerticalHeader->setDefaultSectionSize(18);


    //connect( locatorView->horizontalHeader(), SIGNAL(sectionResized(int, int , int)), this, SLOT( on_sectionResized(int, int , int)));
    connect(locatorView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onLocatorSpotViewClicked(const QModelIndex &)));
    connect(locatorViewVerticalHeader, SIGNAL(sectionClicked(int)), this, SLOT(onLocatorSpotVertHeaderClicked(int)));


    locatorView->setColumnHidden(DXBANDMASK_COL_NUM, true);
    locatorView->setColumnHidden(MODEMASK_COL_NUM, true);
    locatorView->setColumnHidden(DXSPOT_CALL_WORKED_COL_NUM, true);
    locatorView->setColumnHidden(DXLOC_WORKED_COL_NUM, true);
    locatorView->setColumnWidth(TIME_COL_NUM, TIME_COL_WIDTH);
    locatorView->setColumnWidth(FREQ_COL_NUM, FREQ_COL_WIDTH);
    locatorView->setColumnWidth(DXSPOT_CALL_COL_NUM, DXSPOT_CALL_COL_WIDTH);
    locatorView->setColumnWidth(DXSPOT_CALL_WORKED_COL_NUM, DXSPOT_CALL_WKD_COL_WIDTH);
    locatorView->setColumnWidth(DXLOC_COL_NUM, DXLOC_COL_WIDTH);
    locatorView->setColumnWidth(DXLOC_WORKED_COL_NUM, DXLOC_WKD_COL_WIDTH);
    locatorView->setColumnWidth(DXDIST_COL_NUM, DXDIST_COL_WIDTH);
    locatorView->setColumnWidth(DXBRG_COL_NUM, DXBRG_COL_WIDTH);
    locatorView->setColumnWidth(SPOT_CALL_COL_NUM, SPOT_CALL_COL_WIDTH);
    locatorView->setColumnWidth(SPOTLOC_COL_NUM, SPOTLOC_COL_WIDTH);
    locatorView->setColumnWidth(COMMENT_COL_NUM, COMMENT_COL_WIDTH);
}



void ClusterClientFrame::filterButtonSelected()
{
    filterSetup->copyBandFilterMaskToEdit();
    filterSetup->copyModeFilterMaskToEdit();
    filterSetup->copyCallsignFilterListToListWidget();
    filterSetup->copyLocatorFilterListToListWidget();
    filterSetup-> setTabCurrentIndex(filterSetup->getTabCurrentIndex());
    filterSetup->show();

}

void ClusterClientFrame::filtersChanged(int changeMask)
{
    //update views..
    if (changeMask & FREQFILTERUP)
    {
        dxSpotProxyModel->setFilterRegExp("");
    }
    else if (changeMask & CALLSIGNUP)
    {
        callSignProxyModel->setFilterRegExp("");
    }
    else if (changeMask & LOCATORUP)
    {
        locatorProxyModel->setFilterRegExp("");
    }

}

void ClusterClientFrame::setStandAlone()
{
    RPCPubSub::subscribe(rpcConstants::LocalStationCategory);
    RPCPubSub::subscribe(rpcConstants::StationCategory);
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

memoryData::memData ClusterClientFrame::getSpotDataToMemoryVariable(DxSpotSortFilterProxyModel* spotProxyModel, int row)
{
    memoryData::memData spotData;
    spotData.callsign = spotProxyModel->data(spotProxyModel->index(row, DXSPOT_CALL_COL_NUM)).toString();
    spotData.time = spotProxyModel->data(spotProxyModel->index(row, TIME_COL_NUM)).toString();
    spotData.freq = spotProxyModel->data(spotProxyModel->index(row, FREQ_COL_NUM)).toString().remove('.').append(QString("000"));
    spotData.mode = memDefData::DEFAULT_MODE;
    spotData.locator = spotProxyModel->data(spotProxyModel->index(row, DXLOC_COL_NUM)).toString();
    return spotData;
}


void ClusterClientFrame::onDxSpotViewClicked(const QModelIndex &index)
{
    if (index.column() == FREQ_COL_NUM)
    {
        QString freq = dxSpotProxyModel->sourceModel()->data(index).toString();
        sendFreqToRig(freq);
    }
    else if (index.column() == DXSPOT_CALL_COL_NUM )
    {
        // transfer spot details to qsolog

        MinosLoggerEvents::SendSpotToLog(getSpotDataToMemoryVariable(dxSpotProxyModel, index.row()));
    }

}

void ClusterClientFrame::onDXSpotVertHeaderClicked(int row)
{
    // check if spot has been sent to memory
    if (!dxSpotProxyModel->data(dxSpotProxyModel->index(row, DXSPOT_TO_MEMORY_FLAG_COL_NUM)).toBool())
    {
        sendSpotToMemory(dxSpotProxyModel, row);

    }
}

void ClusterClientFrame::onSearchSpotViewClicked(const QModelIndex &index)
{
    if (index.column() == FREQ_COL_NUM)
    {
        QString freq = searchSortProxyModel->sourceModel()->data(index).toString();
        sendFreqToRig(freq);
    }
    else if (index.column() == DXSPOT_CALL_COL_NUM )
    {
        // transfer spot details to qsolog
        MinosLoggerEvents::SendSpotToLog(getSpotDataToMemoryVariable(searchSortProxyModel, index.row()));
    }
}

void ClusterClientFrame::onSearchSpotVertHeaderClicked(int row)
{
    // check if spot has been sent to memory
    if (!searchSortProxyModel->data(searchSortProxyModel->index(row, DXSPOT_TO_MEMORY_FLAG_COL_NUM)).toBool())
    {
        memoryData::memData spotData;
        spotData.callsign = searchSortProxyModel->data(dxSpotProxyModel->index(row, DXSPOT_CALL_COL_NUM)).toString();
        spotData.time = searchSortProxyModel->data(searchSortProxyModel->index(row, TIME_COL_NUM)).toString();
        spotData.freq = searchSortProxyModel->data(searchSortProxyModel->index(row, FREQ_COL_NUM)).toString().remove('.').append(QString("000"));
        spotData.mode = memDefData::DEFAULT_MODE;
        spotData.locator = searchSortProxyModel->data(searchSortProxyModel->index(row, DXLOC_COL_NUM)).toString();

        MinosLoggerEvents::SendSpotToMemory(spotData);
        searchSortProxyModel->setData(searchSortProxyModel->index(row, DXSPOT_TO_MEMORY_FLAG_COL_NUM), BOOL_YES, Qt::EditRole);
    }
}

void ClusterClientFrame::onCallsignSpotViewClicked(const QModelIndex &index)
{
    if (index.column() == FREQ_COL_NUM)
    {
        QString freq = callSignProxyModel->sourceModel()->data(index).toString();
        sendFreqToRig(freq);
    }
    else if (index.column() == DXSPOT_CALL_COL_NUM )
    {
        // transfer spot details to qsolog
        MinosLoggerEvents::SendSpotToLog(getSpotDataToMemoryVariable(callSignProxyModel, index.row()));
    }
}

void ClusterClientFrame::onCallsignSpotVertHeaderClicked(int row)
{
    // check if spot has been sent to memory
    if (!callSignProxyModel->data(callSignProxyModel->index(row, DXSPOT_TO_MEMORY_FLAG_COL_NUM)).toBool())
    {
        memoryData::memData spotData;
        spotData.callsign = callSignProxyModel->data(callSignProxyModel->index(row, DXSPOT_CALL_COL_NUM)).toString();
        spotData.time = callSignProxyModel->data(callSignProxyModel->index(row, TIME_COL_NUM)).toString();
        spotData.freq = callSignProxyModel->data(callSignProxyModel->index(row, FREQ_COL_NUM)).toString().remove('.').append(QString("000"));
        spotData.mode = memDefData::DEFAULT_MODE;
        spotData.locator = callSignProxyModel->data(callSignProxyModel->index(row, DXLOC_COL_NUM)).toString();

        MinosLoggerEvents::SendSpotToMemory(spotData);
        callSignProxyModel->setData(callSignProxyModel->index(row, DXSPOT_TO_MEMORY_FLAG_COL_NUM), BOOL_YES, Qt::EditRole);
    }
}

void ClusterClientFrame::onLocatorSpotViewClicked(const QModelIndex &index)
{
    if (index.column() == FREQ_COL_NUM)
    {
        QString freq = locatorProxyModel->sourceModel()->data(index).toString();
        sendFreqToRig(freq);
    }
    else if (index.column() == DXSPOT_CALL_COL_NUM )
    {
        // transfer spot details to qsolog
        MinosLoggerEvents::SendSpotToLog(getSpotDataToMemoryVariable(locatorProxyModel, index.row()));
    }
}

void ClusterClientFrame::onLocatorSpotVertHeaderClicked(int row)
{
    // check if spot has been sent to memory
    if (!locatorProxyModel->data(locatorProxyModel->index(row, DXSPOT_TO_MEMORY_FLAG_COL_NUM)).toBool())
    {
        memoryData::memData spotData;
        spotData.callsign = locatorProxyModel->data(locatorProxyModel->index(row, DXSPOT_CALL_COL_NUM)).toString();
        spotData.time = locatorProxyModel->data(locatorProxyModel->index(row, TIME_COL_NUM)).toString();
        spotData.freq = locatorProxyModel->data(locatorProxyModel->index(row, FREQ_COL_NUM)).toString().remove('.').append(QString("000"));
        spotData.mode = memDefData::DEFAULT_MODE;
        spotData.locator = locatorProxyModel->data(locatorProxyModel->index(row, DXLOC_COL_NUM)).toString();

        MinosLoggerEvents::SendSpotToMemory(spotData);
        locatorProxyModel->setData(locatorProxyModel->index(row, DXSPOT_TO_MEMORY_FLAG_COL_NUM), BOOL_YES, Qt::EditRole);
    }
}


void ClusterClientFrame::sendFreqToRig(QString freq)
{
    QString f = freq.remove('.').append(QString("000"));
    MinosLoggerEvents::SendFreqStrToRig(f);
}


void ClusterClientFrame::sendBrgToRot(QString brg)
{
    MinosLoggerEvents::SendSpotBrgStrToRot(brg);
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

void ClusterClientFrame::dxSpots(QVector<QString> _spotQueue)
{
    spotQueue = _spotQueue;
    if (!purgeSpotFlag)     // do nothing while purging spots
    {
        handleDxSpots(spotQueue);
    }


}


void ClusterClientFrame::handleDxSpots(QVector<QString> spotQueue)
{
    for ( QVector<QString>::iterator i = spotQueue.begin(); i != spotQueue.end(); i++ )
    {
       //ui->ChatMemo->append( (*i) );
       addDxSpotToTable((*i));
       trace("syncSpots " + (*i));
    }
    spotQueue.clear();
}


void ClusterClientFrame::addDxSpotToTable(QString spot)
{
    if (spot.contains(DXSPOT))
    {
        QStringList sl = spot.split(DXSPOT);
        if (sl.count() == 2)
        {
            QStringList spotlist = sl[1].split(':', QString::KeepEmptyParts);

            if (spotlist.count() == TTLVALUE +1)
            {
                // get time to live value
                if (spotlist[TTLVALUE] == "")
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

                // check to see if call or locator worked
                bool callWorked = false;
                bool locWorked = false;
                checkSpotWorked(spotlist[DXCALL], spotlist[DXLOCATOR], &callWorked, &locWorked);

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

                dxSpotDataModel->rowData = new SpotData(spotlist[SPOTTIME], spotlist[DXFREQ],
                                                        spotlist[DXBANDMASK], spotlist[DXMODEMASK],
                                                        spotlist[DXCALL], callWorked,
                                                        spotlist[DXLOCATOR], locWorked,
                                                        distance, bearing,
                                                        spotlist[SPOTCALL], spotlist[SPOTLOCATOR],
                                                        spotlist[SPOTCOMMENT]);
                dxSpotDataModel->insertRows(0, 1);

            }
        }
    }
}


void ClusterClientFrame::checkSpotWorked(QString &callsign, QString &locator, bool* callWorked, bool* locatorWorked)
{
    if (ct)
    {
        Callsign mcs(callsign);
        mcs.validate();
        for ( LogIterator i = ct->ctList.begin(); i != ct->ctList.end(); i++ )
        {
            if ((*i).wt->cs == mcs)
            {
                *callWorked = true;

            }

        }
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

    }
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

        int locValres = lonlat( locator, longitude, latitude );
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



void ClusterClientFrame::restoreDxSpotViewColumns()
{
    QSettings settings;
    QByteArray state;

    state = settings.value("dxSpotView/state").toByteArray();
    dxSpotView->horizontalHeader()->restoreState(state);
}


void ClusterClientFrame::restoreCallsignViewColumns()
{
    QSettings settings;
    QByteArray state;

    state = settings.value("dxSpotView/state").toByteArray();
    callSignView->horizontalHeader()->restoreState(state);
}


void ClusterClientFrame::restoreLocatorViewColumns()
{
    QSettings settings;
    QByteArray state;

    state = settings.value("dxSpotView/state").toByteArray();
    locatorView->horizontalHeader()->restoreState(state);
}

void ClusterClientFrame::setContest(BaseContestLog *c)
{
    ct = c;
}


void ClusterClientFrame::purgeSpots()
{
    if (timeToLive > 0)            // don't purge spots if == 0
    {
        if (dxSpotDataModel->rowCount() > 0)
        {
           purgeSpotFlag = true;
           while (spotTimedOut(dxSpotDataModel->data(dxSpotDataModel->index(dxSpotDataModel->rowCount() - 1, 0)).toString()))   // spot not timed out
           {
               dxSpotDataModel->removeRows(dxSpotDataModel->rowCount() - 1, 1, QModelIndex());
               if (dxSpotDataModel->rowCount() == 0)
               {
                   break;
               }
           }
           purgeSpotFlag = false;
        }
    }

}



bool ClusterClientFrame::spotTimedOut(QString spotTime)
{

    QRegExp re("\\d\\d\\d\\d");  // a digit (\d)
    if (re.exactMatch(spotTime))
    {
        //trace(QString("Spottime = %1").arg(spotTime));
        //trace(QString("CurrentTime = %1%2").arg(QDateTime::currentDateTimeUtc().time().hour()).arg(QDateTime::currentDateTimeUtc().time().minute()));
        QTime time = QTime(spotTime.mid(0, 2).toInt(), spotTime.mid(2, 4).toInt(), 0, 0);
        int timeDiff = time.secsTo(QDateTime::currentDateTimeUtc().time());
        if (timeDiff < 0)
        {
            timeDiff *= -1;
        }
        //trace(QString("Difference = %1").arg(timeDiff));
        if (timeDiff >= timeToLive)
        {
            return true;
        }
    }

    return false;

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
            QString freq = filterProxyModelList[curTab]->data(filterProxyModelList[curTab]->index(currentRow, FREQ_COL_NUM)).toString();
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
            QString brg = filterProxyModelList[curTab]->data(filterProxyModelList[curTab]->index(currentRow, DXBRG_COL_NUM)).toString();
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
            if (!filterProxyModelList[curTab]->data(filterProxyModelList[curTab]->index(currentRow, DXSPOT_TO_MEMORY_FLAG_COL_NUM)).toBool())
            {
                sendSpotToMemory(filterProxyModelList[curTab], currentRow);
            }

        }

    }
}



void ClusterClientFrame::sendSpotToMemory(DxSpotSortFilterProxyModel* spotProxyModel, int row)
{

    memoryData::memData spotData;
    spotData.callsign = spotProxyModel->data(spotProxyModel->index(row, DXSPOT_CALL_COL_NUM)).toString();
    spotData.time = spotProxyModel->data(spotProxyModel->index(row, TIME_COL_NUM)).toString();
    spotData.freq = spotProxyModel->data(spotProxyModel->index(row, FREQ_COL_NUM)).toString().remove('.').append(QString("000"));
    spotData.mode = memDefData::DEFAULT_MODE;
    spotData.locator = spotProxyModel->data(spotProxyModel->index(row, DXLOC_COL_NUM)).toString();

    MinosLoggerEvents::SendSpotToMemory(spotData);
    spotProxyModel->setData(spotProxyModel->index(row, DXSPOT_TO_MEMORY_FLAG_COL_NUM), BOOL_YES, Qt::EditRole);

}



void ClusterClientFrame::clearSpotActionSelected()
{
    int curTab = ui->dxSpotTab->currentIndex();

    if (filterProxyModelList[curTab]->rowCount() > 0)
    {
        int currentRow = spotViewList[curTab]->currentIndex().row();
        if (currentRow >= 0 && currentRow < filterProxyModelList[curTab]->rowCount())
        {
            int ret = QMessageBox::warning(this, tr("Cluster"),
                                           tr("Confirm you want to delete this spot?"),
                                           QMessageBox::Yes | QMessageBox::No);

            if (ret == QMessageBox::Yes)
            {
                purgeSpotFlag = true;
                filterProxyModelList[curTab]->removeRows(currentRow, 1, QModelIndex());
                purgeSpotFlag = false;
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
                                       tr("Confirm you want to delete all the spots?"),
                                       QMessageBox::Yes | QMessageBox::No);

        if (ret == QMessageBox::Yes)
        {
            purgeSpotFlag = true;
            filterProxyModelList[curTab]->removeRows(0, dxSpotDataModel->rowCount(), QModelIndex());
            purgeSpotFlag = false;
        }
    }
}

void ClusterClientFrame::onSearchEditingFinished()
{
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
}


void ClusterClientFrame::on_AfterLogContact( BaseContestLog *c)
{
      bool worked = false;
      if (c && ct == c)
      {
          int spotCount = dxSpotDataModel->rowCount();
          for (int spotNumber = 0; spotNumber < spotCount; spotNumber++)
          {
              QString callsign = dxSpotDataModel->data(dxSpotDataModel->index(spotNumber, DXSPOT_CALL_COL_NUM,  QModelIndex()), Qt::DisplayRole).toString();
              bool callsignWkd = dxSpotDataModel->data(dxSpotDataModel->index(spotNumber, DXSPOT_CALL_WORKED_COL_NUM,  QModelIndex()), Qt::DisplayRole).toBool();

              QString locator = dxSpotDataModel->data(dxSpotDataModel->index(spotNumber, DXLOC_COL_NUM,  QModelIndex()), Qt::DisplayRole).toString();
              bool locatorWkd = dxSpotDataModel->data(dxSpotDataModel->index(spotNumber, DXLOC_WORKED_COL_NUM,  QModelIndex()), Qt::DisplayRole).toBool();
              locator = locator.mid(0, 4);

              if ( callsign != "" && callsignWkd == BOOL_NO)
              {
                  Callsign mcs(callsign);
                  mcs.validate();

                  for ( LogIterator i = ct->ctList.begin(); i != ct->ctList.end(); i++ )
                  {
                      if ((*i).wt->cs == mcs)
                      {
                          dxSpotDataModel->setData(dxSpotDataModel->index(spotNumber, DXSPOT_CALL_WORKED_COL_NUM,  QModelIndex()), BOOL_YES, Qt::EditRole);
                          worked = true;

                      }
                  }
              }


              if (locator != "" && locatorWkd == BOOL_NO)
              {
                  for ( LogIterator i = ct->ctList.begin(); i != ct->ctList.end(); i++ )
                  {
                      if ((*i).wt->loc.loc.getValue().mid(0,4) == locator)
                      {
                          dxSpotDataModel->setData(dxSpotDataModel->index(spotNumber, DXLOC_WORKED_COL_NUM,  QModelIndex()), BOOL_YES, Qt::EditRole);
                          worked = true;

                      }
                  }
              }


          }
      }
      if (worked)
      {
          // refresh views
          dxSpotProxyModel->setFilterRegExp("");
          callSignProxyModel->setFilterRegExp("");
          locatorProxyModel->setFilterRegExp("");
          searchSortProxyModel->setFilterRegExp("");
      }

}


bool DxSpotSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
    return matchBand(sourceRow);
}

bool DxSpotSortFilterProxyModel::matchBand(int sourceRow) const
{
    bool ok = false;
    unsigned int spotMask = static_cast<unsigned int>(sourceModel()->data(sourceModel()->index(sourceRow, DXBANDMASK_COL_NUM)).toString().toInt(&ok));
    unsigned int filterMask = filterSetup->getBandFilterMask();
    if ( filterMask & spotMask || filterMask == 0)
    {
        return true;
    }

    return false;
}




bool SearchSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
    if (!searchParameter.isEmpty() && matchBand(sourceRow))
    {
        if (searchParameter.contains(SEARCH_LOC_EXP))
        {
            QString loc = sourceModel()->data(sourceModel()->index(sourceRow, DXLOC_COL_NUM)).toString().mid(0,4);
            if (loc.contains(searchParameter, Qt::CaseInsensitive))
            {
                return true;
            }
        }
        else
        {
            Callsign spotCall(sourceModel()->data(sourceModel()->index(sourceRow, DXSPOT_CALL_COL_NUM)).toString());
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
        if (matchBand(sourceRow))
        {
            Callsign spotCall(sourceModel()->data(sourceModel()->index(sourceRow, DXSPOT_CALL_COL_NUM)).toString());
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
        if (matchBand(sourceRow))
        {
            QModelIndex index = sourceModel()->index(sourceRow, DXLOC_COL_NUM);
            QString locator = sourceModel()->data(index).toString().mid(0,4);
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

