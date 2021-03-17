#include <QObject>


#include "MinosRPC.h"
#include "MinosLoggerEvents.h"
#include "base_pch.h"
#include "clusterrpc.h"
#include "clustercommon.h"

Clusterrpc::Clusterrpc()
{
    MinosRPC *rpc = MinosRPC::getMinosRPC();

    QStringList sv;
    rpc->findProviders(rpcConstants::clusterClientServer, sv);
    rpc->findProviders(rpcConstants::qrzServerApp, sv);

    connect(rpc, &MinosRPC::routerCall, this, &Clusterrpc::on_routerCall);
    connect(rpc, &MinosRPC::notify, this, &Clusterrpc::on_notify);
    connect(rpc, &MinosRPC::provider, this, &Clusterrpc::on_provider);

    QString a = rpc->getAppName();
    QString station = MinosConfig::getMinosConfig()->getThisRouterName();
    RPCPubSub::publish(rpcConstants::clusterApp,  a + "@" + station, "", psPublished);


}

Clusterrpc::~Clusterrpc()
{

}


int Clusterrpc::getServerListCount()
{
    MinosRPC *rpc = MinosRPC::getMinosRPC();
    return rpc->getProviders().count();
}



//---------------------------------------------------------------------------

void Clusterrpc::sendDXSpot(QString spot, QString uuid, int frameId)
{
    // We need to send the message to all connected cluster clients, except the spot server
    MinosRPC *rpc = MinosRPC::getMinosRPC();
    for ( auto const &p: rpc->getProviders() )
    {
        for (auto const &s: p)
        {
            trace(QString("SendDxSpot to station = %1").arg(s.app));
            RPCGeneralClient rpc(rpcConstants::clusterMethod);
            QSharedPointer<RPCParam>st(new RPCParamStruct);
            st->addMember( spot, rpcConstants::sendClusterSpot );
            st->addMember(uuid, rpcConstants::loggerUuid);
            st->addMember(frameId, rpcConstants::clusterFrameId);
            rpc.getCallArgs() ->addParam( st );
            rpc.queueCall( s.psn() );
        }
    }
}

void Clusterrpc::askQrzServerForQra(QString dxCall, QString spotterCall)
{
    MinosRPC *rpc = MinosRPC::getMinosRPC();
    for ( auto p = rpc->getProviders().begin(); p != rpc->getProviders().end(); p++ )
    {
        QString cat = p.key();
        for (auto const &s: (*p))
        {
            if (cat == rpcConstants::qrzServerApp)
            {
                trace(QString("Send askQrzServerForQra to station = %1").arg(s.app));
                RPCGeneralClient rpc(rpcConstants::qrzMethod);
                QSharedPointer<RPCParam>st(new RPCParamStruct);
                st->addMember("clusterAsk", rpcConstants::qrzCluster);
                st->addMember( dxCall, rpcConstants::qrzDxCallsign );
                st->addMember(spotterCall, rpcConstants::qrzSpotterCallsign);
                rpc.getCallArgs() ->addParam( st );
                rpc.queueCall( s.psn() );
            }
        }
    }
}


