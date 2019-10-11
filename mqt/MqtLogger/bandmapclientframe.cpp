////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Bandmap Client
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
///
//
//
/////////////////////////////////////////////////////////////////////////////


#include "MinosRPC.h"
#include "cutils.h"
#include "MinosLoggerEvents.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "checkoperatingfreq.h"
#include <QDebug>
#include "ui_bandmapclientframe.h"

#include "bandmapclientframe.h"

// track how many spots on the same freq
class multFreq
{

public:
    multFreq():
        time(0),
        row(0)
    {

    }



    void setTime(qint64 t)
    {
        time = t;
    }

    qint64 getTime()
    {
        return time;
    }

    void setRow(int r)
    {
        row = r;
    }

    int getRow()
    {
        return row;
    }
private:

    qint64 time;
    int row;

};

const int MAX_CALL_SAME_FREQ = 3;

BandmapClientFrame::BandmapClientFrame(QWidget *parent):
    QFrame(parent),
    ui(new Ui::BandmapClientFrame),
    isProtected(false),
    contestBand(-1),
    contestBandFlow(0),
    contestBandFHigh(0),
    contestMode(-1),
    clusterServerLoaded(false),
    clusterServerConnected(false),
    purgeSpotFlag(false),
    holdUpdateFlag(false),
    timeToLive(0),
    rotatorConnected(false)

{

    ui->setupUi(this);

    ui->bandmapFrameTitle->setText("Bandmap");

    int height = ui->bandmapGraphicsView->height();
    int width = ui->bandmapGraphicsView->width();
    qDebug() << "ui frame height = " << height << " ui frame width = " << width;

    connect (ClusterClientServer::getClusterClientServer(), SIGNAL(ClusterServerList(QVector<ClusterServer>)), this, SLOT(clusterClientServerList(QVector<ClusterServer>)));
    connect (ClusterClientServer::getClusterClientServer(), SIGNAL(dxSpot(QVector<QString>)), this, SLOT(dxSpots(QVector<QString>)));



    filterSetup = new BandmapClientFilterDialog(this);


    bandmapDataModel = new BandmapDataModel();

    bandmapView = new BandmapView();
    bandmapView->setFilter(filterSetup);

    bandmapSpotProxyModel = new QSortFilterProxyModel(parent);
    bandmapSpotProxyModel->setSourceModel(bandmapDataModel);
    bandmapSpotProxyModel->sort(FREQ_STR_COL_NUM, Qt::AscendingOrder);

    bandmapView->setModel(bandmapSpotProxyModel);


    bandmapView->initBandmapView(ui->bandmapGraphicsView);


    checkNewSpotsTimer = new QTimer(this);
    connect (checkNewSpotsTimer, SIGNAL(timeout()), this, SLOT(checkNewBandMapSpots()));
    checkNewSpotsTimer->start(CHECKSPOTS_DURATION);

    connect(&MinosLoggerEvents::mle, SIGNAL(FontChanged()), this, SLOT(on_FontChanged()), Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, SIGNAL(AfterLogContactToBandmap(BaseContestLog *, Callsign, QString, QString, QString)), this, SLOT(on_AfterLogContact(BaseContestLog *, Callsign, QString, QString, QString)));

    connect( bandmapView, SIGNAL( contextMenuSelected( const QPoint& ) ), this, SLOT( on_contextMenuSelected( const QPoint& ) ) );
    connect (ui->filtersPushBut, SIGNAL(clicked()), this, SLOT(filterButtonSelected()));

    checkNewFilters = new QTimer(this);
    connect (checkNewFilters, SIGNAL(timeout()), this, SLOT(checkSavedFilters()));

    purgeTimer = new QTimer(this);
    connect (purgeTimer, SIGNAL(timeout()), this, SLOT(purgeSpots()));

    spotsMenu = new QMenu(ui->actionsButton);

    ui->actionsButton->setFocusPolicy(Qt::NoFocus);
    actionInObject = new BMP_MouseInObject(this, this);
    spotsMenu->installEventFilter(actionInObject);


    markSpotAction = new QAction("M&ark Spot", this);
    unMarkSpotAction = new QAction("&Unmark Spot", this);
    freqAction = new QAction("Set &Freq", this);
    bearingAction = new QAction("Set &Bearing", this);
    logAction = new QAction("Send &Log", this);
    memoryAction = new QAction("Send &Memory", this);
    clearSpotAction = new QAction("Clear &Spot", this);

    spotsMenu->addAction(markSpotAction);
    spotsMenu->addAction(unMarkSpotAction);
    spotsMenu->addAction(freqAction);
    spotsMenu->addAction(bearingAction);
    spotsMenu->addAction(logAction);
    spotsMenu->addAction(memoryAction);
    spotsMenu->addAction(clearSpotAction);

    ui->actionsButton->setMenu(spotsMenu);
    connect(spotsMenu, SIGNAL(aboutToShow()), this, SLOT(onMenuShow()));

    connect( markSpotAction, SIGNAL( triggered() ), this, SLOT(on_markSpotActionSelected()) );
    connect( unMarkSpotAction, SIGNAL( triggered() ), this, SLOT(on_unMarkSpotActionSelected()) );
    connect( freqAction, SIGNAL( triggered() ), this, SLOT(on_freqActionSelected()) );
    connect( bearingAction, SIGNAL( triggered() ), this, SLOT(bearingActionSelected()) );
    connect( logAction, SIGNAL( triggered() ), this, SLOT(logActionSelected()) );
    connect( memoryAction, SIGNAL( triggered() ), this, SLOT(memoryActionSelected()) );
    connect( clearSpotAction, SIGNAL( triggered() ), this, SLOT(clearSpotActionSelected()) );

    connect(filterSetup, SIGNAL(filtersChanged(bool)), this, SLOT(on_FitersChanged(bool)));

    connect(this, SIGNAL(freqDisplayClicked()), this, SLOT(on_FreqDisplayClicked()));


    this->setMouseTracking(true);
    mouseInFrameTimer = new QTimer(this);
    connect (mouseInFrameTimer, SIGNAL(timeout()), this, SLOT(mouseTimerCheckNewSpots()));

    loadVhfAndUpBands(bands);

    modeBandPlan = new checkModeAgainstFreq();
    if (modeBandPlan->loadFile(MODE_BANDPLAN_FILE))
    {
        trace(QString("Bandmap Client: Mode frequency bandplan loaded OK"));
        modeBandPlanOk = true;

    }
    else
    {
        trace(QString("Bandmap Client: Mode frequency bandplan loaded failed to Load"));
        modeBandPlanOk = false;

    }

    operatingFreq = new CheckOperatingFreq();
    if (operatingFreq->loadFile(OPERATING_FREQ_FILE))
    {
        trace(QString("Bandmap Client: Operating frequency bandplan loaded OK"));
        operatingFreqPlanOk = true;
    }
    else
    {
        trace(QString("Bandmap Client: Operating frequency bandplan failed to load"));
        operatingFreqPlanOk = false;
    }

    ui->freqDisplay->installEventFilter(this);
    freqDisplayPalette = new QPalette();       // to change colour when tuning


    purgeTimer->start(PURGE_TIME);
    checkNewFilters->start(CHECK_NEWFILTERS_DURATION);

    //QShortcut *shortcut = new QShortcut(QKeySequence("Ctrl+a"), parent);
    //QObject::connect(shortcut, SIGNAL(activated()), this, SLOT(onMenuShow()));
}


