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
#include <QHeaderView>
#include "MinosLink.h"
#include "clientThread.h"
#include "serverThread.h"
#include "PubSubServer.h"
#include "MServerPubSub.h"
#include "MServer.h"

extern bool closeApp;

//---------------------------------------------------------------------------
TPubSubMain *PubSubMain = nullptr;
static const char *stateList[] =
{
   "P",
   "R",
   "NC"
};//---------------------------------------------------------------------------
static void makeRPCObjects()
{
   MinosRPCObj::addRouterObj( QSharedPointer<MinosRPCObj>(new RPCPublishRouter( new TRPCCallback <TPubSubMain> ( PubSubMain, &TPubSubMain::publishCallback ) ) ) );
   MinosRPCObj::addRouterObj( QSharedPointer<MinosRPCObj>(new RPCSubscribeRouter( new TRPCCallback <TPubSubMain> ( PubSubMain, &TPubSubMain::subscribeCallback ) ) ) );
   MinosRPCObj::addRouterObj( QSharedPointer<MinosRPCObj>(new RPCRemoteSubscribeRouter( new TRPCCallback <TPubSubMain> ( PubSubMain, &TPubSubMain::remoteSubscribeCallback ) ) ) );
   MinosRPCObj::addRouterObj( QSharedPointer<MinosRPCObj>(new RPCRouterSubscribeRouter( new TRPCCallback <TPubSubMain> ( PubSubMain, &TPubSubMain::routerSubscribeCallback ) ) ) );
   MinosRPCObj::addRouterObj( QSharedPointer<MinosRPCObj>(new RPCRouterNotifyRouter( new TRPCCallback <TPubSubMain> ( PubSubMain, &TPubSubMain::routerNotifyCallback ) ) ) );

   RPCRouterPubSub::initialisePubSub( new TRPCCallback <TPubSubMain> ( PubSubMain, &TPubSubMain::notifyCallback ) );
}
//---------------------------------------------------------------------------
class Subscriber;
typedef QVector <Subscriber *> SubscriberList;
typedef QVector <Subscriber *>::iterator SubscriberListIterator;
//---------------------------------------------------------------------------
class RemoteSubscriber;
typedef QVector <RemoteSubscriber *> RemoteSubscriberList;
typedef QVector <RemoteSubscriber *>::iterator RemoteSubscriberListIterator;
//---------------------------------------------------------------------------
class RouterSubscriber;
typedef QVector <RouterSubscriber *> RouterSubscriberList;
typedef QVector <RouterSubscriber *>::iterator RouterSubscriberListIterator;
//---------------------------------------------------------------------------
class PublishedCategory;
typedef QVector <PublishedCategory *> PublishedCategoryList;
typedef QVector <PublishedCategory *>::iterator PublishedCategoryListIterator;
//---------------------------------------------------------------------------
class PublishedKey;
typedef QVector <PublishedKey *> PublishedKeyList;
typedef QVector <PublishedKey *>::iterator PublishedKeyListIterator;
//---------------------------------------------------------------------------
// normal subscriber; client prog of this router
class Subscriber
{
      QString sjid;
   public:

      Subscriber( const QString &sjid )
            : sjid( sjid )
      {}
      Subscriber()
      {}
      ~Subscriber()
      {}
      QString getSjid() const
      {
         return sjid;
      }
      void SendTo( const PublishedKey &pk );
};
//---------------------------------------------------------------------------
// normal subscriber; client prog of this router, subscribed to remote router value
class RemoteSubscriber
{
      QString sjid;
      QString router;
   public:

      RemoteSubscriber( const QString &sjid, const QString &router )
            : sjid( sjid ), router( router )
      {}
      RemoteSubscriber()
      {}
      ~RemoteSubscriber()
      {}
      QString getSjid() const
      {
         return sjid;
      }

      QString getRouter() const
      {
         return router;
      }
      void SendTo( const PublishedKey &pk );
};
//---------------------------------------------------------------------------
// remote router subscribed to this router (on bahalf of its client)
class RouterSubscriber
{
      QString sjid;
      QString router;
   public:

