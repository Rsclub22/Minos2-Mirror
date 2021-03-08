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

#ifndef clientThreadH
#define clientThreadH 

#include "MinosLink.h"
//==============================================================================

class MinosClientConnection: public MinosCommonConnection
{
    //Q_OBJECT
   private:
   protected:
   public:
      MinosClientConnection();
      virtual void initialise() override;
      ~MinosClientConnection() override;
      virtual bool checkFrom( TiXmlElement *pak ) override;
      virtual void setFromId( MinosId &from, RPCRequest *req ) override;
      virtual bool isRouter() override
      {
         return false;
      }
      void closeDown() override;
};
//==============================================================================

#endif