BandmapClientFrame::~BandmapClientFrame()
{
    delete ui;

    delete bandmapDataModel;
    delete actionInObject;
    delete freqDisplayPalette;
    bandmapView->deleteLater();

}

void BandmapClientFrame::on_FontChanged()
{
    QFont cf = QApplication::font();
    bandmapView->onFontChanged(cf);
}



void BandmapClientFrame::on_contextMenuSelected(const QPoint& pos)
{
    int selectedSpotViewRowNum = bandmapView->isClickInRegionOfSpot(pos);
    if (selectedSpotViewRowNum != -1)
    {
        bandmapView->getSpotData(selectedSpotDataRowNum, selectedSpotViewRowNum, selectedSpotData);
        QPoint globalPos = ui->bandmapGraphicsView->mapToGlobal( pos );
        spotsMenu->popup(globalPos);

    }


}



void BandmapClientFrame::onMenuShow()
{
   //ui->actionsButton->showMenu();
}


void BandmapClientFrame::on_FitersChanged(bool state)
{
    if (state)
    {
        bandmapView->bandmapUpdate();
    }
}



void BandmapClientFrame::on_markSpotActionSelected()
{
    bandmapSpotType::SPOT_TYPE spotType = static_cast<bandmapSpotType::SPOT_TYPE>(bandmapSpotProxyModel->data(bandmapSpotProxyModel->index(selectedSpotDataRowNum, SPOT_TYPE_COL_NUM), BMP_DataStoredRole).toInt());
    if (spotType == bandmapSpotType::CLUSTER)
    {
        bandmapSpotProxyModel->setData(bandmapSpotProxyModel->index(selectedSpotDataRowNum, SPOT_TYPE_COL_NUM), bandmapSpotType::CLUSTER_MARKED, BMP_DataStoredRole);
        bandmapView->bandmapUpdate();
    }
}

