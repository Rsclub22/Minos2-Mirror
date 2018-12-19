/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#include "XMPP_pch.h"

sendActionCall sendAction = nullptr;
void setSendAction(sendActionCall sa)
{
    sendAction = sa;
}
//---------------------------------------------------------------------------
TRPCFunctor::~TRPCFunctor()
{}
MinosRPCServer::~MinosRPCServer()
{}

//---------------------------------------------------------------------------

QMap <QString, QSharedPointer<MinosRPCObj>> &getServerMethodMap()
{
   static QMap <QString, QSharedPointer<MinosRPCObj> > serverMethodMap;
   return serverMethodMap;
}
//==============================================================================
MinosRPCObj::MinosRPCObj(const QString &methodName, TRPCFunctor *cb , bool gen)
      : general(gen)
      , methodName( methodName )
      , callback( cb )
{}
MinosRPCObj::~MinosRPCObj()
{
}
void MinosRPCObj::clearCallArgs()
{
   callArgs.args.clear();  // clears shared pointers
}
/*static*/ void MinosRPCObj::clearRPCObjects()
{
   for ( QMap <QString, QSharedPointer<MinosRPCObj> >::iterator i = getServerMethodMap().begin(); i != getServerMethodMap().end(); i++ )
   {
      delete i.value()->callback;
      i.value().reset();
   }
   getServerMethodMap().clear();
}

/*static*/ void MinosRPCObj::addServerObj(  QSharedPointer<MinosRPCObj> mro )
{
   getServerMethodMap().insert( mro->methodName, mro );
}

/*static*/ QSharedPointer<MinosRPCObj> MinosRPCObj::makeServerObj(  QString call )
{
   QMap <QString, QSharedPointer<MinosRPCObj> >::iterator mo = getServerMethodMap().find( call );
   QSharedPointer<MinosRPCObj> res;
   if ( mo != getServerMethodMap().end() )
   {
      res = mo.value()->makeObj();
      res->methodName = call;
      return res;
   }
   for (mo = getServerMethodMap().begin(); mo != getServerMethodMap().end(); mo++)
   {
       if (mo.value()->isGeneralObject())
       {
           res = mo.value()->makeObj();
           res->methodName = call;
           return res;
       }
   }
   return res;
}
//==============================================================================
void MinosRPCClient::queueCall( QString to )
{
   RPCRequest * m = new RPCRequest( to, methodName );
   m->setNextId();      // only happens if no Id already
   m->args = callArgs.args;     // copy vector of pointers
   trace("TX " + m->analyse());
   ::sendAction( m );
   delete m;
}
void MinosRPCClient::queueCall(const PubSubName &psn)
{
    QString server = psn.server();
    QString app = psn.appName();
    if (!app.isEmpty() && !server.isEmpty())
    {
        queueCall(app + "@" + server);
    }
}
//==============================================================================

