/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------

#include "MLogFile.h"

//---------------------------------------------------------------------------
static MLogFile mLogFile;
static bool logEnabled = false;

void trace( const QString & mess )
{
   mLogFile.log( mess );
}
//---------------------------------------------------------------------------
void enableTrace(const QString &where , const QString filePrefix)
{
   if ( !logEnabled )
   {
      mLogFile.createLogFile( where, filePrefix, 10 );
      logEnabled = true;
   }
}
void disableTrace( )
{
   if ( logEnabled )
   {
      mLogFile.close();
      logEnabled = false;
   }
}
//---------------------------------------------------------------------------

QString getTraceFileName()
{
   return mLogFile.getTraceFileName();
}
//---------------------------------------------------------------------------

