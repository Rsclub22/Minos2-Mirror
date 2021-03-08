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
#include "TinyUtils.h"

#include "ConfigFile.h"

#include "MinosLink.h"
#include "clientThread.h"
#include "serverThread.h"
#include "MServer.h"

ThisMinosRouter *ThisMinosRouter::singleton = nullptr;
/*static*/
ThisMinosRouter *ThisMinosRouter::getThisMinosRouter()
{
   if ( !singleton )
   {
      singleton = new ThisMinosRouter();
   }
   return singleton;
}
//---------------------------------------------------------------------------

ThisMinosRouter::ThisMinosRouter()
{
    routerName = MinosConfig::getMinosConfig()->getThisRouterName();
}

ThisMinosRouter::~ThisMinosRouter()
{}
//---------------------------------------------------------------------------
bool ThisMinosRouter::forwardStanza( MinosCommonConnection *il, TiXmlElement *tix )
{
   // while we don't know our name, we can accept client connections, but NOT server connections
   bool res = false;
   QString sto = getAttribute( tix, "to" );
   MinosId to( sto );

   if ( sto.size() && to.router.size() && ( to.router.compare( "localhost", Qt::CaseInsensitive ) != 0 ) &&
        ( to.router.compare( getRouterName(), Qt::CaseInsensitive) != 0 ) )
   {
      res = false;        // not our host name in to
   }
   else
      if ( !sto.size() || to.user.size() == 0 )
      {
         // There is a "to" (with our server name??), or no "to" - which means us!
         res = analyseNode( il, tix );
      }
   return res;
}
//---------------------------------------------------------------------------
bool ThisMinosRouter::analyseNode( MinosCommonConnection *il, TiXmlElement *tix )
{
   // response factory - look at the node, and build the correct response stanza object

   // we need to sort out the "not implemented" bits here.
   // what do these stanzas mean to a service?
   // it need to be up to the individual service to see!

   if (  checkElementName( tix, "keepAlive" ))
   {
       return false;
   }

   MinosId from( getAttribute( tix, "from" ) );
   QString subtype = getAttribute( tix, "type" );
   QString id = getAttribute( tix, "id" );

   TiXmlElement *query = findNode( tix, "query" );
   QString ns = getAttribute( query, "xmlns" );
   if ( ns != "minos:iq:rpc" )
   {
      return false;
   }

   TiXmlElement *mcall = findNode( query, "methodCall" );
   if ( subtype == "set" )
   {
      RPCRequest * xs = new RPCRequest( from.fullId, mcall );
      xs->setId( id );
      trace("RX " + xs->analyse());

      dispatchStanza( il, xs );
      delete xs;
      return true;
   }
   return true;      // which may not be right?
}
//---------------------------------------------------------------------------
void ThisMinosRouter::dispatchStanza( MinosCommonConnection *il, RPCRequest *req )
{
   // really need to go to an RPC Object factory here
   trace( "RPCRequest received - " + req->methodName );

   if ( req->methodName == "ServerSetFromId" )
   {
      QString from;
      if ( req->getStringArg( 0, from ) )
      {
         // check that from is what connected to us...
         MinosId id( from );

         QString clientrouterName = ( id.router.compare("localhost", Qt::CaseInsensitive ) == 0 ) ? routerName : id.router ;
         QString fid = QString( id.user ) + "@" + clientrouterName;

         MinosId id2( fid );

         if (il)
             il->setFromId( id2, req );
      }
   }
   else if ( req->methodName == "ClientSetFromId" )
   {
      QString from;
      if ( req->getStringArg( 0, from ) )
      {
         // check that from is what connected to us...
         MinosId id( from );

         QString clientrouterName = ( id.router.compare("localhost", Qt::CaseInsensitive ) == 0 ) ? routerName : id.router;

         QString fid = QString( id.user ) + "@" + clientrouterName;

         MinosId id2( fid );

         if (il)
             il->setFromId( id2, req );   // don't check the reply
      }
   }
   else if ( req->methodName == rpcConstants::publish
        || req->methodName == rpcConstants::routerSubscribe
        || req->methodName == rpcConstants::remoteSubscribe
        || req->methodName == rpcConstants::subscribe
        || req->methodName == rpcConstants::routerNotify
        || req->methodName == rpcConstants::clientNotify )
   {
      makeXMPPEvent( req );
   }
}

//---------------------------------------------------------------------------