void BandmapClientFrame::on_unMarkSpotActionSelected()
{
    bandmapSpotType::SPOT_TYPE spotType = static_cast<bandmapSpotType::SPOT_TYPE>(bandmapSpotProxyModel->data(bandmapSpotProxyModel->index(selectedSpotDataRowNum, SPOT_TYPE_COL_NUM), BMP_DataStoredRole).toInt());
    if (spotType == bandmapSpotType::CLUSTER_MARKED)
    {
        bandmapSpotProxyModel->setData(bandmapSpotProxyModel->index(selectedSpotDataRowNum, SPOT_TYPE_COL_NUM), bandmapSpotType::CLUSTER, BMP_DataStoredRole);
        bandmapView->bandmapUpdate();
    }
}

void BandmapClientFrame::on_FreqDisplayClicked()
{
    bandmapView->makeCursorVisibleInBandmap();
}

void BandmapClientFrame::on_freqActionSelected()
{
    QString freq = selectedSpotData.dxFreqStr;
    sendFreqToRig(freq);
}

void BandmapClientFrame::sendFreqToRig(QString freq)
{
    QString f = freq.remove('.');
    MinosLoggerEvents::SendFreqStrToRig(f);
}

void BandmapClientFrame::bearingActionSelected()
{
    QString brg = selectedSpotData.dxBrg;
    QString loc = selectedSpotData.dxLocator;
    if (loc.count() < 6)
    {
        brg = brg.append(SHORTLOCATOR_IDENTIFIER);

    }
    sendBrgToRot(brg);
}


void BandmapClientFrame::sendBrgToRot(QString brg)
{
    if (!brg.isEmpty())
    {
       MinosLoggerEvents::SendSpotBrgStrToRot(brg);
    }

}


void BandmapClientFrame::logActionSelected()
{
    memoryData::memData spotData;
    spotData.callsign = selectedSpotData.dxCall;
    spotData.time = selectedSpotData.spotTime;
    spotData.freq = selectedSpotData.dxFreqStr;
    spotData.locator = selectedSpotData.dxLocator;
    spotData.bearing = selectedSpotData.dxBrg.toInt();
    spotData.fromBandmap = true;

    MinosLoggerEvents::SendSpotToLog(spotData);
}


void BandmapClientFrame::memoryActionSelected()
{
    memoryData::memData spotData;
    spotData.callsign = selectedSpotData.dxCall;
    spotData.time = selectedSpotData.spotTime;
    spotData.freq = selectedSpotData.dxFreqStr;
    spotData.locator = selectedSpotData.dxLocator;
    spotData.bearing = selectedSpotData.dxBrg.toInt();

    MinosLoggerEvents::SendSpotToMemory(spotData);
}

void BandmapClientFrame::clearSpotActionSelected()
{

    int ret = QMessageBox::warning(this, tr("Bandmap"),
                                   QString("Please confirm you want to delete this spot - %1?").arg(selectedSpotData.dxCall),
                                   QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes)
    {
        bandmapSpotProxyModel->removeRows(selectedSpotDataRowNum, 1);
        bandmapView->bandmapUpdate();
    }
}