      RouterSubscriber( const QString &sjid, const QString &router )
            : sjid( sjid ), router( router )
      {}
      RouterSubscriber()
      {}
      ~RouterSubscriber()
      {}
      QString getSjid() const
      {
         return sjid;
      }

      QString getRouter() const
      {
         return router;
      }
      void SendTo( const PublishedKey &pk );
};
//---------------------------------------------------------------------------
// we need to extend this... we need a list of categories
// each of which has a list of keys, and a subscriber can subscribe at either level
class Published
{
    Q_DECLARE_TR_FUNCTIONS(Published)

   protected:
      QString pubId;
      bool localOnly;

   public:
      static PublishedCategoryList publist;  // base list of categories

      Published( const QString &pubId, bool local );
      virtual ~Published();
      QString getPubId() const
      {
         return pubId;
      }

      static void clearPublist();
      static int GetSubscribedCount();
      static int GetPublishedCount() ;

      static void buildPublishedTree(QTreeWidget *tree);
      static void buildSubscribedTree(QTreeWidget *tree);

      QString getPublisherRouter();

};
class PublishedCategory: public Published
{
   private:
      PublishedCategory();
      QString category;
   public:
      SubscriberList subscribedLocal;
      RemoteSubscriberList subscribedRemote;
      RouterSubscriberList subscribedrouter;
      PublishedKeyList pubkeylist;              // per cat list of key/value pairs
      int GetSubscribedCount();

      static bool publish( const QString &pubId, const QString &category, const QString &key, const QString &value, PublishState state );
      void publish( const QString &pubId, const QString &key, const QString &value , PublishState state );

      static bool routerPublish( const QString &pubId, const QString &svr, const QString &category, const QString &key, const QString &value, PublishState state );
      void routerPublish( const QString &pubId, const QString &svr, const QString &key, const QString &value, PublishState state );

      static void clientSubscribe( const QString &subId, const QString &category );
      static void remoteSubscribe( const QString &subId, const QString &router, const QString &category );
      static void routerSubscribe( const QString &subId, const QString &router, const QString &category );

      // find a published item
      static PublishedCategoryListIterator findPubCategory( const QString &category );

      // find a published item
      PublishedKeyListIterator findPubKey( const QString &svr, const QString &pid, const QString &key );

      PublishedCategory( const QString &pubId, const QString &category );
      ~PublishedCategory();

      QString getCategory()
      {
         return category;
      }

      Subscriber * getClientSubscribed( const QString &subId );
      RemoteSubscriber * getRemoteSubscribed( const QString &subId );
      RouterSubscriber * getRouterSubscribed( const QString &subId );

};
class PublishedKey: public Published
{
   private:
      PublishedKey();

      QString router;  // published from router
      QString key;
      QString value;
      PublishState state;
      PublishedCategory *cat;

   public:

      PublishedKey( bool local, const QString &pubId, const QString &svr, PublishedCategory *pcat, const QString &key, PublishState state );
      ~PublishedKey();

