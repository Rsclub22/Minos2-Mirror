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
#include "ConfigFile.h"

#include "clusterClientServer.h"

static QVector<ClusterMessage> spotQueue;

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

    connect(rpc, SIGNAL(routerCall(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_routerCall(bool,QSharedPointer<MinosRPCObj>,QString)));

    QString a = rpc->getAppName();
    QString station = MinosConfig::getMinosConfig()->getThisRouterName();
    RPCPubSub::publish(rpcConstants::clusterClientServer,  a + "@" + station, "", psPublished);
}

ClusterClientServer::~ClusterClientServer()
{
}

//---------------------------------------------------------------------------
void ClusterClientServer::on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from )
{
    trace(QString("ClusterClientServer: on_routerCallCall - Message from %1").arg(from));
    if ( !err )
    {
        RPCArgs *args = mro->getCallArgs();

        if (args)
        {
            QSharedPointer<RPCParam> psMess;
            QSharedPointer<RPCParam> loggerUuid;
            QSharedPointer<RPCParam> frameId;
            QString pmess;
            QString uuid;
            int frame_id;
            if (args->getStructArgMember(0, rpcConstants::sendClusterSpot, psMess)
                    && args->getStructArgMember(0, rpcConstants::loggerUuid, loggerUuid)
                    && args->getStructArgMember(0, rpcConstants::clusterFrameId, frameId))
            {

                psMess->getString(pmess);
                loggerUuid->getString(uuid);
                frameId->getInt(frame_id);
                trace(QString("ClusterClientServer: on_routerCall - receive cluster spot = %1, uuid = %2").arg(pmess).arg(uuid));
                ClusterMessage msg;
                msg.setMessage(pmess);
                msg.setFrameId(frame_id);
                msg.setLoggerUuid(uuid);
                addSpotQueue( msg );

            }

        }
    }
}
void ClusterClientServer::SyncTimerTimer(  )
{
    if (spotQueue.count())
    {
        emit dxSpot(spotQueue);
        spotQueue.clear();
    }
}



//---------------------------------------------------------------------------

void ClusterClientServer::addSpotQueue(const ClusterMessage spot)
{
    //QDateTime dt = QDateTime::currentDateTime();
    //QString sdt = dt.toString( "HH:mm:ss " ) + spot;
    spotQueue.push_back(spot);
}

//---------------------------------------------------------------------------



