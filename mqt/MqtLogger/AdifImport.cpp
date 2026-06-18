/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include <QTextStream>
#include "cutils.h"
#include "LoggerContest.h"
#include "contacts.h"
#include "BandList.h"
#include "MTrace.h"
#include "AdifImport.h"

//====================================================================
ADIFImport::ADIFImport(LoggerContestLog * c, QSharedPointer<QFile> adifContestFile ) :
      offset( -1 ),
      limit( -1 ),
      adifContestFile( adifContestFile ),
      acontest( c ),
      aqso( nullptr )
{
}
ADIFImport::~ADIFImport()
{
}
//---------------------------------------------------------------------------
Frequency convertAdifStrToFreq(QString frequency)
{
    bool ok = false;
    double f = 0.0;
    f = frequency.toDouble(&ok);
    if (!ok)
    {
        f = 0.0;
    }
    f *= 1000000;
    f += 0.5;       // to get the rounding right
    return Frequency(f);
}
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
         dateOn = FieldContent.right(6);
         if (dateOff.isEmpty())
         {
             dateOff = dateOn;
         }
      }
      if ( Fieldname.toUpper() == "TIME_ON" )
      {
          // 6 Digits representing a UTC time in HHMMSS format
          // or 4 Digits representing a time in HHMM format
          // either way we will take the 1st 4 chars
          timeOn = FieldContent;

      }
      if ( Fieldname.toUpper() == "QSO_DATE_OFF" )
      {
         // ADIF now specifies 8 Digits representing a UTC date in YYYYMMDD format
         dateOff = FieldContent.right(6);
      }
      if ( Fieldname.toUpper() == "TIME_OFF" )
      {
         // 6 Digits representing a UTC time in HHMMSS format
         // or 4 Digits representing a time in HHMM format
         // either way we will take the 1st 4 chars

          timeOff = FieldContent;
      }
      if ( Fieldname.toUpper() == "CALL" )
      {
         strcpysp( temp, FieldContent, FieldLength );
         aqso->cs.setFullCall( temp );
      }
      if ( Fieldname.toUpper() == "RST_SENT" )
      {
         strcpysp( temp, FieldContent, FieldLength );
         aqso->reps.setValue( temp );
      }
      if ( Fieldname.toUpper() == "RST_RCVD" )
      {
         strcpysp( temp, FieldContent, FieldLength );
         aqso->repr.setValue( temp );
      }
      if ( Fieldname.toUpper() == "SRX" || Fieldname.toUpper() == "NO_RCVD" )
      {
         int srx = FieldContent.toInt();

         aqso->serialr.setValue( QString::number(srx) );
      }
      if ( Fieldname.toUpper() == "STX" || Fieldname.toUpper() == "NO_SENT" )
      {
         int stx = FieldContent.toInt();
         aqso->serials.setValue( QString::number(stx) );
         if ( stx > acontest->maxSerial )
            acontest->maxSerial = stx;
      }
      if ( Fieldname.toUpper() == "GRIDSQUARE" )
      {
         strcpysp( temp, FieldContent, FieldLength );
         aqso->loc.setLoc( temp );
      }
      if ( Fieldname.toUpper() == "QSO_PTS" || Fieldname.toUpper() == "POINTS" )
      {
         strcpysp( temp, FieldContent, FieldLength );
         if ( temp.toInt() == 0 )
         {
            aqso->contactFlags.setValue( NON_SCORING );
         }
      }
      if ( Fieldname.toUpper() == "SRX_STRING" )
      {
         strcpysp( temp, FieldContent, FieldLength );
         aqso->extraText.setValue( temp );
      }
      if ( Fieldname.toUpper() == "OPT_EXCH" )
      {
         strcpysp( temp, FieldContent, FieldLength );
         aqso->extraText.setValue( temp );
      }
      if ( Fieldname.toUpper() == "IOTA" )
      {
         strcpysp( temp, FieldContent, FieldLength );
         aqso->extraText.setValue( temp );
      }
      if ( Fieldname.toUpper() == "FREQ" )
      {
          strcpysp( temp, FieldContent, FieldLength );

          Frequency freq = convertAdifStrToFreq(temp);
          QString band;
          freq = acontest->getTxFreqBand(freq, band);
          aqso->setFrequency(freq, band);
      }

      if ( Fieldname.toUpper() == "COMMENT" )
      {
          strcpysp( temp, FieldContent, FieldLength );
          aqso->comments.setValue(temp);
      }
      if ( Fieldname.toUpper() == "MODE" )
      {
          strcpysp( temp, FieldContent, FieldLength );

          temp = temp.toUpper();
          if (temp == hamlibData::CW)
          {
              aqso->mode.setValue(temp);
          }
          else if (temp == hamlibData::USB)
          {
              aqso->mode.setValue(temp);
          }
          else if (temp == hamlibData::LSB)
          {
              aqso->mode.setValue(temp);
          }
          else if (temp == "SSB")
          {
              aqso->mode.setValue(PH);
          }
          else if (temp == hamlibData::FM)
          {
              aqso->mode.setValue(temp);
          }
          else if (temp.startsWith("RTTY"))
          {
              aqso->mode.setValue(RY);
          }
          else if (temp.startsWith("PSK"))
          {
              aqso->mode.setValue(PSK);
          }
          else
          {
              aqso->mode.setValue(MGM);
              if (aqso->mgmSubmode.getValue().isEmpty())
              {
                aqso->mgmSubmode.setValue(temp.trimmed());
              }
          }
      }
      if ( Fieldname.toUpper() == "SUBMODE" )
      {
          strcpysp( temp, FieldContent, FieldLength );

          temp = temp.toUpper();
          aqso->mgmSubmode.setValue(temp.trimmed());
      }
      if ( Fieldname.toUpper() == "OPERATOR" )
      {
          strcpysp( temp, FieldContent, FieldLength );

          temp = temp.toUpper();
          aqso->op1.setValue(temp.trimmed());
      }
   }
}
//---------------------------------------------------------------------------

