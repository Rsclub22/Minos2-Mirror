/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "logger_pch.h"
#include <ctype.h>
#include "rigutils.h"
#include "LoggerContest.h"
#include "AdifImport.h"

//====================================================================
ADIFImport::ADIFImport(LoggerContestLog * c, QSharedPointer<QFile> adifContestFile ) :
    offset( -1 )
    , limit( -1 )
    , adifContestFile( adifContestFile )
    , acontest( c )
{
    next_block = acontest->getNextBlock();
}
ADIFImport::~ADIFImport()
{
}
//---------------------------------------------------------------------------

void ADIFImport::ADIFImportFieldDecode(QString Fieldname, int FieldLength, QString /*FieldType*/,
   QString FieldContent )
{
   if ( aqso )
   {
      // If in QSO accumulate aqso
      //      Trace( "In QSO" );

      QString temp;
      if ( Fieldname.toUpper() == "QSO_DATE" )
      {
         // ADIF now specifies 8 Digits representing a UTC date in YYYYMMDD format
         if ( FieldLength == 6 )
            aqso->time.setDate( FieldContent, DTGLOG );
         else
            if ( FieldLength == 8 )
               aqso->time.setDate( FieldContent.right(6), DTGLOG );
      }
      if ( Fieldname.toUpper() == "TIME_ON" )
      {
         // 6 Digits representing a UTC time in HHMMSS format
         // or 4 Digits representing a time in HHMM format
         // either way we will take the 1st 4 chars

         aqso->time.setTime( FieldContent, DTGLOG );
      }
      if ( Fieldname.toUpper() == "CALL" )
      {
         strcpysp( temp, FieldContent, FieldLength );
         aqso->cs = Callsign( temp.toUpper() );
         aqso->cs.valRes = CS_NOT_VALIDATED;
      }
      if ( Fieldname.toUpper() == "RST_SENT" )
      {
         strcpysp( temp, FieldContent, FieldLength );
         aqso->reps.setInitialValue( temp );
      }
      if ( Fieldname.toUpper() == "RST_RCVD" )
      {
         strcpysp( temp, FieldContent, FieldLength );
         aqso->repr.setInitialValue( temp );
      }
      if ( Fieldname.toUpper() == "SRX" || Fieldname.toUpper() == "NO_RCVD" )
      {
         int srx = FieldContent.toInt();

         aqso->serialr.setInitialValue( QString::number(srx) );
      }
      if ( Fieldname.toUpper() == "STX" || Fieldname.toUpper() == "NO_SENT" )
      {
         int stx = FieldContent.toInt();
         aqso->serials.setInitialValue( QString::number(stx) );
         if ( stx > acontest->maxSerial )
            acontest->maxSerial = stx;
      }
      if ( Fieldname.toUpper() == "GRIDSQUARE" )
      {
         strcpysp( temp, FieldContent, FieldLength );
         aqso->loc.loc.setInitialValue( temp );
         aqso->loc.valRes = LOC_NOT_VALIDATED;
      }
      if ( Fieldname.toUpper() == "QSO_PTS" || Fieldname.toUpper() == "POINTS" )
      {
         strcpysp( temp, FieldContent, FieldLength );
         if ( temp.toInt() == 0 )
         {
            aqso->contactFlags.setInitialValue( NON_SCORING );
         }
      }
      if ( Fieldname.toUpper() == "QTH" )
      {
         strcpysp( temp, FieldContent, FieldLength );
         aqso->extraText.setInitialValue( temp );
      }
      if ( Fieldname.toUpper() == "OPT_EXCH" )
      {
         strcpysp( temp, FieldContent, FieldLength );
         aqso->extraText.setInitialValue( temp );
      }
      if ( Fieldname.toUpper() == "FREQ" )
      {
          strcpysp( temp, FieldContent, FieldLength );

          double freq = convertStrToFreq(temp);
          QString sfreq = QString::number(freq, 'f', 6); //MHz to 6 decimal places
          aqso->frequency.setInitialValue(sfreq);
   }

      if ( Fieldname.toUpper() == "MODE" )
      {
          strcpysp( temp, FieldContent, FieldLength );

          temp = temp.toUpper();
          if (temp == "CW")
          {
              aqso->mode.setInitialValue(hamlibData::CW);
          }
          else if (temp == "USB")
          {
              aqso->mode.setInitialValue(hamlibData::USB);
          }
          else if (temp == "FM")
          {
              aqso->mode.setInitialValue(hamlibData::FM);
          }
          else
          {
              aqso->mode.setInitialValue(hamlibData::MGM);
              aqso->comments.setInitialValue(temp);
          }
      }
   }
}
//---------------------------------------------------------------------------

