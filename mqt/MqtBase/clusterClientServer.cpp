/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Cluster Server
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2018
//
//
//
/////////////////////////////////////////////////////////////////////////////


#include "base_pch.h"
#include "MinosRPC.h"
#include "MinosLoggerEvents.h"

#include "clusterClientServer.h"

static bool syncstat = false;
static QVector<QString> spotQueue;

QString clusterStateIndicator[] =
{
    "Available",
    "NotAvailable",
    "NoContact"
};
QString clusterStateList[] =
{
   "Available",
   "Not Available",
   "No Contact"
};


ClusterClientServer *ClusterClientServer::clusterClientServer = nullptr;


ClusterClientServer *ClusterClientServer::getClusterClientServer()
{
    if (!clusterClientServer)
    {
        clusterClientServer = new ClusterClientServer();
    }
    return clusterClientServer;
}

ClusterClientServer::ClusterClientServer()
{
    connect(&SyncTimer, SIGNAL(timeout()), this, SLOT(SyncTimerTimer()));
    SyncTimer.start(100);

    MinosRPC *rpc = MinosRPC::getMinosRPC();

    connect(rpc, SIGNAL(serverCall(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_serverCall(bool,QSharedPointer<MinosRPCObj>,QString)));
    connect(rpc, SIGNAL(notify(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_notify(bool,QSharedPointer<MinosRPCObj>,QString)));

}

ClusterClientServer::~ClusterClientServer()
{
}

void ClusterClientServer::on_notify(bool err, QSharedPointer<MinosRPCObj> mro, const QString &/*from*/ )
{
    AnalysePubSubNotify an( err, mro );

    if ( an.getOK() )
    {
        if ( an.getCategory() == rpcConstants::LocalStationCategory)
        {
            QString server = an.getKey();
            QVector<ClusterServer>::iterator stat;
            bool pubNeeded = true;
            QString a = MinosRPC::getMinosRPC()->getAppName();
            for ( stat = serverList.begin(); stat != serverList.end(); stat++ )
            {
                if ((*stat).app == a + "@" + server)
                {
                    pubNeeded = false;
                    break;
                }
            }
            if (pubNeeded)
            {
                RPCPubSub::publish(rpcConstants::clusterServer,  a + "@" + server, "", psPublished);
            }
        }
        if (an.getCategory() == rpcConstants::StationCategory)
        {
            QString server = an.getKey();
            QVector<ClusterServer>::iterator stat;
            bool subNeeded = true;
            for ( stat = serverList.begin(); stat != serverList.end(); stat++ )
            {
                if ((*stat).serverName == server)
                {
                    subNeeded = false;
                    break;
                }
            }
            if (subNeeded)
            {
                RPCPubSub::subscribeRemote(server, rpcConstants::clusterCategory);
                RPCPubSub::subscribeRemote(server, rpcConstants::clusterServer);
            }
        }

        if ( an.getCategory() == rpcConstants::clusterServer )
        {
            trace( QString("***" + clusterStateIndicator[an.getState()]) + " " + an.getCategory() + " " + an.getKey() );
            QVector<ClusterServer>::iterator stat;
            for ( stat = serverList.begin(); stat != serverList.end(); stat++ )
            {
                if ((*stat).app == an.getPublisherProgram())
                {
                    if ((*stat).state != an.getState())
                    {
                        (*stat).state = an.getState();
                        QString mess = an.getKey() + " changed state to " + clusterStateList[an.getState()];
                        addSpotQueue( mess );
                        syncstat = true;
                    }
                    break;
                }
            }
            if ( stat == serverList.end() )
            {
                // We have received notification from a previously unknown station - so report on it
                ClusterServer s;
                s.serverName = an.getPublisherServer();
                s.state = an.getState();
                s.app = an.getKey();
                serverList.push_back( s );
                QString mess = an.getKey() + " changed state to " + clusterStateList[an.getState()];
                 addSpotQueue( mess );
                syncstat = true;
            }
        }

        if ( an.getCategory() == rpcConstants::clusterCategory )
        {
            trace( QString("!!!!" + clusterStateIndicator[an.getState()]) + " " + an.getCategory() + " " + an.getKey() + " " + an.getValue() );
            /*
            if (an.getKey() == rpcConstants::ChatServerFrequency)
            {
                QVector<ClusterServer>::iterator stat;
                for ( stat = serverList.begin(); stat != serverList.end(); stat++ )
                {
                    if ((*stat).serverName == an.getPublisherServer())
                    {
                        if ((*stat).freq != an.getValue())
                        {
                            (*stat).freq = an.getValue();
                            syncstat = true;
                        }
                        break;
                    }
                }
            }
            */

        }
    }
}
//---------------------------------------------------------------------------
void ClusterClientServer::on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from )
{
    trace( "cluster callback from " + from + ( err ? ":Error" : ":Normal" ) );

    // Should we use QMap to give a list of name/value pairs?
    // BUT the value isn't always the same type - should it be?
    // We could use QVariant, of course...
    trace(QString("ClusterClientServer: on_serverCall - Message from %1").arg(from));
    if ( !err )
    {
        RPCArgs *args = mro->getCallArgs();

        if (args)
        {
            QSharedPointer<RPCParam> psMess;
            if (args->getStructArgMember(0, rpcConstants::sendClusterSpot, psMess))
            {
                QString pmess;
                if (psMess->getString(pmess))
                {
                    trace(QString("ClusterClientServer: on_serverCall - Message = %1").arg(pmess));
                    // add to chat window
                    QString mess = from + " : " + pmess;
                    addSpotQueue( mess );
                }
            }
        }
    }
}
void ClusterClientServer::SyncTimerTimer(  )
{
    syncStations();
    syncSpots();
}

//---------------------------------------------------------------------------
void ClusterClientServer::syncStations()
{
    if ( syncstat )
    {
        syncstat = false;

        emit ClusterServerList(serverList);
    }
}
void ClusterClientServer::addSpotQueue(const QString &spot)
{
    QDateTime dt = QDateTime::currentDateTime();
    QString sdt = dt.toString( "HH:mm:ss " ) + spot;
    spotQueue.push_back(sdt);
}
void ClusterClientServer::syncSpots()
{
    if (spotQueue.count())
    {
        emit dxSpot(spotQueue);
        spotQueue.clear();
    }
}
//---------------------------------------------------------------------------
void ClusterClientServer::sendDxSpot(QString spot)
{
    // We need to send the message to all connected stations
    for ( QVector<ClusterServer>::iterator i = serverList.begin(); i != serverList.end(); i++ )
    {
        RPCGeneralClient rpc(rpcConstants::clusterMethod);
        QSharedPointer<RPCParam>st(new RPCParamStruct);
        st->addMember( spot, rpcConstants::sendClusterSpot );
        rpc.getCallArgs() ->addParam( st );
        rpc.queueCall( (*i).app );
    }
}


