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

                if (paraName == rpcConstants::cwMessageToPcCwKeyer)
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
                else if (paraName == rpcConstants::cwStopToPcCwKeyer)
                {
                    trace(QString("PcCwKeyer RPC: callback from %1 paraName = %2").arg(mName, paraName));


                    QSharedPointer<RPCParam> stopCwCmd;
                    QString stopCwStr;

                    if (args->getStructArgMember(0, rpcConstants::pcCwKeyerStopMsg, stopCwCmd))
                    {
                        stopCwCmd->getString(stopCwStr);

                        trace(QString("PcCwKeyer RPC: Stop Cw command = %1").arg(stopCwStr));
                        emit cwStopCommandFromLogger(stopCwStr);

                    }


                }
                else if (paraName == rpcConstants::cwWpmToPcCwKeyer)
                {
                    trace(QString("PcCwKeyer RPC: callback from %1 paraName = %2").arg(mName, paraName));

                    QSharedPointer<RPCParam> wpmFromLogger;
                    int wpmFromLoggerValue;

                    if (args->getStructArgMember(0, rpcConstants::pcCwKeyerWpm, wpmFromLogger))
                    {
                        wpmFromLogger->getInt(wpmFromLoggerValue);
                        trace(QString("PcCwKeyer RPC: new wpm from logger = %1").arg(QString::number(wpmFromLoggerValue)));
                        emit wpmFromLog(wpmFromLoggerValue);

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
    trace(QString("PcCwKeyer: on_provider - routerName = %1, app = %2").arg(p.routerName, p.app));
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


void PcCwKeyerRpc::publishState( const QString &comport, const QString &state, const QString &errorMsg )
{
    trace(QString("PCCwKeyerRPc: publishState comport = %1, state = %2, errorMsg = %3").arg(comport).arg(state).arg(errorMsg));
    RPCPubSub::publish( rpcConstants::pcCwKeyerCategory, rpcConstants::pcCwKeyerReport, comport + "<>" + state  + "<>" + errorMsg, psPublished );

}

void PcCwKeyerRpc::publishPttEnable(bool state)
{
    QString stateStr;
    if (state)
    {
        stateStr = "On";

    }
    else
    {
        stateStr = "Off";
    }
    trace(QString("PCCwKeyerRPc: publishPttEnable = %1").arg(stateStr));
    RPCPubSub::publish( rpcConstants::pcCwKeyerCategory, rpcConstants::pcCwKeyerPttEnabled, stateStr, psPublished );
}

void PcCwKeyerRpc::publishTxOn(const QString txOn)
{
    trace(QString("PCCwKeyerRPc: publishTxOn = %1").arg(txOn));
    RPCPubSub::publish( rpcConstants::pcCwKeyerCategory, rpcConstants::pcCwKeyerTxOn, txOn, psPublished );
}


void PcCwKeyerRpc::publishWpm(const int wpm)
{
    trace(QString("PCCwKeyerRPc: publishWpm = %1").arg(QString::number(wpm)));
    RPCPubSub::publish( rpcConstants::pcCwKeyerCategory, rpcConstants::pcCwWpmToLog, QString::number(wpm), psPublished );
}

