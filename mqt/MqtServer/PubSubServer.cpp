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

QVector<RPCRouterSubscriber *> routerSubscribeList;

RPCRouterPubSub::RPCRouterPubSub( const QString &call, TRPCFunctor *cb ) : RPCPubSub( call, cb )
{}
RPCRouterPubSub::~RPCRouterPubSub()
{}
/*static*/
void RPCRouterPubSub::initialisePubSub( TRPCFunctor *notifycb )
{
   static bool objAdded = false;
   if ( !objAdded )
   {
      RPCPubSub::initialisePubSub( notifycb );
      objAdded = true;
   }
}
void RPCRouterPubSub::routerSubscribeRemote( const QString &router, const QString &category )
{
   RPCRouterSubscriber::testAndSubscribe( router, category );
}
void RPCRouterPubSub::routerReconnectRemotePubSub( const QString &router )
{
   for ( auto const &s: qAsConst(routerSubscribeList ))
   {
      if ( s && s->getRouter() == router )
      {
         s->routerReSubscribe();
      }
   }
   connected = true;
}
void RPCRouterPubSub::close( )
{
   RPCPubSub::close( );
   for ( auto &s: routerSubscribeList )
   {
      delete s;
      s = nullptr;
   }
   routerSubscribeList.clear();
}

//==================================================================================
bool RPCRouterSubscriber::isRemoteEqual( const QString &pRouter, const QString &cat )
{
   return router == pRouter && isEqual( cat );
}
RPCRouterSubscriber::RPCRouterSubscriber(const QString &router, const QString &cat)
    : RPCSubscriber( cat ), router( router )
{}

void RPCRouterSubscriber::testAndSubscribe( const QString &router, const QString &cat )
{
    RPCRouterSubscriber * sub = nullptr;
    for ( auto const &s: qAsConst(routerSubscribeList ))
   {
      if ( s ->isRemoteEqual( router, cat ) )
      {
         sub = s;
      }
   }
   if ( !sub )
   {
      sub = new RPCRouterSubscriber( router, cat );
      routerSubscribeList.push_back( sub );
   }
   //if ( RPCPubSub::isConnected() )
   {
      sub->routerReSubscribe();
   }
}
// causes the router to resubscribe to the remote router
void RPCRouterSubscriber::routerReSubscribe()
{
   RPCRouterSubscribeClient rsc( nullptr );
   QSharedPointer<RPCParam>st(new RPCParamStruct);
   st->addMember( router, "Server" );
   st->addMember( category, "Category" );
   rsc.getCallArgs() ->addParam( st );
   rsc.queueCall( router );       // localhost just causes the router to loop
}

QString RPCRouterSubscriber::getRouter()
{
    return router;
}


RPCRouterNotifyRouter::RPCRouterNotifyRouter(TRPCFunctor *cb) : MinosRPCRouter( rpcConstants::routerNotify, cb )
{}

RPCRouterNotifyRouter::~RPCRouterNotifyRouter()
{}

QSharedPointer<MinosRPCObj> RPCRouterNotifyRouter::makeObj()
{
    return QSharedPointer<MinosRPCObj>(new RPCRouterNotifyRouter( callback ));
}

RPCRouterNotifyClient::RPCRouterNotifyClient(TRPCFunctor *cb) : MinosRPCClient( rpcConstants::routerNotify, cb )
{}

RPCRouterNotifyClient::~RPCRouterNotifyClient()
{}

QSharedPointer<MinosRPCObj> RPCRouterNotifyClient::makeObj()
{
    return QSharedPointer<MinosRPCObj>(new RPCRouterNotifyClient( callback ));
}

RPCClientNotifyClient::RPCClientNotifyClient(TRPCFunctor *cb) : MinosRPCClient( rpcConstants::clientNotify, cb )
{}

RPCClientNotifyClient::~RPCClientNotifyClient()
{}

QSharedPointer<MinosRPCObj> RPCClientNotifyClient::makeObj()
{
    return QSharedPointer<MinosRPCObj>(new RPCClientNotifyClient( callback ));
}

RPCRemoteSubscribeRouter::RPCRemoteSubscribeRouter(TRPCFunctor *cb) : MinosRPCRouter( rpcConstants::remoteSubscribe, cb )
{}

RPCRemoteSubscribeRouter::~RPCRemoteSubscribeRouter()
{}

QSharedPointer<MinosRPCObj> RPCRemoteSubscribeRouter::makeObj()
{
    return QSharedPointer<MinosRPCObj>(new RPCRemoteSubscribeRouter( callback ) );
}

RPCSubscribeRouter::RPCSubscribeRouter(TRPCFunctor *cb) : MinosRPCRouter( rpcConstants::subscribe, cb )
{}

RPCSubscribeRouter::~RPCSubscribeRouter()
{}

QSharedPointer<MinosRPCObj> RPCSubscribeRouter::makeObj()
{
    return QSharedPointer<MinosRPCObj>(new RPCSubscribeRouter( callback ) );
}

RPCPublishRouter::RPCPublishRouter(TRPCFunctor *cb) : MinosRPCRouter( rpcConstants::publish, cb )
{}

RPCPublishRouter::~RPCPublishRouter()
{}

QSharedPointer<MinosRPCObj> RPCPublishRouter::makeObj()
{
    return QSharedPointer<MinosRPCObj>(new RPCPublishRouter( callback ));
}

RPCRouterSubscribeRouter::RPCRouterSubscribeRouter(TRPCFunctor *cb) : MinosRPCRouter( rpcConstants::routerSubscribe, cb )
{}

RPCRouterSubscribeRouter::~RPCRouterSubscribeRouter()
{}

QSharedPointer<MinosRPCObj> RPCRouterSubscribeRouter::makeObj()
{
    return QSharedPointer<MinosRPCObj>(new RPCRouterSubscribeRouter( callback ) );
}

RPCRouterSubscribeClient::RPCRouterSubscribeClient(TRPCFunctor *cb) : RPCPubSub( rpcConstants::routerSubscribe, cb )
{}

RPCRouterSubscribeClient::~RPCRouterSubscribeClient()
{}

QSharedPointer<MinosRPCObj> RPCRouterSubscribeClient::makeObj()
{
    return QSharedPointer<MinosRPCObj>(new RPCRouterSubscribeClient( callback ) );
}
