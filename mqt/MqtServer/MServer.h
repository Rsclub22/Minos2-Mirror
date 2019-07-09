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
#define DEFAULT_SERVER_NAME "localhost"

class MinosCommonConnection;

class ThisMinosServer
{
   private:
      static ThisMinosServer *singleton;
      QString serverName;
   public:
      QString getServerName()
      {
         return serverName;
      }

      static ThisMinosServer *getThisMinosServer();
      bool forwardStanza( MinosCommonConnection *, TiXmlElement *pak );
      virtual bool analyseNode( MinosCommonConnection *il, TiXmlElement * pak );
      virtual void dispatchStanza( MinosCommonConnection *il, RPCRequest *a );

      ThisMinosServer();
      virtual ~ThisMinosServer();
};
#endif
