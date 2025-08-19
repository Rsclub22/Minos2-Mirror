#include "QtUtils.h"
#include "MTrace.h"
#include "TinyUtils.h"
#include "XMPPEvents.h"
#include "clientThread.h"
#include "serverThread.h"
#include "MServerZConf.h"
#include "MServer.h"
#include "minoslistener.h"

MinosRouterListener *MinosRouterListener::MSL = nullptr;
MinosClientListener *MinosClientListener::MCL = nullptr;

//=============================================================================
void xperror( int test_val, QString s, bool endit = true )
{
   if ( test_val )
   {
      trace( "xperror: " + s );

      if ( endit )
      {
         throw Exception( QString(("JServer error: " + s )).toStdString().c_str());
      }
   }
}
//=============================================================================
MinosListener::MinosListener():sock(new QTcpServer(this))
{
    connect(&CheckTimer, &QTimer::timeout, this, &MinosListener::on_timeout, Qt::UniqueConnection);
    CheckTimer.start(100);
}
MinosListener::~MinosListener()
{
   clearSockets();
}
//-----------------------------------------------------------------------------
bool MinosListener::initialise( QString type, quint16 port )
{
    QHostAddress::SpecialAddress addr = QHostAddress::Any;

    if (type == "Client")
    {
        addr = QHostAddress::LocalHost;
    }
    connect(sock.data(), &QTcpServer::newConnection, this, &MinosListener::on_newConnection, Qt::UniqueConnection);
   bool res = sock->listen(addr, port); // signals newConnection()
   xperror( res == false ,type + " listen"  );

   return true;
}

void MinosListener::addListenerSlot( MinosCommonConnection *il )
{
    il->strace( QString("addListenerSlot: from %1 %2").arg(isRouter()?"Server":"Client", il->connectHost.toString() ));

    i_array.push_back( il );
    il->initialise();
}
int MinosListener::getConnectionCount()
{
    return i_array.size();
}

MinosCommonConnection *MinosListener::getConnection(int i)
{
    return i_array[i];
}

//==============================================================================
// Check for activity on the sockets

// Predicate function for remove_if
bool nosock( MinosCommonConnection *ip )
{
   if ( ip == nullptr )
      return true;
   else
      return false;
}
void MinosListener::on_newConnection()
{
    // called when we get a new connection from "accept" - i.e. they called us
    QTcpSocket *s = sock->nextPendingConnection();
    if (s)
    {
        MinosCommonConnection *cc = makeConnection(s);

        addListenerSlot(cc);
    }
}
void MinosListener::on_timeout()
{
    bool clearup = false;
    for ( auto &a: i_array )
    {
        if ( a ->remove_socket )
        {
            trace("MinosListener::on_timeout() close_socket = true, removing socket");
            // process says to finish off
            if (a->publish_disconnect)
            {
                a->closeDown();
            }
            a->sock->close();
            a->deleteLater();
            a = nullptr;
            clearup = true;
        }
    }
    if ( clearup )
    {
        i_array.erase( std::remove_if( i_array.begin(), i_array.end(), nosock ), i_array.end() );
    }

}

void MinosListener::clearSockets()
{
      for ( auto  &a: i_array )
      {
        a->sock->close();
        a->deleteLater();
        a = nullptr;
      }
      i_array.clear();
}
//==============================================================================

MinosCommonConnection *MinosRouterListener::makeConnection(QTcpSocket *s)
{
    trace("Creating MinosRouterConnection in MinosRouterListener::makeConnection");
    MinosRouterConnection *c = new MinosRouterConnection(false);

    c->sock = QSharedPointer<QTcpSocket>(s);
    c->connectHost = c->sock->peerAddress();
    c->myAddr = c->sock->localAddress();

    c->strace (QString("Router Connection to peer %1 from local %2").arg(c->sock->peerAddress().toString(), c->sock->localAddress().toString()));

    return c;
}

bool MinosRouterListener::sendRouter( TiXmlElement *tix )
{
    MinosId to( getAttribute( tix, "to" ) );

    if ( to.router.size() == 0 )
        return false;
    if ( to.router.compare( ThisMinosRouter::getThisMinosRouter() ->getRouterName(), Qt::CaseInsensitive) == 0 )
        return false;
    if ( to.router.compare( DEFAULT_ROUTER_NAME, Qt::CaseInsensitive ) == 0 )
        return false;

    // OK, it is not for us... look at connected servers

    for ( auto const &a: QASCONST(i_array ))
    {
        if ( a ->checkRouter( to ) )
        {
            if ( !a->tryForwardStanza( tix ) )
            {
                break;
            }
            break;
        }
    }

// server is not connected; ignore the message.
// The originator should retry sometime later to send message again

return true;   // don't pass it on - either we have dealt with it, or its not useful
}

