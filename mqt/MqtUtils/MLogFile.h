/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		VHF Contest Adjudication
//
// COPYRIGHT         (c) M. J. Goodey G0GJV, 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////

#ifndef MLogFileH
#define MLogFileH 
#include <QString>
#include <QMutexLocker>
#include <QTextStream>
//---------------------------------------------------------------------------
#include "fileutils.h"

class CsGuard
{
      static QRecursiveMutex m_mutex;
   public:
      CsGuard()
      {
          m_mutex.lock();
      }

      ~CsGuard()
      {
          m_mutex.unlock();
      }
      static void ClearDown()
      {
          //m_mutex.unlock();
      }
};


//---------------------------------------------------------------------------
class MLogFile
{
   private:
      QString fLogFileName;
   public:

      MLogFile()
      { }
      // CreateLogFile is called by the boot form at startup
      void createLogFile(const QString &path, const QString filePrefix, int keepDays );

      // Log and LogT are used to log data without and with a timestamp;
      QTextStream &log( );
      QTextStream & logT( );
      // Log logs a string with a time prefix hh:mm:ss
      QTextStream &log( const QString &s );
      void close( );

      // Generates a suffix of the form yyyyddmmhhmmss
      static QString generateLogFileName( const QString &prefix );
      static void tidyFiles( const QString &prefix, int keepDays );
      QString getTraceFileName()
      {
         return fLogFileName;
      }
};
//---------------------------------------------------------------------------
#endif
