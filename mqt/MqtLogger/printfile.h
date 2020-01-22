/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////

#ifndef PrintFileH
#define PrintFileH
#include "base_pch.h"

//---------------------------------------------------------------------------
class LoggerContestLog;

class PrintFileLine
{
   public:
      QString prefix;
      QString data;

      PrintFileLine( const QString &, const QString &dat );
      PrintFileLine();
      ~PrintFileLine();

      PrintFileLine & operator = ( const PrintFileLine & );
};

class PrintFile
{
      LoggerContestLog * ct;
   public:


      PrintFile( LoggerContestLog * const ct );

      ~PrintFile();
      bool exportTest( QSharedPointer<QFile> expfd );
};
#endif
