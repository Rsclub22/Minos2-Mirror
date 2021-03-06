/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Qrz Server
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2021
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////


#include "base_pch.h"
#include "MinosRPC.h"
#include "MinosLoggerEvents.h"
#include "ConfigFile.h"
#include "qrzserverrpc.h"


static bool syncstat = false;
static QVector<QrzServerMessage> qrzRequestsQueue;
const char * QrzServerRpc::stateIndicator[] =
{
    QT_TR_NOOP("Available"),
    QT_TR_NOOP("Not Available"),
    QT_TR_NOOP("No Contact")
};


QrzServerRpc *QrzServerRpc::getQrzServerRpc()
{
    if (!qrzServerRpc)
    {
        qrzServerRpc = new QrzServerRpc();
    }
    return qrzServerRpc;
}



QrzServerRpc *QrzServerRpc::qrzServerRpc = nullptr;

QrzServerRpc::QrzServerRpc()
{
    //connect(&SyncTimer, SIGNAL(timeout()), this, SLOT(SyncTimerTimer()));
    //SyncTimer.start(100);

    MinosRPC *rpc = MinosRPC::getMinosRPC();


    QStringList sv{
        rpcConstants::clusterApp, rpcConstants::qrzDisplayApp
    };

    rpc->initialiseServers(sv);

    connect(rpc, SIGNAL(serverCall(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_serverCall(bool,QSharedPointer<MinosRPCObj>,QString)));
    connect(rpc, SIGNAL(notify(AnalysePubSubNotify ,QString)), this, SLOT(on_notify(AnalysePubSubNotify ,QString)));

    QString a = rpc->getAppName();
    QString station = MinosConfig::getMinosConfig()->getThisServerName();
    //RPCPubSub::publish(rpcConstants::qrzServerApp,  a + "@" + station, "", psPublished);
    RPCPubSub::publish(rpcConstants::qrzServerApp,  a + "@" + station, "", psPublished);

}

QrzServerRpc::~QrzServerRpc()
{
}


void QrzServerRpc::sendQrzResponseToClusterServer(QString dxCall, QString dxQra, QString dxCallState, QString spotterCall, QString spotterQra, QString spotterCallState)
{
    for (auto const &s: qAsConst(serverList))
    {

        trace(QString("Send Qrz Response to Cluster Server = %1").arg(s.app));
        RPCGeneralClient rpc(rpcConstants::qrzMethod);
        QSharedPointer<RPCParam>st(new RPCParamStruct);
        st->addMember(rpcConstants::qrzClusterResponse, rpcConstants::paramName);
        st->addMember( dxCall, rpcConstants::qrzDxCallsign );
        st->addMember(dxQra, rpcConstants::qrzDxGrid);
        st->addDtgMember(dxCallState, rpcConstants::qrzDxReplyState);
        st->addMember(spotterCall, rpcConstants::qrzSpotterCallsign);
        st->addMember(spotterQra, rpcConstants::qrzSpotterGrid);
        st->addDtgMember(spotterCallState, rpcConstants::qrzSpotterReplyState);
        rpc.getCallArgs() ->addParam( st );
        rpc.queueCall( s.app );

    }

}


void QrzServerRpc::sendQrzResponseToLoggerDisplay(QrzCallsignData qrzCallsignData, QString state)
{
    for (auto const &s: qAsConst(serverList))
    {
        trace(QString("Send Qrz Response to Logger Server = %1").arg(s.app));
        RPCGeneralClient rpc(rpcConstants::qrzMethod);
        QSharedPointer<RPCParam>st(new RPCParamStruct);
        st->addMember(rpcConstants::qrzLoggerResponse, rpcConstants::paramName);
        st->addMember( qrzCallsignData.getCallsign(), rpcConstants::qrzDxCallsign );
        st->addMember(qrzCallsignData.getFirstName(), rpcConstants::qrzFirstName );
        st->addMember(qrzCallsignData.getName(), rpcConstants::qrzName );
        st->addMember(qrzCallsignData.getCounty(), rpcConstants::qrzCounty );
        st->addMember(qrzCallsignData.getCountry(), rpcConstants::qrzCountry);
        st->addMember(qrzCallsignData.getLat(), rpcConstants::qrzLat);
        st->addMember(qrzCallsignData.getLon(), rpcConstants::qrzLon);
        st->addMember(qrzCallsignData.getQra(), rpcConstants::qrzDxGrid);
        st->addMember(qrzCallsignData.getCqZone(), rpcConstants::qrzCqZone);
        st->addMember(qrzCallsignData.getItuZone(), rpcConstants::qrzItuZone);
        st->addMember(state, rpcConstants::qrzDxReplyState);

        rpc.getCallArgs() ->addParam( st );
        rpc.queueCall( s.app );

    }




}




void QrzServerRpc::on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from )
{
    trace(QString("QrzServer: on_serverCall - Message from %1").arg(from));
    if ( !err )
    {
        RPCArgs *args = mro->getCallArgs();


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
                msg.setLoggerUuid(loggerId);
                msg.setLoggerFlag(true);
                trace(QString("on_serverCall: callsign %1, received from Logger uuid %2").arg(dxCall, loggerId));
                emit loggerQrzMsg(msg);

            }

        }
    }
}


void QrzServerRpc::on_notify(AnalysePubSubNotify an, const QString /*from*/ )
{


    trace("qrzServer: on_notify");
    if ( an.getOK() )
    {

        if ( an.getCategory() == rpcConstants::clusterApp || an.getCategory() == rpcConstants::qrzDisplayApp )
        {
            trace( QString(stateIndicator[an.getState()]) + " " + an.getCategory() + " " + an.getKey() );
            bool stationFound = false;
            for ( auto &stat: serverList )
            {
                if (stat.app == an.getKey())
                {
                    if (stat.state != an.getState())
                    {
                        stat.state = an.getState();
                        QString mess = tr("%1 changed state to %2").arg(an.getKey()).arg(tr(stateIndicator[an.getState()]));
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
                QrzServer s;
                s.serverName = an.getPublisherServer();
                s.state = an.getState();
                s.publisherProgram = an.getPublisherProgram();
                s.app = an.getKey();
                serverList.push_back( s );
                trace(QString("qrzServerRpc: on_notify - server found %1, publisher program %2, key %3").arg(s.serverName, s.publisherProgram, s.app));
                //QString mess = tr("%1 changed state to %2").arg(an.getKey()).arg(tr(stateIndicator[an.getState()]));

                syncstat = true;
            }
        }

    }

}

/*
void QrzServerRpc::SyncTimerTimer(  )
{
    if (qrzRequestsQueue.count())
    {
        //emit qrzRequestQueue(qrzRequestsQueue);
        //qrzRequestsQueue.clear();
    }
}

*/




