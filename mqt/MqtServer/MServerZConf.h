/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------

#ifndef MServerZConfH
#define MServerZConfH 

#include <QApplication>
#include <QObject>
#include <QTimer>
#include <QTcpSocket>
#include <QTcpServer>
#include <QUdpSocket>

#include <QSharedPointer>
#include <QHostAddress>
#include <QHostInfo>

#include <QSettings>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QDateTime>

#define UPNP_PORT 9999
#define UPNP_GROUP "239.255.0.1"

//---------------------------------------------------------------------------
class Server
{
   public:
      QString uuid;
      QString host;
      QString station;
      quint16 port;
      bool local;

      Server( const QString &uuid, const QString &h, const QString &s, quint16 p );
      Server( const QString &s );
      virtual ~Server();
};
extern QVector<Server *> serverList;
extern Server *findStation( const QString s );

class UDPSocket: public QObject
{
    Q_OBJECT
    QSharedPointer<QUdpSocket> qus;
    QString ifaceName;
    QNetworkAddressEntry qua;

public:
    UDPSocket();
    virtual ~UDPSocket() override;
    bool setup(QNetworkInterface &intr, QNetworkAddressEntry &addr);

    bool sendMessage(const QString &mess );
};
class TZConf: public QObject
{
    Q_OBJECT
   private:  	// User declarations

      static Server *zcPublishServer(const QString &uuid, const QString &name,
                        const QString &hosttarget, quint16 PortAsNumber );
      QString localName;

      QVector<QSharedPointer<UDPSocket> > TxSocks;
      QUdpSocket readSocket;

      QTimer beaconTimer;

      bool sendMessage(bool beaconReq );
      void readServerList();

      static TZConf *ZConf;

      unsigned int beaconInterval = 0;   // once a minute
      QDateTime lastTick;
      bool sendBeaconResponse;

public:  		// User declarations

      TZConf( );
      virtual ~TZConf( );
      static  TZConf *getZConf()
       {
           return ZConf;
       }

      void startZConf(const QString &name);

      QString getName()
      {
         return localName;
      }

      void ServerScan();

      QString getZConfString(bool beaconreq);
      Server *processZConfString(const QString &message, const  QString &recvAddress, bool &beaconResponse);
      void publishDisconnect(const QString &name);
      void closeDown();
private slots:
      void onReadyRead();
      void onTimeout();
};
#endif