      void setPubCat( PublishedCategory *pcat )
      {
         cat = pcat;
      }
      void setPubId( const QString &p )
      {
         pubId = p;
      }
      void setPubValue( const QString &v )
      {
         value = v;
      }
      void setPubState( PublishState pState )
      {
         state = pState;
      }
      PublishedCategory *getPubCat() const
      {
         return cat;
      }
      QString getRouter() const
      {
         return router;
      }
      QString getPubKey() const
      {
         return key;
      }
      QString getPubValue() const
      {
         return value;
      }
      PublishState getPubState() const
      {
         return state;
      }
};
//---------------------------------------------------------------------------
void Subscriber::SendTo ( const PublishedKey &pk )
{
   // Build the stanza, and send it to the subid
   RPCClientNotifyClient rnc( nullptr );
   QSharedPointer<RPCParam>st(new RPCParamStruct);

   // local - no router
   st->addMember( QString(""), "Server" );
   st->addMember( pk.getPubId(), "Publisher" );
   st->addMember( pk.getPubCat() ->getCategory(), "Category" );
   st->addMember( pk.getPubKey(), "Key" );
   st->addMember( pk.getPubValue(), "Value" );
   st->addMember( pk.getPubState(), "State" );

   rnc.getCallArgs() ->addParam( st );
   rnc.queueCall( getSjid() );
}
//---------------------------------------------------------------------------
void RemoteSubscriber::SendTo ( const PublishedKey &pk )
{
   // Build the stanza, and send it to the subid
   RPCClientNotifyClient rnc( nullptr );
   QSharedPointer<RPCParam>st(new RPCParamStruct);

   // router is remote router name (as published)
   st->addMember( router, "Server" );
   st->addMember( pk.getPubId(), "Publisher" );
   st->addMember( pk.getPubCat() ->getCategory(), "Category" );
   st->addMember( pk.getPubKey(), "Key" );
   st->addMember( pk.getPubValue(), "Value" );
   st->addMember( pk.getPubState(), "State" );

   rnc.getCallArgs() ->addParam( st );
   rnc.queueCall( getSjid() );
}
//---------------------------------------------------------------------------
void RouterSubscriber::SendTo ( const PublishedKey &pk )
{
   // Build the stanza, and send it to the subid
   RPCRouterNotifyClient rnc( nullptr );
   QSharedPointer<RPCParam>st(new RPCParamStruct);

   //router is OUR router name
   QString srouter = ThisMinosRouter::getThisMinosRouter() ->getRouterName();
   QString sCategory;
   try
   {
      PublishedCategory *pc = pk.getPubCat();
      sCategory = pc ->getCategory();
   }
   catch(...)
   {
      // sCategory = 0;
   }

   st->addMember( router, "Server" );
   st->addMember( pk.getPubId(), "Publisher" );
   st->addMember( sCategory, "Category" );
   st->addMember( pk.getPubKey(), "Key" );
   st->addMember( pk.getPubValue() , "Value" );
   st->addMember( pk.getPubState(), "State" );

   rnc.getCallArgs() ->addParam( st );
   rnc.queueCall( getSjid() );
}
//---------------------------------------------------------------------------
/*static*/
PublishedCategoryList Published::publist;
//---------------------------------------------------------------------------
Published::Published( const QString &pubId, bool loc ) : pubId( pubId ), localOnly( loc )
{}
QString Published::getPublisherRouter()
{
    QString publisherrouter;
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QStringList p = pubId.split(QChar('@'), Qt::KeepEmptyParts);
#else
    QStringList p = pubId.split(QChar('@'), QString::KeepEmptyParts);
#endif
    if (p.size() > 1)
    {
        publisherrouter = p[1];
    }
    else
    {
        publisherrouter = pubId;
    }
    if (publisherrouter == "localhost")
    {
        QString sname = ThisMinosRouter::getThisMinosRouter()->getRouterName();
        publisherrouter = sname;
    }
    return publisherrouter;
}
//---------------------------------------------------------------------------
void Published::clearPublist()
{
   for ( auto &p: Published::publist )
   {
      delete p;
      p = nullptr;
   }
   Published::publist.clear();
}
int Published::GetSubscribedCount()
{
   int scount = 0;
   for ( auto const &p: qAsConst(Published::publist) )
   {
      scount += p->GetSubscribedCount();
   }
   return scount;
}
int Published::GetPublishedCount()
{
   int pcount = 0;
   for ( auto const &p: qAsConst(Published::publist) )
   {
      pcount += p->pubkeylist.size();
   }
   return pcount;
}
void Published::buildPublishedTree(QTreeWidget *tree)
{
    tree->clear();
    tree->setColumnCount(3);
    QStringList h = {tr("key"), tr("state"), tr("value")};
    tree->setHeaderLabels(h);
    for ( auto const &p: qAsConst(Published::publist) )
    {
        QString cat = p->getCategory();
        QTreeWidgetItem *catItem = new QTreeWidgetItem(tree);
        catItem->setText(0, cat);
        for ( auto const &j: qAsConst(p->pubkeylist))
        {
           QString key = j->getPubKey();
           QString value = j->getPubValue();
           PublishState state = j->getPubState();

           QTreeWidgetItem *keyItem = new QTreeWidgetItem(catItem);
           keyItem->setText(0, key);
           keyItem->setText(1, stateList[state]);
           keyItem->setText(2, value);
        }

    }
    tree->expandAll();
}
void Published::buildSubscribedTree(QTreeWidget *tree)
{
    tree->clear();

    for ( auto const &p: qAsConst(Published::publist ))
    {
        QString cat = p->getCategory();
        QTreeWidgetItem *catItem = new QTreeWidgetItem(tree);
        catItem->setText(0, cat);

        QTreeWidgetItem *stype = new QTreeWidgetItem(catItem);
        stype->setText(0, "Local");
        for ( auto const &i: qAsConst(p->subscribedLocal ))
        {
            QString sub = i->getSjid();
            QTreeWidgetItem *subItem = new QTreeWidgetItem(stype);
            subItem->setText(0, sub);
        }
        stype = new QTreeWidgetItem(catItem);
        stype->setText(0, "Server");
        for ( auto const &i:qAsConst( p->subscribedrouter ))
        {
            QString sub = i->getSjid();
            QTreeWidgetItem *subItem = new QTreeWidgetItem(stype);
            subItem->setText(0, sub);
        }
        stype = new QTreeWidgetItem(catItem);
        stype->setText(0, "Remote");
        for ( auto const &i: qAsConst(p->subscribedRemote ))
        {
            QString sub = i->getSjid();
            QTreeWidgetItem *subItem = new QTreeWidgetItem(stype);
            subItem->setText(0, sub);
        }
    }

    tree->expandAll();
}

