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
#include <QNetworkInterface>
#include <QDateTime>

#define UPNP_PORT 9999

//---------------------------------------------------------------------------
class Router
{
   public:
      QString uuid;
      QHostAddress host;
      QString station;
      quint16 port;
      bool local;

      Router( const QString &uuid, const QHostAddress &h, const QString &s, quint16 p );
      Router( const QString &s );
      virtual ~Router();
};
extern QVector<Router *> routerList;
extern QVector<Router *>::iterator findStation( const QString s );
extern QVector<Router *>::iterator findIp( const QString s );

class UDPSocket: public QObject
{
    Q_OBJECT

public:
    UDPSocket();
    virtual ~UDPSocket() override;
    bool setup(QNetworkInterface &intr, QNetworkAddressEntry &addr);

    bool sendMessage(const QString &mess );
    QSharedPointer<QUdpSocket> qus;
    QString ifaceName;
    QNetworkAddressEntry qua;
};
class TZConf: public QObject
{
    Q_OBJECT
   private:  	// User declarations

      static Router *zcPublishRouter(const QString &uuid, const QString &name,
                        const QHostAddress &host, quint16 PortAsNumber );
      QString localName;

      QVector<QSharedPointer<UDPSocket> > TxSocks;
      QUdpSocket readSocket;

      QTimer beaconTimer;

      bool sendMessage();
      void readRouterListFile();

      static TZConf *ZConf;

      unsigned int beaconInterval = 0;   // once a minute
      unsigned int beaconIncrement = 0;
      unsigned int maxBeaconInterval = 0;
      unsigned int beaconResponseDelay = 0;   // once a minute
      QDateTime lastTick;
      QDateTime sendBeaconResponse;

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

      QString getZConfString(bool beaconreq, const QString &h);
      Router *processZConfString(const QString &message, QHostAddress &host, QDateTime &beaconResponse);
      void publishDisconnect(Router *);
      void closeDown();
private slots:
      void onReadyRead();
      void onTimeout();
};
#endif
