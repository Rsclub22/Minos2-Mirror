/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#include "minos_pch.h"
#include "PubSubServer.h"

QVector<RPCServerSubscriber *> serverSubscribeList;

RPCServerPubSub::RPCServerPubSub( const QString &call, TRPCFunctor *cb ) : RPCPubSub( call, cb )
{}
RPCServerPubSub::~RPCServerPubSub()
{}
/*static*/
void RPCServerPubSub::initialisePubSub( TRPCFunctor *notifycb )
{
   static bool objAdded = false;
   if ( !objAdded )
   {
      RPCPubSub::initialisePubSub( notifycb );
      objAdded = true;
   }
}
void RPCServerPubSub::serverSubscribeRemote( const QString &server, const QString &category )
{
   RPCServerSubscriber::testAndSubscribe( server, category );
}
void RPCServerPubSub::serverReconnectRemotePubSub( const QString &server )
{
   for ( auto const &s: qAsConst(serverSubscribeList ))
   {
      if ( s && s->getServer() == server )
      {
         s->serverReSubscribe();
      }
   }
   connected = true;
}
void RPCServerPubSub::close( )
{
   RPCPubSub::close( );
   for ( auto &s: serverSubscribeList )
   {
      delete s;
      s = nullptr;
   }
   serverSubscribeList.clear();
}

//==================================================================================
bool RPCServerSubscriber::isRemoteEqual( const QString &pServer, const QString &cat )
{
   return server == pServer && isEqual( cat );
}
RPCServerSubscriber::RPCServerSubscriber(const QString &server, const QString &cat)
    : RPCSubscriber( cat ), server( server )
{}

void RPCServerSubscriber::testAndSubscribe( const QString &server, const QString &cat )
{
    RPCServerSubscriber * sub = nullptr;
    for ( auto const &s: qAsConst(serverSubscribeList ))
   {
      if ( s ->isRemoteEqual( server, cat ) )
      {
         sub = s;
      }
   }
   if ( !sub )
   {
      sub = new RPCServerSubscriber( server, cat );
      serverSubscribeList.push_back( sub );
   }
   //if ( RPCPubSub::isConnected() )
   {
      sub->serverReSubscribe();
   }
}
// causes the server to resubscribe to the remote server
void RPCServerSubscriber::serverReSubscribe()
{
   RPCServerSubscribeClient rsc( nullptr );
   QSharedPointer<RPCParam>st(new RPCParamStruct);
   QSharedPointer<RPCParam>sServer(new RPCStringParam( server ));
   QSharedPointer<RPCParam>sCat(new RPCStringParam( category ));
   st->addMember( sServer, "Server" );
   st->addMember( sCat, "Category" );
   rsc.getCallArgs() ->addParam( st );
   rsc.queueCall( server );       // localhost just causes the server to loop
}

QString RPCServerSubscriber::getServer()
{
    return server;
}


RPCServerNotifyServer::RPCServerNotifyServer(TRPCFunctor *cb) : MinosRPCServer( rpcConstants::serverNotify, cb )
{}

RPCServerNotifyServer::~RPCServerNotifyServer()
{}

QSharedPointer<MinosRPCObj> RPCServerNotifyServer::makeObj()
{
    return QSharedPointer<MinosRPCObj>(new RPCServerNotifyServer( callback ));
}

RPCServerNotifyClient::RPCServerNotifyClient(TRPCFunctor *cb) : MinosRPCClient( rpcConstants::serverNotify, cb )
{}

RPCServerNotifyClient::~RPCServerNotifyClient()
{}

QSharedPointer<MinosRPCObj> RPCServerNotifyClient::makeObj()
{
    return QSharedPointer<MinosRPCObj>(new RPCServerNotifyClient( callback ));
}

RPCClientNotifyClient::RPCClientNotifyClient(TRPCFunctor *cb) : MinosRPCClient( rpcConstants::clientNotify, cb )
{}

RPCClientNotifyClient::~RPCClientNotifyClient()
{}

QSharedPointer<MinosRPCObj> RPCClientNotifyClient::makeObj()
{
    return QSharedPointer<MinosRPCObj>(new RPCClientNotifyClient( callback ));
}

RPCRemoteSubscribeServer::RPCRemoteSubscribeServer(TRPCFunctor *cb) : MinosRPCServer( rpcConstants::remoteSubscribe, cb )
{}

RPCRemoteSubscribeServer::~RPCRemoteSubscribeServer()
{}

QSharedPointer<MinosRPCObj> RPCRemoteSubscribeServer::makeObj()
{
    return QSharedPointer<MinosRPCObj>(new RPCRemoteSubscribeServer( callback ) );
}

RPCSubscribeServer::RPCSubscribeServer(TRPCFunctor *cb) : MinosRPCServer( rpcConstants::subscribe, cb )
{}

RPCSubscribeServer::~RPCSubscribeServer()
{}

QSharedPointer<MinosRPCObj> RPCSubscribeServer::makeObj()
{
    return QSharedPointer<MinosRPCObj>(new RPCSubscribeServer( callback ) );
}

RPCPublishServer::RPCPublishServer(TRPCFunctor *cb) : MinosRPCServer( rpcConstants::publish, cb )
{}

RPCPublishServer::~RPCPublishServer()
{}

QSharedPointer<MinosRPCObj> RPCPublishServer::makeObj()
{
    return QSharedPointer<MinosRPCObj>(new RPCPublishServer( callback ));
}

RPCRServerSubscribeServer::RPCRServerSubscribeServer(TRPCFunctor *cb) : MinosRPCServer( rpcConstants::serverSubscribe, cb )
{}

RPCRServerSubscribeServer::~RPCRServerSubscribeServer()
{}

QSharedPointer<MinosRPCObj> RPCRServerSubscribeServer::makeObj()
{
    return QSharedPointer<MinosRPCObj>(new RPCRServerSubscribeServer( callback ) );
}

RPCServerSubscribeClient::RPCServerSubscribeClient(TRPCFunctor *cb) : RPCPubSub( rpcConstants::serverSubscribe, cb )
{}

RPCServerSubscribeClient::~RPCServerSubscribeClient()
{}

QSharedPointer<MinosRPCObj> RPCServerSubscribeClient::makeObj()
{
    return QSharedPointer<MinosRPCObj>(new RPCServerSubscribeClient( callback ) );
}
