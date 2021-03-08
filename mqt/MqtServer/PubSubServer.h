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

#ifndef PubSubServerH
#define PubSubServerH
//---------------------------------------------------------------------------
class RPCRouterPubSub : public RPCPubSub
{
   protected:
      RPCRouterPubSub( const QString &call, TRPCFunctor *cb );
      virtual ~RPCRouterPubSub();
      virtual QSharedPointer<MinosRPCObj> makeObj() = 0;
   public:
      static void initialisePubSub( TRPCFunctor *notifycb );

      static void routerSubscribeRemote( const QString &router, const QString &category );
      static void routerReconnectRemotePubSub( const QString &router );
      static void close( );
};
//---------------------------------------------------------------------------
// this is the router/router subscription
class RPCRouterSubscriber : public RPCSubscriber
{
   private:
      QString router;
      RPCRouterSubscriber( const QString &router, const QString &category );
   public:
      static void testAndSubscribe( const QString &router, const QString &category );
      virtual bool isRemoteEqual( const QString &router, const QString &category );
      virtual void routerReSubscribe();
      QString getRouter();
};
extern QVector<RPCRouterSubscriber *> routerSubscribeList;
//---------------------------------------------------------------------------
class RPCRouterSubscribeClient: public RPCPubSub
{
   public:
      RPCRouterSubscribeClient( TRPCFunctor *cb );
      ~RPCRouterSubscribeClient();
      virtual QSharedPointer<MinosRPCObj> makeObj();
};
class RPCRouterSubscribeRouter: public MinosRPCRouter
{
   public:
      RPCRouterSubscribeRouter( TRPCFunctor *cb );
      ~RPCRouterSubscribeRouter();
      virtual QSharedPointer<MinosRPCObj> makeObj();

};
//---------------------------------------------------------------------------
class RPCPublishRouter: public MinosRPCRouter
{
   public:
      RPCPublishRouter( TRPCFunctor *cb );
      ~RPCPublishRouter();
      virtual QSharedPointer<MinosRPCObj> makeObj();
};
class RPCSubscribeRouter: public MinosRPCRouter
{
   public:
      RPCSubscribeRouter( TRPCFunctor *cb );
      ~RPCSubscribeRouter();
      virtual QSharedPointer<MinosRPCObj> makeObj();

};
//---------------------------------------------------------------------------
class RPCRemoteSubscribeRouter: public MinosRPCRouter
{
   public:
      RPCRemoteSubscribeRouter( TRPCFunctor *cb );
      ~RPCRemoteSubscribeRouter();
      virtual QSharedPointer<MinosRPCObj> makeObj();

};


//---------------------------------------------------------------------------

class RPCClientNotifyClient: public MinosRPCClient
{
   public:
      RPCClientNotifyClient( TRPCFunctor *cb ); // base class "callback" gets set to cb
      ~RPCClientNotifyClient();
      virtual QSharedPointer<MinosRPCObj> makeObj();
};

//---------------------------------------------------------------------------

class RPCRouterNotifyClient: public MinosRPCClient
{
   public:
      RPCRouterNotifyClient( TRPCFunctor *cb ); // base class "callback" gets set to cb
      ~RPCRouterNotifyClient();
      virtual QSharedPointer<MinosRPCObj>makeObj();
};
class RPCRouterNotifyRouter: public MinosRPCRouter
{
   public:
      RPCRouterNotifyRouter( TRPCFunctor *cb ); // base class "callback" gets set to cb
      ~RPCRouterNotifyRouter();
      virtual QSharedPointer<MinosRPCObj> makeObj();
};
//---------------------------------------------------------------------------
#endif
