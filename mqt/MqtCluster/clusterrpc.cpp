#include <QObject>


#include "MinosRPC.h"
#include "MinosLoggerEvents.h"
#include "base_pch.h"
#include "clusterrpc.h"
#include "clustercommon.h"

Clusterrpc::Clusterrpc()
{

    connect (ClusterClientServer::getClusterClientServer(), SIGNAL(ClusterServerList(QVector<ClusterServer>)), this, SLOT(clusterClientServerList(QVector<ClusterServer>)));
    connect (ClusterClientServer::getClusterClientServer(), SIGNAL(dxSpot(QVector<QString>)), this, SLOT(dxSpots(QVector<QString>)));

}

Clusterrpc::~Clusterrpc()
{

}

void Clusterrpc::setStandAlone()
{
    RPCPubSub::subscribe(rpcConstants::LocalStationCategory);
    RPCPubSub::subscribe(rpcConstants::StationCategory);
}

//---------------------------------------------------------------------------
void Clusterrpc::clusterClientServerList(QVector<ClusterServer> serverList)
{
    //ui->StationList->clear();
    for ( QVector<ClusterServer>::iterator i = serverList.begin(); i != serverList.end(); i++ )
    {
        QString state = clusterStateIndicator[(*i).state] + " " + (*i).app + "\r\n";
        trace(QString("clusterClientServerList standalone rpc - state = %1").arg(state));
        //ui->StationList->addItem( state );
    }
}

// do we need this? maybe not if we use a common frame.
void Clusterrpc::dxSpots(QVector<QString> spotQueue)
{
    for ( QVector<QString>::iterator i = spotQueue.begin(); i != spotQueue.end(); i++ )
    {
       //ui->ChatMemo->append( (*i) );
       trace("syncChat " + (*i));
    }
    spotQueue.clear();
}


void Clusterrpc::sendDXSpot(QString spot)
{
    QString msg = DXSPOT + spot;
    ClusterClientServer::getClusterClientServer()->sendDxSpot(msg);

}