void BandmapClientFrame::setContest(BaseContestLog *c)
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


        QString bandplanLimits = readBandmapFreqLimit(contestBandStr, contestModeStr);

        if (bandplanLimits.isEmpty())
        {
            getBandLimitsFromBandListXML();


        }
        else
        {
            // use user bandplan limits ini file
            QStringList bpl = bandplanLimits.split(',');
            if (bpl.count() == 2)
            {
                bool okL;
                bool okH;
                double flow = bpl[0].trimmed().remove('.').toDouble(&okL);
                double fhigh = bpl[1].trimmed().remove('.').toDouble(&okH);
                if (okL && okH)
                {
                    contestBandFlow = flow * 1000;
                    contestBandFHigh = fhigh * 1000;
                    bandmapView->setBandFreqLimits(contestBandFlow, contestBandFHigh);
                    bandmapView->setBandmapHeight(contestBandFlow, contestBandFHigh);
                }
                else
                {
                    getBandLimitsFromBandListXML();
                }

            }
            else
            {
                getBandLimitsFromBandListXML();
            }
        }





        if (!contest->bandmapFilterSettingsExist)       // have settings been saved before?
        {
            if (contestModeStr == "MGM")       //  have mode settings been saved before?
            {
                for (int m = 4; m < clusterModes.count(); m++)
                {
                    filterSetup->setModeFilter(true, m);  // set all the mgm modes in filter
                }
            }
            else
            {
                // no, save current mode filter for this contest
                filterSetup->setModeFilter(true, contestMode);
            }

            filterSetup->saveBandmapFilterToContest();
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

void BandmapClientFrame::getBandLimitsFromBandListXML()
{

    // use band list file
    BandList blist = BandList::getBandList();
    BandInfo bi;


    for (int i = 0; i < blist.bandList.count(); i++)
    {
        bi = blist.bandList[i];
        if (bi.uk == contestBandStr)
        {

            contestBandFlow = bi.flow;
            contestBandFHigh = bi.fhigh;
            bandmapView->setBandFreqLimits(contestBandFlow, contestBandFHigh);
            bandmapView->setBandmapHeight(contestBandFlow, contestBandFHigh);
            break;
        }
    }



}


int BandmapClientFrame::getBandOffSet(QString contestBandStr)
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

int BandmapClientFrame::getModeOffSet(QString contestModeStr)
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


// returns true if freq ok, false if it is not...it will return true and error to tracelog is mode or band
// is missing from operating freq file
bool BandmapClientFrame::isFreqLegal(const double freq, const QString band, const QString mode)
{

    int retCode;
    if (operatingFreqPlanOk)
    {
            retCode =  operatingFreq->freqValid(band, mode, freq);
            switch (retCode)
            {
                case FREQ_OK:
                    return true;
                case FREQ_NO_MATCH:
                    return false;
                case MODE_MISSING:
                    trace(QString("Bandmap isFreqLegal: mode is missing from file - band %1, mode %2").arg(band).arg(mode));
                    return true;
                case BAND_MISSING:
                    trace(QString("Bandmap isFreqLegal: band is missing from file - band %1, mode %2").arg(band).arg(mode));
                    return true;
            }
    }

    trace(QString("Bandmap isFreqLegal: Operating Freq file not loaded"));
    return true;

}


//---------------------- Cluster Spots -------------------------------------

void BandmapClientFrame::clusterClientServerList(QVector<ClusterServer> serverList)
{
    //ui->StationList->clear();
    for ( QVector<ClusterServer>::iterator i = serverList.begin(); i != serverList.end(); i++ )
    {
        QString state = clusterStateIndicator[(*i).state] + " " + (*i).app + "\r\n";
        trace(QString("bandmapClientServerList - state = %1").arg(state));
        //ui->StationList->addItem( state );
    }
}

void BandmapClientFrame::dxSpots(QVector<QString> spotMsg)
{
    // if contest is protected ignore
    if (!isProtected)
    {
        //get spot Message from queue
        for (int i = 0; i < spotMsg.count(); i++)
        {
            QString msg = spotMsg[i];

            if (msg.contains(DXSPOT))
            {
                spotQueue += spotMsg[i];

            }


        }
    }

 }


void BandmapClientFrame::checkNewBandMapSpots()
{
    if (!purgeSpotFlag && !holdUpdateFlag)     // do nothing while purging spots
    {
        // any cluster spots
        int sqsize = spotQueue.count();
        for (int i = sqsize -1 ; i > -1; i--)
        {
             addDxSpotToBandmapTable(spotQueue[i]);
             trace("Bandmapframe New Cluster Spot: " + spotQueue[i]);


        }

        spotQueue.clear();


        // any logger spots
        if (!logSpotQueue.isEmpty())
        {
            for (int i = 0; i < logSpotQueue.count(); i++)
            {
                addLogSpotToBandmapTable(logSpotQueue[i]);
                trace(QString("Bandmapframe New Logger Spot: %1 %2 %3 %4").arg(logSpotQueue[i]->getCallsign().fullCall.getValue()).arg(logSpotQueue[i]->getFreq()).arg(logSpotQueue[i]->getLocator()));
                delete logSpotQueue[i];
            }

            logSpotQueue.clear();
        }





    }
}


void BandmapClientFrame::addDxSpotToBandmapTable(const QString spot)
{

    QDateTime spotDateTime = QDateTime::currentDateTimeUtc();
    QStringList sl = spot.split(DXSPOT);
    if (sl.count() == 2)
    {
        QStringList spotlist = sl[1].split(':', QString::KeepEmptyParts);

        if (!checkSpotInTable(spotlist))
        {
            return; // spot logged or marked and moved
        }

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

            //-------------------------------------------------------

            //timeToLive = 120; // for testing.....

            //--------------------------------------------------------

            // check to see if spot is for this contest band

            if (spotlist[DXBANDSTR] != contestBandStr)
            {
                return;
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

            spotDateTime = getSpotDateTime(spotlist[SPOTDATE], spotlist[SPOTTIME]);
            qint64 rxTime = spotDateTime.toMSecsSinceEpoch() / 1000;

            // convert freq
            bool ok = false;
            QString f = spotlist[DXFREQ].remove('.') + "000";
            qint64 dxFreq = f.toLongLong(&ok, 10);
            if (!ok)
            {
                dxFreq = 0;
            }

            bandmapDataModel->rowData = new BandmapData(rxTime, spotlist[SPOTTIME],
                                                    spotlist[DXFREQ], dxFreq, spotlist[DXBANDSTR],  spotlist[DXBANDMASK],
                                                    spotlist[DXMODESTR], spotlist[DXMODEMASK], spotlist[DXCALL],
                                                    callWorked, spotlist[DXLOCATOR],
                                                    locWorked,distance,
                                                    bearing, "", false, spotlist[SPOTCALL],        // ignore rotator bearing
                                                    spotlist[SPOTLOCATOR], spotlist[DXPROPMODE], spotlist[SPOTCOMMENT], bandmapSpotType::SPOT_TYPE::CLUSTER);

            bandmapDataModel->insertRows(bandmapDataModel->rowCount(), 1);


       }
    }

}



void BandmapClientFrame::addLogSpotToBandmapTable(LoggerSpots* spot)
{

    // look for an existing spot if the marker is a LOGGED or SAVE type

    if (spot->getSpotType() == bandmapSpotType::LOGGED || spot->getSpotType() == bandmapSpotType::SAVED)
    {
        for (int row = 0; row < bandmapDataModel->rowCount(); row++)
        {
            if (spot->getCallsign().fullCall.getValue() == bandmapDataModel->data(bandmapDataModel->index(row, DXSPOT_CALL_COL_NUM ),  BMP_DataStoredRole).toString())
            {
                if (static_cast<bandmapSpotType::SPOT_TYPE>(bandmapDataModel->data(bandmapDataModel->index(row, SPOT_TYPE_COL_NUM ),  BMP_DataStoredRole).toInt()) == bandmapSpotType::LOGGED
                        && spot->getSpotType() == bandmapSpotType::SAVED)
                {
                    // we want to move the freq of the LOGGED spot
                    bandmapDataModel->setData(bandmapDataModel->index(row, FREQ_STR_COL_NUM ), spot->getFreq() , BMP_DataStoredRole);
                    bandmapDataModel->setData(bandmapDataModel->index(row, FREQ_INT64_COL_NUM), spot->getFreq().toLongLong(), BMP_DataStoredRole);

                    // do we need to update the time as well????
                    // we don't need to save this incomming logger spot as we have moved it..
                    return;
                }
                else if  (static_cast<bandmapSpotType::SPOT_TYPE>(bandmapDataModel->data(bandmapDataModel->index(row, SPOT_TYPE_COL_NUM ),  BMP_DataStoredRole).toInt()) == bandmapSpotType::SAVED
                          || static_cast<bandmapSpotType::SPOT_TYPE>(bandmapDataModel->data(bandmapDataModel->index(row, SPOT_TYPE_COL_NUM ),  BMP_DataStoredRole).toInt()) == bandmapSpotType::CLUSTER)
                {
                    bandmapDataModel->removeRows(row, 1);
                }

            }
        }
    }

    bool locWorked = false;
    // update worked locators
    QString loc = spot->getLocator();
    if (!loc.isEmpty())
    {
        QString locMajor = loc.mid(0,4);

        if (spot->getSpotType() == bandmapSpotType::LOGGED)
        {
            locWorked = true;  // this spot from logger is logged - mark locator worked
            for (int row = 0; row < bandmapDataModel->rowCount(); row++)
            {
                QString storedLoc = bandmapDataModel->data(bandmapDataModel->index(row, DXLOC_COL_NUM ),  BMP_DataStoredRole).toString();
                if (!storedLoc.isEmpty())
                {
                    if (locMajor == storedLoc)
                    {
                        bandmapDataModel->setData(bandmapDataModel->index(row, DXLOC_WORKED_COL_NUM ), true ,BMP_DataStoredRole);
                    }
                }
            }
        }


    }






    QString rotBrg;

    // find distance to station
    double dist = 0;
    int brg = 0;
    QString distance;


    if (!spot->getLocator().isEmpty())
    {
        calcSpotDistanceBearing(spot->getLocator(), &dist, &brg);
        distance = QString::number(static_cast<int>(dist));
    }


    if (rotatorConnected)
    {
        rotBrg = curRotBearing;   // get rotator bearing
    }
    else
    {
        rotBrg = "0";
    }



    qint64 logTime = spot->getTime().toMSecsSinceEpoch() / 1000;

    QString logTimeStr = spot->getTime().time().toString("HH:MM");

    // convert freq
    bool ok = false;
    qint64 logFreq = spot->getFreq().toLongLong(&ok, 10);
    if (!ok)
    {
        logFreq = 0;
    }



    bandmapDataModel->rowData = new BandmapData(logTime, logTimeStr,
                                            spot->getFreq(), logFreq, spot->getbandStr(),  spot->getBandMask(),
                                            spot->getModeStr(), spot->getModeMask(), spot->getCallsign().fullCall.getValue(),
                                            spot->getWorked(), spot->getLocator(),
                                            locWorked, distance,
                                            spot->getBearing(), rotBrg, rotatorConnected, "",
                                            "", "", "", spot->getSpotType());

    bandmapDataModel->insertRows(bandmapDataModel->rowCount(), 1);


}

//checks spot freq against stored freq, return the row number to insert before

int BandmapClientFrame::findRowToInsert(QString f)
{

    qint64 spotf = f.toLongLong();
    qint64 freq;
    qint64 nextFreq;

    if (bandmapDataModel->rowCount() != 0)
    {
        for (int row = 0; row < bandmapDataModel->rowCount(); row++)
        {
            freq = bandmapDataModel->data(bandmapDataModel->index(row, FREQ_STR_COL_NUM), BMP_DataStoredRole).toLongLong();

            if (row == 0)
            {
                if (spotf < freq)
                {
                    // prepend
                    return row;
                }
            }

            if (spotf > freq)
            {
                if (row == bandmapDataModel->rowCount() - 1)
                {
                    // reached end append
                    return row + 1;
                }
                else
                {
                    nextFreq = bandmapDataModel->data(bandmapDataModel->index(row + 1, FREQ_STR_COL_NUM), BMP_DataStoredRole).toLongLong();
                    if (spotf < nextFreq)
                    {
                        return row + 1;
                    }
                }

            }



        }
    }
    else
    {
        return 0;
    }

   return 0;        // shouldn't reach here!

}

bool BandmapClientFrame::checkSpotInTable(QStringList &sl)
{
    QStringList spotlist = sl;
    QString dxCallsign = spotlist[DXCALL];
    QString dxFreq = spotlist[DXFREQ].remove('.');
    //qint64 dxFreqInt64 = dxFreq.toLongLong();
    multFreq matchFreq;
    QVector<multFreq> listOfFreq;

    if (bandmapDataModel->rowCount() != 0)
    {

        // check for repeat call
        for (int row = 0; row < bandmapDataModel->rowCount(); row++)
        {

            if (dxCallsign == bandmapDataModel->data(bandmapDataModel->index(row, DXSPOT_CALL_COL_NUM ), BMP_DataStoredRole).toString())
            {
                bandmapSpotType::SPOT_TYPE spotType = static_cast<bandmapSpotType::SPOT_TYPE>(bandmapDataModel->data(bandmapDataModel->index(row, SPOT_TYPE_COL_NUM ), BMP_DataStoredRole).toInt());
                if ( spotType == bandmapSpotType::LOGGED || spotType == bandmapSpotType::SAVED || spotType == bandmapSpotType::CLUSTER_MARKED)
                {
                    // move the logged or marked spot to new freq
                    bandmapDataModel->setData(bandmapDataModel->index(row, FREQ_STR_COL_NUM), dxFreq, BMP_DataStoredRole);
                    bandmapDataModel->setData(bandmapDataModel->index(row, FREQ_INT64_COL_NUM), dxFreq.toLongLong(), BMP_DataStoredRole);
                    return  false;          // don't save this spot to the bandmap spot list

                }else if (spotType == bandmapSpotType::CLUSTER)
                {
                    // yes, remove old spot
                    bandmapDataModel->removeRows(row, 1);
                }


            }
        }

        // check for multiple spots on the same freq
        for (int row = 0; row < bandmapDataModel->rowCount(); row++)
        {

            QString df = bandmapDataModel->data(bandmapDataModel->index(row, FREQ_STR_COL_NUM), BMP_DataStoredRole).toString();
            if (dxFreq == bandmapDataModel->data(bandmapDataModel->index(row, FREQ_STR_COL_NUM), BMP_DataStoredRole).toString())
            {
                // found a spot on this freq
                matchFreq.setRow(row);
                qint64 timeInt64 = bandmapDataModel->data(bandmapDataModel->index(row, TIME_COL_NUM), BMP_DataStoredRole).toLongLong();
                matchFreq.setTime(timeInt64);
                listOfFreq.append(matchFreq);



                if (listOfFreq.count() == MAX_CALL_SAME_FREQ)
                {
                    // remove oldest spot
                    int position = 0;
                    qint64 oldest = listOfFreq[0].getTime();
                    for(int i = 1; i < listOfFreq.count(); i++)
                    {
                        if(oldest > listOfFreq[1].getTime())
                        {
                           oldest = listOfFreq[1].getTime();
                           position = i;
                         }
                    }
                    // remove oldest
                    bandmapDataModel->removeRows(listOfFreq[position].getRow(), 1);

                }

            }
        }

    }

    return true;
}




void BandmapClientFrame::checkSpotWorked(QString &callsign, QString &locator, bool* callWorked, bool* locatorWorked)
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


void BandmapClientFrame::calcSpotDistanceBearing(const QString& _locator, double* distance, int* bearing)
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


void BandmapClientFrame::checkSavedFilters()
{
    // this looks for changed saved settings
    LoggerContestLog* contest = dynamic_cast<LoggerContestLog *>( ct);
    if (contest)
    {
        QString cUuuid = ct->uuid;
        BandmapClientFilterSettings bfs = contest->bandmapFilterSettings.getValue();
        if (bfs != filterSetup->filterSettings)
        {
            filterSetup->filterSettings = bfs;

        }
    }
}






void BandmapClientFrame::setClusterServerState(QString stateMsg)
{


    if (stateMsg.contains("Connected"))
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

        ui->statusIndicator->setToolTip(stateMsg);
        trace(QString("Bandmap Cluster Status: %1").arg(stateMsg));
    }
    else
    {
        ui->statusIndicator->setToolTip("Cluster Server Not Running");
    }
}

