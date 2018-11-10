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
    //dxSpotView->setSelectionBehavior( QAbstractItemView::SelectRows );
    dxSpotView->setSelectionMode( QAbstractItemView::SingleSelection );
    dxSpotView->setSelectionBehavior(QAbstractItemView::SelectItems);
    //dxSpotView->setSelectionMode( QAbstractItemView::NoSelection );

    QHeaderView *spotVerticalHeader = dxSpotView->verticalHeader();
    spotVerticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    spotVerticalHeader->setDefaultSectionSize(18);


    //connect( dxSpotView->horizontalHeader(), SIGNAL(sectionResized(int, int , int)), this, SLOT( on_sectionResized(int, int , int)));
    connect(dxSpotView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onDxSpotViewClicked(const QModelIndex &)));

    dxSpotView->setColumnHidden(DXBANDMASK_COL_NUM, true);
    dxSpotView->setColumnHidden(MODEMASK_COL_NUM, true);
    dxSpotView->setColumnWidth(TIME_COL_NUM, TIME_COL_WIDTH);
    dxSpotView->setColumnWidth(FREQ_COL_NUM, FREQ_COL_WIDTH);
    dxSpotView->setColumnWidth(DXSPOT_CALL_COL_NUM, DXSPOT_CALL_COL_WIDTH);
    dxSpotView->setColumnWidth(DXSPOT_CALL_WORKED_COL_NUM, DXSPOT_CALL_WKD_COL_WIDTH);
    dxSpotView->setColumnWidth(DXLOC_COL_NUM, DXLOC_COL_WIDTH);
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

    searchView->setColumnHidden(DXBANDMASK_COL_NUM, true);
    searchView->setColumnHidden(MODEMASK_COL_NUM, true);
    searchView->setColumnWidth(TIME_COL_NUM, TIME_COL_WIDTH);
    searchView->setColumnWidth(FREQ_COL_NUM, FREQ_COL_WIDTH);
    searchView->setColumnWidth(DXSPOT_CALL_COL_NUM, DXSPOT_CALL_COL_WIDTH);
    searchView->setColumnWidth(DXSPOT_CALL_WORKED_COL_NUM, DXSPOT_CALL_WKD_COL_WIDTH);
    searchView->setColumnWidth(DXLOC_COL_NUM, DXLOC_COL_WIDTH);
    searchView->setColumnWidth(DXLOC_WORKED_COL_NUM, DXLOC_WKD_COL_WIDTH);
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

    callSignView->setColumnHidden(DXBANDMASK_COL_NUM, true);
    callSignView->setColumnHidden(MODEMASK_COL_NUM, true);
    callSignView->setColumnWidth(TIME_COL_NUM, TIME_COL_WIDTH);
    callSignView->setColumnWidth(FREQ_COL_NUM, FREQ_COL_WIDTH);
    callSignView->setColumnWidth(DXSPOT_CALL_COL_NUM, DXSPOT_CALL_COL_WIDTH);
    callSignView->setColumnWidth(DXSPOT_CALL_WORKED_COL_NUM, DXSPOT_CALL_WKD_COL_WIDTH);
    callSignView->setColumnWidth(DXLOC_COL_NUM, DXLOC_COL_WIDTH);
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

    locatorView->setColumnHidden(DXBANDMASK_COL_NUM, true);
    locatorView->setColumnHidden(MODEMASK_COL_NUM, true);
    locatorView->setColumnWidth(TIME_COL_NUM, TIME_COL_WIDTH);
    locatorView->setColumnWidth(FREQ_COL_NUM, FREQ_COL_WIDTH);
    locatorView->setColumnWidth(DXSPOT_CALL_COL_NUM, DXSPOT_CALL_COL_WIDTH);
    locatorView->setColumnWidth(DXSPOT_CALL_WORKED_COL_NUM, DXSPOT_CALL_WKD_COL_WIDTH);
    locatorView->setColumnWidth(DXLOC_COL_NUM, DXLOC_COL_WIDTH);
    locatorView->setColumnWidth(DXLOC_WORKED_COL_NUM, DXLOC_WKD_COL_WIDTH);
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
        memoryData::memData spotData;
        spotData.callsign = dxSpotProxyModel->data(index).toString();
        spotData.freq = dxSpotProxyModel->data(dxSpotProxyModel->index(index.row(), FREQ_COL_NUM)).toString().remove('.').append(QString("000"));
        spotData.mode = memDefData::DEFAULT_MODE;
        spotData.locator = dxSpotProxyModel->data(dxSpotProxyModel->index(index.row(), DXLOC_COL_NUM)).toString();
        MinosLoggerEvents::SendSpotToLog(spotData);
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
        memoryData::memData spotData;
        spotData.callsign = searchSortProxyModel->data(index).toString();
        spotData.freq = searchSortProxyModel->data(searchSortProxyModel->index(index.row(), FREQ_COL_NUM)).toString().remove('.').append(QString("000"));
        spotData.mode = memDefData::DEFAULT_MODE;
        spotData.locator = searchSortProxyModel->data(searchSortProxyModel->index(index.row(), DXLOC_COL_NUM)).toString();
        MinosLoggerEvents::SendSpotToLog(spotData);
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
        memoryData::memData spotData;
        spotData.callsign = callSignProxyModel->data(index).toString();
        spotData.freq = callSignProxyModel->data(callSignProxyModel->index(index.row(), FREQ_COL_NUM)).toString().remove('.').append(QString("000"));
        spotData.mode = memDefData::DEFAULT_MODE;
        spotData.locator = callSignProxyModel->data(callSignProxyModel->index(index.row(), DXLOC_COL_NUM)).toString();
        MinosLoggerEvents::SendSpotToLog(spotData);
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
        memoryData::memData spotData;
        spotData.callsign = locatorProxyModel->data(index).toString();
        spotData.freq = locatorProxyModel->data(locatorProxyModel->index(index.row(), FREQ_COL_NUM)).toString().remove('.').append(QString("000"));
        spotData.mode = memDefData::DEFAULT_MODE;
        spotData.locator = locatorProxyModel->data(locatorProxyModel->index(index.row(), DXLOC_COL_NUM)).toString();
        MinosLoggerEvents::SendSpotToLog(spotData);
    }
}




