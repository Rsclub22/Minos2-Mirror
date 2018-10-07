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
#include <QUuid>

#include "tinyxml.h"
#include "TinyUtils.h"

#include "MinosLink.h"
#include "clientThread.h"
#include "serverThread.h"

#include "minoslistener.h"

#include "MServerZConf.h"
#include "MServerPubSub.h"


TZConf *TZConf::ZConf = nullptr;
QVector<Server *> serverList;
//---------------------------------------------------------------------------
static quint16 toQUint16 ( const QString &s, int def )
{
    int num = def;
    if ( !s.isEmpty() )
    {
        bool ok;
        int i = s.toInt(&ok);
        if (ok)
            num = i;
    }
    return static_cast<quint16>(num);
}
static QString makeUuid()
{
    QUuid uuid = QUuid::createUuid();
    return uuid.toString();
}
static QString getServerId()
{
   const QString key = "ServerUUID";

   QSettings settings ;

   QString uuid = settings.value(key, "").toString();
   if (uuid.size() == 0)
   {
       uuid = makeUuid();
       settings.setValue(key, uuid);
   }
   return uuid;

}
//---------------------------------------------------------------------------
TZConf::TZConf( )
{
   ZConf = this;
   sendBeaconResponse = false;  // so we send a true beacon with request
   beaconInterval = 60000;   // once a minute
   lastTick = QDateTime::currentDateTimeUtc().addSecs(-59); // force a beacon soon
}
TZConf::~TZConf( )
{
   for ( QVector<Server *>::iterator i = serverList.begin(); i != serverList.end(); i++ )
   {
      delete ( *i );
   }
   serverList.clear();
}
void TZConf::closeDown()
{
    trace("closeDown");

}

/*
 * We need one socket per interface. Loop through the interfaces
 * and broadcast on all the IPV4 ones (NB not localhost...)
 *
 * One socket manages all the receives
 *
 */
void TZConf::startZConf(const QString &name)
{
    trace("TZConf::runThread()");

    localName = name.trimmed();

    // set up the receiver
    readSocket.bind(UPNP_PORT);
    connect(&readSocket, SIGNAL(readyRead( )), this, SLOT(onReadyRead()), Qt::QueuedConnection);

    // Get network interfaces list

    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();

    // Interfaces iteration
    for (int i = 0; i < ifaces.size(); i++)
    {
        if (!ifaces[i].flags().testFlag( QNetworkInterface::IsUp))
            continue;
        if (!ifaces[i].flags().testFlag(QNetworkInterface::IsRunning))
            continue;

        // Now get all IP addresses for this interface
        QList<QNetworkAddressEntry> addrs = ifaces[i].addressEntries();

        // And for any IP address, if it is IPv4 and the interface is active, make a socket
        for (int j = 0; j < addrs.size(); j++)
        {
            if ((addrs[j].ip().protocol() == QAbstractSocket::IPv4Protocol)
                    && !addrs[j].ip().toString().isEmpty())
            {
                QSharedPointer<UDPSocket> qus(new UDPSocket());

                bool res = qus->setup(ifaces[i], addrs[j]);

                if (res)
                {
                    TxSocks.push_back(qus);
                }
            }
        }
    }


   /*
      <minosServer UUID='xxx' name='name' port='port' request='true' />

      UUID is unique to this "machine" (more likely username)
      , and is to allow us to manage
      station name changes.

      "request" true asks recipient to beacon immediately
      beacon messages do NOT force recipients to beacon

      ALL servers will periodically set "request", but if a
      request has been received "recently" then we don't... (?)

      This mechanism should allow for networks that fragment, and
      then the server comes online - once the network heals then
      we rely on the periodic beacon

      We also need to time out stations which we haven't heard from.
   */

   connect (&beaconTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));

   beaconTimer.setInterval(100);
   beaconTimer.start();
}
void TZConf::onTimeout()
{
    if (lastTick.msecsTo(QDateTime::currentDateTime()) > beaconInterval)
    {
        readServerList();
    }
    if (readSocket.hasPendingDatagrams())
    {
        trace("TZConf::onTimeout() - datagrams pending");
        onReadyRead();      // catcher for missed datagrams - don't know why they go missing
    }

   if (sendBeaconResponse || lastTick.msecsTo(QDateTime::currentDateTime()) > beaconInterval )
   {
      trace(QString("Timeout: Sending beacon, sendBeaconResponse = ") + (sendBeaconResponse?"true":"false"));
      lastTick = QDateTime::currentDateTime();

      sendMessage( sendBeaconResponse );   // timer requests beaconing, beacon just responds
      sendBeaconResponse = false;
   }
}