void BandmapClientFrame::setClusterServerLoaded(bool loaded)
{
    clusterServerLoaded = loaded;
}

void BandmapClientFrame::statusIndicatorToggle(bool on)
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

void BandmapClientFrame::setFreq(QString freq)
{
    trace(QString("Bandmap frame Set Freq: = %1").arg(freq));


    if (lastfreq != freq)
    {
        lastfreq = freq;
        if (freq.count() >= 4)
        {

            ui->freqDisplay->setInputMask(maskData::freqMask[freq.count() - 4]);
            if (isFreqLegal(freq.toDouble(), contestBandStr, contestModeStr))
            {

                freqDisplayPalette->setColor(QPalette::Text, Qt::black);
                ui->freqDisplay->setPalette(*freqDisplayPalette);
                legalFreq = true;
            }
            else
            {
                freqDisplayPalette->setColor(QPalette::Text,Qt::red);
                ui->freqDisplay->setPalette(*freqDisplayPalette);
                legalFreq = false;
            }


            ui->freqDisplay->setText(freq);
        }

        curFreq = freq.toDouble();
        bandmapView->setFreq(curFreq, legalFreq);



    }

}



void BandmapClientFrame::filterButtonSelected()
{

    filterSetup->copyModeFiltersToDialog();

    filterSetup->exec();

}


