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

#include "MinosLink.h"
#include "clientThread.h"
#include "serverThread.h"

#include "minoslistener.h"
#include "MServer.h"

extern bool closeApp;


bool isHostLocal(const QString &host)
{
    QHostInfo connaddr = QHostInfo::fromName( host );
    if ( connaddr.addresses().contains(QHostInfo::fromName( "127.0.0.1" ).addresses().first() ))
    {
       return true;             // connection address is 127.0.0.1
    }
    return false;
}
//==============================================================================

bool MinosId::empty()
{
   return fullId.size() == 0;
}

MinosId::MinosId()
{
}
MinosId::MinosId( const QString &s )
{
   setId( s );
}
MinosId::~MinosId()
{
}

void MinosId::setId( const QString &s )
{
   fullId = s;
   int atpos = s.indexOf( "@" );
   if ( atpos < 0 )
   {
      server = s;
   }
   else
   {
      user = s.left(atpos );
      server = s.right( s.size() - atpos - 1 );
   }
}

//=============================================================================

// called from XMPPRPCObj instead of the one in MinosConnection
static void serversendAction( XStanza *a )
{
   // stanza has a "to" - but this is internal, so we need to dispatch it
   TIXML_STRING mess = a->getActionMessage();

   // convert from a RPCParam structure to a DOM

   TiXmlBase::SetCondenseWhiteSpace( false );
   TiXmlDocument xdoc;
   xdoc.Parse( mess.c_str(), nullptr );
   TiXmlElement *x = xdoc.RootElement();

   if ( a->getFrom().size() == 0 )
   {
      // insert a from of ourselves.

      QString from = MinosServer::getMinosServer() ->getServerName();
      if ( from.length() )
      {
         x->SetAttribute( "from", from.toStdString().c_str() );
      }
   }
   QString to = a->getTo();
   if ( to.size() != 0 )
   {
      x->SetAttribute( "to", to.toStdString().c_str() );
   }
   // and now dispatch to its destination

   if ( !MinosServer::getMinosServer() ->forwardStanza( nullptr, x ) )              // our own services
   {
      if ( !MinosClientListener::getListener() ->sendClient( x ) )         // look at real and potential clients
      {
         if ( !MinosServerListener::getListener() ->sendServer( x ) )         // look at real and potential servers
         {
            // or no valid destination found
            return ;
         }
      }
   }
   // or no valid destination found
}
//==============================================================================
MinosCommonConnection::MinosCommonConnection()
    :
    remove_socket( false )
  , fromIdSet( false )
{
    setSendAction(serversendAction);

    lastRx = QDateTime::currentMSecsSinceEpoch() + 5000;
}
MinosCommonConnection::~MinosCommonConnection()
{}
QString MinosCommonConnection::getClientServer() const
{
    return clientServer;
}

QString MinosCommonConnection::getClientUser() const
{
    return clientUser;
}

