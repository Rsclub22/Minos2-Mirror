/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef PubSubClientH
#define PubSubClientH 
#include "RPCCommandConstants.h"
#include "MinosRPC.h"
//---------------------------------------------------------------------------
void clearPubSub();
class RPCPublishClient: public RPCPubSub
{
   public:
      RPCPublishClient( TRPCFunctor *cb ) : RPCPubSub( rpcConstants::publish, cb )
      {}    // method name should be shared
      // plus we want factory access to it
      // so it would be nice to have a static
      // member/method  that could go "up"
      ~RPCPublishClient() override;

      virtual QSharedPointer<MinosRPCObj>makeObj() override
      {
         return QSharedPointer<MinosRPCObj>(new RPCPublishClient( callback ));
      }
};

// Subscribe is a call - the subcribed thing might not be there
class RPCSubscribeClient: public RPCPubSub
{
   public:
      RPCSubscribeClient( TRPCFunctor *cb ) : RPCPubSub( rpcConstants::subscribe, cb )
      {}
      ~RPCSubscribeClient() override;

      virtual QSharedPointer<MinosRPCObj>makeObj() override
      {
         return QSharedPointer<MinosRPCObj>(new RPCSubscribeClient( callback ));
      }
};
class RPCRemoteSubscribeClient: public RPCPubSub
{
   public:
      RPCRemoteSubscribeClient( TRPCFunctor *cb ) : RPCPubSub( rpcConstants::remoteSubscribe, cb )
      {}
      virtual ~RPCRemoteSubscribeClient() override;

      virtual QSharedPointer<MinosRPCObj>makeObj() override
      {
         return QSharedPointer<MinosRPCObj>(new RPCRemoteSubscribeClient( callback ));
      }
};
class RPCNotifyServer: public MinosRPCServer
{
   public:
      RPCNotifyServer( TRPCFunctor *cb ) : MinosRPCServer( rpcConstants::clientNotify, cb )
      {}
      virtual ~RPCNotifyServer() override;

      virtual QSharedPointer<MinosRPCObj>makeObj() override
      {
         return QSharedPointer<MinosRPCObj>(new RPCNotifyServer( callback ));
      }
};
//---------------------------------------------------------------------------
class RPCSubscriber
{
   protected:
      QString category;
      RPCSubscriber( const QString &category )
            : category( category )
      {}
   public:
      static void testAndSubscribe( const QString &category );
      virtual ~RPCSubscriber(){}
      virtual void reSubscribe();
      virtual bool isEqual( const QString &category );
      virtual bool isRemoteEqual( const QString &server, const QString &category );
};
class RPCRemoteSubscriber : public RPCSubscriber
{
   private:
      QString server;
      RPCRemoteSubscriber( const QString &server, const QString &cat )
            : RPCSubscriber( cat ), server( server )
      {}
      ~RPCRemoteSubscriber() override
      {}
   public:
      static void testAndSubscribe( const QString &server, const QString &category );
      virtual bool isRemoteEqual( const QString &server, const QString &category ) override;
      virtual void reSubscribe() override;
      QString getServer()
      {
         return server;
      }
};
class RPCPublisher
{
      QString category;
      QString key;
      QString value;
      PublishState state;
      RPCPublisher( const QString &category, const QString &key, const QString &value, PublishState pState )
            : category( category ), key( key ), value( value ), state(pState)
      {}

      public:
      static void testAndPublish( const QString &category, const QString &key, const QString &value, PublishState pState );
      virtual ~RPCPublisher();
      void rePublish();

};
extern QVector<RPCSubscriber *> subscribeList;
extern QVector<RPCPublisher *> publishList;
#endif
