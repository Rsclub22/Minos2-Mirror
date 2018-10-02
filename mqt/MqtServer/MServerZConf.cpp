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
      : waitNameReply( false )
{
   ZConf = this;
   sendBeaconResponse = false;
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
 * We seem to need one socket per interface. Our old QT code looped through the interfaces
 * and broadcast (not multicast) on all the IPV4 ones (NB not localhost...)
 *
 * It then bound on "any IPV4 address" so one socket managed all the receives
 *
 * BCB code creates a socket per interface, and adds an extra for localhost, binds each for
 * receive, and then loops through them for sending
 *
 */
void TZConf::startZConf(const QString &name)
{
    trace("TZConf::runThread()");

    localName = name.trimmed();

    groupAddress = QHostAddress(UPNP_GROUP);

    //QSharedPointer<MCReadSocket> mcrs(new MCReadSocket());
    //mcrs->setupRO();
    //rxSocket = mcrs;

    //connect(rxSocket.data(), SIGNAL(readyRead(QString, QString )), this, SLOT(onReadyRead(QString, QString)), Qt::QueuedConnection);

    // Get network interfaces list

    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();

    // Interfaces iteration
    for (int i = 0; i < ifaces.size(); i++)
    {
        if (!ifaces[i].flags().testFlag( QNetworkInterface::IsUp))
            continue;
        if (!ifaces[i].flags().testFlag(QNetworkInterface::IsRunning))
            continue;
        // Now get all IP addresses for the current interface
        QList<QNetworkAddressEntry> addrs = ifaces[i].addressEntries();

        // And for any IP address, if it is IPv4 and the interface is active, make a socket
        for (int j = 0; j < addrs.size(); j++)
        {
            if ((addrs[j].ip().protocol() == QAbstractSocket::IPv4Protocol)
                    && !addrs[j].ip().toString().isEmpty())
            {
                {
                    QSharedPointer<UDPSocket> qus(new UDPSocket());

                    bool res = qus->setup(ifaces[i], addrs[j]);

                    if (res)
                    {
                        TxSocks.push_back(qus);
                        connect(qus.data(), SIGNAL(readyRead(QString,QString)), this, SLOT(onReadyRead(QString,QString)));
                    }
                }
            }
        }
    }


   /*
      <minosServer UUID='xxx' name='name' port='port' request='true' />

      UUID is unique to this "machine" (more likely username)
      , and is to allow us to manage
      station name changes. So where should we keep it?
      Looks like registry may be favourite; we don't really
      want two serevrs running on a single machine.

      Actually we can use the GUID as the event name for servers...

      "request" true asks recipient to beacon immediately
      beacon messages do NOT force recipients to beacon

      ALL servers will periodically set "request", but if a
      request has been received "recently" then we don't... (?)

      This mechanism should allow for networks that fragment, and
      then the server comes online - once the network heals then
      we rely on the periodic beacon

      We also need to time out stations which we haven't heard from.
   */

   // and now we go and sit on a select on all the notify sockets
   // but also remember to beacon occasionally...

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

   if (sendBeaconResponse || lastTick.msecsTo(QDateTime::currentDateTime()) > beaconInterval )
   {
      trace(QString("Timeout: Sending beacon, sendBeaconResponse = ") + (sendBeaconResponse?"true":"false"));
      lastTick = QDateTime::currentDateTime();

      sendMessage( sendBeaconResponse );   // timer requests beaconing, beacon just responds
      sendBeaconResponse = false;
   }
   //rxSocket->onTimeout();
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

void TZConf::onReadyRead(QString datagram, QString sender)
{
    // from MCReadSocket

    processZConfString(datagram, sender, sendBeaconResponse);
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
void TZConf::ServerScan()
{
   for ( QVector<Server *>::iterator i = serverList.begin(); !closeApp && i != serverList.end(); i++ )
   {
      trace("Server scan - checking " + (*i)->station);
      MinosServerListener::getListener() ->checkServerConnected( ( *i ), false );
      trace("Server scan - checked " + (*i)->station);
   }
}
void TZConf::readServerList()
{
   trace("Reading Server List File");
   // Read the server override file
   QSettings servers(GetCurrentDir() + "/Configuration/Servers.ini", QSettings::IniFormat);
   QStringList sl = servers.childGroups();

   for ( int i = 0; i < sl.count(); i++ )
   {
      servers.beginGroup(sl[i]);

      QString uuid = servers.value( "Uuid" ).toString();
      QString host = servers.value( "Host" ).toString();
      QString station = servers.value( "Station" ).toString();
      QString port = servers.value( "Port" ).toString();


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

      zcPublishServer( uuid, station, host, toQUint16(port, MinosServerPort ) );

   }
   ServerScan();
   trace("Finished reading Server List File");
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
                              const QString &hosttarget, quint16 PortAsNumber )
{
    trace( "zcPublishServer Host " + hosttarget + " Station " + name +
           " Port " + QString::number( PortAsNumber ) + " uuid " + uuid  );
    Server *s = findStation( name );
    if ( s )
    {
        trace("Station " + name + " found");
    }
    else
    {
        trace("Station " + name + " not found");
        s = new Server( uuid, hosttarget, name, PortAsNumber );
        if ( name == getZConf()->getName() )
        {
            s->local = true;
        }
        else
        {
            MinosServerConnection *msc = new MinosServerConnection();
            msc->mConnect(s);
            MinosServerListener *msl = MinosServerListener::getListener();
            msl->addListenerSlot(msc);
        }
        serverList.push_back( s );
    }
    if ( s->local )
    {
        PubSubMain->publish( "", rpcConstants::LocalStationCategory, name, hosttarget, psPublished );
    }
    PubSubMain->publish( "", rpcConstants::StationCategory, name, hosttarget, psPublished );
    trace("zcPublishServer finished");
    return s;
}
void TZConf::publishDisconnect(const QString &name)
{
    trace("publishDisconnect");
   Server *s = findStation( name );
   if ( s )
   {
      PubSubMain->publish( "", rpcConstants::StationCategory, name, s->host, psNotConnected );
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
               + (beaconreq?"":" request='true'")
               + " />";
}
//==============================================================================
Server *TZConf::processZConfString(const QString &message, const QString &recvHost, bool &sendBeaconResponse)
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

        // publish what came in - possibly we should publish the XML string
        // against the UUID?
        iPort = toQUint16(port, MinosServerPort);
        srv = zcPublishServer( UUID, station, recvHost, iPort );
        if ( request.size() && UUID != getServerId())
        {
            sendBeaconResponse = true;   // delay the response, give the other end a chance...
            // BUT this means we could try to connect before they will recognise us.
            // Should we queue the publish until then as well?

            // But the UDP may fail to get there, so we need to be able to cope with new
            // unadvertised connectors
        }
    }
    return srv;
}
//==============================================================================

Server::Server( const QString &uuid, const QString &h, const QString &s, quint16 p )
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
    qui = iface;
    qus = QSharedPointer<QUdpSocket>(new QUdpSocket);

    //connect(qus.data(), SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));

    connect(qus.data(), SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onSocketStateChange(QAbstractSocket::SocketState)));
    connect(qus.data(), SIGNAL(readyRead( )), this, SLOT(onReadyRead()), Qt::QueuedConnection);

    qui = iface;
    trace("Binding to " + addr.ip().toString());
    bool res = qus->bind(
                addr.ip(),//addr.ip(), QHostAddress::AnyIPv4,//TZConf::getZConf()->groupAddress,
                UPNP_PORT,
                QUdpSocket::DefaultForPlatform//QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint
                );

    if (res)
    {
        qus->setSocketOption(QAbstractSocket::MulticastTtlOption, QVariant(4));
    }
    trace(QString("UDP bind returns ") + (res?"true":"false") + " on " + addr.ip().toString() + (res?"":(" error " + qus->errorString())));
    return res;
}
void UDPSocket::onSocketStateChange (QAbstractSocket::SocketState state)
{
    if ( state == QAbstractSocket::BoundState )
    {
        bool res = qus->joinMulticastGroup(TZConf::getZConf()->groupAddress, qui);
        if (res)
        {
            trace("Bound and set multicast interface " + TZConf::getZConf()->groupAddress.toString()
                  + " on " + qus->localAddress().toString() + " interface :" + ifaceName);
        }
        else
        {
            trace("Failed to set multicast interface " + TZConf::getZConf()->groupAddress.toString()
                  + " on " + qus->localAddress().toString() + " interface :" + ifaceName);
        }
    }
}
void UDPSocket::onReadyRead()
{
    while (qus->hasPendingDatagrams())
    {
        char buf[1024];
        QHostAddress host;
        quint16 port;
        qint64 res = qus->readDatagram(buf, 1023, &host, &port);
        buf[res] = 0;
        QString dg = QString(buf);

        trace("Datagram received from " + host.toString() + " " + dg);
        emit readyRead(dg, host.toString());
    }
}
bool UDPSocket::sendMessage(const QString &mess )
{
    QByteArray packet = QByteArray(mess.toStdString().c_str());

    /*int ret =*/ qus->writeDatagram(packet.data(), packet.length(), TZConf::getZConf()->groupAddress, UPNP_PORT);

    trace("send datagram on " + ifaceName
          + " : " + mess);

    return true;
}
