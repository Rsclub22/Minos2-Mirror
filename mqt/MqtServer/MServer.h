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

#include "minos_pch.h"

#ifndef MServerH
#define MServerH 
//---------------------------------------------------------------------------
#define DEFAULT_ROUTER_NAME "localhost"

class MinosCommonConnection;

class ThisMinosRouter
{
   private:
      static ThisMinosRouter *singleton;
      QString routerName;
   public:
      QString getRouterName()
      {
         return routerName;
      }

      static ThisMinosRouter *getThisMinosRouter();
      bool forwardStanza( MinosCommonConnection *, TiXmlElement *pak );
      virtual bool analyseNode( MinosCommonConnection *il, TiXmlElement * pak );
      virtual void dispatchStanza( MinosCommonConnection *il, RPCRequest *a );

      ThisMinosRouter();
      virtual ~ThisMinosRouter();
};
#endif