void MinosRouterListener::buildTable(QTableWidget *tab)
{
    tab->clear();
    tab->setRowCount(i_array.count());
    tab->setColumnCount(4);
    QStringList h = {"name", "sequence", "address", "uuid"};
    tab->setHorizontalHeaderLabels(h);
    int row = 0;
    for ( auto const &a: QASCONST(i_array ))
    {
        MinosRouterConnection *msc = dynamic_cast<MinosRouterConnection *>(a);
        QString router = msc->getClientRouter();
        QTableWidgetItem *s = new QTableWidgetItem(router);
        tab->setItem(row, 0, s);
        s = new QTableWidgetItem(QString::number(msc->mySeq));
        tab->setItem(row, 1, s);
        s = new QTableWidgetItem(msc->router()->host.toString());
        tab->setItem(row, 2, s);
        s = new QTableWidgetItem(msc->router()->uuid);
        tab->setItem(row, 3, s);
        row++;
    }
}

void MinosRouterListener::closeDown()
{
    MSL = nullptr;
}

MinosRouterConnection *MinosRouterListener::findConnection(const QHostAddress &h)
{
    for ( auto const &a: QASCONST(i_array ))
    {
        if (h.toIPv4Address() == a->connectHost.toIPv4Address())
        {
            return dynamic_cast<MinosRouterConnection *>(a);
        }
    }
    return nullptr;
}

//==============================================================================
MinosClientListener::MinosClientListener()
{
   MCL = this;
}
MinosClientListener::~MinosClientListener()
{
   MCL = nullptr;
}
//==============================================================================
MinosCommonConnection *MinosClientListener::makeConnection(QTcpSocket *s)
{
    MinosClientConnection *c = new MinosClientConnection();
    c->sock = QSharedPointer<QTcpSocket>(s);
    c->connectHost = c->sock->peerAddress();

    c->strace (QString("Client Connection from %1 to %2").arg(c->sock->peerAddress().toString(), c->sock->localAddress().toString()));

    return c;
}
//==============================================================================
bool MinosClientListener::sendClient( TiXmlElement *tix )
{
   MinosId from( getAttribute( tix, "from" ) );
   MinosId to( getAttribute( tix, "to" ) );

   if ( to.user.size() == 0 )
   {
      // invalid to servername (still default) or no "to" user
      return false;
   }
   bool fromLocal = false;
   bool fromLocalHost = false;
   bool toLocal = false;
   bool toLocalHost = false;

   bool addressOK = false;

   if ( from.empty() || from.router.compare( ThisMinosRouter::getThisMinosRouter() ->getRouterName(), Qt::CaseInsensitive ) == 0 )
      fromLocal = true;

   if ( from.empty() && from.router.compare( DEFAULT_ROUTER_NAME, Qt::CaseInsensitive ) == 0 )
      fromLocalHost = true;

   if ( to.router.compare( ThisMinosRouter::getThisMinosRouter() ->getRouterName(), Qt::CaseInsensitive ) == 0 )
      toLocal = true;

   if ( to.router.compare( DEFAULT_ROUTER_NAME, Qt::CaseInsensitive ) == 0 )
      toLocalHost = true;

   if ( toLocal )
      addressOK = true;      // proper "to" server, from anywhere

   if ( ( fromLocal || fromLocalHost ) && toLocalHost )      // "to" localhost OK if "from" local app
      addressOK = true;

   if ( fromLocal && !toLocal && !toLocalHost )      // from proper address, to remote address
      addressOK = true;

   if ( addressOK && !toLocal && !toLocalHost )
      return false;

   /* BUT (and we don't need to check...
   if (fromLocalHost && !(toLocal || toLocalHost))
      addressOK = false;
   */

   if ( !addressOK )
   {
      return true;   // don't pass it on - its not useful
   }

   // OK, it is for what might be one of our clients

   for ( auto const &a: QASCONST(i_array ))
   {
      // worry about the details
      if (a && a->checkUser( to ) )
      {
         if ( !a->tryForwardStanza( tix ) )
         {
            // send failed
            break;    // OK, we can't send it, forget it
         }
         break; // sent OK
      }
   }
   return true;   // Either we have dealt with it, or its not useful
}

void MinosClientListener::buildTable(QTableWidget *tab)
{
    tab->clear();
    tab->setRowCount(i_array.count());
    tab->setColumnCount(1);
    int row = 0;
    for ( auto const &a: QASCONST(i_array ))
    {
        QString client = a->getClientUser();
        QTableWidgetItem *s = new QTableWidgetItem(client);
        tab->setItem(row++, 0, s);
    }
}
void MinosClientListener::closeDown()
{

}
