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
#ifndef serverThreadH
#define serverThreadH

#include <QDateTime>
#include <QTimer>
#include "MinosLink.h"

class Server;

class MinosServerConnection: public MinosCommonConnection
{
    Q_OBJECT
   private:
      Server *srv;
      bool resubscribed;

      QTimer resubscribeTimer;
   protected:
      virtual bool checkLastRx() override;
      qint64 lastKeepAlive = 0;
   public:
      MinosServerConnection();
      virtual void initialise() override;
      ~MinosServerConnection() override;
      virtual bool checkFrom( TiXmlElement *pak ) override;
      virtual bool isServer() override
      {
         return true;
      }
      const Server *server() const
      {
          return srv;
      }
      virtual void setFromId( MinosId &from, RPCRequest *req ) override;

      virtual void mConnect( Server *srv );
      virtual void sendAction( XStanza *a );
      void closeDown() override;
private slots:
      void on_connected();
      virtual void sendKeepAlive( ) override;
};
//==============================================================================
#endif
