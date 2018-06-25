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
   for ( QVector<RPCServerSubscriber *>::iterator i = serverSubscribeList.begin(); i != serverSubscribeList.end(); i++ )
   {
      if ( ( *i ) && ( *i ) ->getServer() == server )
      {
         ( *i ) ->serverReSubscribe();
      }
   }
   connected = true;
}
void RPCServerPubSub::close( )
{
   RPCPubSub::close( );
   for ( QVector<RPCServerSubscriber *>::iterator i = serverSubscribeList.begin(); i != serverSubscribeList.end(); i++ )
   {
      delete ( *i );
      ( *i ) = nullptr;
   }
   serverSubscribeList.clear();
}

//==================================================================================
bool RPCServerSubscriber::isRemoteEqual( const QString &pServer, const QString &category )
{
   return server == pServer && isEqual( category );
}
void RPCServerSubscriber::testAndSubscribe( const QString &server, const QString &category )
{
   RPCServerSubscriber * sub = nullptr;
   for ( QVector<RPCServerSubscriber *>::iterator i = serverSubscribeList.begin(); i != serverSubscribeList.end(); i++ )
   {
      if ( ( *i ) ->isRemoteEqual( server, category ) )
      {
         sub = ( *i );
      }
   }
   if ( !sub )
   {
      sub = new RPCServerSubscriber( server, category );
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

