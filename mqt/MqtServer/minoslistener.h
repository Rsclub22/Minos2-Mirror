#ifndef MINOSLISTENER_H
#define MINOSLISTENER_H

/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------

#include <QTcpServer>
#include <QTimer>
#include <QTableWidget>

#include "MinosLink.h"
class Router;
class MinosRouterConnection;

extern bool closeApp;

class MinosListener:public QObject
{
    Q_OBJECT
   protected:

     QTimer CheckTimer;
      QSharedPointer<QTcpServer> sock;
      QVector<MinosCommonConnection *>i_array;

      virtual MinosCommonConnection *makeConnection(QTcpSocket *s) = 0;

      virtual bool isRouter() const = 0;
    public:

      void clearSockets();
      void addListenerSlot( MinosCommonConnection * );
      int getConnectionCount();
      MinosListener();
      virtual ~MinosListener();
      virtual bool initialise(QString type, quint16 port );

      virtual MinosCommonConnection *getConnection(int);
      virtual void closeDown() = 0;

private slots:
      void on_newConnection();
      void on_timeout();
};
typedef QVector<MinosCommonConnection *>::iterator CommonIterator;
//==============================================================================
class MinosRouterListener: public MinosListener
{
    //Q_OBJECT
   private:
      static MinosRouterListener *MSL;
   protected:
      virtual MinosCommonConnection *makeConnection(QTcpSocket *s) override;
      virtual bool isRouter() const override
      {
          return true;
      }
   public:
      static MinosRouterListener *getListener()
      {
         return MSL;
      }
      MinosRouterListener()
      {
         MSL = this;
      }
      ~MinosRouterListener() override
      {
         MSL = nullptr;
      }
      bool sendRouter(TiXmlElement *pak );

      void buildTable(QTableWidget *tab);

      void closeDown() override;

      MinosRouterConnection *findConnection(const QHostAddress &h);
};
//==============================================================================
class MinosClientListener: public MinosListener
{
    Q_OBJECT
   private:
      static MinosClientListener *MCL;
   protected:
      virtual MinosCommonConnection *makeConnection(QTcpSocket *s) override;
      virtual bool isRouter() const override
      {
          return false;
      }

   public:
      static MinosClientListener *getListener()
      {
         return MCL;
      }
      MinosClientListener();
      ~MinosClientListener() override;
      bool sendClient(TiXmlElement *pak );
      void buildTable(QTableWidget *tab);
      virtual void closeDown() override;
};
//==============================================================================
#endif // MINOSLISTENER_H