bool BandmapClientFrame::event(QEvent *event)
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
           checkNewBandMapSpots();
       }
       setHoldUpdateFlag(false);

   }

   return QWidget::event(event);
}





bool BandmapClientFrame::eventFilter(QObject *obj, QEvent *event)
{

   if (obj == ui->freqDisplay && event->type() == QEvent::MouseButtonPress)
   {
       emit freqDisplayClicked();
   }

   return false;
}


void BandmapClientFrame::setHoldUpdateFlag(bool state)
{

    holdUpdateFlag = state;
    if (state)
    {
        ui->bandmapFrameTitle->setText("Bandmap - <font color='Red'>Mouse within frame!</font>");
    }
    else
    {
        ui->bandmapFrameTitle->setText("Bandmap");
    }
}


bool BandmapClientFrame::isSpotQueueEmpty()
{
    return spotQueue.isEmpty();
}

void BandmapClientFrame::buttonHandleDxSpots()
{
    checkNewBandMapSpots();
}

void BandmapClientFrame::mouseMoveEvent(QMouseEvent *event)
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

void BandmapClientFrame::mouseTimerCheckNewSpots()
{
    if (holdUpdateFlag)
    {
        if (!spotQueue.isEmpty())
        {
            checkNewBandMapSpots();
        }
        mouseInFrameTimer->start(MOUSE_IN_FRAME_TIMEOUT);
    }
}

