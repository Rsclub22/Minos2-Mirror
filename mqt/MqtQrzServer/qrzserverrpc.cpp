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
static QVector<ClusterMessage> qrzRequestsQueue;
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
    connect(&SyncTimer, SIGNAL(timeout()), this, SLOT(SyncTimerTimer()));
    SyncTimer.start(100);

    MinosRPC *rpc = MinosRPC::getMinosRPC();


    QStringList sv{
        rpcConstants::clusterCategory
    };
    rpc->initialiseServers(sv);

    connect(rpc, SIGNAL(serverCall(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_serverCall(bool,QSharedPointer<MinosRPCObj>,QString)));
    connect(rpc, SIGNAL(notify(AnalysePubSubNotify ,QString)), this, SLOT(on_notify(AnalysePubSubNotify ,QString)));

}

QrzServerRpc::~QrzServerRpc()
{
}


void QrzServerRpc::on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from )
{
    trace(QString("ClusterClientServer: on_serverCall - Message from %1").arg(from));
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
                trace(QString("QrzServerRpc: on_serverCall - receive cluster spot = %1, uuid = %2").arg(pmess, uuid));
                ClusterMessage msg;
                msg.setMessage(pmess);
                msg.setFrameId(frame_id);
                msg.setLoggerUuid(uuid);
                addQrzRequestsQueue( msg );

            }

        }
    }
}


void QrzServerRpc::on_notify(AnalysePubSubNotify an, const QString /*from*/ )
{
/*

    trace("on_notify");
    if ( an.getOK() )
    {

        if ( an.getCategory() == rpcConstants::clusterCategory )
        {
            trace( QString(stateIndicator[an.getState()]) + " " + an.getCategory() + " " + an.getKey() );
            bool chatFound = false;
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
                    chatFound = true;
                    break;
                }
            }
            if ( !chatFound )
            {
                // We have received notification from a previously unknown station - so report on it
                QrzServer s;
                s.serverName = an.getPublisherServer();
                s.state = an.getState();
                s.app = an.getKey();
                foreach (auto &s, serverList)
                {
                    trace(QString("servername = %1, app = %2").arg(s.serverName, s.app));
                }
                serverList.push_back( s );
                QString mess = tr("%1 changed state to %2").arg(an.getKey()).arg(tr(stateIndicator[an.getState()]));
                //addChat( mess );
                syncstat = true;
            }
        }

    }
*/
}


void QrzServerRpc::SyncTimerTimer(  )
{
    if (qrzRequestsQueue.count())
    {
        emit qrzRequest(qrzRequestsQueue);
        qrzRequestsQueue.clear();
    }
}


void QrzServerRpc::addQrzRequestsQueue(const ClusterMessage spot)
{
    //QDateTime dt = QDateTime::currentDateTime();
    //QString sdt = dt.toString( "HH:mm:ss " ) + spot;
    qrzRequestsQueue.push_back(spot);
}


void QrzServerRpc::sendQraToClusterServer(QString dxQra, QString spotterQra, QString state)
{
    RPCGeneralClient rpc(rpcConstants::qrzMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);
    st->addMember( dxQra, rpcConstants::qrzdXGrid);
    st->addMember(spotterQra, rpcConstants::qrzSpotterGrid);
    st->addMember(state, rpcConstants::qrzReplyState);
    rpc.getCallArgs() ->addParam( st );
    //rpc.queueCall( clusterApp  );

}
