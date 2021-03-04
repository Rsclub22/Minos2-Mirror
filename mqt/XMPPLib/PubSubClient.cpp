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
RPCNotifyRouter::~RPCNotifyRouter()
{}
RPCPublisher::~RPCPublisher()
{}

bool RPCSubscriber::isEqual( const QString &pcategory )
{
   return ( pcategory == category );
}
bool RPCSubscriber::isRemoteEqual( const QString &/*prouter*/, const QString &/*pcategory*/ )
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
bool RPCRemoteSubscriber::isRemoteEqual( const QString &pRouter, const QString &cat )
{
   return router == pRouter && isEqual( cat );
}
void RPCRemoteSubscriber::testAndSubscribe( const QString &router, const QString &cat )
{
   RPCRemoteSubscriber * sub = nullptr;
   for ( auto const &i: qAsConst(subscribeList ))
   {
      if ( i->isRemoteEqual( router, cat ) )
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
      sub = new RPCRemoteSubscriber( router, cat );
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
   QSharedPointer<RPCParam>sRouter(new RPCStringParam( "localhost" ));
   QSharedPointer<RPCParam>sCat(new RPCStringParam( category ));
   st->addMember( sRouter, "Server" );
   st->addMember( sCat, "Category" );
   rsc.getCallArgs() ->addParam( st );
   rsc.queueCall( "localhost" );
}
// client resubscribing to remote event
void RPCRemoteSubscriber::reSubscribe()
{
   RPCRemoteSubscribeClient rsc( nullptr );
   QSharedPointer<RPCParam>st(new RPCParamStruct);
   QSharedPointer<RPCParam>sRouter(new RPCStringParam( router ));
   QSharedPointer<RPCParam>sCat(new RPCStringParam( category ));
   st->addMember( sRouter, "Server" );
   st->addMember( sCat, "Category" );
   rsc.getCallArgs() ->addParam( st );
   rsc.queueCall( "localhost" );       // localhost just causes the router to loop
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