bool noCategoryConnection( PublishedCategory *pc )
{
   if ( pc == nullptr )
      return true;
   else
      return false;
}
Published::~Published()
{}
//---------------------------------------------------------------------------
int PublishedCategory::GetSubscribedCount()
{
   int scount = subscribedLocal.size();
   scount += subscribedRemote.size();
   scount += subscribedrouter.size();

   return scount;
}
//---------------------------------------------------------------------------
/*static*/ void PublishedCategory::clientSubscribe(  const QString &subId,  const QString &category  )
{
   PublishedCategoryListIterator f = PublishedCategory::findPubCategory( category );
   if ( f == publist.end() )
   {
      PublishedCategory * p = new PublishedCategory( "", category );
      publist.push_back( p );
      f = findPubCategory( category );
   }
   if ( f != publist.end() )
   {
      // we subscribe to ALL keys under the category
      Subscriber * s = ( *f ) ->getClientSubscribed( subId );
      if ( !s )
      {
         s = new Subscriber( subId );
         ( *f ) ->subscribedLocal.push_back( s );
      }

      // and we now need to send them all...
      for ( auto const &pk: qAsConst(( *f ) ->pubkeylist ))
      {
         // Here we are sending all the already published values
         if ( pk->getRouter().size() == 0 || pk->getRouter() == "localhost" || pk ->getRouter() == ThisMinosRouter::getThisMinosRouter() ->getRouterName() )
         {
            s->SendTo( *pk );
         }
      }
   }
}
//---------------------------------------------------------------------------
/*static*/ void PublishedCategory::remoteSubscribe(  const QString &subId,  const QString &router,  const QString &category  )
{
   PublishedCategoryListIterator f = PublishedCategory::findPubCategory( category );
   if ( f == publist.end() )
   {
      PublishedCategory * p = new PublishedCategory( "", category );
      publist.push_back( p );
      f = findPubCategory( category );
   }
   if ( f != publist.end() )
   {
      // we subscribe to ALL keys under the category
      RemoteSubscriber * s = ( *f ) ->getRemoteSubscribed( subId );
      if ( !s )
      {
         // not yet published
         s = new RemoteSubscriber( subId, router );
         ( *f ) ->subscribedRemote.push_back( s );

         //**** and we need to subscribe to the remote router
      }

      // and we now need to send them all...
      for ( auto const &pk: qAsConst(( *f ) ->pubkeylist ))
      {
         // Here we are sending all the already published values
         if ( pk->getRouter() == router )
         {
            s->SendTo( *pk );
         }
      }
      RPCRouterPubSub::routerSubscribeRemote( router, category );
   }
}

