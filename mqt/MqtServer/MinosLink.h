/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef MinosLinkH
#define MinosLinkH 
#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QHostAddress>
#include <QSharedPointer>

#include "tinyxml.h"
//---------------------------------------------------------------------------
class XStanza;
class RPCRequest;

class MinosId
{
   public:
      QString user;
      QString server;
      QString fullId;

      bool empty();

      MinosId();
      MinosId( const QString & );
      virtual ~MinosId();

      void setId( const QString & );
};

//==============================================================================
#define RXBUFFLEN 4096

extern bool isHostLocal(const QString &host);
class MinosCommonConnection: public QObject
{
    Q_OBJECT
   private:
      char rxbuff[ RXBUFFLEN ];
      QString packetbuff;
      QString rxBuff;

    protected:
       bool connected;
       qint64 lastRx = 0;

      // who is connected?
      QString clientServer;     // server name
      QString clientUser;       // client name (or empty)
      QString makeJid();        // return the Jabber ID

      void onLog ( const char *data, size_t size, int is_incoming );
      bool sendRaw (const TIXML_STRING xmlstr );
      virtual bool checkLastRx()
      {
          return true;
      }
   public:

      QSharedPointer<QTcpSocket> sock;

      bool remove_socket;
      bool fromIdSet;
      QHostAddress connectHost;

      MinosCommonConnection();
      virtual void initialise( ) = 0;
      virtual ~MinosCommonConnection();

      virtual bool analyseNode( TiXmlElement *pak );
      virtual bool tryForwardStanza( TiXmlElement *pak );

      virtual bool isServer() = 0;
      virtual bool checkFrom( TiXmlElement *pak ) = 0;
      virtual void setFromId( MinosId &from, RPCRequest *req ) = 0;

      virtual bool checkServer( const MinosId &s )
      {
         return ( clientServer.compare(s.server, Qt::CaseInsensitive) == 0 );
      }
      virtual bool checkServer( const QString &s )
      {
          return ( clientServer.compare(s, Qt::CaseInsensitive) == 0 );
      }
      virtual bool checkUser( const MinosId &u )
      {
         return ( clientUser.compare(u.user, Qt::CaseInsensitive) == 0 );
      }
      virtual void closeDown() = 0;
      void closeSocket()
      {
        if (sock)
            sock->close();
      }
      virtual void sendKeepAlive( )
      {}
      QString getClientServer() const;

      QString getClientUser() const;

private slots:
      void on_readyRead();
      void on_disconnected();
};
//==============================================================================
#endif
