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
#include "clustercommon.h"
#include "contest.h"
#include "cutils.h"
#include "rigmemcommondata.h"
#include "htmldelegate.h"
#include "ui_clusterclientframe.h"


ClusterClientFrame::ClusterClientFrame(QWidget *parent, int instanceNum):
    QFrame(parent),
    ui(new Ui::ClusterClientFrame),
    purgeTimer(nullptr),
    timeToLive(0),
    purgeSpotFlag(false),
    instanceNum(instanceNum)
{

    ui->setupUi(this);

    int lcf;
    MinosParameters::getMinosParameters() ->getIntDisplayProfile(edpListCompression, lcf);
    delegate = new HtmlDelegate(1.0, lcf/100.0) ;


    filterSetup = new ClusterClientFilterDialog(this, instanceNum);

    purgeTimer = new QTimer(this);

    qDebug() << "new cluster frame, instance num = " << instanceNum;

    checkNewSpotsTimer = new QTimer(this);
    connect (checkNewSpotsTimer, SIGNAL(timeout()), this, SLOT(checkNewSpots()));
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

    dxSpotDataModel = new DxSpotDataModel();
    dxSpotDataModel->delegate = delegate;

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

    connect(filterSetup, SIGNAL(filtersChanged(bool, bool, bool, bool)), this, SLOT(filtersChanged(bool, bool, bool, bool)));

    purgeTimer->start(PURGE_TIME);

    newCallsignSpotIndToggle(false);
    newLocatorSpotIndToggle(false);
    checkNewSpotsTimer->start(1000);




}


ClusterClientFrame::~ClusterClientFrame()
{
    delete ui;
}



