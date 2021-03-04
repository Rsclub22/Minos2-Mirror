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

#ifndef RPCPubSubH
#define RPCPubSubH 
#include "XMPPRPCObj.h"
//---------------------------------------------------------------------------

class RPCPubSub : public MinosRPCClient
{
   protected:
      static bool connected;
      RPCPubSub( const QString &call, TRPCFunctor *cb ) : MinosRPCClient( call, cb )
      {}
      virtual ~RPCPubSub() override;

      virtual QSharedPointer<MinosRPCObj>makeObj() override = 0;
   public:
      static bool isConnected()
      {
         return connected;
      }

      static void subscribe( const QString &category );
      static void subscribeRemote( const QString &router, const QString &category );

      static void publish( const QString &category, const QString &key, const QString &value, PublishState pState );

      static void initialisePubSub( TRPCFunctor *notifycb );
      static void reconnectPubSub( );
      static void close( );
};

//---------------------------------------------------------------------------
#endif
