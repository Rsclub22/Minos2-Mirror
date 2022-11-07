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
#include "fileutils.h"

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
      QString path = GetCurrentDir();
      if ( where[ 0 ] == '.' )
      {
         path += where.right( where.size() - 1 );
      }
      else
      {
         path += where;
      }
      mLogFile.createLogFile( path, filePrefix, 10 );
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

