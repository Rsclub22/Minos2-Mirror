/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "logger_pch.h"

#include "BandList.h"
#include "LoggerContest.h"
#include "MinosTestImport.h"
#include "rigutils.h"

ContestContact::ContestContact( LoggerContestLog * ct, bool time_now ) : DisplayContestContact( ct, time_now )
{}
ContestContact::~ContestContact()
{}
void ContestContact::clearDirty()
{
   DisplayContestContact::clearDirty();
}
void ContestContact::setDirty()
{
   DisplayContestContact::setDirty();
}
static void catmult( QString &multbuff, const QString &text, int limit = 255 )
{
   multbuff += "*";
   multbuff += text.left(limit );
   multbuff += "* ";
}
void ContestContact::getPrintFileText( QString &sdest, short maxlen )
{
   BaseContestLog * clp = contest;
   int thisscore = 0;

   QString multbuff;
   QString dest;

   if ( contactFlags.getValue() & DONT_PRINT )
   {
      placestr( ContactBuffs::buff2, time.getDate( DTGPRINT ), 0, 10 );
      placestr( ContactBuffs::buff2, time.getTime( DTGPRINT ), 11, 5 );
      placestr( ContactBuffs::buff2, "DON'T PRINT", 21, 14 );
   }
   else if ( contactFlags.getValue() & LOCAL_COMMENT  )
   {
      placestr( ContactBuffs::buff2, time.getDate( DTGDISP ), 0, 10 );
      placestr( ContactBuffs::buff2, time.getTime( DTGDISP ), 11, 5 );
      placestr( ContactBuffs::buff2, "LOCAL COMMENT", 23, 14 );
      placestr( ContactBuffs::buff2, comments.getValue(), 31, 60 );
   }
   else if ( contactFlags.getValue() & COMMENT_ONLY )
   {
      placestr( ContactBuffs::buff2, time.getDate( DTGDISP ), 0, 10 );
      placestr( ContactBuffs::buff2, time.getTime( DTGDISP ), 11, 5 );
      placestr( ContactBuffs::buff2, "LOGGED COMMENT", 23, 14 );
      placestr( ContactBuffs::buff2, comments.getValue(), 31, 60 );

   }
   else
   {
      makestrings( clp->serialField.getValue() );

      QString exp_buff;

      if ( contactFlags.getValue() & NON_SCORING )
      {
         ContactBuffs::scorebuff = "0";
         exp_buff = "No Score Claimed " ;
      }
      else
         if ( cs.valRes == ERR_DUPCS )
         {
             ContactBuffs::scorebuff = "0";
             exp_buff = "Duplicate ";
         }
         else
         {
            if ( contactFlags.getValue() & VALID_DUPLICATE )
               exp_buff = "BackPacker ";

            if ( contactFlags.getValue() & VALID_DISTRICT )
               exp_buff += "No district code ";

            thisscore = contactScore.getValue();
            switch ( clp->scoreMode.getValue() )
            {
               case PPKM:
                  if ( contactFlags.getValue() & XBAND )
                  {
                     thisscore = ( thisscore + 1 ) / 2;
                  }
                  break;

               case PPQSO:
                  if ( thisscore > 0 )
                     thisscore = 1;
                  else
                     thisscore = 0;

                  break;

            }
            if ( contactScore.getValue() < 0 )
               thisscore = 0;
            ContactBuffs::scorebuff = QString::number(thisscore );
         }
      ContactBuffs::scorebuff.truncate(5);;
      multbuff.clear();

      if ( multCount )
      {
         if ( clp->countryMult.getValue() && newCtry )
            catmult( multbuff, ctryMult->basePrefix );

         if ( clp->districtMult.getValue() && newDistrict )
            catmult( multbuff, districtMult->districtCode );

         if ( clp->locMult.getValue() && locCount > 0 )
         {
            catmult( multbuff, loc.loc.getValue(), 4 );
         }
      }
      strcpysp( ContactBuffs::buff, comments.getValue(), 42 );
      if ( !ContactBuffs::buff.isEmpty() )
      {
         strcpysp( ContactBuffs::buff2, ContactBuffs::qthbuff, 20 );
         ContactBuffs::buff2 += " | ";
         ContactBuffs::buff2 += ContactBuffs::buff;
      }
      else
      {
         strcpysp( ContactBuffs::buff2, ContactBuffs::qthbuff, 42 );
      }
      ContactBuffs::buff = QString("%1 %2 %3").arg(ContactBuffs::buff2).arg(exp_buff).arg(multbuff );

      int next = 0;
      next = placestr( ContactBuffs::buff2, time.getDate( DTGPRINT ), next, 10 );
      next = placestr( ContactBuffs::buff2, time.getTime( DTGPRINT ), next + 1, 5 );
      next = placestr( ContactBuffs::buff2, cs.fullCall.getValue(), next + 1, 10 );
      next = placestr( ContactBuffs::buff2, reps.getValue(), next + 1, 3 );
      next = placestr( ContactBuffs::buff2, ContactBuffs::ssbuff, next, -4 );
      next = placestr( ContactBuffs::buff2, repr.getValue(), next + 1, 3 );
      next = placestr( ContactBuffs::buff2, ContactBuffs::srbuff, next, -4 );
      next = placestr( ContactBuffs::buff2, loc.loc.getValue(), next + 1, clp->allowLoc8.getValue() ? 8 : 6 );
      if ( districtMult )
      {
         placestr( ContactBuffs::buff2, districtMult->districtCode, next + 1, 3 );
      }
      next += 4;
      next = placestr( ContactBuffs::buff2, ContactBuffs::scorebuff, next + 1, -6 );
/*
  // looks like bearing is normally good
      if ( bearing < 0 )
      {
         double lon = 0.0;
         double lat = 0.0;
         double dist = 0.0;
         int brg;

         if ( lonlat( loc.loc.getValue(), lon, lat ) == LOC_OK )
         {
            clp->disbear( lon, lat, dist, brg );
            bearing = brg;
         }
      }
*/
      const QChar degreeChar(0260); // octal value
      QString sbrg = QString::number(bearing) + degreeChar;
      next = placestr( ContactBuffs::buff2, sbrg, next + 1, -6 );

      ContactBuffs::qthbuff += extraText.getValue().trimmed();
      ContactBuffs::qthbuff.truncate( EXTRALENGTH);
      next = placestr( ContactBuffs::buff2, ContactBuffs::qthbuff, next + 1, ContactBuffs::qthbuff.length() );

      next = placestr( ContactBuffs::buff2, ContactBuffs::buff, next + 1, ContactBuffs::buff.length() );

   }
   dest += ContactBuffs::buff2;

   sdest = dest.left( maxlen );

}
void ContestContact::addReg1TestComment( QStringList &remarks )
{
   QString cmnt;
   cmnt += time.getDate( DTGLOG );
   cmnt += ';';
   cmnt += time.getTime( DTGLOG );
   cmnt += ';';
   if ( contactFlags.getValue() & ( LOCAL_COMMENT | DONT_PRINT ) )
      return ;
   if ( contactFlags.getValue() & ( COMMENT_ONLY ) )
   {
      cmnt += comments.getValue();
      remarks.append( cmnt );
      return ;
   }

   if ( !comments.getValue().isEmpty() )
   {
      cmnt += cs.fullCall.getValue();
      cmnt += ';';
      cmnt += comments.getValue().trimmed();
      remarks.append( cmnt );
      return ;
   }
   return ;
}
void ContestContact::getReg1TestText(QString &sdest , bool noSerials)
{
   /*
      Date(6);Time(4);Call(3 - 14);Mode code(1 below);
      Sent-RST(0 or 2 or 3);Sent QSO number(0 or 3 or 4, with leading zeroes);
      Received-RST(0 or 2 or 3);Received QSO number(0 or 3 or 4, with leading zeroes);
      Received exchange(0 or 1-6);Received-WWL(0 or 4 or 6);
      QSO-Points(1-6 inc band mult);New-Exchange-(N)(0 or 1);
      New-WWL-(N)(0 or 1);New-DXCC-(N)(0 or 1);
      Duplicate-QSO-(D)(0 or 1)

      (N) says either "N;" or ";"

      code TXmode/RXmode
   0 none/none (or not specified)
   1 SSB/SSB
   2 CW/CW
   3 SSB/CW
   4 CW/SSB
   5 AM/AM
   6 FM/FM
   7 RTTY/RTTY
   8 SSTV/SSTV
   9 ATV/ATV
   If the mode is not important it can be left blank, i.e. not stated
   in rules/invitation.

   If the format is used for a LoggerContestLog which does not use some
   of the QSO exchanges, i.e. QSO no., WWL and Exchange,
   these fields are left blank. Proper interpretation must be
   ensured by manager program.

   Faulty QSOs

   A duplicate QSO is marked with a "D" in the Duplicate-QSO
   field, and the QSO-points field is set to 0 (zero). The format
   does not define when a QSO is a duplicate.
   An incomplete QSO is written with the information received,
   and the QSO-points field is set to zero (0).
   In case of a mistake, an error mark must be inserted in the
   Callsign field to keep a correct flow in the number of QSOs
   records. The error mark must be an "ERROR" and the other
   fields except Time and Sent QSO no., if used, can be left
   empty. In case the empty field is accumulated, e.g. QSOpoints,
   it is set to 0 (zero).

   Missing information

   If a LoggerContestLog log program can not fill in all the information, the
   missing information can be left blank, except if information is
   needed for claiming/calculating scores, e.g. log program
   cannot identify WWLs, DXCCs etc. If the information is
   required for the scores this log program can not be used for
   this particular LoggerContestLog anyway.

   */
   sdest = "";
   if ( contactFlags.getValue() & ( LOCAL_COMMENT | COMMENT_ONLY | DONT_PRINT ) )
      return ;

   BaseContestLog * clp = contest;

   sdest += time.getDate( DTGReg1Test );
   sdest += ';';
   sdest += time.getTime( DTGReg1Test );
   sdest += ';';

   //callsign
   sdest += cs.fullCall.getValue();
   sdest += ';';

   QString smode = mode.getValue().toUpper();
   if (  smode.compare( hamlibData::CW) == 0 )
      sdest += "2";
   else
      if ( smode.compare(hamlibData::USB ) == 0 )
         sdest += "1";
      else
         if ( smode.compare( hamlibData::FM ) == 0 )
            sdest += "6";
         else
            if ( smode.compare( hamlibData::MGM ) == 0 )
               sdest += "7";
             else
                sdest += "0";
   sdest += ';';

   sdest += reps.getValue();   // TX RST
   sdest += ';';

   if (noSerials)
   {
    // no data
   }
   else
   {
       QString ssbuff;
       int ss = serials.getValue().toInt();
       if ( ss )
          ssbuff = QString("%1").arg(ss, 3, 10, QChar('0')  );
       sdest += ssbuff;   // TX sno
   }
   sdest += ';';

   sdest += repr.getValue();   // RX RST
   sdest += ';';

   if (noSerials)
   {
    // no data
   }
   else
   {
       QString srbuff;
       int sr = serialr.getValue().toInt();
       if ( sr )
           srbuff = QString("%1").arg(sr, 3, 10, QChar('0') );
       sdest += srbuff;   // RX sno
   }
   sdest += ';';

   // exch
   if ( clp->districtMult.getValue() )
   {
      if ( districtMult )
         sdest += districtMult->districtCode;
   }
   else
      if ( clp->otherExchange.getValue() )
      {
         sdest += extraText.getValue();
      }
   sdest += ';';
   sdest += loc.loc.getValue();
   sdest += ';';
   //points
   int temp = contactScore.getValue();
   switch ( clp->scoreMode.getValue() )
   {
      case PPKM:
         break;

      case PPQSO:
         if ( temp > 0 )
            temp = 1;
         else
            temp = 0;
         break;

   }
   if ( contactScore.getValue() <= 0 )
      temp = 0;

   ContactBuffs::scorebuff = QString::number(temp );
   sdest += ContactBuffs::scorebuff;

   sdest += ';';
   if ( clp->districtMult.getValue() && newDistrict )
      sdest += 'N';
   sdest += ';';

   if (  clp->locMult.getValue() && locCount > 0 )
      sdest += 'N';
   sdest += ';';

   if ( clp->countryMult.getValue() && newCtry )
      sdest += 'N';
   sdest += ';';

   if ( cs.valRes == ERR_DUPCS )
      sdest += 'D';
}