bool TZConf::sendMessage(bool beaconReq )
{
    for (QVector<QSharedPointer<UDPSocket> >::iterator i = TxSocks.begin(); i != TxSocks.end(); i++)
    {
        QString mess = getZConfString(beaconReq);
        (*i)->sendMessage(mess);
    }
   return true;
}


void TZConf::onReadyRead()
{
    trace("TZConf::onReadyRead()");
    QString datagram;
    QString sender;
    while (readSocket.hasPendingDatagrams())
    {
        QByteArray buf;
        buf.resize(static_cast<int>(readSocket.pendingDatagramSize()));
        QHostAddress host;
        quint16 port;
        qint64 res = readSocket.readDatagram(buf.data(), buf.size(), &host, &port);
        QString dg = QString(buf);

        trace("Datagram received from " + host.toString() + " " + dg);
        if (res > 0)
        {
            processZConfString(dg, host, sendBeaconResponse);
        }
    }
}

//---------------------------------------------------------------------------


Server *findStation( const QString s )
{
   for ( QVector<Server *>::iterator i = serverList.begin(); i != serverList.end(); i++ )
   {
      if ( ( *i ) ->station.compare( s, Qt::CaseInsensitive ) == 0 )
      {
         return ( *i );
      }
   }
   return nullptr;
}
Server *findIp( const QHostAddress &h )
{
   for ( QVector<Server *>::iterator i = serverList.begin(); i != serverList.end(); i++ )
   {
      if (h.isEqual((*i)->host))
      {
         return ( *i );
      }
   }
   return nullptr;
}

void TZConf::readServerList()
{
   trace("Reading Server List File");

   // Read the server override file

   QSettings servers(GetCurrentDir() + "/Configuration/Servers.ini", QSettings::IniFormat);
   QStringList sl = servers.childGroups();

//   trace(QString::number(sl.size()) + " child groups");
   for ( int i = 0; i < sl.count(); i++ )
   {
      servers.beginGroup(sl[i]);

      QString uuid = servers.value( "Uuid" ).toString();
      QString host = servers.value( "Host" ).toString();
      QString station = servers.value( "Station" ).toString();
      QString port = servers.value( "Port" ).toString();

    servers.endGroup();

//    trace(QString::number(i) + " " + sl[i] + " " + uuid + " " + host + " " + station + " " + port);

      if ( host.size() == 0 )
      {
         if ( station.size() == 0 )
         {
            continue;
         }
         host = station;
      }
      else
         if ( station.size() == 0 )
         {
            station = host;
         }

      if ( port.size() == 0 )
      {
         port = QString::number(MinosServerPort);
      }

      QHostAddress ha;
      ha.setAddress(host);
      zcPublishServer( uuid, station, ha, toQUint16(port, MinosServerPort ) );

   }
   //trace("Finished reading Server List File");
}

//---------------------------------------------------------------------------
// uuid is the machines uuid (more unique than the server name!)
// name is server name
// hosttarget is the IP address/DNS name
// PortAsNumber is the remote port
// state is available/not available

// ONLY trouble is... clients will now address their servers by UUID!
// when they have subscribed to stations.