//---------------------------------------------------------------------------
/*static*/ void PublishedCategory::routerSubscribe(  const QString &subId,  const QString &router,  const QString &category  )
{
   PublishedCategoryListIterator f = PublishedCategory::findPubCategory( category );
   if ( f == publist.end() )
   {
      PublishedCategory * p = new PublishedCategory( "", category );
      publist.push_back( p );
      f = findPubCategory( category );
   }
   if ( f != publist.end() )
   {
      // we subscribe to ALL keys under the category
      RouterSubscriber * s = ( *f ) ->getRouterSubscribed( subId );
      if ( !s )
      {
         s = new RouterSubscriber( subId, router );
         ( *f ) ->subscribedrouter.push_back( s );
      }

      // and we now need to send them all...
      for ( auto const &pk: qAsConst(( *f ) ->pubkeylist ))
      {
         // make sure that the key is one published by THIS router; we don't
         // want to re-publish
         // BUT it can publish as blank
         QString prouter = pk->getRouter();
         if ( prouter.size() == 0 || prouter == "localhost" || prouter == ThisMinosRouter::getThisMinosRouter() ->getRouterName() )
         {
            // Here we are sending all the already published values

            // THIS DOES NASTY LOCAL THINGS - it DOESN't just send!
            s->SendTo( *pk);
         }
      }
   }
}


//---------------------------------------------------------------------------
// return entry for this Name
/*static*/ PublishedCategoryListIterator PublishedCategory::findPubCategory(  const QString &category  )
{
   for ( PublishedCategoryListIterator i = publist.begin(); i != publist.end(); i++ )
   {
      if ( category == ( *i ) ->category )
         return i;
   }
   return publist.end();
}

