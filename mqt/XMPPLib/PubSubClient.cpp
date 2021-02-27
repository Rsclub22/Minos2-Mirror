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

//---------------------------------------------------------------------------
QVector<RPCSubscriber *> subscribeList;
QVector<RPCPublisher *> publishList;

void clearPubSub()
{
    for(auto const &v: qAsConst(subscribeList))
    {
        delete v;
    }
    subscribeList.clear();
    for(auto const &v: qAsConst(publishList))
    {
        delete v;
    }
    publishList.clear();
}


RPCPublishClient::~RPCPublishClient()
{}
RPCSubscribeClient::~RPCSubscribeClient()
{}
RPCRemoteSubscribeClient::~RPCRemoteSubscribeClient()
{}
RPCNotifyServer::~RPCNotifyServer()
{}
RPCPublisher::~RPCPublisher()
{}

bool RPCSubscriber::isEqual( const QString &pcategory )
{
   return ( pcategory == category );
}
bool RPCSubscriber::isRemoteEqual( const QString &/*pserver*/, const QString &/*pcategory*/ )
{
   return false;
}
void RPCSubscriber::testAndSubscribe( const QString &category )
{
   RPCSubscriber * sub = nullptr;
   for ( auto const &i: qAsConst(subscribeList ))
   {
      if ( i ->isEqual( category ) )
      {
         sub = i;
         break;
      }
   }
   if ( !sub )
   {
      sub = new RPCSubscriber( category );
      subscribeList.push_back( sub );
   }
   if ( RPCPubSub::isConnected() )
   {
      sub->reSubscribe();
   }
}
bool RPCRemoteSubscriber::isRemoteEqual( const QString &pServer, const QString &cat )
{
   return server == pServer && isEqual( cat );
}
void RPCRemoteSubscriber::testAndSubscribe( const QString &server, const QString &cat )
{
   RPCRemoteSubscriber * sub = nullptr;
   for ( auto const &i: qAsConst(subscribeList ))
   {
      if ( i->isRemoteEqual( server, cat ) )
      {
         sub = dynamic_cast<RPCRemoteSubscriber *>( i );
         if (sub)
         {
            break;
         }
      }
   }
   if ( !sub )
   {
      sub = new RPCRemoteSubscriber( server, cat );
      subscribeList.push_back( sub );
   }
   if ( RPCPubSub::isConnected() )
   {
      sub->reSubscribe();
   }
}
void RPCSubscriber::reSubscribe()
{
   RPCSubscribeClient rsc( nullptr );
   QSharedPointer<RPCParam>st(new RPCParamStruct);
   QSharedPointer<RPCParam>sServer(new RPCStringParam( "localhost" ));
   QSharedPointer<RPCParam>sCat(new RPCStringParam( category ));
   st->addMember( sServer, "Server" );
   st->addMember( sCat, "Category" );
   rsc.getCallArgs() ->addParam( st );
   rsc.queueCall( "localhost" );
}
// client resubscribing to remote event
void RPCRemoteSubscriber::reSubscribe()
{
   RPCRemoteSubscribeClient rsc( nullptr );
   QSharedPointer<RPCParam>st(new RPCParamStruct);
   QSharedPointer<RPCParam>sServer(new RPCStringParam( server ));
   QSharedPointer<RPCParam>sCat(new RPCStringParam( category ));
   st->addMember( sServer, "Server" );
   st->addMember( sCat, "Category" );
   rsc.getCallArgs() ->addParam( st );
   rsc.queueCall( "localhost" );       // localhost just causes the server to loop
}

void RPCPublisher::testAndPublish( const QString &category, const QString &key, const QString &value, PublishState pState )
{
   RPCPublisher * pub = nullptr;
   for ( auto const &i: qAsConst(publishList ))
   {
      if ( i->category == category && i->key == key )
      {
         pub = i;
      }
   }
   if ( !pub )
   {
      pub = new RPCPublisher( category, key, value, pState );
      publishList.push_back( pub );
   }
   else
   {
      pub->value = value;
      pub->state = pState;
   }
   if ( RPCPubSub::isConnected() )
   {
      pub->rePublish();
   }
}
void RPCPublisher::rePublish()
{
   RPCPublishClient rpc( nullptr );
   QSharedPointer<RPCParam>st(new RPCParamStruct);
   QSharedPointer<RPCParam>sCat(new RPCStringParam( category ));
   QSharedPointer<RPCParam>sKey(new RPCStringParam( key ));
   QSharedPointer<RPCParam>sValue(new RPCStringParam( value ));
   QSharedPointer<RPCParam>sState(new RPCIntParam( state ));
   st->addMember( sCat, "Category" );
   st->addMember( sKey, "Key" );
   st->addMember( sValue, "Value" );
   st->addMember( sState, "State" );
   rpc.getCallArgs() ->addParam( st );
   rpc.queueCall( "localhost" );
}

//---------------------------------------------------------------------------

