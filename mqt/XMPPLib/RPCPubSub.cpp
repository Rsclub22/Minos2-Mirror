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
//---------------------------------------------------------------------------
/*static*/
void RPCPubSub::initialisePubSub( TRPCFunctor *notifycb )
{
   static bool objAdded = false;
   if ( !objAdded )
   {
      objAdded = true;
      MinosRPCObj::addServerObj( QSharedPointer<MinosRPCObj>(new RPCNotifyServer( notifycb )) );
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
void RPCPubSub::subscribeRemote( const QString &server, const QString &category )
{
   RPCRemoteSubscriber::testAndSubscribe( server, category );
}
void RPCPubSub::reconnectPubSub( )
{
   // iterate the publish and subscribe objects and re-do them
   for ( QVector<RPCPublisher *>::iterator i = publishList.begin(); i != publishList.end(); i++ )
   {
      if ( *i )
      {
         ( *i ) ->rePublish();
      }
   }
   for ( QVector<RPCSubscriber *>::iterator i = subscribeList.begin(); i != subscribeList.end(); i++ )
   {
      if ( *i )
      {
         ( *i ) ->reSubscribe();
      }
   }
   connected = true;
}

void RPCPubSub::close( )
{
   for ( QVector<RPCPublisher *>::iterator i = publishList.begin(); i != publishList.end(); i++ )
   {
      delete ( *i );
      ( *i ) = 0;
   }
   publishList.clear();
   for ( QVector<RPCSubscriber *>::iterator i = subscribeList.begin(); i != subscribeList.end(); i++ )
   {
      delete ( *i );
      ( *i ) = 0;
   }
   subscribeList.clear();
}
//---------------------------------------------------------------------------
AnalysePubSubNotify::AnalysePubSubNotify(bool err, QSharedPointer<MinosRPCObj> mro ) :
      OK( false )
{
   if ( !err )
   {
      QSharedPointer<RPCParam> psPublisher;
      QSharedPointer<RPCParam> psServer;
      QSharedPointer<RPCParam> psCategory;
      QSharedPointer<RPCParam> psKey;
      QSharedPointer<RPCParam> psValue;
      QSharedPointer<RPCParam> psState;
      RPCArgs *args = mro->getCallArgs();
      if (
              // NB publisher is initiated by the "from" from the publisher
              // and persists with the publihe entity
         args->getStructArgMember( 0, "Publisher", psPublisher )
         && args->getStructArgMember( 0, "Server", psServer )
         && args->getStructArgMember( 0, "Category", psCategory )
         && args->getStructArgMember( 0, "Key", psKey )
         && args->getStructArgMember( 0, "Value", psValue )
         && args->getStructArgMember( 0, "State", psState )
      )
      {
         int stemp;
         QString pub;
         if (
            psPublisher->getString(pub) &&
            psServer->getString( server ) &&
            psCategory->getString( category ) &&
            psKey->getString( key ) &&
            psValue->getString( value ) &&
            psState->getInt( stemp )
         )
         {
            QStringList p = pub.split(QChar('@'), QString::KeepEmptyParts);
            if (p.size() > 1)
            {
                publisherServer = p[1];
            }
            if (publisherServer == "localhost")
            {
                publisherServer == MinosConfig::getMinosConfig()->getThisServerName();
            }
            publisherProgram = p[0];
            state = static_cast<PublishState>(stemp);
            OK = true;
         }
      }
   }
}

