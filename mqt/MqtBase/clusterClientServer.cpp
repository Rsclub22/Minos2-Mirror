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

    MinosRPC *rpc = MinosRPC::getMinosRPC();    // DO NOT use the environment variable - use "Chat" everywhere

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
            for ( stat = serverList.begin(); stat != serverList.end(); stat++ )
            {
                if ((*stat).name == server)
                {
                    pubNeeded = false;
                    break;
                }
            }
            if (pubNeeded)
            {
                QString a = MinosRPC::getMinosRPC()->getAppName();
                RPCPubSub::publish(rpcConstants::ChatCategory, rpcConstants::ChatServer, a + "@" + server, psPublished);
            }
        }
        if (an.getCategory() == rpcConstants::StationCategory)
        {
            QString server = an.getKey();
            QVector<ClusterServer>::iterator stat;
            bool subNeeded = true;
            for ( stat = serverList.begin(); stat != serverList.end(); stat++ )
            {
                if ((*stat).name == server)
                {
                    subNeeded = false;
                    break;
                }
            }
            if (subNeeded)
            {
                RPCPubSub::subscribeRemote(server, rpcConstants::ChatCategory);
            }
        }

        if ( an.getCategory() == rpcConstants::ChatCategory )
        {
            trace( QString(clusterStateIndicator[an.getState()]) + " " + an.getKey() + " " + an.getValue() );

            if (an.getKey() == rpcConstants::ChatServer)
            {
                QVector<ClusterServer>::iterator stat;
                for ( stat = serverList.begin(); stat != serverList.end(); stat++ )
                {
                    if ((*stat).name == an.getPublisherServer())
                    {
                        if ((*stat).state != an.getState())
                        {
                            (*stat).state = an.getState();
                            QString mess = an.getPublisherServer() + " changed state to " + clusterStateList[an.getState()];
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
                    s.name = an.getPublisherServer();
                    s.state = an.getState();
                    s.app = an.getValue();
                    serverList.push_back( s );
                    QString mess = an.getPublisherServer() + " changed state to " + clusterStateList[an.getState()];
                    addSpotQueue( mess );
                    syncstat = true;
                }
            }
            else if (an.getKey() == rpcConstants::ChatServerFrequency)
            {
                QVector<ClusterServer>::iterator stat;
                for ( stat = serverList.begin(); stat != serverList.end(); stat++ )
                {
                    if ((*stat).name == an.getPublisherServer())
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
        }
    }
}
//---------------------------------------------------------------------------
void ClusterClientServer::on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from )
{
    trace( "chat callback from " + from + ( err ? ":Error" : ":Normal" ) );

    // Should we use QMap to give a list of name/value pairs?
    // BUT the value isn't always the same type - should it be?
    // We could use QVariant, of course...

    if ( !err )
    {
        RPCArgs *args = mro->getCallArgs();

        if (args)
        {
            QSharedPointer<RPCParam> psMess;
            if (args->getStructArgMember(0, rpcConstants::SendChatMessage, psMess))
            {
                QString pmess;
                if (psMess->getString(pmess))
                {
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
    syncChat();
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
void ClusterClientServer::syncChat()
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
        RPCGeneralClient rpc(rpcConstants::chatMethod);
        QSharedPointer<RPCParam>st(new RPCParamStruct);
        st->addMember( spot, rpcConstants::SendChatMessage );
        rpc.getCallArgs() ->addParam( st );
        rpc.queueCall( (*i).app );
    }
}