void ADIFImport::ADIFImportEndOfRecord( )
{
   if ( aqso )
   {
      // save contact

      // we have to have log_sequence set before we insert - or it will cause
      // duplicates

      aqso->setLogSequence( static_cast< unsigned long> ( next_block++ ) << 16 );

      MapWrapper<BaseContact> waqso(aqso);
      acontest->ctList.insert( waqso, waqso );

      QSharedPointer<BaseContact> bct;
      acontest->makeContact( false, bct );
      aqso = bct;
      aqso->setLogSequence( 0 );     // will be derived from the contest

   }
}
//---------------------------------------------------------------------------
/*static*/bool ADIFImport::doImportADIFLog(LoggerContestLog * c,  QSharedPointer<QFile> hFile )
{
   if ( !c )
      return false;

   ADIFImport aimp( c, hFile );

   if ( aimp.importAdifFile() )
   {
      return aimp.executeImport();
   }
   return false;
}
//====================================================================
bool ADIFImport::getNextChar( char &ic )
{
   if ( offset <= limit )
   {
      ic = fileContent[ offset++ ].toLatin1();
      return true;
   }

   return false;
}
//====================================================================
bool ADIFImport::importAdifFile()
{
   //Create and Open File Stream
    QTextStream in( adifContestFile.data() );
    fileContent =in.readAll();
    offset = 1;
    limit = fileContent.size();
    return true;
}
//====================================================================
bool ADIFImport::executeImport()
{

   char InChar;
   QString Header;

   if ( !getNextChar( InChar ) )
      return false;

   bool inHeader = true;

   if ( InChar != '<' ) 	//if file does not start with < it must start with a header
   {
      //skip to > symbol accumulating header
      Header += InChar;
      while ( inHeader )
      {
         do
         {
            if ( !getNextChar( InChar ) )
               return false;
            Header += InChar;
         }
         while ( InChar != '<' );

         //skip to >, Ignore field name
         //if it is not <EOH> then cannot do much about it!
         QString qEOH;
         do
         {
            if ( !getNextChar( InChar ) )
               return false;
            Header = Header + InChar;
            if ( InChar != '>' )
               qEOH = qEOH + InChar;
         }
         while ( InChar != '>' );

         if ( qEOH.toUpper() == "EOH" )
         {
            inHeader = false;
         }
      }
      // end of header - start the qso's
      QSharedPointer<BaseContact> bct;
      acontest->makeContact( false, aqso );
      aqso->setLogSequence( 0 );     // will be derived from the contest
   }

   bool atEOR = false;
   do
   {

      // Clear the accumulator strings
      QString FieldName;
      QString FieldLengthString;
      int FieldLength = 0;
      QString FieldType;
      QString FieldContent;


      while ( InChar != '<' )
      {
         if ( !getNextChar( InChar ) )
            return atEOR;              // OK if at EOR and no more real data
         if ( InChar != '<' )
            Header = Header + InChar;
      }


      //accumulate field name
      do
      {
         if ( !getNextChar( InChar ) )
            return false;
         if ( InChar != ':' && InChar != '>' && InChar != ' ' )
            FieldName = FieldName + static_cast< char>(toupper( InChar ));
      }
      while ( InChar != ':' && InChar != '>' );

      //acmulate field length string
      if ( InChar != '>' )
      {
         do
         {
            if ( !getNextChar( InChar ) )
               return false;
            if ( InChar != ':' && InChar != '>' && InChar != ' ' )
               FieldLengthString = FieldLengthString + InChar;
         }
         while ( InChar != ':' && InChar != '>' );

         // convert string to integer
         FieldLength = FieldLengthString.toInt();
      }

      //accumulate field type (if present)or skip to >

      if ( InChar != '>' )
      {
         do
         {
            if ( !getNextChar( InChar ) )
               return false;
            if ( InChar != '>' && InChar != ' ' )
               FieldType = FieldType + InChar;
         }
         while ( InChar != '>' );

         // convert string to integer
         FieldLength = toInt(FieldLengthString);
      }


      //accumulate field content
      for ( int i = 0; i < FieldLength; i++ )
      {
         if ( !getNextChar( InChar ) )
            return false;
         FieldContent = FieldContent + InChar;
      }

      //do action for the field

      if ( FieldName == "EOR" )
      {
         //do actions for End of Record: call OnEndOfRecord event
         ADIFImportEndOfRecord();
         atEOR = true;
      }
      else
      {
         //do actions for other fields: call OnFieldDecode event
         ADIFImportFieldDecode( FieldName, FieldLength, FieldType, FieldContent );
      }

   }
   while ( true );    //?????
   //    return false;
}
