/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////

#include "MinosLink.h"
#include "serverThread.h"
#include "PubSubServer.h"
#include "MServerZConf.h"
#include "MServer.h"
#include "MServerPubSub.h"
#include "MTrace.h"

//==============================================================================
void MinosRouterConnection::strace(const QString &mess)
{
    trace(QString(mess + " ServerSequence %1 PeerAddress %2 connectAddress %3")
              .arg(QString::number(mySeq), sock?sock->peerAddress().toString():"No socket", connectHost.toString()));
}
MinosRouterConnection::MinosRouterConnection(bool fromDatagram) : fromDatagram(fromDatagram)
{
    serverSequence++;
    mySeq = serverSequence;
}
void MinosRouterConnection::initialise()
{
    QHostAddress h = sock->peerAddress();
    connectHost = h;
    connect(sock.data(), &QTcpSocket::readyRead, this, &MinosRouterConnection::on_readyRead, Qt::UniqueConnection);
    connect(sock.data(), &QTcpSocket::disconnected, this, &MinosRouterConnection::on_disconnected, Qt::UniqueConnection);
    connect(sock.data(), &QTcpSocket::hostFound, this, &MinosRouterConnection::on_hostFound, Qt::UniqueConnection);
    connect(sock.data(), &QTcpSocket::connected, this, &MinosRouterConnection::on_connected, Qt::UniqueConnection);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(sock.data(), &QTcpSocket::errorOccurred, this, &MinosRouterConnection::connectionError);
#else
    connect(sock.data(), SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(MinosRouterConnection(QAbstractSocket::SocketError)));
#endif


    connect(&resubscribeTimer, &QTimer::timeout, this, &MinosRouterConnection::sendKeepAlive, Qt::UniqueConnection);
    resubscribeTimer.start(1000);
}
void MinosRouterConnection::connectionError(QAbstractSocket::SocketError /*error*/)
{
    strace("MinosRouterConnection::connectionError Socket error: " + sock->errorString());
}
void MinosRouterConnection::on_hostFound()
{
    strace("MinosRouterConnection::on_hostFound");
}
MinosRouterConnection::~MinosRouterConnection()
{
}
void MinosRouterConnection::closeDown()
{
   strace( "Server Link: Closing" );

   if (PubSubMain)
       PubSubMain->disconnectRouter(makeJid());
   if (srv)
   {
      TZConf::getZConf()->publishDisconnect(srv);
   }
}

void MinosRouterConnection::disconnected()
{
    // if server we need to see if is a true disconnect, or a "spare"
    remove_socket = true;
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
    strace( QString( "MinosRouterConnection::setRouter: Connecting to " ) + srv->station + " host " + srv->host.toString() );
}

void MinosRouterConnection::mConnect( Router *psrv )
{
   srv = psrv;
   clientRouter = srv->station;
   connectHost = srv->host;

   strace( QString( "MinosRouterConnection::mConnect: Connecting to " ) + srv->station + " host " + srv->host.toString() );

   // connect to endpoint
   // We need to connect out to the end point - looks much like a client connection!
    sock = QSharedPointer<QTcpSocket>(new QTcpSocket);

    connect(sock.data(), &QTcpSocket::connected, this, &MinosRouterConnection::on_connected, Qt::UniqueConnection);
    connect(sock.data(), &QTcpSocket::disconnected, this, &MinosRouterConnection::on_disconnected, Qt::UniqueConnection);
    connect(sock.data(), &QTcpSocket::readyRead, this, &MinosRouterConnection::on_readyRead, Qt::UniqueConnection);
    sock->connectToHost(srv->host, srv->port);
}
void MinosRouterConnection::on_connected()
{
    strace( QString( "Server: Connected OK to %1 host %2" ).arg(srv->station, srv->host.toString() ) );
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
      strace( "ServerSetFromId: No \"from\" from server " + srv->station );
      return;
   }
   if ( srv && srv->station.compare( id.router, Qt::CaseInsensitive) != 0 )
   {
      strace( "ServerSetFromId: Mismatch from server " + srv->station + " we received \"" + id.router + "\"" );
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
      else
      {
         srvi = findIp( sock->peerAddress() );
          if (srvi != routerList.end())
          {
              srv = *srvi;
          }
      }
      if ( srv )
      {
         strace( "ServerSetFromId: server " + srv->station + " connected to us" );
      }
      else
      {
         strace( "ServerSetFromId: server " + QString( id.router ) + " tried to connect to us - not recognised" );
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
      strace( "ServerSetFromId: server " + id.router + " connected to us - srv already set up as " + srv->station );
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
            strace(QString("MinosRouterConnection::checkLastRx failed, removing socket"));
            publish_disconnect = false;
            remove_socket = true;
            return;
        }
        // Every ??? send a heartbeat to make sure the link stays open
        if ( !resubscribed && srv )
        {
            if ( clientRouter.size() && clientRouter.compare( "localhost", Qt::CaseInsensitive ) != 0 &&
                 clientRouter.compare( ThisMinosRouter::getThisMinosRouter() ->getRouterName(), Qt::CaseInsensitive) != 0 )
            {
                strace(QString("MinosRouterConnection::checkLastRx - resubscribtion needed"));
                RPCRouterPubSub::routerReconnectRemotePubSub( srv->station );
                resubscribed = true;
                return ;
            }
        }
        static int seqno = 0;
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (now - lastKeepAlive > resubscribeTimer.interval() * 2 )
        {
            strace(QString("MinosRouterConnection::checkLastRx - send keepAlive"));
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

