/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#include "MTrace.h"
#include "MinosLink.h"
#include "MServerPubSub.h"
#include "clientThread.h"

//==============================================================================
void MinosClientConnection::strace(const QString &mess)
{
    trace(QString(mess + " PeerAddress %1 connectAddress %2")
              .arg(sock?sock->peerAddress().toString():"No socket", connectHost.toString()));
}
MinosClientConnection::MinosClientConnection()
{
}
void MinosClientConnection::initialise()
{
   QHostAddress h = sock->peerAddress();
   connectHost = h;
   connect(sock.data(), &QTcpSocket::readyRead, this, &MinosClientConnection::on_readyRead, Qt::UniqueConnection);
   connect(sock.data(), &QTcpSocket::disconnected, this, &MinosClientConnection::on_disconnected, Qt::UniqueConnection);
}

MinosClientConnection::~MinosClientConnection()
{
}
//==============================================================================
void MinosClientConnection::closeDown()
{
    strace( "Client Link: Closing" );
    // here we need to revoke all of this clients published keys
    PubSubMain->revokeClient(makeJid());
}

void MinosClientConnection::disconnected()
{
    remove_socket = true;
}
//==============================================================================
bool MinosClientConnection::checkFrom( TiXmlElement *tix )
{
   if ( !fromIdSet )
   {
      return true;
   }
   // if no "from" insert one

   const char *sfrom = tix->Attribute( "from" );
   if ( !sfrom )
   {
      // insert a from of ourselves

      QString from = makeJid();
      if ( from.size() )
      {
         tix->SetAttribute( "from", from.toStdString() );
      }
   }
   else
   {
      MinosId from( sfrom );
      if ( !checkRouter( from ) )
         return false;

      if ( !checkUser( from ) )
         return false;
   }
   return true;
}
//==============================================================================
void MinosClientConnection::setFromId( MinosId &from, RPCRequest * /*req*/ )
{
   // this should always be a local client
   clientRouter = from.router;
   clientUser = from.user;
   fromIdSet = true;
}
//==============================================================================

