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

#include "MinosLink.h"
#include "clientThread.h"
#include "serverThread.h"
#include "PubSubServer.h"
#include "MServerZConf.h"
#include "MServer.h"
#include "MServerPubSub.h"
#include "minoslistener.h"

//==============================================================================
//==============================================================================
MinosRouterConnection::MinosRouterConnection(bool fromDatagram) : fromDatagram(fromDatagram)
{}
void MinosRouterConnection::initialise()
{
    QHostAddress h = sock->peerAddress();
    connectHost = h;
    connect(sock.data(), SIGNAL(readyRead()), this, SLOT(on_readyRead()), Qt::UniqueConnection);
    connect(sock.data(), SIGNAL(disconnected()), this, SLOT(on_disconnected()), Qt::UniqueConnection);

    connect(&resubscribeTimer, SIGNAL(timeout()), this, SLOT(sendKeepAlive()), Qt::UniqueConnection);
    resubscribeTimer.start(1000);
}

MinosRouterConnection::~MinosRouterConnection()
{
}
void MinosRouterConnection::closeDown()
{
   trace( "Server Link: Closing" );

   if (PubSubMain)
       PubSubMain->disconnectRouter(makeJid());
   if (srv)
   {
      TZConf::getZConf()->publishDisconnect(srv);
   }
}

bool MinosRouterConnection::checkFrom( TiXmlElement *tix )
{
   // No "from" on a server link - kill the link
   const char * from = tix->Attribute( "from" );

   if ( !from )
      return false;

   // but "from" doesn't actually need to be correct - it could have been proxied...

   return true;
}

void MinosRouterConnection::setRouter(Router *s)
{
    srv = s;
    clientRouter = srv->station;
    trace( QString( "Server: Connecting to " ) + srv->station + " host " + srv->host.toString() );
}

void MinosRouterConnection::mConnect( Router *psrv )
{
   srv = psrv;
   clientRouter = srv->station;
   connectHost = srv->host;

   trace( QString( "Server: Connecting to " ) + srv->station + " host " + srv->host.toString() );

   // connect to endpoint
   // We need to connect out to the end point - looks much like a client connection!
    sock = QSharedPointer<QTcpSocket>(new QTcpSocket);

    connect(sock.data(), SIGNAL(connected()), this, SLOT(on_connected()), Qt::UniqueConnection);
    connect(sock.data(), SIGNAL(disconnected()), this, SLOT(on_disconnected()), Qt::UniqueConnection);
    connect(sock.data(), SIGNAL(readyRead()), this, SLOT(on_readyRead()), Qt::UniqueConnection);
    sock->connectToHost(srv->host, srv->port);
}
void MinosRouterConnection::on_connected()
{
    trace( QString( "Server: Connected OK to " ) + srv->station + " host " + srv->host.toString() );
    RPCRequest *rpa = new RPCRequest( clientRouter, ThisMinosRouter::getThisMinosRouter() ->getRouterName(), "ServerSetFromId" );   // for our local server, this one MUST have a from
    rpa->addParam( ThisMinosRouter::getThisMinosRouter() ->getRouterName() );
    rpa->addParam( TZConf::getZConf()->getZConfString(false, connectHost.toString() ) );
    sendAction( rpa );
    delete rpa;
}

//==============================================================================
void MinosRouterConnection::setFromId( MinosId &id, RPCRequest *req )
{
   resubscribed = false;
   // and we need to check that the originator is who we think they ought to be
   if ( !id.router.size() )
   {
      trace( "ServerSetFromId: No \"from\" from server " + srv->station );
      return;
   }
   if ( srv && srv->station.compare( id.router, Qt::CaseInsensitive) != 0 )
   {
      trace( "ServerSetFromId: Mismatch from server " + srv->station + " we received \"" + id.router + "\"" );
      return;
   }

   if ( !srv )
   {
      // we need to find who is connecting to us
      QVector<Router *>::iterator srvi = findStation( id.router );
      if (srvi != routerList.end())
      {
          srv = *srvi;
      }
      if ( srv )
      {
         trace( "ServerSetFromId: server " + srv->station + " connected to us" );
      }
      else
      {
         trace( "ServerSetFromId: server " + QString( id.router ) + " tried to connect to us - not recognised" );
         // SO we need to set up a server

         QString message;
         if (req->getStringArg(1, message))
         {
             QDateTime sb;   // ignored response
             QHostAddress host = connectHost;
             srv = TZConf::getZConf()->processZConfString(message, host, sb);
         }
      }
   }
   else
   {
      trace( "ServerSetFromId: server " + id.router + " connected to us - srv already set up as " + srv->station );
   }
   clientRouter = id.router;
}
//==============================================================================
void MinosRouterConnection::sendAction( XStanza *a )
{
   // use the stanza to send itself
   a->setNextId();   // only happens if no Id already

   TIXML_STRING s = a->getActionMessage();
   sendRaw( s );
}
//==============================================================================
void MinosRouterConnection::sendKeepAlive( )
{
    if (srv)
    {
        if (!checkLastRx())
        {
            // abort the connection
            trace("MinosServerConnection - checkLastRx failed, removing socket");
            remove_socket = true;
            return;
        }
        // Every ??? send a heartbeat to make sure the link stays open
        if ( !resubscribed && srv )
        {
            if ( clientRouter.size() && clientRouter.compare( "localhost", Qt::CaseInsensitive ) != 0 &&
                 clientRouter.compare( ThisMinosRouter::getThisMinosRouter() ->getRouterName(), Qt::CaseInsensitive) != 0 )
            {
                RPCRouterPubSub::routerReconnectRemotePubSub( srv->station );
                resubscribed = true;
                return ;
            }
        }
        static int seqno = 0;
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (now - lastKeepAlive > resubscribeTimer.interval() * 2 )
        {
            sendRaw(QString("<keepAlive seq='" + QString::number(seqno++) + "'/>").toStdString());
            lastKeepAlive = now;
        }
    }
}
bool MinosRouterConnection::checkLastRx()
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - lastRx > resubscribeTimer.interval() * 5)
    {
        return false;
    }

    return true;
}
//==============================================================================
void MinosRouterConnection::sendCloseSocket()
{
    if (srv)
    {
       sendRaw(QString("<closeSocket/>").toStdString());
       sock->waitForBytesWritten();
    }
}
//==============================================================================