QString MinosCommonConnection::makeJid()
{
    QString id;
    if ( clientUser.length() )
      id = clientUser + "@";
   id += clientServer;

   return id;
}
bool MinosCommonConnection::sendRaw ( const TIXML_STRING xmlstr )
{
   size_t xmllen = xmlstr.length();
   if ( xmllen )
   {
      char * xmlbuff = new char[ 10 + 1 + xmllen + 1 ];
      sprintf( xmlbuff, "&&%lu%s&&", static_cast<unsigned long>(xmllen), xmlstr.c_str() );
      xmllen = strlen( xmlbuff );

      qint64 ret = sock->write ( xmlbuff, xmllen );
      onLog ( xmlbuff, xmllen, 0 );
      delete [] xmlbuff;

      if ( ret == -1 )  // QIOdevice::write returned an error.
         return false;
   }
   return true;
}
void MinosCommonConnection::onLog ( const char *data, size_t /*size*/, int is_incoming )
{
   QString logbuff;
   if ( is_incoming )
      logbuff += "RECV";
   else
      logbuff += "SEND";

   logbuff += "[";
   logbuff += data;
   logbuff += "]";

   logbuff += " : " + sock->peerAddress().toString();

   trace( "MinosCommonConnection: " + logbuff );
}
//---------------------------------------------------------------------------
bool MinosCommonConnection::tryForwardStanza( TiXmlElement *tix )
{
   TIXML_STRING s;
   s << *tix;
   bool res = sendRaw( s );
   return res;
}
//==============================================================================
void MinosCommonConnection::on_readyRead()
{
   // select says we have data, so read it
   // and send the data through the parser
   trace ( "MinosCommonConnection::on_readyRead called to receive data from " + connectHost.toString() );

   // documntation says this may occasionally fail on Windows
   while (sock->bytesAvailable() > 0)
   {

       qint64 rxlen = sock->read(rxbuff, 4096 - 1);
       if ( rxlen > 0 )
       {
          rxbuff[ rxlen ] = 0;

          // We might have embedded nulls between message parts - so strip them
          int rxpt = 0;
          while ( rxpt < rxlen )
          {
             size_t ptlen = strlen( &rxbuff[ rxpt ] );
             if ( ptlen )
             {
                onLog ( &rxbuff[ rxpt ], ptlen, 1 );  // but this ignores the wrapper
                packetbuff += &rxbuff[ rxpt ];   // which will strip out any nulls
             }
             rxpt += ptlen + 1;
          }

          while ( packetbuff.size() > 2 && packetbuff.left( 2 ) == "&&" )
          {
             int packetoffset = packetbuff.indexOf( '<' );
             if ( packetoffset > 0 )    // length field should always be followed by XML
             {
                 QStringRef slen = packetbuff.midRef(2, packetoffset - 2);
                 int packetlen = slen.toInt();
                if ( (packetlen <= static_cast<int> (packetbuff.size()) - 2) && packetbuff.indexOf( ">&&" ) >= 0 )
                {
                   QString packet = packetbuff.mid( packetoffset, packetlen );
                   int pbsize = packetbuff.size();
                   int rlen = pbsize - 2 - packetlen - packetoffset;
                   if (rlen < 0)
                       rlen = 0;    // try to fix non-utf characters, e.g. degree character
                   packetbuff = packetbuff.right(  rlen );

                   if (packet.length())
                   {
                       TiXmlBase::SetCondenseWhiteSpace( false );
                       TiXmlDocument xdoc;
                       TIXML_STRING p = packet.toStdString();
                       xdoc.Parse( p.c_str(), nullptr );
                       if ( xdoc.Error())
                       {
                           trace(QString("parse failed; ") + xdoc.ErrorDesc());
                       }
                       else
                       {
                           TiXmlElement *tix = xdoc.RootElement();
                           analyseNode( tix );
                       }
                   }
                   else
                   {
                       trace("empty packet!");
                   }
                }
                else
                {
                   // partial message, keep receiving until we get more
                   break ;
                }
             }
             else
             {
                 // another form of partial message
                break;
             }
          }
       }
       else if (rxlen < 0)
       {
           trace("Bad read in MinosCommonConnection::on_readyRead; remove_socket = true");
          remove_socket = true;
       }
       if (rxlen >= 0)
       {
           // rxlen == 0 is valid
           lastRx = QDateTime::currentMSecsSinceEpoch();
       }
   }
}
//==============================================================================
bool MinosCommonConnection::analyseNode( TiXmlElement *tix )
{
   // response factory - look at the node, and build the correct response stanza object

   // checkFrom looks at the "from" address, and validates it against the connection
   // A server connection has to have a "from" (but it isn't necessarily correct, if its been proxied)
   // A client must either have a from address, or nothing - when checkFrom will insert it

    if (  checkElementName( tix, "keepAlive" ) )
    {
        return true;
    }

    if ( !checkFrom( tix ) )
   {
      if ( isServer() )
      {
         closeSocket();
         trace("Bad checkFrom in MinosCommonConnection::analyseNode; remove_socket = true");
         remove_socket = true;
      }
      return false;
   }
   // Dispatch to its destination

   // we should first look for "built in" clients - initially this means PubSub
   // ZConf is a possibility, but it (currently) works via PubSub
   // Actually, we could build these in to the server as RPC calls
   // - don't need a decent "to", just the server name
   if ( !MinosServer::getMinosServer() ->forwardStanza( this, tix ) )              // our own services
   {
      if ( !MinosClientListener::getListener() ->sendClient( tix ) )         // look at real and potential clients
      {
         if ( !MinosServerListener::getListener() ->sendServer( tix ) )         // look at real and potential servers
         {
            // or no valid destination found
            return false;
         }
      }
   }
    return true;
}
//=============================================================================
void MinosCommonConnection::on_disconnected()
{
    trace("MinosCommonConnection::on_disconnected() " + clientServer + " " + clientUser + "; remove_socket = true");
    remove_socket = true;
}