void Clusterrpc::on_routerCall( bool err, QSharedPointer<MinosRPCObj>mro, const QString from )
{
   trace( "Cluster RPC: callback from " + from + ( err ? ":Error" : ":Normal" ) );

   if ( !err )
   {
      QString mName = mro->getMethodName();

      if (mName == rpcConstants::clusterMethod)
      {

          RPCArgs *args = mro->getCallArgs();
          QSharedPointer<RPCParam> psName;

          if (args->getStructArgMember(0, rpcConstants::paramName, psName))
          {
              QString paraName;
              psName->getString(paraName);

              if (paraName == rpcConstants::txSpotToCluster)
              {
                  trace(QString("Cluster RPC: callback from %1 paraName = %2").arg(mName, paraName));

                  QSharedPointer<RPCParam> psFreq;
                  QSharedPointer<RPCParam> psCall;
                  QSharedPointer<RPCParam> psLoc;

                  QString freq;
                  QString call;
                  QString loc;

                  if (args->getStructArgMember(0, rpcConstants::txSpotParamFreq, psFreq)
                          && args->getStructArgMember(0, rpcConstants::txSpotParamCallsign, psCall)
                          && args->getStructArgMember(0, rpcConstants::txSpotParamLocator, psLoc))
                  {
                      psFreq->getString(freq);
                      psCall->getString(call);
                      psLoc->getString(loc);

                      trace(QString("Cluster RPC: send spot to cluster node, call = %1, loc = %2, freq = %3").arg(call, loc, freq));

                      emit sendSpotToDXCluster(Frequency(freq), call, loc);


                  }


              }
              else if (paraName == rpcConstants::clusterResendSpots)
              {
                  trace(QString("Cluster RPC: callback from %1 paraName = %2").arg(mName, paraName));


                  QSharedPointer<RPCParam> resendSpotCmd;
                  QSharedPointer<RPCParam> clusterFrameId;
                  QSharedPointer<RPCParam> bandmask;
                  QSharedPointer<RPCParam> loggerUuid;
                  QString cmd;
                  QString logUuid;
                  QString bandMask;
                  int frameId;
                  if (args->getStructArgMember(0, rpcConstants::clusterResendSpotsCmd, resendSpotCmd)
                          && args->getStructArgMember(0, rpcConstants::clusterFrameId, clusterFrameId)
                          && args->getStructArgMember(0, rpcConstants::loggerUuid, loggerUuid)
                           && args->getStructArgMember(0, rpcConstants::clusterBandmask, bandmask))
                  {
                      resendSpotCmd->getString(cmd);
                      loggerUuid->getString(logUuid);
                      bandmask->getString(bandMask);
                      clusterFrameId->getInt(frameId);


                      trace(QString("Cluster RPC: resendspots command to cluster = %1, bandmask = %2, from loggerUuid = %3, frameId %4").arg(cmd).arg(bandMask).arg(logUuid).arg(frameId));
                      emit resendSpotToClients(frameId, logUuid, cmd, bandMask);

                  }


              }
              else if (paraName == rpcConstants::clusterReconnect)
              {
                  trace(QString("Cluster RPC: callback from %1 paraName = %2").arg(mName, paraName));


                  QSharedPointer<RPCParam> reconnectState;
                  bool state;

                  if (args->getStructArgMember(0, rpcConstants::clusterReconnect, reconnectState))
                  {
                      reconnectState->getBoolean(state);
                      trace(QString("Cluster RPC: reconnect command to cluster = %1").arg(state));
                      emit reconnectCmdFromLog(state);
                  }
              }

          }

      }
      else if (mName == rpcConstants::qrzMethod)
      {
          RPCArgs *args = mro->getCallArgs();
          QSharedPointer<RPCParam> psName;

          if (args->getStructArgMember(0, rpcConstants::paramName, psName))
          {

                QString paraName;
                psName->getString(paraName);

                if (paraName == rpcConstants::qrzClusterResponse)
                {
                    trace(QString("Cluster RPC: callback from %1 paraName = %2").arg(mName, paraName));

                    QSharedPointer<RPCParam> psDxCall;
                    QSharedPointer<RPCParam> psDxGrid;
                    QSharedPointer<RPCParam> psDxCallState;
                    QSharedPointer<RPCParam> psSpotterCall;
                    QSharedPointer<RPCParam> psSpotterGrid;
                    QSharedPointer<RPCParam> psSpotterCallState;

                    QString dxCall;
                    QString dxGrid;
                    QString callState;
                    QString spotterCall;
                    QString spotterGrid;
                    QString spotterState;


                    if (args->getStructArgMember(0, rpcConstants::qrzDxCallsign, psDxCall)
                        && args->getStructArgMember(0, rpcConstants::qrzDxGrid, psDxGrid)
                        && args->getStructArgMember(0, rpcConstants::qrzDxReplyState, psDxCallState)
                        && args->getStructArgMember(0, rpcConstants::qrzSpotterCallsign, psSpotterCall)
                        && args->getStructArgMember(0, rpcConstants::qrzSpotterGrid, psSpotterGrid)
                        && args->getStructArgMember(0, rpcConstants::qrzSpotterReplyState, psSpotterCallState))
                    {
                         psDxCall->getString(dxCall);
                         psDxGrid->getString(dxGrid);
                         psDxCallState->getString(callState);
                         psSpotterCall->getString(spotterCall);
                         psSpotterGrid->getString(spotterGrid);
                         psSpotterCallState->getString(spotterState);

                         emit clusterQrzResponse(dxCall, dxGrid, callState, spotterCall, spotterGrid, spotterState);

                    }

                }
                else if (paraName == rpcConstants::qrzServerState)
                {
                    trace(QString("Cluster RPC: callback from %1 paraName = %2").arg(mName, paraName));


                    QSharedPointer<RPCParam> msgQrzLogonState;
                    QSharedPointer<RPCParam> msgQrzServerMessage;
                    bool loggedState = false;


                    if (args->getStructArgMember(0, rpcConstants::qrzServerLogonState, msgQrzLogonState)
                        && args->getStructArgMember(0, rpcConstants::qrzServerStateMessage, msgQrzServerMessage))

                    {
                        QString logStateStr;
                        QString stateMessage;

                        msgQrzLogonState->getString(logStateStr);
                        if (logStateStr == rpcConstants::qrzServerLoggedIn)
                        {
                            loggedState = true;
                        }

                        msgQrzServerMessage->getString(stateMessage);

                        emit qrzServerLoggedState(loggedState, stateMessage);
                    }


                }

            }

      }


      mro->clearCallArgs();
      QSharedPointer<RPCParam>st(new RPCParamStruct);


   }
}

void Clusterrpc::on_provider(Provider p  )
{
    trace(QString("clusterServer: on_provider - routerName = %1, app = %2").arg(p.routerName, p.app));
}
void Clusterrpc::on_notify(AnalysePubSubNotify /*an*/, const QString /*from*/ )
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
//---------------------------------------------------------------------------
void Clusterrpc::publishState( const QString &raw, const QString &state )
{

  RPCPubSub::publish( rpcConstants::clusterCategory, rpcConstants::clusterReport, raw + "<>" + state, psPublished );

}

void Clusterrpc::publishTXEnable(const QString txOnOff)
{
    static QString old;

    if (txOnOff != old)
    {
        old = txOnOff;
        RPCPubSub::publish( rpcConstants::clusterCategory, rpcConstants::clusterTXSpotEnableState, txOnOff, psPublished );

    }
}

