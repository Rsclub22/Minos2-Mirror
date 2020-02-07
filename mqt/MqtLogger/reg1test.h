/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef Reg1testH
#define Reg1testH
#include "base_pch.h"

class LoggerContestLog;
class reg1testLine
{
   public:
      QString prefix;
      QString data;

      reg1testLine( const QString &, const QString &dat );
      reg1testLine( );
      ~reg1testLine();

      reg1testLine & operator = ( const reg1testLine & );
};

class reg1test
{
      LoggerContestLog * ct;
      QSharedPointer<QFile> regContestFile;
      int nextBlock;
      QStringList remarks;
      bool parseHeader( QString line );
      bool parseRemark( QString line );
      bool parseQSO( QString line );
      bool importReg1test();
   public:

      reg1test( LoggerContestLog * c, QSharedPointer<QFile> hFile );
      reg1test( LoggerContestLog * const ct );

      ~reg1test();
      bool exportTest( QSharedPointer<QFile> expfd, bool noSerials );
      static bool doImportReg1test( LoggerContestLog * c, QSharedPointer<QFile> hFile );

};
#endif