void BandmapClientFrame::purgeSpots()
{
    if (timeToLive > 0 && !holdUpdateFlag /*&& (ct && ct == TContestApp::getContestApp()->getCurrentContest())*/)      // don't purge spots if == 0 and holdupdateflag is on
    {
        if (bandmapDataModel->rowCount() > 0)
        {
           purgeSpotFlag = true;
           bandmapSpotType::SPOT_TYPE spotType;


           int idx = bandmapDataModel->rowCount() - 1;
           while (idx >= 0 && bandmapDataModel->rowCount() > 0)
           {
               spotType = static_cast<bandmapSpotType::SPOT_TYPE>(bandmapDataModel->data(bandmapDataModel->index(idx, SPOT_TYPE_COL_NUM), BMP_DataStoredRole).toInt());
               if (spotType == bandmapSpotType::CLUSTER)
               {
                   if (spotTimedOut(bandmapDataModel->data(bandmapDataModel->index(idx, RXTIME_COL_NUM), BMP_DataStoredRole).toLongLong(), timeToLive))
                   {
                         bandmapDataModel->removeRows(idx, 1, QModelIndex());
                   }
               }

               idx--;
           }
           purgeSpotFlag = false;
        }
    }

    bandmapView->bandmapUpdate();
}

void BandmapClientFrame::on_AfterLogContact(BaseContestLog *c, Callsign cs, QString loc, QString brg, QString freq)
{
    Q_UNUSED(c)

    //QString time = QDateTime::currentDateTimeUtc().time().toString("HH:MM");
    QDateTime time = QDateTime::currentDateTimeUtc();

    QString logBandStr;
    QString logBandMask;
    QString logModeStr;
    QString logModeMask;

    getBand(bands, freq, logBandStr, logBandMask);
    getMode(modeBandPlan, freq, logBandStr, logModeStr, logModeMask);


    LoggerSpots* spot = new LoggerSpots(cs, loc, brg,
                                        logModeStr, logModeMask,
                                        freq.remove('.'), logBandStr, logBandMask,
                                        true, time, bandmapSpotType::LOGGED);
    logSpotQueue.append(spot);
}



