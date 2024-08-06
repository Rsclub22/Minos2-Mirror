/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Winkeyer Server
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2024
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////


#include "QtUtils.h"
#include "MinosRPC.h"
#include "ConfigFile.h"
#include "RPCCommandConstants.h"
#include "RPCPubSub.h"
#include "MTrace.h"
#include "winkeyerrpc.h"


const char * WinkeyerRpc::winkeyerServerStateIndicator[] =
    {
        QT_TR_NOOP("Available"),
        QT_TR_NOOP("Not Available"),
        QT_TR_NOOP("No Contact")
};

WinkeyerRpc *WinkeyerRpc::winkeyerServerRpc = nullptr;

WinkeyerRpc::WinkeyerRpc()
{
    MinosRPC *rpc = MinosRPC::getMinosRPC();


    QStringList sv{
        rpcConstants::winkeyerApp, rpcConstants::qrzDisplayApp
    };
    rpc->initialiseRouters(sv);

    connect(rpc, &MinosRPC::routerCall, this, & WinkeyerRpc::on_routerCall);
    connect(rpc, &MinosRPC::notify, this, & WinkeyerRpc::on_notify);

    QString a = rpc->getAppName();
    QString station = MinosConfig::getMinosConfig()->getThisRouterName();
    RPCPubSub::publish(rpcConstants::qrzServerApp,  a + "@" + station, "", psPublished);

}

WinkeyerRpc::~WinkeyerRpc()
{
}


WinkeyerRpc *WinkeyerRpc::getWinkeyerRpc()
{
    if (!winkeyerServerRpc)
    {
        winkeyerServerRpc = new WinkeyerRpc();
    }
    return winkeyerServerRpc;
}


void WinkeyerRpc::on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from )
{
    trace(QString("ClusterClientServer: on_routerCall - Message from %1").arg(from));
    if ( !err )
    {
        RPCArgs *args = mro->getCallArgs();

        /*
        if (args)
        {
            QSharedPointer<RPCParam> qrzCluster;
            QSharedPointer<RPCParam> qrzLogger;
            QSharedPointer<RPCParam> msgDxCall;
            QSharedPointer<RPCParam> msgSpotterCall;
            QSharedPointer<RPCParam> msgLogFrameId;
            QString dxCall;
            QString spotterCall;
            QString loggerId;


            // look for message from cluster server
            if (args->getStructArgMember(0, rpcConstants::qrzCluster, qrzCluster)
                && args->getStructArgMember(0, rpcConstants::qrzDxCallsign, msgDxCall)
                && args->getStructArgMember(0, rpcConstants::qrzSpotterCallsign, msgSpotterCall))
            {


                msgDxCall->getString(dxCall);
                msgSpotterCall->getString(spotterCall);
                trace(QString("QrzServerRpc: on_serverCall - receive qrz request from cluster server dxCall = %1, spotterCall = %2").arg(dxCall, spotterCall));
                QrzServerMessage msg;
                msg.setDxCall(dxCall);
                msg.setSpotterCall(spotterCall);
                msg.setLoggerFlag(false);
                msg.setFromStationName(from);
                trace(QString("on_serverCall: callsign %1, received from Cluster Server").arg(dxCall));
                emit clusterQrzMsg(msg);
                //qrzRequestsQueue.push_back(msg);

            }
            // look for message from qrz Display server
            else if (args->getStructArgMember(0, rpcConstants::qrzLogger, qrzLogger)
                     && args->getStructArgMember(0, rpcConstants::qrzDxCallsign, msgDxCall)
                     && args->getStructArgMember(0, rpcConstants::qrzLogFrameId, msgLogFrameId))
            {
                msgDxCall->getString(dxCall);
                msgLogFrameId->getString(loggerId);
                QrzServerMessage msg;
                msg.setDxCall(dxCall);
                msg.setFromStationName(from);
                msg.setLoggerUuid(loggerId);
                msg.setLoggerFlag(true);
                trace(QString("on_serverCall: callsign %1, received from Logger uuid %2").arg(dxCall, loggerId));
                emit loggerQrzMsg(msg);

            }

        }
        */
    }
}


void WinkeyerRpc::on_notify(AnalysePubSubNotify an, const QString /*from*/ )
{
    //    trace(QString("qrzServer: on_notify - routerName = %1, publisherProgram = %2, app = %3").arg(an.getPublisherRouter(), an.getPublisherProgram(), an.getKey()));
    if ( an.getOK() )
    {

        if ( an.getCategory() == rpcConstants::clusterApp || an.getCategory() == rpcConstants::qrzDisplayApp )
        {
            trace( QString("*** QrzDisplayServer::on_notify") + QString(winkeyerServerStateIndicator[an.getState()]) + " " + an.getCategory() + " " + an.getKey() );
            bool stationFound = false;
            for ( auto &stat: serverList )
            {
                if (stat.app == an.getKey())
                {
                    if (stat.state != an.getState())
                    {
                        stat.state = an.getState();
                        //QString mess = tr("%1 changed state to %2").arg(an.getKey()).arg(tr(qrzServerStateIndicator[an.getState()]));
                        //addChat( mess );
                        syncstat = true;
                    }
                    stationFound = true;
                    break;
                }
            }
            if ( !stationFound )
            {
                // We have received notification from a previously unknown station - so report on it
                WinkeyerServer s;
                s.routerName = an.getPublisherRouter();
                s.state = an.getState();
                s.publisherProgram = an.getPublisherProgram();
                s.app = an.getKey();
                serverList.push_back( s );
                trace(QString("WinkeyerServerRpc: routerName = %1, app = %2, publisher programe = %3").arg(s.routerName, s.app, s.publisherProgram));
                //QString mess = tr("%1 changed state to %2").arg(an.getKey()).arg(tr(stateIndicator[an.getState()]));

                syncstat = true;
            }
        }

    }
}


