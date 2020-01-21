#include <QObject>


#include "MinosRPC.h"
#include "MinosLoggerEvents.h"
#include "base_pch.h"
#include "clusterrpc.h"
#include "clustercommon.h"

Clusterrpc::Clusterrpc()
{
    MinosRPC *rpc = MinosRPC::getMinosRPC();

    connect(rpc, SIGNAL(serverCall(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_serverCall(bool,QSharedPointer<MinosRPCObj>,QString)));
    connect(rpc, SIGNAL(notify(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_notify(bool,QSharedPointer<MinosRPCObj>,QString)));


    RPCPubSub::subscribe(rpcConstants::LocalStationCategory);
    RPCPubSub::subscribe(rpcConstants::StationCategory);
}

Clusterrpc::~Clusterrpc()
{

}


int Clusterrpc::getServerListCount()
{
    return serverList.count();
}



//---------------------------------------------------------------------------

void Clusterrpc::sendDXSpot(QString spot)
{
    // We need to send the message to all connected cluster clients, except the spot server
    for ( QVector<ClusterServer>::iterator i = serverList.begin(); i != serverList.end(); i++ )
    {
        trace(QString("SendDxSpot to station = %1").arg((*i).app));
        RPCGeneralClient rpc(rpcConstants::clusterMethod);
        QSharedPointer<RPCParam>st(new RPCParamStruct);
        st->addMember( spot, rpcConstants::sendClusterSpot );
        rpc.getCallArgs() ->addParam( st );
        rpc.queueCall( (*i).app );
    }
}

void Clusterrpc::on_serverCall( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from )
{
   trace( "Cluster RPC: callback from " + from + ( err ? ":Error" : ":Normal" ) );

   if ( !err )
   {
      QSharedPointer<RPCParam> psName;
      QSharedPointer<RPCParam>piValue;
      QSharedPointer<RPCParam> psFreq;
      QSharedPointer<RPCParam> psCall;
      QSharedPointer<RPCParam> psLoc;
      RPCArgs *args = mro->getCallArgs();

      QString freq;
      QString call;
      QString loc;


      if (args->getStructArgMember(0, rpcConstants::txSpotParamFreq, psFreq))
      {
          if (psFreq->getString(freq))
          {
              trace(QString("Cluster RPC: freq to send to cluster = %1").arg(freq));
          }
      }
      if (args->getStructArgMember(0, rpcConstants::txSpotParamCallsign, psCall))
      {
          if (psCall->getString(call))
          {
              trace(QString("Cluster RPC: callsign to send to cluster = %1").arg(call));
          }
      }
      if (args->getStructArgMember(0, rpcConstants::txSpotParamLocator, psLoc))
      {
          if (psLoc->getString(loc))
          {
              trace(QString("Cluster RPC: locator to send to cluster = %1").arg(loc));
          }

      }

      emit sendSpotToDXCluster(freq, call, loc);

      mro->clearCallArgs();
      QSharedPointer<RPCParam>st(new RPCParamStruct);


   }
}



void Clusterrpc::on_notify(bool err, QSharedPointer<MinosRPCObj> mro, const QString &/*from*/ )
{
    AnalysePubSubNotify an( err, mro );

    if ( an.getOK() )
    {
        if (an.getCategory() == rpcConstants::StationCategory)
        {
            QString server = an.getKey();
            QVector<ClusterServer>::iterator stat;
            bool subNeeded = true;
            for ( stat = serverList.begin(); stat != serverList.end(); stat++ )
            {
                if ((*stat).serverName == server)
                {
                    subNeeded = false;
                    break;
                }
            }
            if (subNeeded)
            {
                RPCPubSub::subscribeRemote(server, rpcConstants::clusterClientServer);
            }
        }
        if ( an.getCategory() == rpcConstants::clusterClientServer )
        {
            trace( QString("***") + clusterStateIndicator[an.getState()] + " " + an.getCategory() + " " + an.getKey());
            QVector<ClusterServer>::iterator stat;
            bool clusterFound = false;
            for ( stat = serverList.begin(); stat != serverList.end(); stat++ )
            {
                if ((*stat).app == an.getKey())
                {
                    if ((*stat).state != an.getState())
                    {
                        (*stat).state = an.getState();
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
                s.serverName = an.getPublisherServer();
                s.state = an.getState();
                s.app = an.getKey();
                serverList.push_back( s );
                QString mess = an.getKey() + " changed state to " + clusterStateList[an.getState()] + " and added";
                trace(mess);

            }
        }
    }
}
//---------------------------------------------------------------------------
void Clusterrpc::publishState( const QString &state )
{
  //static QString old;

  //if ( state != old )
  //{
  //   old = state;
  RPCPubSub::publish( rpcConstants::clusterCategory, rpcConstants::clusterReport, state, psPublished );
  //}
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
