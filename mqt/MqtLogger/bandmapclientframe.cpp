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
    contestMode(-1)
{

    ui->setupUi(this);

    connect (ClusterClientServer::getClusterClientServer(), SIGNAL(ClusterServerList(QVector<ClusterServer>)), this, SLOT(clusterClientServerList(QVector<ClusterServer>)));
    connect (ClusterClientServer::getClusterClientServer(), SIGNAL(dxSpot(QVector<QString>)), this, SLOT(dxSpots(QVector<QString>)));

    bandmapDataModel = new BandmapDataModel();

    bandmap = new Bandmap(ui->bandmapFrame);
    bandmap->initBandmap(bandmapDataModel);





    BandmapData *bandmapData0 = new BandmapData();
    BandmapData *bandmapData1 = new BandmapData();
    BandmapData *bandmapData2 = new BandmapData();
    BandmapData *bandmapData3 = new BandmapData();

    bandmapData0->dxCall = "G8FKH";
    bandmapData0->dxFreq = "14420000";
    bandmapData0->dxLocator = "IO91SN";
    bandmapDataModel->rowData = bandmapData0;
    bandmapDataModel->insertRows(bandmapDataModel->rowCount(), 1);

    bandmapData1->dxCall = "G4CLB";
    bandmapData1->dxFreq = "14421000";
    bandmapData1->dxLocator = "IO92AB";
    bandmapDataModel->rowData = bandmapData1;
    bandmapDataModel->insertRows(bandmapDataModel->rowCount(), 1);

    bandmapData2->dxCall = "GM3SEK";
    bandmapData2->dxFreq = "14421500";
    bandmapData2->dxLocator = "IO94FG";
    bandmapDataModel->rowData = bandmapData2;
    bandmapDataModel->insertRows(bandmapDataModel->rowCount(), 1);

    bandmapData3->dxCall = "M0SAT";
    bandmapData3->dxFreq = "14431500";
    bandmapData3->dxLocator = "IO91TP";
    bandmapDataModel->rowData = bandmapData3;
    bandmapDataModel->insertRows(bandmapDataModel->rowCount(), 1);

    int a = 0;

    connect(&MinosLoggerEvents::mle, SIGNAL(FontChanged()), this, SLOT(on_FontChanged()), Qt::QueuedConnection);


}


BandmapClientFrame::~BandmapClientFrame()
{
    delete ui;
}

void BandmapClientFrame::on_FontChanged()
{
    QFont cf = QApplication::font();
}


/*
void BandmapClientFrame::drawDial(double frequency)
{


    if (!almost_equal(frequency, dial->getCurFreq(), 2))
    {
        dial->setCurFreq(frequency);
        dial->setCurHeight(ui->bandmapView->height());
        dial->update();
    }

}


void BandmapClientFrame::resizeEvent(QResizeEvent *event)
{
    mapViewHeight = ui->bandmapView->height() - 2;
    if (dial->getCurHeight() != mapViewHeight)
    {
        dial->changeBoundingRect(mapViewHeight);
        bandmapScene->setSceneRect(bandmapScene->itemsBoundingRect());
    }
    qDebug() << "view height" << mapViewHeight;
    qDebug() << "dial height" << dial->boundingRect().height();

    dial->setCurFreq(curFreq);
    //dial->setCurHeight(mapViewHeight);
    dial->update();

    QWidget::resizeEvent(event);
}
*/
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


//void BandmapClientFrame::paintEvent(QPaintEvent  * /*event */)
//{

//}



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
        handleDxSpots(spotQueue);
    //}

 }


void BandmapClientFrame::handleDxSpots(QVector<QString> &spotQueue)
{
    int sqsize = spotQueue.count();
    for (int i = sqsize -1 ; i > -1; i--)
    {
       //addDxSpotToTable(spotQueue[i]);
       trace("Bandmapframe syncSpots " + spotQueue[i]);
    }


    spotQueue.clear();
}

/*
void BandmapClientFrame::addDxSpotToTable(const QString spot)
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

            dxSpotDataModel->rowData = new SpotData(rxTime, spotlist[SPOTTIME],
                                                    spotlist[DXFREQ], spotlist[DXBANDMASK],
                                                    spotlist[DXMODEMASK], spotlist[DXCALL],
                                                    callWorked, spotlist[DXLOCATOR],
                                                    locWorked,distance,
                                                    bearing, spotlist[SPOTCALL],
                                                    spotlist[SPOTLOCATOR], spotlist[SPOTCOMMENT]);

            dxSpotDataModel->insertRows(dxSpotDataModel->rowCount(), 1);

       }
    }

}

*/

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
        bandmap->setFreq(curFreq);



    }






}