// this is the "archive" copy that forms the real log

// we form our log by having a TSortedCollection derivative (class LoggerContestLog)
// this has to be sorted on the logSequence field


QString ContestContact::getADIFLine()
{
   //band
   //power
   //date
   //time
   //call
   //mode
   //RST sent
   //SN sent
   //RST Received
   //SN received
   //GridSquare
   //QTH, district, whatever
   //Comments
   //QSO Pts

   QString outstr;
   BaseContestLog * clp = contest;
   if ( !bool( clp ) )
      return outstr;

   QString exp_buff;

   if ( contactFlags.getValue() & ( LOCAL_COMMENT | DONT_PRINT ) )
      return outstr;

   exp_buff = time.getDate( DTGLOG ).left(6 );
   QString cent = "19";
   if ( exp_buff.toInt() < 300000 )
      cent = "20";

   outstr += makeADIFField( "QSO_DATE", cent + exp_buff );

   exp_buff = time.getTime( DTGLOG ).left( 4 );

   outstr += makeADIFField( "TIME_ON", exp_buff );

   QString cband = clp->band.getValue();

   QString cb = cband.trimmed();
   BandList &blist = BandList::getBandList();
   BandInfo bi;
   bool bandOK = blist.findBand(cb, bi);
   if (bandOK)
   {
      cb = bi.adif;
   }

   outstr += makeADIFField( "BAND", cb );

   QString freq = frequency.getValue();
   if (!freq.isEmpty())
   {
       QString newfreq = freq.trimmed().remove('.');
       double dfreq = convertStrToFreq(newfreq);
       dfreq = dfreq/1000000.0;  // MHz

       freq = QString::number(dfreq, 'f', 3); //MHz to 3 decimal places
       outstr += makeADIFField("FREQ", freq);
   }

   QString smode = mode.getValue().toUpper();
   if (  smode.compare( hamlibData::CW ) == 0 )
      outstr += makeADIFField( "MODE", "CW" );
   else
      if ( smode.compare( hamlibData::USB ) == 0 )
         outstr += makeADIFField( "MODE", "SSB" );
      else
         if ( smode.compare( hamlibData::FM ) == 0 )
            outstr += makeADIFField( "MODE", "FM" );
         else
            outstr += makeADIFField( "MODE", mode.getValue() );

   if ( contactFlags.getValue() & COMMENT_ONLY )
   {
      outstr += makeADIFField( "COMMENT", comments.getValue() );
      return outstr;
   }
   outstr += makeADIFField( "CALL", cs.fullCall.getValue() );

   outstr += makeADIFField( "RST_SENT", reps.getValue() );
   outstr += makeADIFField( "STX", serials.getValue() );
   outstr += makeADIFField( "RST_RCVD", repr.getValue() );
   outstr += makeADIFField( "SRX", serialr.getValue() );

   outstr += makeADIFField( "GRIDSQUARE", loc.loc.getValue() );
   if ( districtMult )
      outstr += makeADIFField( "QTH", districtMult->districtCode );
   else
      outstr += makeADIFField( "QTH", extraText.getValue() );

   outstr += makeADIFField( "TX_PWR", clp->power.getValue() );
   outstr += makeADIFField( "COMMENT", comments.getValue() );

   int temp = 0;
   if ( contactFlags.getValue() & NON_SCORING )
   {
      temp = 0;
   }
   else
      if ( cs.valRes == ERR_DUPCS )
      {
         temp = 0;
      }
      else
      {
         temp = contactScore.getValue();
         switch ( clp->scoreMode.getValue() )
         {
            case PPKM:
               if ( contactFlags.getValue() & XBAND )
               {
                  temp = ( temp + 1 ) / 2;
               }
               break;

            case PPQSO:
               if ( temp > 0 )
                  temp = 1;
               else
                  temp = 0;
               break;

         }
         if ( contactScore.getValue() < 0 )
            temp = 0;
      }
   outstr += makeADIFField( "OPERATOR", op1.getValue() );
   outstr += makeADIFField( "QSO_PTS", temp );
   outstr += makeADIFField( "QSO_COMPL", temp ? "YES" : "NO" );

   return outstr;
}
bool ContestContact::commonSave(QSharedPointer<BaseContact> tct)
{
   bool ret = false;
   LoggerContestLog * clp = dynamic_cast<LoggerContestLog *>( contest );
   if ( !clp->isReadOnly() )
   {
      if ( clp->isGJVFile() )
      {
         MinosParameters::getMinosParameters() ->mshowMessage( "GJV file should have been opened read only!" );
         return false;
      }
      else
         if ( clp->isMinosFile() )
         {
            ret = minosSave(tct);
         }

      if ( ret )
      {
         // search for the country, district, locator multipliers as required
         // Add to the worked counts as required

         cs.valRes = CS_NOT_VALIDATED;
         loc.valRes = LOC_NOT_VALIDATED;
         loc.validate();
         checkContact( );                    // in commonSave, AFTER saved, to update stats etc
      }
   }
   return ret;
}
bool ContestContact::minosSave(QSharedPointer<BaseContact> tct )
{
   LoggerContestLog * clp = dynamic_cast<LoggerContestLog *>( contest );
   clp->minosSaveContestContact( tct );
   QSharedPointer<BaseContact> bc( new BaseContact(*this ));
   bc->updtime = dtg( true ); // update time is now
   getHistory().push_back( bc );
   setModificationCount( getModificationCount() + 1 );
   return true;
}
bool ContestContact::GJVsave( GJVParams &gp )
{
   const QString nulc;
   QString temp;
   buffpt = 0;

   // write it all into sbuff

   int thisDiskBlock = gp.diskBlock++;
   setLogSequence( static_cast< unsigned long > ( thisDiskBlock ) << 16 );
   QSharedPointer<QFile> fd = gp.fd;

   temp = QString::number(thisDiskBlock );
   strtobuf( temp );

   temp = QString::number( getLogSequence() );
   strtobuf( temp );

   temp = QString::number( static_cast< unsigned int > (contactFlags.getValue()) );
   strtobuf( temp );

   strtobuf( op1.getValue() );
   strtobuf( op2.getValue() );
   strtobuf( time.getDate( DTGDISP ) );
   strtobuf( time.getTime( DTGDISP ) );

   if ( contactFlags.getValue() & ( LOCAL_COMMENT | COMMENT_ONLY ) )
   {
      strtobuf( nulc );   //cs
      strtobuf( nulc );   //reps
      strtobuf( nulc );   //serials
      strtobuf( nulc );   //repr
      strtobuf( nulc );   //serialr
      strtobuf( nulc );   //loc
      strtobuf( nulc );   //extra
   }
   else
   {
      strtobuf( cs.fullCall.getValue() );
      strtobuf( reps.getValue() );
      strtobuf( serials.getValue() );
      strtobuf( repr.getValue() );
      strtobuf( serialr.getValue() );
      strtobuf( loc.loc.getValue() );
      strtobuf( extraText.getValue() );
   }
   strtobuf( comments.getValue() );

   //	strtobuf(power);
   strtobuf( "0" );			// power - removed
   strtobuf( mode.getValue() );

   temp = QString::number(contactScore.getValue() );
   strtobuf( temp );

   strtobuf( forcedMult.getValue() );
   //   opyn(PowerWatts);

   strtobuf();    // clear tail
   LoggerContestLog * clp = dynamic_cast<LoggerContestLog *>( contest );
   clp->writeBlock( fd, thisDiskBlock );

   return true;

}
bool ContestContact::GJVload( int diskBlock )
{
   LoggerContestLog * clp = dynamic_cast<LoggerContestLog *>( contest );
   QString temp;
   clearBuffer();
   clp->readBlock( diskBlock );
   buffpt = 0;

   buftostr( temp );
   if ( temp.toInt() != diskBlock )
   {
      MinosParameters::getMinosParameters() ->mshowMessage( "Invalid block number in contact block" );
      return false;
   }

   buftostr( temp );
   setLogSequence( temp.toInt());

   buftostr( temp );
   contactFlags.setInitialValue( temp.toInt() );

   buftostr( temp );
   op1.setInitialValue( temp );
   buftostr( temp );
   op2.setInitialValue( temp );

   buftostr( temp );
   time.setDate( temp, DTGDISP );
   buftostr( temp );
   time.setTime( temp, DTGDISP );

   buftostr( temp );
   cs = callsign( strupr( temp ) );
   cs.valRes = CS_NOT_VALIDATED;

   buftostr( temp );
   reps.setInitialValue( temp );
   buftostr( temp );
   serials.setInitialValue( temp );
   buftostr( temp );
   repr.setInitialValue( temp );
   buftostr( temp );
   serialr.setInitialValue( temp );
   buftostr( temp );
   loc.loc.setInitialValue( temp );
   loc.valRes = LOC_NOT_VALIDATED;
   buftostr( temp );
   extraText.setInitialValue( temp );

   buftostr( temp );
   comments.setInitialValue( temp );

   buftostr( temp );              // power - removed

   buftostr( temp );
   mode.setInitialValue( temp );

   buftostr( temp );
   contactScore.setInitialValue( temp.toInt());
   bearing = -1;

   buftostr( temp );
   forcedMult.setInitialValue( temp );
   return true;
}
bool ContestContact::setField(QSharedPointer<BaseContact> tct, int ACol, const QString Value )
{
    //#warning never used! There to allow grid editing
    // This really ought to validate it first...
    switch ( ACol )
    {
    /*case egDate:
            break;
         */
    case egTime:
        time.setTime( Value, DTGDISP );  // VALIDATE!
        break;
    case egCall:
        cs.valRes = CS_NOT_VALIDATED;
        cs = callsign( Value );
        cs.validate( );
        break;
    case egRSTTx:
        reps.setValue( Value );
        break;
    case egSNTx:
        serials.setValue( Value );
        break;
    case egRSTRx:
        repr.setValue( Value );
        break;
    case egSNRx:
        serialr.setValue( Value );
        break;
    case egLoc:
        loc.loc.setValue( Value );
        loc.valRes = LOC_NOT_VALIDATED;
        loc.validate();
        break;
    case egBrg:
        return false;
    case egScore:
        return false;
    case egExchange:
        extraText.setValue( Value );
        break;
    case egComments:
        comments.setValue( Value );
        break;
    case egRigName:
        rigName.setValue(Value);
        break;
    case egFrequency:
        frequency.setValue(Value);
        break;
    case egRotatorHeading:
        rotatorHeading.setValue(Value);
        break;
    }
    // here we should save the contact
    commonSave(tct);
    return true;
}
void ContestContact::processMinosStanza( const QString &methodName, MinosTestImport * const mt )
{
   DisplayContestContact::processMinosStanza( methodName, mt );
}

