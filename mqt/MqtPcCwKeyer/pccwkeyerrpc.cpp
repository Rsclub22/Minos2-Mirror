/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Pc Serial Port DTR CW Keyer
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2025
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////

#include "MinosRPC.h"
#include "RPCCommandConstants.h"
#include "RPCPubSub.h"
#include "MTrace.h"
#include "pccwkeyerrpc.h"





const char * PcCwKeyerRpc::pcCwKeyerServerStateIndicator[] =
{
        QT_TR_NOOP("Available"),
        QT_TR_NOOP("Not Available"),
        QT_TR_NOOP("No Contact")
};

PcCwKeyerRpc* PcCwKeyerRpc::pcCwKeyerRpc = nullptr;

PcCwKeyerRpc *PcCwKeyerRpc::getPcCwKeyerRpc()
{
    if (!pcCwKeyerRpc)
    {
        pcCwKeyerRpc = new PcCwKeyerRpc();
    }
    return pcCwKeyerRpc;
}



PcCwKeyerRpc::PcCwKeyerRpc()
{

    MinosRPC *rpc = MinosRPC::getMinosRPC();


    QStringList sv;

    rpc->findProviders(rpcConstants::pcCwKeyerCategory, sv);


    connect(rpc, &MinosRPC::routerCall, this, &PcCwKeyerRpc::on_routerCall);
    connect(rpc, &MinosRPC::notify, this, &PcCwKeyerRpc::on_notify);
    connect(rpc, &MinosRPC::provider, this, &PcCwKeyerRpc::on_provider);


    QString a = rpc->getAppName();
    QString station = MinosConfig::getMinosConfig()->getThisRouterName();
    RPCPubSub::publish(rpcConstants::pcCwKeyerApp,  a + "@" + station, "", psPublished);

}

PcCwKeyerRpc::~PcCwKeyerRpc()
{
}

int PcCwKeyerRpc::getServerListCount()
{
    MinosRPC *rpc = MinosRPC::getMinosRPC();
    return rpc->getProviders().count();
}


void PcCwKeyerRpc::on_routerCall( bool err, QSharedPointer<MinosRPCObj>mro, const QString /*from*/ )
{
    //trace( "PcCwKeyerRpc RPC: callback from " + from + ( err ? ":Error" : ":Normal" ) );

    if ( !err )
    {
        QString mName = mro->getMethodName();

        if (mName == rpcConstants::pcCwKeyerMethod)
        {

            RPCArgs *args = mro->getCallArgs();
            QSharedPointer<RPCParam> psName;

            if (args->getStructArgMember(0, rpcConstants::paramName, psName))
            {
                QString paraName;
                psName->getString(paraName);

                if (paraName == rpcConstants::pcCwKeyerCwMessage)
                {
                    trace(QString("PcCwKeyerRpc RPC: callback from %1 paraName = %2").arg(mName, paraName));

                    QSharedPointer<RPCParam> cwMsg;

                    QString cwText;


                    if (args->getStructArgMember(0, rpcConstants::pcCwKeyerCwMessage, cwMsg))
                    {
                        cwMsg->getString(cwText);

                        trace(QString("PcCwKeyer RPC: cw message from logger = %1").arg(cwText));

                        emit cwMessageFromLoggerToKeyer(cwText);


                    }


                }
                else if (paraName == rpcConstants::pcCwKeyerStopCw)
                {
                    trace(QString("Cluster RPC: callback from %1 paraName = %2").arg(mName, paraName));


                    QSharedPointer<RPCParam> stopCwCmd;
                    QString stopCwStr;

                    if (args->getStructArgMember(0, rpcConstants::pcCwKeyerStopCwParam, stopCwCmd))
                    {
                        stopCwCmd->getString(stopCwStr);

                        trace(QString("PcCwKeyer RPC: Stop Cw command = %1").arg(stopCwStr));
                        emit cwStopCommandFromLogger(stopCwStr);

                    }


                }
            }
        }


    mro->clearCallArgs();
    QSharedPointer<RPCParam>st(new RPCParamStruct);


    }
}

void PcCwKeyerRpc::on_provider(Provider p, QString /*cat*/  )
{
    trace(QString("clusterServer: on_provider - routerName = %1, app = %2").arg(p.routerName, p.app));
}


void PcCwKeyerRpc::on_notify(AnalysePubSubNotify /*an*/, const QString /*from*/ )
{
    //    trace(QString("clusterServer: on_notify - routerName = %1, publisherProgram = %2, app = %3").arg(an.getPublisherRouter(), an.getPublisherProgram(), an.getKey()));

    //    if ( an.getOK() )
    //    {
    //        if ( an.getCategory() == rpcConstants::clusterClientServer || an.getCategory() == rpcConstants::qrzServerApp )
    //        {
    //            trace( QString("***") + clusterStateList[an.getState()] + " " + an.getCategory() + " " + an.getKey());
    //            bool clusterFound = false;
    //            for ( auto &stat: serverList )
    //            {
    //                if (stat.app == an.getKey())
    //                {
    //                    if (stat.state != an.getState())
    //                    {
    //                        stat.state = an.getState();
    //                        QString mess = an.getKey() + " changed state to " + clusterStateList[an.getState()];
    //                        trace(QString("On notify: %1").arg(mess));
    //                    }
    //                    clusterFound = true;
    //                    break;
    //                }
    //            }
    //            if ( !clusterFound )
    //            {
    //                // We have received notification from a previously unknown station - so report on it
    //                ClusterServer s;
    //                s.routerName = an.getPublisherRouter();
    //                s.state = an.getState();
    //                s.app = an.getKey();
    //                s.publisherProgram = an.getPublisherProgram();
    //                serverList.push_back( s );
    //                trace(QString("clusterServerRpc: routerName = %1, app = %2, publisher programe = %3").arg(s.routerName, s.app, s.publisherProgram));
    //                QString mess = an.getKey() + " changed state to " + clusterStateList[an.getState()] + " and added";
    //                trace(mess);

    //            }
    //        }
    //    }
}


void PcCwKeyerRpc::publishState( const QString &raw, const QString &state )
{

    RPCPubSub::publish( rpcConstants::pcCwKeyerCategory, rpcConstants::pcCwKeyerReport, raw + "<>" + state, psPublished );

}