//---------------------------------------------------------------------------
// return entry for this Name
PublishedKeyListIterator PublishedCategory::findPubKey(const QString &svr, const QString &pid, const QString &key )
{
   for ( PublishedKeyListIterator i = pubkeylist.begin(); i != pubkeylist.end(); i++ )
   {
      PublishedKey *pk = (*i);
      if (pk && key == pk ->getPubKey() && pk->getPubId() == pid && pk ->getRouter() == svr )
         return i;
   }
   return pubkeylist.end();
}
//---------------------------------------------------------------------------
/*static*/ bool PublishedCategory::publish(  const QString &pubId,   const QString &category,  const QString &key ,   const QString &value, PublishState pState )
{
   PublishedCategoryListIterator f = PublishedCategory::findPubCategory( category );
   if ( f == publist.end() )
   {
       if (pState == psRevoked)
       {
           // not found, but revoked - do nothing
           // revocation is in a loop on publist, so we don't want to change it
           return false;
       }
      // create it...
      PublishedCategory * p = new PublishedCategory( pubId, category );
      publist.push_back( p );
      f = PublishedCategory::findPubCategory( category );
   }
   ( *f ) ->publish( pubId, key, value, pState );
   return true;
}
//---------------------------------------------------------------------------
void PublishedCategory::publish( const QString &pubId, const QString &k, const QString &v , PublishState pState)
{
   PublishedKeyListIterator kl = findPubKey( "", pubId, k );
   bool doPub = false;
   if ( kl == pubkeylist.end() && pState == psPublished)
   {
      PublishedKey * p = new PublishedKey( false, pubId, "", this, k, pState );
      pubkeylist.push_back( p );
      kl = findPubKey( "", pubId, k );
      doPub = true;
   }

   if ( ( kl != pubkeylist.end() ) && ( doPub || ( *kl ) ->getPubValue() != v || (*kl)->getPubState() != pState) )     // first time, force broadcast anyway
   {
      ( *kl ) ->setPubValue( v );
      ( *kl ) ->setPubState( pState );
      for ( SubscriberListIterator i = subscribedLocal.begin(); i != subscribedLocal.end(); i++ )
      {
         // send to all who have subscribed to the category
         ( *i ) ->SendTo( *( *kl ) );
      }
      for ( RouterSubscriberListIterator i = subscribedrouter.begin(); i != subscribedrouter.end(); i++ )
      {
         // send to all who have subscribed to the category
         ( *i ) ->SendTo( *( *kl ) );
      }
   }
}
//---------------------------------------------------------------------------
/*static*/ bool PublishedCategory::routerPublish(  const QString &pubId,   const QString &svr, const QString &category, const QString &key , const QString &value, PublishState pState )
{
   PublishedCategoryListIterator f = PublishedCategory::findPubCategory( category );
   if ( f == publist.end() )
   {
      // create it...
      PublishedCategory * p = new PublishedCategory( pubId, category );
      publist.push_back( p );
      f = PublishedCategory::findPubCategory( category );
   }
   ( *f ) ->routerPublish( pubId, svr, key, value, pState );
   return true;
}
//---------------------------------------------------------------------------
void PublishedCategory::routerPublish( const QString &pubId, const QString &svr, const QString &k, const QString &v, PublishState pState )
{

//#warning What happens here? Why do remote contestlogs get reported with no router to the local monitor?
   PublishedKeyListIterator kl = findPubKey( svr, pubId, k );
   bool doPub = false;
   if ( kl == pubkeylist.end() )
   {
      PublishedKey * p = new PublishedKey( true, pubId, svr, this, k, pState );
      pubkeylist.push_back( p );
      kl = findPubKey( svr, pubId, k );
      doPub = true;
   }

   if ( ( kl != pubkeylist.end() ) && ( doPub || ( *kl ) ->getPubValue() != v || ( *kl) ->getPubState() != pState ) )     // first time, force broadcast anyway
   {
      ( *kl ) ->setPubValue( v );
      ( *kl ) ->setPubState( pState );
      for ( auto const &s: qAsConst(subscribedRemote ))
      {
         // send to all who have subscribed to the category
         s->SendTo( *( *kl ) );
      }
   }
}
PublishedCategory::PublishedCategory( const QString &publId, const QString &category ) :
      Published( publId, false ), category( category )
{}
PublishedCategory::~PublishedCategory()
{
   for ( auto &i: subscribedLocal )
   {
      delete i;
      i = nullptr;
   }
   subscribedLocal.clear();
   for ( auto &i: subscribedRemote )
   {
      delete i;
      i = nullptr;
   }
   subscribedRemote.clear();
   for ( auto &i: subscribedrouter)
   {
      delete i;
      i = nullptr;
   }
   subscribedrouter.clear();

   for ( auto &i: pubkeylist )
   {
      delete i;
      i = nullptr;
   }
   pubkeylist.clear();
}
//---------------------------------------------------------------------------
Subscriber * PublishedCategory::getClientSubscribed( const QString &subId )
{
   for ( auto const &i: qAsConst(subscribedLocal ))
   {
      if ( i->getSjid() == subId )
         return i;
   }
   return nullptr;
}
//---------------------------------------------------------------------------
RemoteSubscriber * PublishedCategory::getRemoteSubscribed( const QString &subId )
{
   for ( auto const &i: qAsConst(subscribedRemote ))
   {
      if ( i ->getSjid() == subId )
         return i;
   }
   return nullptr;
}
//---------------------------------------------------------------------------
RouterSubscriber * PublishedCategory::getRouterSubscribed( const QString &subId )
{
   for ( auto const &i: qAsConst(subscribedrouter ))
   {
      if ( i->getSjid() == subId )
         return i;
   }
   return nullptr;
}
//---------------------------------------------------------------------------
PublishedKey::PublishedKey( bool local, const QString &publId, const QString &svr, PublishedCategory *pcat, const QString &key, PublishState pState ) :
      Published( publId, local )
    , router( svr )
    , key( key )
    , state ( pState )
    , cat( pcat )
{}
PublishedKey::~PublishedKey()
{}
//---------------------------------------------------------------------------
TPubSubMain::TPubSubMain( )
{
   PubSubMain = this;
   makeRPCObjects();
}
//---------------------------------------------------------------------------
TPubSubMain::~TPubSubMain()
{
}
//---------------------------------------------------------------------------

