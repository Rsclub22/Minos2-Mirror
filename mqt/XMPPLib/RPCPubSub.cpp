/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#include "XMPP_pch.h"
#include "ConfigFile.h"

//---------------------------------------------------------------------------
bool RPCPubSub::connected = false;
//---------------------------------------------------------------------------
RPCPubSub::~RPCPubSub()
{}
//---------------------------------------------------------------------------
/*static*/
void RPCPubSub::initialisePubSub( TRPCFunctor *notifycb )
{
   static bool objAdded = false;
   if ( !objAdded )
   {
      objAdded = true;
      MinosRPCObj::addRouterObj( QSharedPointer<MinosRPCObj>(new RPCNotifyRouter( notifycb )) );
   }
}
void RPCPubSub::publish( const QString &category, const QString &key, const QString &value, PublishState pState )
{
   RPCPublisher::testAndPublish( category, key, value, pState );
}

void RPCPubSub::subscribe( const QString &category )
{
   RPCSubscriber::testAndSubscribe( category );
}
void RPCPubSub::subscribeRemote( const QString &router, const QString &category )
{
   RPCRemoteSubscriber::testAndSubscribe( router, category );
}
void RPCPubSub::reconnectPubSub( )
{
   // iterate the publish and subscribe objects and re-do them
   for ( auto const &i: qAsConst(publishList ))
   {
      if ( i )
      {
         i->rePublish();
      }
   }
   for ( auto const &i: qAsConst(subscribeList ))
   {
      if ( i )
      {
         i->reSubscribe();
      }
   }
   connected = true;
}

void RPCPubSub::close( )
{
   for ( auto &i: publishList )
   {
      delete ( i );
      i = nullptr;
   }
   publishList.clear();
   for ( auto &i: subscribeList )
   {
      delete ( i );
      i = nullptr;
   }
   subscribeList.clear();
}
//---------------------------------------------------------------------------

