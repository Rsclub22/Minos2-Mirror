/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#include <QString>
#include "XMPPStanzas.h"
#include "tinyxml.h"
#include "TinyUtils.h"

#include "Dispatcher.h"
#include "MTrace.h"

RPCDispatcher::RPCDispatcher()
{}
RPCDispatcher::~RPCDispatcher()
{}
void dispatchResponse( RPCDispatcher *RPCDisp, XStanza *xs )
{
   if ( RPCDisp )
   {
      RPCDisp->dispatchResponse( xs );
   }
}

bool analyseNode( RPCDispatcher *RPCDisp, TiXmlElement *tix )
{
   if ( !tix || !checkElementName( tix, "iq" ) )
   {
      return false;
   }

   QString from = getAttribute( tix, "from" );
   //QString to = getAttribute( tix, "to" );
   QString id = getAttribute( tix, "id" );
   QString subtype = getAttribute( tix, "type" );

   TiXmlElement *query = findNode( tix, "query" );
   if ( !query )
   {
      return false;
   }
   QString ns = getAttribute( query, "xmlns" );
   if ( ns != "minos:iq:rpc" )
   {
      return false;
   }

   TiXmlElement *mcall = findNode( query, "methodCall" );

   if ( subtype == "set" && mcall )
   {
      RPCRequest * xs = new RPCRequest( from, mcall );
      xs->setId( id );
      dispatchResponse( RPCDisp, xs );
      delete xs;
      return true;
   }
   return false;
}
bool analyseNode(RPCDispatcher *RPCDisp, TIXML_STRING UTF8XML )
{
   TiXmlBase::SetCondenseWhiteSpace( false );
   TiXmlDocument xdoc;
   xdoc.Parse( UTF8XML.c_str(), nullptr );
   if (xdoc.Error())
   {
       trace(QString("parse error: ") + xdoc.ErrorDesc());
   }
   else
   {
       TiXmlElement *tix = xdoc.RootElement();
       bool ret = analyseNode( RPCDisp, tix );
       return ret;
   }
   return false;
}

