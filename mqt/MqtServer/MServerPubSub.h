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

#include <QTreeWidget>

#ifndef MServerPubSubH
#define MServerPubSubH 

extern int GetSubscribedCount();
extern int GetPublishedCount();

extern void buildPublishedTree(QTreeWidget *tree);
extern void buildSubscribedTree(QTreeWidget *tree);

//---------------------------------------------------------------------------
class MinosRPCObj;
class TPubSubMain
{
   private:  	// User declarations
   public:  		// User declarations
      TPubSubMain( );
      ~TPubSubMain( );
      void publishCallback(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from );
      void subscribeCallback( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from );
      void remoteSubscribeCallback(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from );
      void serverSubscribeCallback( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from );
      void notifyCallback(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from );
      void serverNotifyCallback( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from );

      bool publish( const QString &pubId, const QString &category, const QString &key, const QString &value, PublishState state );
      bool serverPublish( const QString &pubId, const QString &svr, const QString &category, const QString &key, const QString &value, PublishState state );

      void revokeClient(const QString &pubId);
      void disconnectServer(const QString &pubId);
      void closeDown();

};
extern TPubSubMain *PubSubMain;
#endif
