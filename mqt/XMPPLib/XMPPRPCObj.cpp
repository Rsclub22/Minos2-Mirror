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
MinosRPCRouter::~MinosRPCRouter()
{}

//---------------------------------------------------------------------------

QMap <QString, QSharedPointer<MinosRPCObj>> &getRouterMethodMap()
{
   static QMap <QString, QSharedPointer<MinosRPCObj> > routerMethodMap;
   return routerMethodMap;
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
   for ( auto &i: getRouterMethodMap() )
   {
      delete i->callback;
      i.reset();
   }
   getRouterMethodMap().clear();
}

/*static*/ void MinosRPCObj::addRouterObj(  QSharedPointer<MinosRPCObj> mro )
{
   getRouterMethodMap().insert( mro->methodName, mro );
}

/*static*/ QSharedPointer<MinosRPCObj> MinosRPCObj::makeRouterObj(  QString call )
{
   QMap <QString, QSharedPointer<MinosRPCObj> >::iterator mo = getRouterMethodMap().find( call );
   QSharedPointer<MinosRPCObj> res;
   if ( mo != getRouterMethodMap().end() )
   {
      res = mo.value()->makeObj();
      res->methodName = call;
      return res;
   }
   for (auto &mo: getRouterMethodMap())
   {
       if (mo->isGeneralObject())
       {
           res = mo->makeObj();
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
    QString router = psn.router();
    QString app = psn.appName();
    if (!app.isEmpty() && !router.isEmpty())
    {
        queueCall(app + "@" + router);
    }
}
//==============================================================================