bool TPubSubMain::publish( const QString &pubId, const QString &category, const QString &key, const QString &value, PublishState pState )
{
   if (closeApp)
   {
      return false;
   }
   trace("Publishing from <" + pubId + "> cat " + category + " key " + key + " state " + stateList[pState]);
   return PublishedCategory::publish( pubId, category, key, value, pState );
}
bool TPubSubMain::routerPublish( const QString &pubId, const QString &svr, const QString &category, const QString &key, const QString &value, PublishState pState )
{
   if (closeApp)
   {
      return false;
   }
   trace("router Publishing from svr <" + svr + "> pubid <" + pubId + "> cat " + category + " key " + key+ " state " + stateList[pState]);
   return PublishedCategory::routerPublish( pubId, svr, category, key, value, pState );
}
int GetSubscribedCount()
{
   return Published::GetSubscribedCount();
}
int GetPublishedCount()
{
   return Published::GetPublishedCount();
}

void buildPublishedTree(QTreeWidget *tree)
{
    Published::buildPublishedTree(tree);
}
void buildSubscribedTree(QTreeWidget *tree)
{
    Published::buildSubscribedTree(tree);
}

//---------------------------------------------------------------------------

// callback to publish local router - this may get proxied when we have a remote router subscriber

void TPubSubMain::publishCallback( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from )
{
   trace( "Publish callback from " + from + ( err ? ":Error" : ":Normal" ) );

   if ( !err )
   {
      QSharedPointer<RPCParam> psCategory;
      QSharedPointer<RPCParam> psKey;
      QSharedPointer<RPCParam> psValue;
      QSharedPointer<RPCParam> psState;
      RPCArgs *args = mro->getCallArgs();
      if ( args->getStructArgMember( 0, "Category", psCategory )
           && args->getStructArgMember( 0, "Key", psKey )
           && args->getStructArgMember( 0, "Value", psValue )
           && args->getStructArgMember( 0, "State", psState )
           )
      {
         QString Category;
         QString Key;
         QString Value;
         PublishState State;
         int temps;

         if ( psCategory->getString( Category ) && psKey->getString( Key )
                  && psValue->getString( Value )  && psState->getInt( temps ))
         {
            State = static_cast<PublishState>(temps);
            TPubSubMain::publish( from, Category, Key, Value, State );
         }
      }
   }
}
//---------------------------------------------------------------------------

// callback to subscribe client - local router (there should not be a router member)

void TPubSubMain::subscribeCallback(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from )
{
   trace( "Client Subscribe callback from " + from + ( err ? ":Error" : ":Normal" ) );
   if ( !err )
   {
      QSharedPointer<RPCParam>st(new RPCParamStruct);
      QSharedPointer<RPCParam> psCategory;
      RPCArgs *args = mro->getCallArgs();
      if ( args->getStructArgMember( 0, "Category", psCategory ) )
      {
         QString Category;
         bool resc = psCategory->getString( Category );
         if ( resc )
         {
            PublishedCategory::clientSubscribe( from, Category );
         }
      }
   }
}
//---------------------------------------------------------------------------

// callback to subscribe client  - remote router

void TPubSubMain::remoteSubscribeCallback( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from )
{
   trace( "Remote Subscribe callback from " + from + ( err ? ":Error" : ":Normal" ) );
   if ( !err )
   {
      QSharedPointer<RPCParam>st(new RPCParamStruct);
      QSharedPointer<RPCParam> psrouter;
      QSharedPointer<RPCParam> psCategory;
      RPCArgs *args = mro->getCallArgs();
      if ( args->getStructArgMember( 0, "Server", psrouter ) &&
           args->getStructArgMember( 0, "Category", psCategory ) )
      {
         QString router;
         bool ress = psrouter->getString( router );
         QString Category;
         bool resc = psCategory->getString( Category );
         if ( ress && resc )
         {
            if ( !router.size() || router == "localhost" || router == ThisMinosRouter::getThisMinosRouter() ->getRouterName() )
            {
               subscribeCallback( err, mro, from );     // actually, for local router
               return ;
            }

            PublishedCategory::remoteSubscribe( from, router, Category );
         }
      }
   }
}
//---------------------------------------------------------------------------

// callback to subscribe router  - remote router

