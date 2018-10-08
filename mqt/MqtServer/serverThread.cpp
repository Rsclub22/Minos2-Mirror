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
MinosServerConnection::MinosServerConnection(bool fromDatagram) : srv( nullptr ), resubscribed( false ), fromDatagram(fromDatagram)
{}
void MinosServerConnection::initialise()
{
    QHostAddress h = sock->peerAddress();
    connectHost = h;
    connect(sock.data(), SIGNAL(readyRead()), this, SLOT(on_readyRead()));
    connect(sock.data(), SIGNAL(disconnected()), this, SLOT(on_disconnected()));

    connect(&resubscribeTimer, SIGNAL(timeout()), this, SLOT(sendKeepAlive()));
    resubscribeTimer.start(1000);
}

MinosServerConnection::~MinosServerConnection()
{
}
void MinosServerConnection::closeDown()
{
   trace( "Server Link: Closing" );

   if (PubSubMain)
       PubSubMain->disconnectServer(makeJid());
   if (srv)
      TZConf::getZConf()->publishDisconnect(srv->station);
}

bool MinosServerConnection::checkFrom( TiXmlElement *tix )
{
   // No "from" on a server link - kill the link
   const char * from = tix->Attribute( "from" );

   if ( !from )
      return false;

   // but "from" doesn't actually need to be correct - it could have been proxied...

   return true;
}

void MinosServerConnection::setServer(Server *s)
{
    srv = s;
    clientServer = srv->station;
    trace( QString( "Server: Connecting to " ) + srv->station + " host " + srv->host.toString() );
}

void MinosServerConnection::mConnect( Server *psrv )
{
   srv = psrv;
   clientServer = srv->station;

   trace( QString( "Server: Connecting to " ) + srv->station + " host " + srv->host.toString() );

   // connect to endpoint
   // We need to connect out to the end point - looks much like a client connection!
    sock = QSharedPointer<QTcpSocket>(new QTcpSocket);

    connect(sock.data(), SIGNAL(connected()), this, SLOT(on_connected()));
    connect(sock.data(), SIGNAL(disconnected()), this, SLOT(on_disconnected()));
    connect(sock.data(), SIGNAL(readyRead()), this, SLOT(on_readyRead()));
    sock->connectToHost(srv->host, srv->port);
}
void MinosServerConnection::on_connected()
{
    trace( QString( "Server: Connected OK to " ) + srv->station + " host " + srv->host.toString() );
    RPCRequest *rpa = new RPCRequest( clientServer, MinosServer::getMinosServer() ->getServerName(), "ServerSetFromId" );   // for our local server, this one MUST have a from
    rpa->addParam( MinosServer::getMinosServer() ->getServerName() );
    rpa->addParam( TZConf::getZConf()->getZConfString(false, connectHost.toString() ) );
    sendAction( rpa );
    delete rpa;
}

//==============================================================================
void MinosServerConnection::setFromId( MinosId &id, RPCRequest *req )
{
   resubscribed = false;
   // and we need to check that the originator is who we think they ought to be
   if ( !id.server.size() )
   {
      trace( "ServerSetFromId: No \"from\" from server " + srv->station );
      return;
   }
   if ( srv && srv->station.compare( id.server, Qt::CaseInsensitive) != 0 )
   {
      trace( "ServerSetFromId: Mismatch from server " + srv->station + " we received \"" + id.server + "\"" );
      return;
   }

   if ( !srv )
   {
      // we need to find who is connecting to us
      srv = findStation( id.server );
      if ( srv )
      {
         trace( "ServerSetFromId: server " + srv->station + " connected to us" );
      }
      else
      {
         trace( "ServerSetFromId: server " + QString( id.server ) + " tried to connect to us - not recognised" );
         // SO we need to set up a server

         QString message;
         if (req->getStringArg(1, message))
         {
             MinosServerListener *msl = MinosServerListener::getListener();
             MinosClientListener *mcl = MinosClientListener::getListener();

             QDateTime sb;   // ignored response
             QHostAddress host = connectHost;
             srv = TZConf::getZConf()->processZConfString(message, host, sb);
         }
      }
   }
   else
   {
      trace( "ServerSetFromId: server " + id.server + " connected to us - srv already set up as " + srv->station );
   }
   clientServer = id.server;
}
//==============================================================================
void MinosServerConnection::sendAction( XStanza *a )
{
   // use the stanza to send itself
   a->setNextId();   // only happens if no Id already

   TIXML_STRING s = a->getActionMessage();
   sendRaw( s );
}
//==============================================================================
void MinosServerConnection::sendKeepAlive( )
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
            if ( clientServer.size() && clientServer.compare( "localhost", Qt::CaseInsensitive ) != 0 &&
                 clientServer.compare( MinosServer::getMinosServer() ->getServerName(), Qt::CaseInsensitive) != 0 )
            {
                RPCServerPubSub::serverReconnectRemotePubSub( srv->station );
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
bool MinosServerConnection::checkLastRx()
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - lastRx > resubscribeTimer.interval() * 5)
    {
        return false;
    }

    return true;
}
//==============================================================================

