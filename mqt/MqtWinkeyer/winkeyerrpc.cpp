/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Winkeyer Server
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2025
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////


#include "MinosRPC.h"
#include "ConfigFile.h"
#include "RPCCommandConstants.h"
#include "RPCPubSub.h"
#include "MTrace.h"
#include "winkeyerrpc.h"




WinkeyerRpc::WinkeyerRpc()
{

    MinosRPC *rpc = MinosRPC::getMinosRPC();


    QStringList sv;

    rpc->findProviders(rpcConstants::pcCwKeyerCategory, sv);


    connect(rpc, &MinosRPC::routerCall, this, &WinkeyerRpc::on_routerCall);
    connect(rpc, &MinosRPC::notify, this, &WinkeyerRpc::on_notify);
    connect(rpc, &MinosRPC::provider, this, &WinkeyerRpc::on_provider);


    QString a = rpc->getAppName();
    QString station = MinosConfig::getMinosConfig()->getThisRouterName();
    RPCPubSub::publish(rpcConstants::winKeyerApp,  a + "@" + station, "", psPublished);

}

WinkeyerRpc::~WinkeyerRpc()
{
}

int WinkeyerRpc::getServerListCount()
{
    MinosRPC *rpc = MinosRPC::getMinosRPC();
    return rpc->getProviders().count();
}


void WinkeyerRpc::on_routerCall( bool err, QSharedPointer<MinosRPCObj>mro, const QString /*from*/ )
{
    //trace( "WinkeyerRpc RPC: callback from " + from + ( err ? ":Error" : ":Normal" ) );

    if ( !err )
    {
        QString mName = mro->getMethodName();

        if (mName == rpcConstants::winKeyerMethod)
        {

            RPCArgs *args = mro->getCallArgs();
            QSharedPointer<RPCParam> psName;

            if (args->getStructArgMember(0, rpcConstants::paramName, psName))
            {
                QString paraName;
                psName->getString(paraName);

                if (paraName == rpcConstants::cwMessageToWinKeyer)
                {
                    trace(QString("PcCwKeyerRpc RPC: callback from %1 paraName = %2").arg(mName, paraName));

                    QSharedPointer<RPCParam> cwMsg;

                    QString cwText;


                    if (args->getStructArgMember(0, rpcConstants::winKeyerCwMessage, cwMsg))
                    {
                        cwMsg->getString(cwText);

                        trace(QString("PcCwKeyer RPC: cw message from logger = %1").arg(cwText));

                        emit cwMessageFromLoggerToKeyer(cwText);


                    }


                }
                else if (paraName == rpcConstants::cwStopToWinKeyer)
                {
                    trace(QString("PcCwKeyer RPC: callback from %1 paraName = %2").arg(mName, paraName));


                    QSharedPointer<RPCParam> stopCwCmd;
                    QString stopCwStr;

                    if (args->getStructArgMember(0, rpcConstants::winKeyerStopMsg, stopCwCmd))
                    {
                        stopCwCmd->getString(stopCwStr);

                        trace(QString("PcCwKeyer RPC: Stop Cw command = %1").arg(stopCwStr));
                        emit cwStopCommandFromLogger(stopCwStr);

                    }


                }
                else if (paraName == rpcConstants::cwWpmToWinKeyer)
                {
                    trace(QString("PcCwKeyer RPC: callback from %1 paraName = %2").arg(mName, paraName));

                    QSharedPointer<RPCParam> wpmFromLogger;
                    int wpmFromLoggerValue;

                    if (args->getStructArgMember(0, rpcConstants::winKeyerWpm, wpmFromLogger))
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

void WinkeyerRpc::on_provider(Provider p, QString /*cat*/  )
{
    trace(QString("WinKeyer: on_provider - routerName = %1, app = %2").arg(p.routerName, p.app));
}


void WinkeyerRpc::on_notify(AnalysePubSubNotify /*an*/, const QString /*from*/ )
{

}


void WinkeyerRpc::publishState( const QString &comport, const QString &state, const QString &errorMsg )
{
    trace(QString("PCCwKeyerRPc: publishState comport = %1, state = %2, errorMsg = %3").arg(comport).arg(state).arg(errorMsg));
    RPCPubSub::publish( rpcConstants::winKeyerCategory, rpcConstants::winKeyerReport, comport + "<>" + state  + "<>" + errorMsg, psPublished );

}

void WinkeyerRpc::publishPttEnable(bool state)
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
    RPCPubSub::publish( rpcConstants::winKeyerCategory, rpcConstants::winKeyerPttEnabled, stateStr, psPublished );
}

void WinkeyerRpc::publishTxOn(const QString txOn)
{
    trace(QString("PCCwKeyerRPc: publishTxOn = %1").arg(txOn));
    RPCPubSub::publish( rpcConstants::winKeyerCategory, rpcConstants::winKeyerTxOn, txOn, psPublished );
}


void WinkeyerRpc::publishWpm(const int wpm)
{
    trace(QString("PCCwKeyerRPc: publishWpm = %1").arg(QString::number(wpm)));
    RPCPubSub::publish( rpcConstants::winKeyerCategory, rpcConstants::winWpmToLog, QString::number(wpm), psPublished );
}











