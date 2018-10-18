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

#include "base_pch.h"
#include "ui_clusterclientframe.h"


ClusterClientFrame::ClusterClientFrame(QWidget *parent):
    QFrame(parent)
    , ui(new Ui::ClusterClientFrame)
{

    ui->setupUi(this);
    filterSetup = new ClusterClientFilterTab();

    connect (ClusterClientServer::getClusterClientServer(), SIGNAL(ClusterServerList(QVector<ClusterServer>)), this, SLOT(clusterClientServerList(QVector<ClusterServer>)));
    connect (ClusterClientServer::getClusterClientServer(), SIGNAL(dxSpot(QVector<QString>)), this, SLOT(dxSpots(QVector<QString>)));
    connect(&MinosLoggerEvents::mle, SIGNAL(FontChanged()), this, SLOT(on_FontChanged()), Qt::QueuedConnection);

    connect (ui->filtersBut, SIGNAL(clicked()), this, SLOT(filterButtonSelected()));

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
    dxSpotView->setSelectionMode( QAbstractItemView::NoSelection );
    //dxSpotView->setStyleSheet("QHeaderView::section { font: bold; height: 14px }");

    QHeaderView *verticalHeader = dxSpotView->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(18);


    connect( dxSpotView->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( on_sectionResized(int, int , int)));

    dxSpotView->setColumnWidth(TIME_COL_NUM, TIME_COL_WIDTH);
    dxSpotView->setColumnWidth(FREQ_COL_NUM, FREQ_COL_WIDTH);
    dxSpotView->setColumnWidth(DXSPOT_CALL_COL_NUM, DXSPOT_CALL_COL_WIDTH);
    dxSpotView->setColumnWidth(LOC_COL_NUM, LOC_COL_WIDTH);
    dxSpotView->setColumnWidth(SPOT_CALL_COL_NUM, SPOT_CALL_COL_WIDTH);
    dxSpotView->setColumnWidth(COMMENT_COL_NUM, COMMENT_COL_WIDTH);

    restoreDxSpotViewColumns();


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

void ClusterClientFrame::dxSpots(QVector<QString> spotQueue)
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
    if (spot.contains("DXSPOT:"))
    {
        QStringList sl = spot.split("DXSPOT:");
        if (sl.count() == 2)
        {
            QStringList spotlist = sl[1].split(':', QString::KeepEmptyParts);

            // check spot against filter setting
            bool ok;
            unsigned int spotMask = static_cast<unsigned int>(spotlist[DXBANDMASK].toInt(&ok));
            unsigned int filterMask = filterSetup->getBandFilterMask();
            if ( filterMask & spotMask || filterMask == 0)
            {
                //dxSpotDataModel->rowData = QStringList {spotTime, displayFreq, dxCall, dxLocator, spotCall, spotComment };
                dxSpotDataModel->rowData = QStringList {spotlist[SPOTTIME], spotlist[DXFREQ], spotlist[DXCALL], spotlist[DXLOCATOR], spotlist[SPOTCALL], spotlist[SPOTCOMMENT]};
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


void ClusterClientFrame::setContest(BaseContestLog *c)
{
    contest = c;
}
