#include <QObject>


#include "MinosRPC.h"
#include "MinosLoggerEvents.h"
#include "base_pch.h"
#include "clusterrpc.h"
#include "clustercommon.h"

Clusterrpc::Clusterrpc()
{
    MinosRPC *rpc = MinosRPC::getMinosRPC();

    QStringList sv = {rpcConstants::clusterClientServer, rpcConstants::qrzServerApp};
    rpc->initialiseRouters(sv);

    connect(rpc, &MinosRPC::routerCall, this, &Clusterrpc::on_routerCall);
    connect(rpc, &MinosRPC::notify, this, &Clusterrpc::on_notify);

    QString a = rpc->getAppName();
    QString station = MinosConfig::getMinosConfig()->getThisRouterName();
    RPCPubSub::publish(rpcConstants::clusterApp,  a + "@" + station, "", psPublished);


}

Clusterrpc::~Clusterrpc()
{

}


int Clusterrpc::getServerListCount()
{
    return serverList.count();
}



//---------------------------------------------------------------------------

void Clusterrpc::sendDXSpot(QString spot, QString uuid, int frameId)
{
    // We need to send the message to all connected cluster clients, except the spot server
    for ( auto const &s: qAsConst(serverList) )
    {

        trace(QString("SendDxSpot to station = %1").arg(s.app));
        RPCGeneralClient rpc(rpcConstants::clusterMethod);
        QSharedPointer<RPCParam>st(new RPCParamStruct);
        st->addMember( spot, rpcConstants::sendClusterSpot );
        st->addMember(uuid, rpcConstants::loggerUuid);
        st->addMember(frameId, rpcConstants::clusterFrameId);
        rpc.getCallArgs() ->addParam( st );
        rpc.queueCall( s.app );

    }
}

void Clusterrpc::askQrzServerForQra(QString dxCall, QString spotterCall)
{
    for (auto const &s: qAsConst(serverList))
    {
        if (s.publisherProgram == "QrzServer")
        {
            trace(QString("Send askQrzServerForQra to station = %1").arg(s.app));
            RPCGeneralClient rpc(rpcConstants::qrzMethod);
            QSharedPointer<RPCParam>st(new RPCParamStruct);
            st->addMember("clusterAsk", rpcConstants::qrzCluster);
            st->addMember( dxCall, rpcConstants::qrzDxCallsign );
            st->addMember(spotterCall, rpcConstants::qrzSpotterCallsign);
            rpc.getCallArgs() ->addParam( st );
            rpc.queueCall( s.app );
        }
    }
}


void Clusterrpc::on_routerCall( bool err, QSharedPointer<MinosRPCObj>mro, const QString from )
{
   trace( "Cluster RPC: callback from " + from + ( err ? ":Error" : ":Normal" ) );

   if ( !err )
   {
      QSharedPointer<RPCParam> psName;
      QSharedPointer<RPCParam> psFreq;
      QSharedPointer<RPCParam> psCall;
      QSharedPointer<RPCParam> psLoc;
      QSharedPointer<RPCParam> loggerUuid;
      QSharedPointer<RPCParam> clusterFrameId;
      QSharedPointer<RPCParam> resendSpotCmd;
      QSharedPointer<RPCParam> bandmask;
      QSharedPointer<RPCParam> reconnectState;
      QSharedPointer<RPCParam> psDxCall;
      QSharedPointer<RPCParam> psDxGrid;
      QSharedPointer<RPCParam> psDxCallState;
      QSharedPointer<RPCParam> psSpotterCall;
      QSharedPointer<RPCParam> psSpotterGrid;
      QSharedPointer<RPCParam> psSpotterCallState;

      RPCArgs *args = mro->getCallArgs();

      QString paraName;
      args->getStructArgMember(0, rpcConstants::paramName, psName);
      psName->getString(paraName);
      if (paraName == rpcConstants::txSpotToCluster)
      {
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
          bool state;
          if (args->getStructArgMember(0, rpcConstants::clusterReconnect, reconnectState))
          {
              reconnectState->getBoolean(state);
              trace(QString("Cluster RPC: reconnect command to cluster = %1").arg(state));
              emit reconnectCmdFromLog(state);
          }
      }
      else if (paraName == rpcConstants::qrzClusterResponse)
      {
          QString dxCall;
          QString dxGrid;
          QString dxCallState;
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
               psDxCallState->getString(dxCallState);
               psSpotterCall->getString(spotterCall);
               psSpotterGrid->getString(spotterGrid);
               psSpotterCallState->getString(spotterState);

               emit clusterQrzResponse(dxCall, dxGrid, dxCallState, spotterCall, spotterGrid, spotterState);

          }



      }


      mro->clearCallArgs();
      QSharedPointer<RPCParam>st(new RPCParamStruct);


   }
}



void Clusterrpc::on_notify(AnalysePubSubNotify an, const QString /*from*/ )
{
    if ( an.getOK() )
    {
        if ( an.getCategory() == rpcConstants::clusterClientServer || an.getCategory() == rpcConstants::qrzServerApp )
        {
            trace( QString("***") + clusterStateList[an.getState()] + " " + an.getCategory() + " " + an.getKey());
            bool clusterFound = false;
            for ( auto &stat: serverList )
            {
                if (stat.app == an.getKey())
                {
                    if (stat.state != an.getState())
                    {
                        stat.state = an.getState();
                        QString mess = an.getKey() + " changed state to " + clusterStateList[an.getState()];
                        trace(QString("On notify: %1").arg(mess));
                    }
                    clusterFound = true;
                    break;
                }
            }
            if ( !clusterFound )
            {
                // We have received notification from a previously unknown station - so report on it
                ClusterServer s;
                s.routerName = an.getPublisherRouter();
                s.state = an.getState();
                s.app = an.getKey();
                s.publisherProgram = an.getPublisherProgram();
                serverList.push_back( s );
                QString mess = an.getKey() + " changed state to " + clusterStateList[an.getState()] + " and added";
                trace(mess);

            }
        }
    }
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

