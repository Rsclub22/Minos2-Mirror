/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////

#ifndef ListContactH
#define ListContactH 
#include "base_pch.h"
#include "contacts.h"
//---------------------------------------------------------------------------
class ListContact
{
   public:
      BaseContact& operator =( const BaseContact & );

      virtual ~ListContact(){}
      Callsign cs;   //CONTAIN MinosItem
      Locator loc;   //CONTAIN MinosItem
      QString extraText;
      QString comments;
      virtual QString getField( int ACol, const BaseContestLog * const curcon ) const;
};
#endif