void TPubSubMain::routerSubscribeCallback(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from )
{
   trace( "router Subscribe callback from " + from + ( err ? ":Error" : ":Normal" ) );
   if ( !err )
   {
      QSharedPointer<RPCParam>st(new RPCParamStruct);
      QSharedPointer<RPCParam> psrouter;
      QSharedPointer<RPCParam> psCategory;
      RPCArgs *args = mro->getCallArgs();
      if ( args->getStructArgMember( 0, "Server", psrouter ) &&
           args->getStructArgMember( 0, "Category", psCategory ) )
      {
         QString router;
         bool ress = psrouter->getString( router );
         QString Category;
         bool resc = psCategory->getString( Category );
         if ( ress && resc )
         {
            PublishedCategory::routerSubscribe( from, router, Category );
         }
      }
   }
}
//---------------------------------------------------------------------------

// callback for responses to notify messages

void TPubSubMain::notifyCallback( bool err, QSharedPointer<MinosRPCObj> /*mro*/, const QString &from )
{
   // response to pubsub calls
   trace( "Notify callback from " + from + ( err ? ":Error" : ":Normal" ) );
}
//---------------------------------------------------------------------------

// this we get when we get a subscribe notification from a remote router

void TPubSubMain::routerNotifyCallback(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from )
{
   // we need to pass it on to any of our subscribers who are interested
   // in this event from this router
   // But why aren't we sending a result?
   trace( "PubSub Notify callback from " + from + ( err ? ":Error" : ":Normal" ) );
   AnalysePubSubNotify an( err, mro );

   if ( an.getOK() )
   {
      QString router = an.getRouter();
      QString category = an.getCategory();
      QString key = an.getKey();
      QString value = an.getValue();
      QString publisherProgram = an.getPublisherProgram();
      QString publisherRouter = an.getPublisherRouter();
      PublishState state = an.getState();
      routerPublish( /*from*/publisherProgram + "@" + publisherRouter, router, category, key, value, state );       // but we mustn't publish this back to any remote routers
      // even if they ARE subscribed
   }
}
bool nopub( PublishedKey *ip )
{
   if ( ip == nullptr )
      return true;
   else
      return false;
}
void TPubSubMain::revokeClient(const QString &pubId)
{
   if (closeApp)
   {
      return;
   }
   for ( auto &f: Published::publist )
   {
      for ( auto &i: f ->pubkeylist )
      {
         if (i->getPubId() == pubId)
         {
            // publish revoke
            TPubSubMain::publish( pubId, f->getCategory(), i->getPubKey(), "", psRevoked );
            delete i;
            i = nullptr;
         }
      }
      f->pubkeylist.erase( std::remove_if( f->pubkeylist.begin(), f->pubkeylist.end(), nopub ), f->pubkeylist.end() );
      // and now clear up the published key list for the category
   }
}
void TPubSubMain::disconnectRouter(const QString &pubId)
{
   if (closeApp)
   {
      return;
   }
   QString publisherRouter;
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QStringList p = pubId.split(QChar('@'), Qt::KeepEmptyParts);
#else
    QStringList p = pubId.split(QChar('@'), QString::KeepEmptyParts);
#endif
   if (p.size() > 1)
   {
       publisherRouter = p[1];
   }
   else
   {
       publisherRouter = pubId;
   }
   if (publisherRouter == "localhost")
   {
        QString sname = ThisMinosRouter::getThisMinosRouter()->getRouterName();
        publisherRouter = sname;
   }

   for ( auto &f: Published::publist )
   {
      for ( auto &pk: f->pubkeylist )
      {
         if (pk->getPublisherRouter() == publisherRouter)
         {
            // publish revoke
            TPubSubMain::routerPublish( pk->getPubId(), publisherRouter, f->getCategory(), pk->getPubKey(), "", psRevoked );
            delete pk;
            pk = nullptr;
         }
      }
      f->pubkeylist.erase( std::remove_if( f->pubkeylist.begin(), f->pubkeylist.end(), nopub ), f->pubkeylist.end() );
   }
}

void TPubSubMain::closeDown()
{
    // need to clear all the lists
    Published::clearPublist();
    MinosRPCObj::clearRPCObjects();
    RPCRouterPubSub::close( );
}
