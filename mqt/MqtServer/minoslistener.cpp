#include "minos_pch.h"

#include "minoslistener.h"
#include "clientThread.h"
#include "serverThread.h"
#include "MServerZConf.h"
#include "MServer.h"

MinosServerListener *MinosServerListener::MSL = nullptr;
MinosClientListener *MinosClientListener::MCL = nullptr;

//=============================================================================
void xperror( int test_val, QString s, bool endit = true )
{
   if ( test_val )
   {
      trace( "xperror: " + s );

      if ( endit )
      {
         throw Exception( ("JServer error: " + s ).toStdString().c_str());
      }
   }
}
//=============================================================================
MinosListener::MinosListener():sock(new QTcpServer(this))
{
    connect(&CheckTimer, SIGNAL(timeout()), this, SLOT(on_timeout()), Qt::UniqueConnection);
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
    connect(sock.data(), SIGNAL(newConnection()), this, SLOT(on_newConnection()), Qt::UniqueConnection);
   bool res = sock->listen(addr, port); // signals newConnection()
   xperror( res == false ,type + " listen"  );

   return true;
}

void MinosListener::addListenerSlot( MinosCommonConnection *il )
{
    trace( QString("addListenerSlot: from %1 %2").arg(isServer()?"Server":"Client").arg(il->connectHost.toString() ));

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
    if (isServer())
    {
        // This is intended to remove the second link each process
        // gets initially
        for ( CommonIterator i = i_array.begin(); i != i_array.end(); i++ )
        {
            QString hi = (*i)->getClientServer();
            if ((*i)->remove_socket)
                continue;
            for ( CommonIterator j = i + 1; j != i_array.end(); j++ )
            {
                QString hj = (*j)->getClientServer();
                if (hi == hj)
                {
                    quint32 remIP = (*j)->sock->peerAddress().toIPv4Address();
                    quint32 locIP = (*j)->sock->localAddress().toIPv4Address();

                    // make sure only one end does the removal
                    if (remIP < locIP)
                    {
                        (*j)->remove_socket = true;
                        (*j)->publish_disconnect = false;
                        trace("removing socket for " + (*j)->getClientServer());
                    }
                }
            }
        }

    }
    bool clearup = false;
    for ( CommonIterator i = i_array.begin(); i != i_array.end(); i++ )
    {
        if ( ( *i ) ->remove_socket )
        {
            // process says to finish off
            MinosCommonConnection *mcc = (*i);
            if ((*i)->publish_disconnect)
            {
                mcc->closeDown();
            }
            delete mcc;
            *i = nullptr;
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
      for ( CommonIterator i = i_array.begin(); i != i_array.end(); i++ )
      {
          delete ( *i );
      }
      i_array.clear();
}
//==============================================================================

MinosCommonConnection *MinosServerListener::makeConnection(QTcpSocket *s)
{
    trace("Creating MinosServerConnection makeConnection");
    MinosServerConnection *c = new MinosServerConnection(false);

    c->sock = QSharedPointer<QTcpSocket>(s);
    c->connectHost = c->sock->peerAddress();

    return c;
}

bool MinosServerListener::sendServer( TiXmlElement *tix )
{
    MinosId to( getAttribute( tix, "to" ) );

    if ( to.server.size() == 0 )
        return false;
    if ( to.server.compare( ThisMinosServer::getThisMinosServer() ->getServerName(), Qt::CaseInsensitive) == 0 )
        return false;
    if ( to.server.compare( DEFAULT_SERVER_NAME, Qt::CaseInsensitive ) == 0 )
        return false;

    // OK, it is not for us... look at connected servers

    for ( CommonIterator i = i_array.begin(); i != i_array.end(); i++ )
    {
        if ( ( *i ) ->checkServer( to ) )
        {
            if ( !( *i ) ->tryForwardStanza( tix ) )
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

void MinosServerListener::buildTable(QTableWidget *tab)
{
    tab->clear();
    tab->setRowCount(i_array.count());
    tab->setColumnCount(3);
    QStringList h = {"name", "address", "uuid"};
    tab->setHorizontalHeaderLabels(h);
    int row = 0;
    for ( CommonIterator i = i_array.begin(); i != i_array.end(); i++ )
    {
        MinosServerConnection *msc = dynamic_cast<MinosServerConnection *>(*i);
        QString server = msc->getClientServer();
        QTableWidgetItem *s = new QTableWidgetItem(server);
        tab->setItem(row, 0, s);
        s = new QTableWidgetItem(msc->server()->host.toString());
        tab->setItem(row, 1, s);
        s = new QTableWidgetItem(msc->server()->uuid);
        tab->setItem(row, 2, s);
        row++;
    }
}

void MinosServerListener::closeDown()
{
    MSL = nullptr;
}

MinosServerConnection *MinosServerListener::findConnection(const QHostAddress &h)
{
    for ( CommonIterator i = i_array.begin(); i != i_array.end(); i++ )
    {
        if (h.toIPv4Address() == (*i)->connectHost.toIPv4Address())
        {
            return dynamic_cast<MinosServerConnection *>(*i);
        }
    }
    return nullptr;
}
//==============================================================================
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

   if ( from.empty() || from.server.compare( ThisMinosServer::getThisMinosServer() ->getServerName(), Qt::CaseInsensitive ) == 0 )
      fromLocal = true;

   if ( from.empty() && from.server.compare( DEFAULT_SERVER_NAME, Qt::CaseInsensitive ) == 0 )
      fromLocalHost = true;

   if ( to.server.compare( ThisMinosServer::getThisMinosServer() ->getServerName(), Qt::CaseInsensitive ) == 0 )
      toLocal = true;

   if ( to.server.compare( DEFAULT_SERVER_NAME, Qt::CaseInsensitive ) == 0 )
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

   for ( CommonIterator i = i_array.begin(); i != i_array.end(); i++ )
   {
      // worry about the details
      if ((*i) && ( *i ) ->checkUser( to ) )
      {
         if ( !( *i ) ->tryForwardStanza( tix ) )
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
    for ( CommonIterator i = i_array.begin(); i != i_array.end(); i++ )
    {
        QString client = (*i)->getClientUser();
        QTableWidgetItem *s = new QTableWidgetItem(client);
        tab->setItem(row++, 0, s);
    }
}
void MinosClientListener::closeDown()
{

}