void ClusterClientFrame::setupDXSpotView()
{
    dxSpotView = new QTableView();

    dxSpotProxyModel = new DxSpotSortFilterProxyModel(filterSetup);
    dxSpotProxyModel->setSourceModel(dxSpotDataModel);

    ui->dxSpotTab->addTab(dxSpotView, "DX Spots");
    //dxSpotView = ui->dxSpotView;
    dxSpotView->setModel(dxSpotProxyModel);
    dxSpotView->setAlternatingRowColors(true);
    dxSpotView->setSelectionBehavior( QAbstractItemView::SelectRows );
    dxSpotView->setSelectionMode( QAbstractItemView::SingleSelection );
    //dxSpotView->setSelectionBehavior(QAbstractItemView::SelectItems);
    dxSpotView->verticalHeader()->setDefaultSectionSize(10);
    dxSpotView->verticalHeader()->setMinimumSectionSize(10);


    dxSpotView->setItemDelegate( delegate);
    dxSpotView->resizeRowsToContents();

    QHeaderView *spotVerticalHeader = dxSpotView->verticalHeader();

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
    searchView = new QTableView();

    searchSortProxyModel = new SearchSortFilterProxyModel(filterSetup);
    searchSortProxyModel->setSourceModel(dxSpotDataModel);

    ui->dxSpotTab->addTab(searchView, "Search Spots");


    searchView->setModel(searchSortProxyModel);
    searchView->setAlternatingRowColors(true);
    searchView->setSelectionMode( QAbstractItemView::SingleSelection );
    searchView->setSelectionBehavior(QAbstractItemView::SelectItems);
    //dxSpotView->setSelectionMode( QAbstractItemView::NoSelection );

    searchView->setItemDelegate( delegate);
    searchView->resizeRowsToContents();

    searchView->verticalHeader()->setDefaultSectionSize(10);
    searchView->verticalHeader()->setMinimumSectionSize(10);

    QHeaderView *searchVerticalHeader = searchView->verticalHeader();
    //connect( callSignView->horizontalHeader(), SIGNAL(sectionResized(int, int , int)), this, SLOT( on_sectionResized(int, int , int)));
    connect(searchView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onSearchSpotViewClicked(const QModelIndex &)));
    connect(searchVerticalHeader, SIGNAL(sectionClicked(int)), this, SLOT(onSearchSpotVertHeaderClicked(int)));

    searchView->setColumnHidden(DXBANDMASK_COL_NUM, true);
    searchView->setColumnHidden(MODEMASK_COL_NUM, true);
    searchView->setColumnHidden(DXSPOT_CALL_WORKED_COL_NUM, true);
    searchView->setColumnHidden(DXLOC_WORKED_COL_NUM, true);
    searchView->setColumnHidden(DXSPOT_TO_MEMORY_FLAG_COL_NUM, true);

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
    callSignView = new QTableView();

    callSignProxyModel = new CallsignSortFilterProxyModel(filterSetup);
    callSignProxyModel->setSourceModel(dxSpotDataModel);

    ui->dxSpotTab->addTab(callSignView, "Callsign Spots");


    callSignView->setModel(callSignProxyModel);
    callSignView->setAlternatingRowColors(true);
    callSignView->setSelectionMode( QAbstractItemView::SingleSelection );
    callSignView->setSelectionBehavior(QAbstractItemView::SelectItems);
    //dxSpotView->setSelectionMode( QAbstractItemView::NoSelection );

    callSignView->setItemDelegate( delegate);
    callSignView->resizeRowsToContents();

    callSignView->verticalHeader()->setDefaultSectionSize(10);
    callSignView->verticalHeader()->setMinimumSectionSize(10);


    QHeaderView *callSignVerticalHeader = callSignView->verticalHeader();
    //connect( callSignView->horizontalHeader(), SIGNAL(sectionResized(int, int , int)), this, SLOT( on_sectionResized(int, int , int)));
    connect(callSignView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onCallsignSpotViewClicked(const QModelIndex &)));
    connect(callSignVerticalHeader, SIGNAL(sectionClicked(int)), this, SLOT(onCallsignSpotVertHeaderClicked(int)));

    callSignView->setColumnHidden(DXBANDMASK_COL_NUM, true);
    callSignView->setColumnHidden(MODEMASK_COL_NUM, true);
    callSignView->setColumnHidden(DXSPOT_CALL_WORKED_COL_NUM, true);
    callSignView->setColumnHidden(DXLOC_WORKED_COL_NUM, true);
    callSignView->setColumnHidden(DXSPOT_TO_MEMORY_FLAG_COL_NUM, true);

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
    locatorView = new QTableView();
    locatorView->setItemDelegate(delegate);
    locatorProxyModel = new LocatorSortFilterProxyModel(filterSetup);
    locatorProxyModel->setSourceModel(dxSpotDataModel);

    ui->dxSpotTab->addTab(locatorView, "Locator Spots");

    locatorView->setModel(locatorProxyModel);
    locatorView->setAlternatingRowColors(true);
    locatorView->setSelectionMode( QAbstractItemView::SingleSelection );
    locatorView->setSelectionBehavior(QAbstractItemView::SelectItems);
    //dxSpotView->setSelectionMode( QAbstractItemView::NoSelection );

    locatorView->setItemDelegate( delegate);
    locatorView->resizeRowsToContents();

    locatorView->setItemDelegate( delegate);
    locatorView->resizeRowsToContents();


    QHeaderView *locatorViewVerticalHeader = locatorView->verticalHeader();
    locatorView->verticalHeader()->setDefaultSectionSize(10);
    locatorView->verticalHeader()->setMinimumSectionSize(10);


    //connect( locatorView->horizontalHeader(), SIGNAL(sectionResized(int, int , int)), this, SLOT( on_sectionResized(int, int , int)));
    connect(locatorView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onLocatorSpotViewClicked(const QModelIndex &)));
    connect(locatorViewVerticalHeader, SIGNAL(sectionClicked(int)), this, SLOT(onLocatorSpotVertHeaderClicked(int)));


    locatorView->setColumnHidden(DXBANDMASK_COL_NUM, true);
    locatorView->setColumnHidden(MODEMASK_COL_NUM, true);
    locatorView->setColumnHidden(DXSPOT_CALL_WORKED_COL_NUM, true);
    locatorView->setColumnHidden(DXLOC_WORKED_COL_NUM, true);
    locatorView->setColumnHidden(DXSPOT_TO_MEMORY_FLAG_COL_NUM, true);

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
    filterSetup->copyBandFiltersToEdit();
    filterSetup->copyModeFiltersToEdit();
    filterSetup->copyCallsignFilterListToListWidget();
    filterSetup->copyLocatorFilterListToListWidget();
    filterSetup-> setTabCurrentIndex(filterSetup->getTabCurrentIndex());
    filterSetup->show();

}