void ClusterClientFrame::sendFreqToRig(QString freq)
{
    QString f = freq.remove('.').append(QString("000"));
    MinosLoggerEvents::SendFreqStrToRig(f);
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

                //dxSpotDataModel->rowData = QStringList {spotTime, displayFreq, dxCall, dxLocator, spotCall, spotLocator, spotComment };
                dxSpotDataModel->rowData = new SpotData(spotlist[SPOTTIME], spotlist[DXFREQ], spotlist[DXBANDMASK], spotlist[DXMODEMASK], spotlist[DXCALL], spotlist[DXLOCATOR], spotlist[SPOTCALL], spotlist[SPOTLOCATOR], spotlist[SPOTCOMMENT]);
                dxSpotDataModel->insertRows(0, 1);

            }
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

}




void ClusterClientFrame::bearingActionSelected()
{

}



void ClusterClientFrame::logActionSelected()
{

}


void ClusterClientFrame::memoryActionSelected()
{

}

void ClusterClientFrame::clearSpotActionSelected()
{


}



void ClusterClientFrame::clearAllSpotsActionSelected()
{


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
        ui->searchLineEdit->selectAll();
        searchSortProxyModel->setFilterRegExp("");
    }
}


void ClusterClientFrame::on_AfterLogContact( BaseContestLog *c)
{
      if (c && ct == c)
      {
          int spotCount = dxSpotDataModel->rowCount();
          for (int spotNumber = 0; spotNumber < spotCount; spotNumber++)
          {
              QString callsign = dxSpotDataModel->data(dxSpotDataModel->index(spotNumber, DXSPOT_CALL_COL_NUM,  QModelIndex()), Qt::DisplayRole).toString();
              QString callsignWkd = dxSpotDataModel->data(dxSpotDataModel->index(spotNumber, DXSPOT_CALL_WORKED_COL_NUM,  QModelIndex()), Qt::DisplayRole).toString();

              QString locator = dxSpotDataModel->data(dxSpotDataModel->index(spotNumber, DXLOC_COL_NUM,  QModelIndex()), Qt::DisplayRole).toString();
              QString locatorWkd = dxSpotDataModel->data(dxSpotDataModel->index(spotNumber, DXLOC_WORKED_COL_NUM,  QModelIndex()), Qt::DisplayRole).toString();
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

                      }
                  }
              }


          }
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


