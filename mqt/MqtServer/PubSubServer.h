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
class RPCServerPubSub : public RPCPubSub
{
   protected:
      RPCServerPubSub( const QString &call, TRPCFunctor *cb );
      virtual ~RPCServerPubSub();
      virtual QSharedPointer<MinosRPCObj> makeObj() = 0;
   public:
      static void initialisePubSub( TRPCFunctor *notifycb );

      static void serverSubscribeRemote( const QString &server, const QString &category );
      static void serverReconnectRemotePubSub( const QString &server );
      static void close( );
};
//---------------------------------------------------------------------------
// this is the server/server subscription
class RPCServerSubscriber : public RPCSubscriber
{
   private:
      QString server;
      RPCServerSubscriber( const QString &server, const QString &category )
            : RPCSubscriber( category ), server( server )
      {}
   public:
      static void testAndSubscribe( const QString &server, const QString &category );
      virtual bool isRemoteEqual( const QString &server, const QString &category );
      virtual void serverReSubscribe();
      QString getServer()
      {
         return server;
      }
};
extern QVector<RPCServerSubscriber *> serverSubscribeList;
//---------------------------------------------------------------------------
class RPCServerSubscribeClient: public RPCPubSub
{
   public:
      RPCServerSubscribeClient( TRPCFunctor *cb ) : RPCPubSub( rpcConstants::serverSubscribe, cb )
      {}
      ~RPCServerSubscribeClient()
      {}
      virtual QSharedPointer<MinosRPCObj> makeObj()
      {
         return QSharedPointer<MinosRPCObj>(new RPCServerSubscribeClient( callback ) );
      }
};
class RPCRServerSubscribeServer: public MinosRPCServer
{
   public:
      RPCRServerSubscribeServer( TRPCFunctor *cb ) : MinosRPCServer( rpcConstants::serverSubscribe, cb )
      {}
      ~RPCRServerSubscribeServer()
      {}
      virtual QSharedPointer<MinosRPCObj> makeObj()
      {
         return QSharedPointer<MinosRPCObj>(new RPCRServerSubscribeServer( callback ) );
      }

};
//---------------------------------------------------------------------------
class RPCPublishServer: public MinosRPCServer
{
   public:
      RPCPublishServer( TRPCFunctor *cb ) : MinosRPCServer( rpcConstants::publish, cb )
      {}
      ~RPCPublishServer()
      {}
      virtual QSharedPointer<MinosRPCObj> makeObj()
      {
         return QSharedPointer<MinosRPCObj>(new RPCPublishServer( callback ));
      }
};
class RPCSubscribeServer: public MinosRPCServer
{
   public:
      RPCSubscribeServer( TRPCFunctor *cb ) : MinosRPCServer( rpcConstants::subscribe, cb )
      {}
      ~RPCSubscribeServer()
      {}
      virtual QSharedPointer<MinosRPCObj> makeObj()
      {
         return QSharedPointer<MinosRPCObj>(new RPCSubscribeServer( callback ) );
      }

};
//---------------------------------------------------------------------------
class RPCRemoteSubscribeServer: public MinosRPCServer
{
   public:
      RPCRemoteSubscribeServer( TRPCFunctor *cb ) : MinosRPCServer( rpcConstants::remoteSubscribe, cb )
      {}
      ~RPCRemoteSubscribeServer()
      {}
      virtual QSharedPointer<MinosRPCObj> makeObj()
      {
         return QSharedPointer<MinosRPCObj>(new RPCRemoteSubscribeServer( callback ) );
      }

};


//---------------------------------------------------------------------------

class RPCClientNotifyClient: public MinosRPCClient
{
   public:
      RPCClientNotifyClient( TRPCFunctor *cb ) : MinosRPCClient( rpcConstants::clientNotify, cb )
      {} // base class "callback" gets set to cb
      ~RPCClientNotifyClient()
      {}
      virtual QSharedPointer<MinosRPCObj> makeObj()
      {
         return QSharedPointer<MinosRPCObj>(new RPCClientNotifyClient( callback ));
      }
};

//---------------------------------------------------------------------------

class RPCServerNotifyClient: public MinosRPCClient
{
   public:
      RPCServerNotifyClient( TRPCFunctor *cb ) : MinosRPCClient( rpcConstants::serverNotify, cb )
      {} // base class "callback" gets set to cb
      ~RPCServerNotifyClient()
      {}
      virtual QSharedPointer<MinosRPCObj>makeObj()
      {
         return QSharedPointer<MinosRPCObj>(new RPCServerNotifyClient( callback ));
      }
};
class RPCServerNotifyServer: public MinosRPCServer
{
   public:
      RPCServerNotifyServer( TRPCFunctor *cb ) : MinosRPCServer( rpcConstants::serverNotify, cb )
      {} // base class "callback" gets set to cb
      ~RPCServerNotifyServer()
      {}
      virtual QSharedPointer<MinosRPCObj> makeObj()
      {
         return QSharedPointer<MinosRPCObj>(new RPCServerNotifyServer( callback ));
      }
};
//---------------------------------------------------------------------------
#endif
