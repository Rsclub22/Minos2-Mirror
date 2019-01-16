/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		VHF Contest Adjudication
//
// COPYRIGHT         M. J. Goodey, 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
#ifndef AdifImportH
 #define AdifImportH

#include "base_pch.h"
class LoggerContestLog;
class BaseContact;

class ADIFImport
{
      int offset;
      int limit;
      bool getNextChar( char &ic );

      ADIFImport( LoggerContestLog * c, QSharedPointer<QFile> hFile );
      ~ADIFImport();

      bool importAdifFile();
      bool executeImport();

      void ADIFImportFieldDecode(QString Fieldname, int FieldLength, QString FieldType, QString FieldContent );
      void ADIFImportEndOfRecord( );

      QSharedPointer<QFile> adifContestFile;
      QString fileContent;

      LoggerContestLog * acontest;
      QSharedPointer<BaseContact> aqso;
   public:
      static bool doImportADIFLog( LoggerContestLog * c, QSharedPointer<QFile> hFile );
      static bool doImportADIFString(LoggerContestLog * c, const QString &adif);

};

#endif
