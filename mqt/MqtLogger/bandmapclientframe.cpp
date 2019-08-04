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
#include <QDebug>
#include "ui_bandmapclientframe.h"

#include "bandmapclientframe.h"

BandmapClientFrame::BandmapClientFrame(QWidget *parent):
    QFrame(parent),
    ui(new Ui::BandmapClientFrame),
    isProtected(false),
    contestBand(-1),
    contestMode(-1),
    purgeSpotFlag(false),
    holdUpdateFlag(false),
    timeToLive(0)
{

    ui->setupUi(this);

    connect (ClusterClientServer::getClusterClientServer(), SIGNAL(ClusterServerList(QVector<ClusterServer>)), this, SLOT(clusterClientServerList(QVector<ClusterServer>)));
    connect (ClusterClientServer::getClusterClientServer(), SIGNAL(dxSpot(QVector<QString>)), this, SLOT(dxSpots(QVector<QString>)));

    bandmapDataModel = new BandmapDataModel();

    bandmapGraphicsView = ui->bandmapView;
    //bandmapGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff );
    //bandmapGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff );
    bandmapGraphicsView->setVisible(true);

    bandmapView = new BandmapView(bandmapGraphicsView);
    bandmapView->setModel(bandmapDataModel);



    //bandmap = new Bandmap();
    //bandmap->initBandmap(bandmapDataModel, ui->bandmapView);

    checkNewSpotsTimer = new QTimer(this);
    connect (checkNewSpotsTimer, SIGNAL(timeout()), this, SLOT(checkBandMapSpots()));
    checkNewSpotsTimer->start();

    connect(&MinosLoggerEvents::mle, SIGNAL(FontChanged()), this, SLOT(on_FontChanged()), Qt::QueuedConnection);


}


BandmapClientFrame::~BandmapClientFrame()
{
    delete ui;
}

void BandmapClientFrame::on_FontChanged()
{
    QFont cf = QApplication::font();
    bandmapView->onFontChanged(cf);
}



void BandmapClientFrame::setContest(BaseContestLog *c)
{
    ct = c;
    LoggerContestLog* contest = dynamic_cast<LoggerContestLog *>( ct);

    if (ct != nullptr)
    {
        contestUuid = ct->uuid;
        trace(QString("Cluster ClientFrame Set Contest: contest uuid =  ContestUuid = %1").arg(contestUuid));
        contestBandStr = ct->band.getValue();
        contestBand = getBandOffSet(contestBandStr);
        contestModeStr = ct->currentMode.getValue();
        contestMode = getModeOffSet(contestModeStr);
//        if (!contest->clusterFilterSettingsExist)       // have settings been saved before?
//        {
//            // no, save current band filter for this contest
//            filterSetup->setBandFilter(contestBand);    // set cluster filter to current band - can be overidden
//            filterSetup->saveClusterFilterToContest();
//        }

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


    //if (!purgeSpotFlag && !holdUpdateFlag)     // do nothing while purging spots
    //{
        //handleDxSpots(spotQueue);
    //}

 }


void BandmapClientFrame::checkBandMapSpots()
{
    if (!purgeSpotFlag && !holdUpdateFlag)     // do nothing while purging spots
    {
        // any cluster spots
        int sqsize = spotQueue.count();
        for (int i = sqsize -1 ; i > -1; i--)
        {
             addDxSpotToBandmapTable(spotQueue[i]);
             trace("Bandmapframe syncSpots " + spotQueue[i]);
        }

        spotQueue.clear();


    }
}


void BandmapClientFrame::addDxSpotToBandmapTable(const QString spot)
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

            //-------------------------------------------------------

            //timeToLive = 120; // for testing.....

            //--------------------------------------------------------


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

            // convert freq
            bool ok = false;
            QString f = spotlist[DXFREQ].remove('.') + "000";
            qint64 dxFreq = f.toLongLong(&ok, 10);
            if (!ok)
                dxFreq = 0;

            bandmapDataModel->rowData = new BandmapData(rxTime, spotlist[SPOTTIME],
                                                    spotlist[DXFREQ], dxFreq, spotlist[DXBANDMASK],
                                                    spotlist[DXMODEMASK], spotlist[DXCALL],
                                                    callWorked, spotlist[DXLOCATOR],
                                                    locWorked,distance,
                                                    bearing, spotlist[SPOTCALL],
                                                    spotlist[SPOTLOCATOR], spotlist[SPOTCOMMENT]);

            bandmapDataModel->insertRows(bandmapDataModel->rowCount(), 1);

       }
    }

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

void BandmapClientFrame::handleClusterStatusMessage(QString &msg)
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
        trace(QString("Bandmap Cluster Status: %1").arg(statusMsg));
    }
    else
    {
        ui->statusIndicator->setToolTip("");
    }
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
            ui->freqDisplay->setText(freq);
        }

        curFreq = freq.toDouble();
        bandmapView->setFreq(curFreq);



    }






}