void BandmapClientFrame::setBandmapMarkFreq(QString cs, QString _freq, QString loc, QString brg)
{
    Q_UNUSED(cs)
    QDateTime time = QDateTime::currentDateTimeUtc();

    QString logBandStr;
    QString logBandMask;
    QString logModeStr;
    QString logModeMask;

    QString freq = _freq.remove('.');

    getBand(bands, freq, logBandStr, logBandMask);
    getMode(modeBandPlan, freq, logBandStr, logModeStr, logModeMask);


    LoggerSpots* spot = new LoggerSpots(Callsign("????"), loc, brg,
                                        logModeStr, logModeMask,
                                        freq, logBandStr, logBandMask,
                                        false, time, bandmapSpotType::MARKED);
    logSpotQueue.append(spot);
}


void BandmapClientFrame::setBandmapSaveFreq(QString cs, QString _freq, QString loc, QString brg)
{
    Q_UNUSED(cs)

    QDateTime time = QDateTime::currentDateTimeUtc();

    QString logBandStr;
    QString logBandMask;
    QString logModeStr;
    QString logModeMask;

    QString freq = _freq.remove('.');

    getBand(bands, freq, logBandStr, logBandMask);
    getMode(modeBandPlan, freq, logBandStr, logModeStr, logModeMask);

    LoggerSpots* spot = new LoggerSpots(cs, loc, brg,
                                        logModeStr, logModeMask,
                                        freq, logBandStr, logBandMask,
                                        false, time, bandmapSpotType::SAVED);
    logSpotQueue.append(spot);
}

void BandmapClientFrame::setRotatorBearing(QString s)
{
    QStringList sl = s.split(':');
    if (sl.size() < 3)
    {
        return;
    }

    curRotBearing = sl[1];
}

void BandmapClientFrame::setRotatorConnected(bool connected)
{
    rotatorConnected = connected;
}


void BandmapClientFrame::updateZoom(bool dir)
{
    bandmapView->updateZoom(dir);
}


QString BandmapClientFrame::readBandmapFreqLimit(QString band, QString mode)
{
    QString limitFreqs = "";
    QFile limitFile(BANDPLAN_FREQ_LIMITS_FILE);
    if (limitFile.exists())
    {
        trace(QString("bandmap: bandmapLimit file found - %").arg(BANDPLAN_FREQ_LIMITS_FILE));
        QString fileName = BANDPLAN_FREQ_LIMITS_FILE;
        QSettings settings(fileName, QSettings::IniFormat);
        QStringList limitBands = settings.childGroups();
        if (limitBands.contains(band))
        {
            trace(QString("bandmap: bandmapLimit band = %1, mode = %2").arg(band).arg(mode));
            if (mode.isEmpty())
            {
                mode = "USB";
            }
            settings.beginGroup(band);
            limitFreqs = settings.value(mode, "").toString();
            settings.endGroup();
        }

    }

    return limitFreqs;

}