Server *TZConf::zcPublishServer( const QString &uuid, const QString &name,
                              const QHostAddress &host, quint16 PortAsNumber )
{
    trace( "zcPublishServer Host " + host.toString() + " Station " + name +
           " Port " + QString::number( PortAsNumber ) + " uuid " + uuid  );
    MinosServerListener *msl = MinosServerListener::getListener();
    Server *s = findStation( name );
    if ( s )
    {
        trace("Station " + name + " found by name");
    }
    if (!s)
    {
        s = findIp(host);
        if (s)
        {
            trace("Station " + host.toString() + " found by ip");
            MinosServerConnection *m = msl->findConnection(host);
            m->setServer(s);
        }
    }
    if (!s)
    {
        trace("Station " + name + " not found");
        s = new Server( uuid, host, name, PortAsNumber );
        if ( name == getZConf()->getName() )
        {
            s->local = true;
        }
        else
        {
            // we must have a server connection already
            trace("Creating MinosServerConnection zcPublishServer for " + name);
            MinosServerConnection *msc = new MinosServerConnection();
            msc->mConnect(s);
            msl->addListenerSlot(msc);
        }
        serverList.push_back( s );
    }
    if ( s->local )
    {
        PubSubMain->publish( "", rpcConstants::LocalStationCategory, name, host.toString(), psPublished );
    }
    PubSubMain->publish( "", rpcConstants::StationCategory, name, host.toString(), psPublished );
    trace("zcPublishServer finished");
    return s;
}
void TZConf::publishDisconnect(const QString &name)
{
   trace("publishDisconnect");
   Server *s = findStation( name );
   if ( s )
   {
      PubSubMain->publish( "", rpcConstants::StationCategory, name, s->host.toString(), psNotConnected );
   }
}
//==============================================================================
QString TZConf::getZConfString(bool beaconreq)
{
   static int sequence = 0;
   QString Uuid = getServerId();
   return  QString("<minosServer ")
               + "seq='" + QString::number(sequence++)
               + "' UUID='" + Uuid
               + "' name='" + getName()
               + "' port='" + QString::number(MinosServerPort) + "'"
               + (beaconreq?"":" request='true'")       // if a beacon response, don't request another beacon
               + " />";
}
//==============================================================================
Server *TZConf::processZConfString(const QString &message, const QHostAddress &host, bool &sendBeaconResponse)
{
    sendBeaconResponse = false;
    Server *srv = nullptr;
    TiXmlDocument xdoc;
    TIXML_STRING smessage = message.toStdString();// allowed conversion through TIXML_STRING
    xdoc.Parse( smessage.c_str(), nullptr );
    TiXmlElement * tix = xdoc.RootElement();
    if ( tix && checkElementName( tix, "minosServer" ) )
    {
        //"<minosServer UUID='" + Uuid + "' name='" + getName() + "' port='7778' request='true' />";
        QString UUID = getAttribute( tix, "UUID" );
        QString station = getAttribute( tix, "name" );
        QString port = getAttribute( tix, "port" );
        QString request = getAttribute( tix, "request" );

        // publish what came in

        quint16 iPort = toQUint16(port, MinosServerPort);
        srv = zcPublishServer( UUID, station, host, iPort );
        if ( request.size() && UUID != getServerId())
        {
            sendBeaconResponse = true;   // delay the response, give the other end a chance...
        }
    }
    return srv;
}
//==============================================================================

Server::Server( const QString &uuid, const QHostAddress &h, const QString &s, quint16 p )
    :
      uuid(uuid),
      host( h ),
      station( s ),
      port( p ),
      local( false )
{}
Server::Server( const QString &s )
    :
      station( s ),
      port( static_cast<quint16>(-1) ),
      local( false )
{}
Server::~Server()
{}
//==============================================================================

UDPSocket::UDPSocket()
{
}
UDPSocket::~UDPSocket()
{

}

bool UDPSocket::setup(QNetworkInterface &iface, QNetworkAddressEntry &addr)
{
    ifaceName = iface.humanReadableName();
    qus = QSharedPointer<QUdpSocket>(new QUdpSocket);
    qua = addr;

    return true;
}

bool UDPSocket::sendMessage(const QString &mess )
{
    QByteArray packet = QByteArray(mess.toStdString().c_str());

    qint64 res = qus->writeDatagram(packet.data(), packet.length(), qua.broadcast(), UPNP_PORT);

    QString err = "No error";
    if (res < 0)
        err = qus->error();
    trace("send datagram on " + ifaceName + " result " + err
          + " : " + mess);

    return true;
}
