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

#include "base_pch.h"
#include "ui_clusterclientframe.h"


ClusterClientFrame::ClusterClientFrame(QWidget *parent):
    QFrame(parent)
    , ui(new Ui::ClusterClientFrame)
    , purgeTimer(nullptr)
    , timeToLive(0)
    , purgeSpotFlag(false)
{

    ui->setupUi(this);
    filterSetup = new ClusterClientFilterTab();

    purgeTimer = new QTimer(this);

    spotQueue.clear();

    connect (ClusterClientServer::getClusterClientServer(), SIGNAL(ClusterServerList(QVector<ClusterServer>)), this, SLOT(clusterClientServerList(QVector<ClusterServer>)));
    connect (ClusterClientServer::getClusterClientServer(), SIGNAL(dxSpot(QVector<QString>)), this, SLOT(dxSpots(QVector<QString>)));
    connect(&MinosLoggerEvents::mle, SIGNAL(FontChanged()), this, SLOT(on_FontChanged()), Qt::QueuedConnection);

    connect (ui->filtersBut, SIGNAL(clicked()), this, SLOT(filterButtonSelected()));
    connect (purgeTimer, SIGNAL(timeout()), this, SLOT(purgeSpots()));

    on_FontChanged();

/*
    dxSpotDataModel = new DxSpotDataModel();
    dxSpotView = ui->dxSpotView;
    dxSpotView->setModel(dxSpotDataModel);
    dxSpotView->setSelectionMode( QAbstractItemView::NoSelection );
    restoreDxSpotViewColumns();
    dxSpotView->resizeRowsToContents();
*/

    dxSpotDataModel = new DxSpotDataModel();
    //dxSpotView = new QTableView();
    dxSpotView = ui->dxSpotView;
    dxSpotView->setModel(dxSpotDataModel);
    //dxSpotView->setSelectionBehavior( QAbstractItemView::SelectRows );
    //dxSpotView->setSelectionMode( QAbstractItemView::SingleSelection );
    dxSpotView->setSelectionMode( QAbstractItemView::NoSelection );

    QHeaderView *verticalHeader = dxSpotView->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(18);


    connect( dxSpotView->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( on_sectionResized(int, int , int)));

    dxSpotView->setColumnWidth(TIME_COL_NUM, TIME_COL_WIDTH);
    dxSpotView->setColumnWidth(FREQ_COL_NUM, FREQ_COL_WIDTH);
    dxSpotView->setColumnWidth(DXSPOT_CALL_COL_NUM, DXSPOT_CALL_COL_WIDTH);
    dxSpotView->setColumnWidth(DXLOC_COL_NUM, DXLOC_COL_WIDTH);
    dxSpotView->setColumnWidth(SPOT_CALL_COL_NUM, SPOT_CALL_COL_WIDTH);
    dxSpotView->setColumnWidth(SPOTLOC_COL_NUM, SPOTLOC_COL_WIDTH);
    dxSpotView->setColumnWidth(COMMENT_COL_NUM, COMMENT_COL_WIDTH);

    restoreDxSpotViewColumns();

    purgeTimer->start(PURGE_TIME);


}


ClusterClientFrame::~ClusterClientFrame()
{
    delete ui;
}


void ClusterClientFrame::filterButtonSelected()
{
    filterSetup->copyBandFilterMaskToEdit();
    filterSetup->copyModeFilterMaskToEdit();
    filterSetup->show();

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


                // check spot against filter setting
                bool ok;
                unsigned int spotMask = static_cast<unsigned int>(spotlist[DXBANDMASK].toInt(&ok));
                unsigned int filterMask = filterSetup->getBandFilterMask();
                if ( filterMask & spotMask || filterMask == 0)
                {
                    //dxSpotDataModel->rowData = QStringList {spotTime, displayFreq, dxCall, dxLocator, spotCall, spotLocator, spotComment };
                    dxSpotDataModel->rowData = QStringList {spotlist[SPOTTIME], spotlist[DXFREQ], spotlist[DXCALL], spotlist[DXLOCATOR], spotlist[SPOTCALL], spotlist[SPOTLOCATOR], spotlist[SPOTCOMMENT]};
                    dxSpotDataModel->insertRows(0, 1);

                }
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


void ClusterClientFrame::setContest(BaseContestLog *c)
{
    contest = c;
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

           //if (!spotQueue.empty())
           //{
           //   handleDxSpots(spotQueue);
           //}
           purgeSpotFlag = false;
        }
    }

}


/*


           do
           {
             a = dxSpotDataModel->rowCount();
             st = dxSpotDataModel->data(dxSpotDataModel->index(dxSpotDataModel->rowCount() - 1, 0)).toString();

             if (spotTimedOut(dxSpotDataModel->data(dxSpotDataModel->index(dxSpotDataModel->rowCount() - 1, 0)).toString()))     // spot timed out
             {
                 a = dxSpotDataModel->rowCount();
                 QString s = dxSpotDataModel->data(dxSpotDataModel->index(dxSpotDataModel->rowCount() - 1, 0)).toString();
                 //dxSpotDataModel->removeRow(dxSpotDataModel->rowCount() - 1);
                 dxSpotDataModel->removeRows(a - 1, 1, QModelIndex());

                 a1 = dxSpotDataModel->rowCount();
                 st1 = dxSpotDataModel->data(dxSpotDataModel->index(dxSpotDataModel->rowCount() - 1, 0)).toString();

             }
           } while (spotTimedOut(dxSpotDataModel->data(dxSpotDataModel->index(dxSpotDataModel->rowCount() - 1, 0)).toString())); // spot not timed out

           if (!spotQueue.empty())
           {
              handleDxSpots(spotQueue);
           }

           purgeSpotFlag = false;
        }
    }


}

*/
 /*
        int row = dxSpotDataModel->rowCount();
        if (row > 0)
        {
           purgeSpotFlag = true;
           //int col = dxSpotDataModel->columnCount();
           for (int i = row - 1; i >= 0 ; --i)
           {
              QVariant spotTime = dxSpotDataModel->data(dxSpotDataModel->index(i, 0));
              QString s = spotTime.toString();
              if (spotTimedOut(spotTime.toString()))
              {
                  dxSpotDataModel->removeRow(i);
              }
              else
              {
                 break;
              }
           }
           if (!spotQueue.empty())
           {
              handleDxSpots(spotQueue);
           }

           purgeSpotFlag = false;

        }
    }

}
*/

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
