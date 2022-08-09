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

#ifndef MServerH
#define MServerH 

#include "XMPPStanzas.h"
#include "tinyxml.h"

//---------------------------------------------------------------------------
#define DEFAULT_ROUTER_NAME "localhost"

class MinosCommonConnection;
class ThisMinosRouter;

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