void ClusterClientFrame::filtersChanged(bool bandfilterChanged, bool modefilterChanged,  bool callsignfilterChanged, bool locatorfilterChanged)
{
    //update views..
    if (bandfilterChanged)
    {
        dxSpotProxyModel->setFilterRegExp("");
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



void ClusterClientFrame::handleClickedItems(DxSpotSortFilterProxyModel* spotProxyModel, const QModelIndex &index)
{
    if (index.column() == FREQ_COL_NUM)
    {
        QString freq = spotProxyModel->data(index).toString();
        sendFreqToRig(freq);
    }
    else if (index.column() == DXSPOT_CALL_COL_NUM )
    {
        // transfer spot details to qsolog

        MinosLoggerEvents::SendSpotToLog(getSpotDataToMemoryVariable(spotProxyModel, index.row()));
    }
    else if (index.column() == DXBRG_COL_NUM)
    {
        QString brg = spotProxyModel->data(index).toString();
        sendBrgToRot(brg);
    }
}

void ClusterClientFrame::handleVertHeaderClickedItems(DxSpotSortFilterProxyModel* spotProxyModel, int row)
{
    // check if spot has been sent to memory
    if (!spotProxyModel->data(spotProxyModel->index(row, DXSPOT_TO_MEMORY_FLAG_COL_NUM)).toBool())
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
    dxSpotView->resizeRowsToContents();
    searchView->resizeRowsToContents();
    callSignView->resizeRowsToContents();
    locatorView->resizeRowsToContents();
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

    // set the contest in the filter dialog
    filterSetup->setContest(c);
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

    memoryData::memData spotData = getSpotDataToMemoryVariable(spotProxyModel, row);

    MinosLoggerEvents::SendSpotToMemory(spotData);
    spotProxyModel->setData(spotProxyModel->index(row, DXSPOT_TO_MEMORY_FLAG_COL_NUM), BOOL_YES, Qt::EditRole);

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

void ClusterClientFrame::clearSpotActionSelected()
{
    int curTab = ui->dxSpotTab->currentIndex();

    if (filterProxyModelList[curTab]->rowCount() > 0)
    {
        int currentRow = spotViewList[curTab]->currentIndex().row();
        if (currentRow >= 0 && currentRow < filterProxyModelList[curTab]->rowCount())
        {
            int ret = QMessageBox::warning(this, tr("Cluster"),
                                           tr("Please confirm you want to delete this spot?"),
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
                                       tr("Please confirm you want to delete all the spots?"),
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


void ClusterClientFrame::checkNewSpots()
{
    // look for new spots in callsign and locator view
    static int oldCallsignCount = 0;
    static int oldLocatorCount = 0;

    int newCallsignCount =  callSignProxyModel->rowCount();
    int newLocatorCount = locatorProxyModel->rowCount();

    if (newCallsignCount == 0 || ui->dxSpotTab->currentIndex() == CALLSIGN_TAB)
    {
       newCallsignSpotIndToggle(false);
    }

    if (newLocatorCount == 0 || ui->dxSpotTab->currentIndex() == LOCATOR_TAB)
    {
       newLocatorSpotIndToggle(false);
    }


    if (newCallsignCount > oldCallsignCount)
    {
        oldCallsignCount = newCallsignCount;
        if (ui->dxSpotTab->currentIndex() != CALLSIGN_TAB)
        {
            newCallsignSpotIndToggle(true);
        }

    }

    if (newLocatorCount > oldLocatorCount)
    {
        oldLocatorCount = newLocatorCount;
        if (ui->dxSpotTab->currentIndex() != LOCATOR_TAB)
        {
            newLocatorSpotIndToggle(true);
        }

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


bool DxSpotSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
    return matchBand(sourceRow);
}

bool DxSpotSortFilterProxyModel::matchBand(int sourceRow) const
{
    bool ok = false;
    int spotMask = sourceModel()->data(sourceModel()->index(sourceRow, DXBANDMASK_COL_NUM)).toString().toInt(&ok);
    if (spotMask < NUMBANDS)
    {
       return filterSetup->filterSettings.getBandFilter(spotMask);
    }
    else
    {
        return false;
    }

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

