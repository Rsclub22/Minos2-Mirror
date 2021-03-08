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

class Router;

class MinosRouterConnection: public MinosCommonConnection
{
    Q_OBJECT
   private:
      Router *srv = nullptr;
      bool resubscribed = false;
      bool fromDatagram = false;

      QTimer resubscribeTimer;
   protected:
      virtual bool checkLastRx() override;
      qint64 lastKeepAlive = 0;
   public:
      MinosRouterConnection(bool fromDatagram);
      virtual void initialise() override;
      ~MinosRouterConnection() override;
      virtual bool checkFrom( TiXmlElement *pak ) override;
      virtual bool isRouter() override
      {
         return true;
      }
      const Router *router() const
      {
          return srv;
      }
      virtual bool isFromDatagram() const override
      {
          return fromDatagram;
      }
      void setRouter(Router *s);
      virtual void setFromId( MinosId &from, RPCRequest *req ) override;

      virtual void mConnect( Router *srv );
      virtual void sendAction( XStanza *a );
      void closeDown() override;
private slots:
      void on_connected();
      virtual void sendKeepAlive( ) override;
      virtual void sendCloseSocket( ) override;
};
//==============================================================================
#endif