void ADIFImport::ADIFImportEndOfRecord( )
{
   if ( aqso )
   {
       if (dateOn.isEmpty())
       {
           dateOn = dateOff;
       }
       if (dateOff.isEmpty())
       {
           dateOff = dateOn;
       }
       if (timeOn.isEmpty())
       {
           timeOn = timeOff;
       }
       if (timeOff.isEmpty())
       {
           timeOff = timeOn;
       }
       aqso->timeOn.setDate( dateOn, DTGLOG );
       aqso->timeOn.setTime( timeOn, DTGLOG );

       aqso->timeOff.setDate( dateOff, DTGLOG );
       aqso->timeOff.setTime( timeOff, DTGLOG );

       dateOn.clear();
       dateOff.clear();
       timeOn.clear();
       timeOff.clear();

       // save contact

      // we have to have log_sequence set before we insert - or it will cause
      // duplicates

       int stx = aqso->serials.getValue().toInt();
       if ( stx > acontest->maxSerial )
          acontest->maxSerial = stx;

      acontest->addToContestList(aqso);

      QSharedPointer<BaseContact> bct;
      acontest->makeContact( false, bct );

      acontest->setNextBlock(acontest->getNextBlock() + 1);
      bct->setLogSequence( acontest->getNextBlock() << 16 );
      aqso = bct;

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
/*static*/bool ADIFImport::doImportADIFString(LoggerContestLog * c, const QString &adif)
{
    if ( !c )
       return false;

    // read only has already been checked, so we should only have live contests

    bool qsoOK = true;

    LoggerContestLog test(c->isHF());
    test.contestBands = c->contestBands;
    test.currentBand = c->currentBand;
    test.DTGStart = c->DTGStart;
    test.DTGEnd = c->DTGEnd;

    ADIFImport aimp( &test, QSharedPointer<QFile>() );

    aimp.fileContent = adif;
    aimp.limit = aimp.fileContent.size();
    aimp.offset = 0;

    if (aimp.executeImport())
    {
        QSharedPointer<BaseContact> bct = test.pcontactAt(0);

        if (bct && !bct->getFrequency().getValue().isClear())
        {
            bool ok = false;
            BandList &blist = BandList::getBandList();
            QSharedPointer<BandInfo>  bi;
            bool bandOK = false;
            QString current = test.currentBand.getValue();

            ok = blist.findBand(current, bi);

            if (ok)
            {
                if (test.isHF())
                {
                    if (bi->getType() == HF_BANDTYPE)
                    {
                        bandOK = true;
                    }
                }
                else
                {
                    Frequency freq = bct->getFrequency().getValue();
                    if (freq <= bi->fHigh && freq >= bi->fLow)
                    {
                        bandOK = true;
                    }
                }
            }

            if (!bandOK )
            {
                trace("ADIF frequency not in contest band");
                qsoOK = false;
            }
        }

        dtg d = bct->timeOff;
        QDate date = d.getDate();

        QDateTime DTGStart = CanonicalToTDT(c->DTGStart.getValue());
        QDate ds = DTGStart.date();
        QDateTime DTGEnd = CanonicalToTDT(c->DTGEnd.getValue());
        QDate de = DTGEnd.date();
        if (date < ds || date > de)
        {
            trace("ADIF day not in contest period of days");
            qsoOK = false;
        }

    }
    else
    {
        qsoOK = false;
    }

    if (qsoOK)
    {
        ADIFImport aimp( c, QSharedPointer<QFile>() );

        aimp.fileContent = adif;
        aimp.limit = aimp.fileContent.size();
        aimp.offset = 0;

        return aimp.executeImport();
    }
    return false;
}
//====================================================================
bool ADIFImport::getNextChar( char &ic )
{
   if ( offset < limit )
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
    fileContent = in.readAll();
    offset = 0;
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

   // Be a little generous - allow blank lines to NOT be the start of a header

   if ( InChar != '<' ) 	//if file does not start with < it must start with a header
   {
       while(InChar == ' ' || InChar == '\r' || InChar == '\n' || InChar == '\t')
       {
           Header += InChar;
           getNextChar(InChar);
       }

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
      acontest->makeContact( false, bct );

      acontest->setNextBlock(acontest->getNextBlock() + 1);
      bct->setLogSequence( acontest->getNextBlock() << 16 );

      aqso = bct;

   }

   bool atEOR = false;
   dateOn.clear();
   dateOff.clear();
   timeOn.clear();
   timeOff.clear();

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

      //accumulate field length string
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
