/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"
#include "Calendar.h"
#include "CalendarList.h"

BaseContestLog::BaseContestLog( )
{
   uuid = makeUuid();
   bearingOffset.setValue(0);
   currentMode.setValue( "USB" );

   protectedContest.setValue( false );
  allowLoc8.setValue( false );
  allowLoc4.setValue ( false );
  RSTField.setValue( true );
  serialField.setValue( true );
  locatorField.setValue( true );
  otherExchange.setValue( false );
  countryMult.setValue( false );
  nonGCountryMult.setValue( false );
  districtMult.setValue( false );
  locMult.setValue( false );
  GLocMult.setValue(false);
  M7Mults.setValue(false);
  usesBonus.setValue(false);
  scoreMode.setValue( PPKM );
  powerWatts.setValue( true );
  MGMContestRules.setValue(false);

   int nc = MultLists::getMultLists() ->getCtryListSize();
   countryWorked = new int[ nc ];
   for ( int i = 0; i < nc; i++ )
      countryWorked[ i ] = 0;

   nc = MultLists::getMultLists() ->getDistListSize();
   districtWorked = new int[ nc ];
   for (int i = 0; i < nc; i++ )
      districtWorked[ i ] = 0;

}
BaseContestLog::~BaseContestLog()
{
   delete [] districtWorked;
   delete [] countryWorked;
   districtWorked = nullptr;
   countryWorked = nullptr;

   closeFile();
}
int BaseContestLog::indexOf(QSharedPointer<BaseContact> item )
{
    int i = 0;
    for (LogIterator m = ctList.begin(); m != ctList.end(); m++)
    {
      if (m->wt.data() == item.data())
          return i;

      i++;
    }
    return -1;
}
int BaseContestLog::getContactCount( )
{
   return ctList.size();
}

QSharedPointer<BaseContact> BaseContestLog::pcontactAt( int i )
{
   if ( i < ctList.size() )
   {
       QSharedPointer<BaseContact> ce = std::next(ctList.begin(), i)->wt;
       return ce;
   }
   return QSharedPointer<BaseContact>();
}

QSharedPointer<BaseContact> BaseContestLog::pcontactAtSeq( unsigned long logSequence )
{
   for ( LogIterator i = ctList.begin(); i != ctList.end(); i++ )
   {
      if ( i->wt ->getLogSequence() == logSequence )
         return i->wt;
   }
   return QSharedPointer<BaseContact>();
}
void BaseContestLog::clearDirty()
{
   protectedContest.clearDirty();
   mycall.fullCall.clearDirty();

   name.clearDirty();
   location.clearDirty();
   myloc.loc.clearDirty();
   allowLoc4.clearDirty();
   allowLoc8.clearDirty();
   RSTField.clearDirty();
   serialField.clearDirty();
   locatorField.clearDirty();
   power.clearDirty();
   currentMode.clearDirty();
   band.clearDirty();
   otherExchange.clearDirty();
   countryMult.clearDirty();
   nonGCountryMult.clearDirty();
   locMult.clearDirty();
   GLocMult.clearDirty();
   districtMult.clearDirty();

   M7Mults.clearDirty();
   usesBonus.clearDirty();
   bonusType.clearDirty();
   MGMContestRules.clearDirty();

   powerWatts.clearDirty();
   scoreMode.clearDirty();
   DTGStart.clearDirty();
   DTGEnd.clearDirty();
   for ( LogIterator i = ctList.begin(); i != ctList.end(); i++ )
   {
      i->wt->clearDirty();
   }
}
void BaseContestLog::setDirty()
{
   protectedContest.setDirty();
   mycall.fullCall.setDirty();
   name.setDirty();
   location.setDirty();
   myloc.loc.setDirty();
   allowLoc4.setDirty();
   allowLoc8.setDirty();
   RSTField.setDirty();
   serialField.setDirty();
   locatorField.setDirty();
   power.setDirty();
   currentMode.setDirty();
   band.setDirty();
   otherExchange.setDirty();
   countryMult.setDirty();
   nonGCountryMult.setDirty();
   locMult.setDirty();
   GLocMult.setDirty();
   districtMult.setDirty();

   M7Mults.setDirty();
   usesBonus.setDirty();
   bonusType.setDirty();
   MGMContestRules.setDirty();

   powerWatts.setDirty();
   scoreMode.setDirty();
   DTGStart.setDirty();
   DTGEnd.setDirty();
   for ( LogIterator i = ctList.begin(); i != ctList.end(); i++ )
   {
      i->wt->setDirty();
   }
}
void BaseContestLog::makeContact( bool timeNow, QSharedPointer<BaseContact>&lct )
{
   lct = QSharedPointer<BaseContact>(new BaseContact( this, timeNow ));
}
void BaseContestLog::validateLoc( )
{
    if (MGMContestRules.getValue())
    {
        Locator nloc;
        nloc.loc.setValue(myloc.loc.getValue().left(4) + "MM");
        if ( nloc.validate( ode, odn ) == LOC_OK )
        {
            cosodn = cos( odn );
            sinodn = sin( odn );
            locValid = true;
            myloc.valRes = LOC_OK;
        }
        else
            locValid = false;
    }
    else if ( myloc.validate( ode, odn ) == LOC_OK )
    {
        cosodn = cos( odn );
        sinodn = sin( odn );
        locValid = true;
    }
    else
        locValid = false;

}
/*********************************************************************/
/* calculate distance and bearing of contact from our station        */
/*                                                                   */
/* uses global double variables odn, ode, sinodn, and cosodn, and pi */
/*                                                                   */
/* pi must be set to value of pi before calling                      */
/* our station longtitude and latitude must be set up in ode and odn */
/* before invoking this routine.   Also, cos(odn) must be set up in  */
/* cosodn and sin(odn) in sindodn.                                   */
/*                                                                   */
/* Latitude is NS, Longtitude is EW                                  */
/*                                                                   */
/*********************************************************************/
void BaseContestLog::disbear( double lon, double lat, double &dist, int &brg ) const
//double lon ;                    /* other stations longtitude */
//double lat ;                    /* other stations latitude */
//int *dist ;                     /* resulting distance */
//int *brg ;                      /* resulting bearing */

{
   if ( myloc.valRes != LOC_OK )
   {
      dist = 1;
      brg = 0;
      return ;
   }
   if ( almost_equal(lon, ode, 2) && almost_equal(lat, odn, 2) )        /* same square ! */ /* testing doubles for equality! */
   {
      // same square - commenced Kilometre, so always 1
      dist = 1;
      brg = 0 ;                       /* or bearing */
      return ;                         /* just exit */
   }

   double coslat = cos( lat );                    /* pre-calculate values */
   double sinlat = sin( lat );
   double coscos = cosodn * coslat ;

   // first distance into dx

   double co = cos( ode - lon ) * coscos + sinodn * sinlat ;
   double ca = atan( fabs( sqrt( 1.0 - co * co ) / co ) );
   if ( co < 0.0 )
      ca = pi - ca ;
   double dx = 6371.291 * ca ;       /* 6371.291 is approved radius of earth */

   // and then the bearing

   double si = sin( lon - ode ) * coscos ;
   co = sinlat - sinodn * cos( ca );
   double az = atan( fabs( si / co ) );
   if ( co < 0.0 )
      az = pi - az ;
   if ( si <  0.0 )
      az = -az ;
   if ( az < 0.0 )
      az = az + 2.0 * pi ;

   az = az / dr ;                      /* convert to degrees */
   az += 0.5 ;                /* correct angle */
   dx = ceil( dx );                      // adjust for commenced kilometer
   dx += 0.5 ;              // make sure double truncates properly back to int
   dist = dx ;			                  /* return result */
   if ( static_cast<int>(az) == 0 )                  /* due north */
      az = 360.00 ;                 	/* so show valid */
   brg = static_cast< int > (az) ;                   /* and give it back as integer */
}
//---------------------------------------------------------------------------
bool BaseContestLog::getsdist( const QString &loc, QString &minloc, double &mindist )
{
   int brg;
   double dist = 0.0;
   double lon = 0.0;
   double lat = 0.0;

   int lres = lonlat( loc, lon, lat );
   if ( lres == LOC_OK || lres == LOC_SHORT )
   {
      disbear( lon, lat, dist, brg );
      if ( dist < mindist )
      {
         mindist = dist;
         minloc = loc;
      }
      return true;
   }
   return false;
}
//---------------------------------------------------------------------------
int BaseContestLog::CalcNearest( const QString &qscalcloc )
{
   if ( qscalcloc.length() != 4 )
      return 0;	// only valid 4 fig locs

   // calculate the nearest point of loc2 from loc1

   double mindist = 1000000.0;

   QString minloc;
   QString temploc;

   for ( char i = 'A'; i <= 'X'; i++ )
   {
       temploc = qscalcloc;
      temploc += 'A';
      temploc += i;

      if ( !getsdist( temploc, minloc, mindist ) )
      {
         return -1;
      }

      temploc[ 4 ] = 'X';

      if ( !getsdist( temploc, minloc, mindist ) )
      {
         return -1;
      }

      temploc[ 4 ] = i;
      temploc[ 5 ] = 'X';
      if ( !getsdist( temploc, minloc, mindist ) )
      {
         return -1;
      }

      temploc[ 5 ] = 'A';
      if ( !getsdist( temploc, minloc, mindist ) )
      {
         return -1;
      }
   }
   return static_cast<int>(mindist);
}
//---------------------------------------------------------------------------
int BaseContestLog::CalcCentres( const QString &qscalcloc )
{
   if ( qscalcloc.length() < 4 )
      return 0;	// only valid 4 or more fig locs

   // calculate the nearest point of loc2 from loc1

   QString temploc = qscalcloc.left(4) + "MM";

   int brg;
   double dist = 0.0;
   double lon = 0.0;
   double lat = 0.0;

   int lres = lonlat( temploc, lon, lat );
   if (lres == LOC_OK)
   {
        disbear(lon, lat, dist, brg);
   }
   return static_cast<int>(dist);
}
void BaseContestLog::getMatchText( QSharedPointer<BaseContact> pct, QString &disp, const BaseContestLog *const ct ) const
{
   if ( DupSheet.isCurDup( pct ) )
   {
      disp = "DUPLICATE OF ";
   }
   QString temp;
   pct->getText( temp, ct );
   disp += temp;

   disp = disp.trimmed();
}
bool BaseContestLog::isCurDup( QSharedPointer<BaseContact> pct) const
{
   return pct && DupSheet.isCurDup( pct );
}
void BaseContestLog::getMatchField(QSharedPointer<BaseContact> pct, int col, QString &disp, const BaseContestLog *const ct ) const
{
   if ( col ==0 && isCurDup( pct ) )
   {
      disp = "DUP OF";
      return ;
   }
   QString temp;
   if ( pct )
      temp = pct->getField( col, ct );

   disp = temp.trimmed();
}
bool BaseContestLog::updateStat( QSharedPointer<BaseContact> cct, int sp1, int sp2 )
{
   // need to check if a valid DTG
   bool acted = false;

   QDateTime cttime;

   if ( ( cct->contactScore.getValue() <= 0 ) || !cct->time.getDtg( cttime ) )
      return true;

   QDateTime t = QDateTime::currentDateTimeUtc().addSecs( MinosParameters::getMinosParameters() ->getBigClockCorrection());

   qint64 tdiff = cttime.secsTo(t);
   if ( tdiff < 0 )
      return true;

   long thisscore = cct->contactScore.getValue();
   switch ( scoreMode.getValue() )
   {
      case PPKM:
         break;

      case PPQSO:
         if ( thisscore > 0 )
            thisscore = 1;
         else
            thisscore = 0;
         break;

   }

   // find the time since the beginning of the contest

   QDateTime  contestStart = CanonicalToTDT(DTGStart.getValue());
   qint64 fromContestStart = contestStart.secsTo(QDateTime::currentDateTime());
   if (sp1 > fromContestStart/2)
   {
      sp1 = static_cast<int>(fromContestStart/2);
   }
   if (sp2 > fromContestStart/2)
   {
      sp2 = static_cast<int>(fromContestStart/2);
   }

   if ( tdiff < sp1 )
   {
      // need a common routine
      // This period
      QSO1++;
      kms1 += thisscore;
      mults1 += cct->multCount;
      bonus1 += cct->bonus;
      acted = true;
   }
   else
      if ( tdiff < sp1 * 2 )
      {
         // need a common routine
         // previous period
         QSO1p++;
         kms1p += thisscore;
         mults1p += cct->multCount;
         bonus1p += cct->bonus;
         acted = true;
      }

   if ( tdiff < sp2 )
   {
      // need a common routine
      // this period
      QSO2++;
      kms2 += thisscore;
      mults2 += cct->multCount;
      bonus2 += cct->bonus;
      acted = true;
   }
   else
      if ( tdiff < sp2 * 2 )
      {
         // need a common routine
         // previous period
         QSO2p++;
         kms2p += thisscore;
         mults2p += cct->multCount;
         bonus2p += cct->bonus;
         acted = true;
      }

   return acted;
}
void BaseContestLog::updateStats( int p1, int p2 )
{
   QSO1 = 0;
   kms1 = 0;
   mults1 = 0;
   QSO2 = 0;
   kms2 = 0;
   mults2 = 0;
   QSO1p = 0;
   kms1p = 0;
   mults1p = 0;
   QSO2p = 0;
   kms2p = 0;
   mults2p = 0;
   bonus1 = 0;
   bonus1p = 0;
   bonus2 = 0;
   bonus2p = 0;

   auto it = ctList.end(), end = ctList.begin();
   while ( it != end ) {
       --it;
       if ( !updateStat( it.value().wt, p1, p2 ) )
          break;
   }

}
int BaseContestLog::getValidQSOs()
{
   int nvalid = 0;
   foreach(MapWrapper<BaseContact> i, ctList)
   {
      QSharedPointer<BaseContact> dct = i.wt;

      if ( dct->contactFlags.getValue() & ( LOCAL_COMMENT | COMMENT_ONLY | DONT_PRINT ) )
         continue;

      if ( dct->contactScore.getValue() > 0 )
         nvalid++;
   }
   return nvalid;
}

static void isBestDX( QSharedPointer<BaseContact> cct, QSharedPointer<BaseContact> *bestDX )
{

   if ( cct->contactFlags.getValue() & ( NON_SCORING | COMMENT_ONLY | LOCAL_COMMENT | DONT_PRINT ) )
      return ;

   if ( cct->cs.valRes != CS_OK )
      return ;

   if ( ( !*bestDX ) || ( ( cct->contactScore.getValue() > ( *bestDX ) ->contactScore.getValue() ) ) )
      * bestDX = cct;
}
QSharedPointer<BaseContact> BaseContestLog::getBestDX( )
{
   QSharedPointer<BaseContact> bestDX;
   for ( LogIterator i = ctList.begin(); i != ctList.end(); i++ )
      isBestDX( i->wt, &bestDX );
   return bestDX;
}
QString BaseContestLog::dateRange( DTG dstyle )
{
   QString date1 = "999999";
   QString date2 = "000000";
   LogIterator low = ctList.end();
   LogIterator high = ctList.end();
   for ( LogIterator i = ctList.begin(); i != ctList.end(); i++ )
   {
      if ( i->wt ->contactScore.getValue() > 0 )
      {
         QString qsodate = i->wt ->time.getDate( DTGLOG );
         if ( qsodate < date1 )
         {
            low = i;
            date1 = i->wt ->time.getDate( DTGLOG );
         }
         if ( qsodate > date2 )
         {
            high = i;
            date2 = i->wt ->time.getDate( DTGLOG );
         }
      }
   }
   if ( low == ctList.end() || high == ctList.end() )
   {
      return "";
   }
   return low->wt->time.getDate( dstyle ) + ";" + high->wt->time.getDate( dstyle );
}

void BaseContestLog::setScore( QString &buff )
{
   ContestScore cs(this);
   getScoresTo(cs, QDateTime::currentDateTime());
   buff = cs.disp();
}
// and we need to do this a bit more often to pick up unfilled properly
void BaseContestLog::scanContest( )
{
   DupSheet.clear();

   locs.llist.clear();

   delete [] districtWorked;
   delete [] countryWorked;
   districtWorked = nullptr;
   countryWorked = nullptr;

   int nc = MultLists::getMultLists() ->getDistListSize();
   districtWorked = new int[ nc ];
   int si = static_cast<int>(sizeof (int));
   memset( districtWorked, 0, static_cast<size_t>(nc * si) );

   nc = MultLists::getMultLists() ->getCtryListSize();
   countryWorked = new int[ nc ];
   memset( countryWorked, 0, static_cast<size_t>(nc * si) );

   // set up for the idle loop scan
   // NB we may need to clear e.g. the accumulated score

   // we will need to clear the multiplier work counts


   contestScore = 0;
   ndistrict = 0;
   nctry = 0;
   nlocs = 0;

   nextScan = -1;
   unfilledCount = 0;

//   oplist.clear();
   QString curop1 = currentOp1.getValue();
   oplist.insert( curop1, curop1 );
   QString curop2 = currentOp2.getValue();
   oplist.insert( curop2, curop2 );

   foreach(MapWrapper<BaseContact> wnct, ctList)
   {
      // get the next contact in sequence and do any required scan checks
      QSharedPointer<BaseContact> nct = wnct.wt;

      if (nct->contactFlags.getValue() & TO_BE_ENTERED)
      {
         unfilledCount++;
      }
      QString temp = nct->op1.getValue();
      if (temp.size())
      {
         curop1 = temp;
         oplist.insert( curop1, curop1 );
      }
      curop2 = nct->op2.getValue();
      oplist.insert( curop2, curop2 );

      if ( nct->contactFlags.getValue() & ( NON_SCORING | DONT_PRINT | LOCAL_COMMENT | COMMENT_ONLY ) )
      {
         nct->contactScore.setValue( -1 );		// force it!
         continue;
      }

      validationPoint = nct->getLogSequence();

      // check for duplicates; accumulate the current points score

      nct->cs.valRes = CS_NOT_VALIDATED;
      nct->cs.validate( );

      nct->bearing = -1;		// force a recalc
      nct->loc.validate();

      if ( DupSheet.checkCurDup( this, nct->getLogSequence(), 0, true ) )    // check for dup, insert it if required
         nct->cs.valRes = ERR_DUPCS;


      nct->multCount = 0;
      nct->newDistrict = false;
      nct->newCtry = false;
      nct->locCount = 0;
      nct->newGLoc = false;
      nct->newNonGLoc = false;
      nct->bonus = 0;
      nct->newBonus = false;
      nct->checkContact( );   // in scanContest

      if (nct->time.notEntered() == 0 && !(nct->contactFlags.getValue() & TO_BE_ENTERED))
      {
         nct->time = nct->getHistory()[0]->updtime;
         nct->time.clearDirty();
         nct->time.setBadDtg();
      }
   }
   if ( isReadOnly() )
   {
      DupSheet.clear();
   }
   if (currentOp1.getValue().size() == 0)
   {
      currentOp1.setValue( curop1 );
   }
}
void BaseContestLog::getScoresTo(ContestScore &cs, QDateTime limit)
{
// #warning doesn't pick up invalid QSOs - e.g. needs district and doesn't have one
   cs.nctry = 0;
   cs.ndistrict = 0;
   cs.nlocs = 0;
   cs.nGlocs = 0;
   cs.nonGlocs = 0;
   cs.nqsos = 0;
   cs.contestScore = 0;
   cs.bonus = 0;
   cs.nbonus = 0;

   foreach(MapWrapper<BaseContact> i, ctList)
   {
       // get the next contact in sequence and do any required scan checks
      QSharedPointer<BaseContact> nct = i.wt;

// NB this doesn't cope with crazy times from test contests and QSOs

      QDateTime start = CanonicalToTDT( DTGStart.getValue() );

      QString dtgstr = nct->time.getDate(DTGFULL) + nct->time.getTime(DTGLOG);
      QDateTime ncheck = CanonicalToTDT( dtgstr );

      if (ncheck > limit)
      {
         break;
      }


      if ( nct->contactFlags.getValue() & ( NON_SCORING | DONT_PRINT | LOCAL_COMMENT | COMMENT_ONLY | TO_BE_ENTERED ) )
      {
         continue;
      }
      if (nct->cs.valRes != CS_OK)
      {
         continue;
      }

     if ( locatorField.getValue() || nct->contactScore.getValue() >= 0 )   		// don't add -1 scores in, but DO add zero km
                                                                                // as it is 1 point.
      {
         int cscore = nct->contactScore.getValue();
         switch ( scoreMode.getValue() )
         {
            case PPKM:
               {
                  if ( nct->contactFlags.getValue() & XBAND )
                  {
                     cscore = ( cscore + 1 ) / 2;
                  }
                  cs.contestScore += cscore;
               }
               break;

            case PPQSO:
               if ( cscore > 0 )
                  cs.contestScore++;
               break;

         }
         cs.nctry += nct->newCtry?1:0;
         cs.ndistrict += nct->newDistrict?1:0;
         cs.nlocs += nct->locCount;
         cs.nqsos++;

         cs.bonus += nct->bonus;
         cs.nbonus += nct->newBonus?1:0;

         if (nct->newGLoc)
         {
            cs.nGlocs++;
         }
         else if ((nct->newNonGLoc))
         {
            cs.nonGlocs++;
         }
      }
      else
      {
         trace(QString("neg score ") + nct->cs.fullCall.getValue() + " " + nct->serials.getValue());
      }
   }
   cs.nmults = 0;
   if ( countryMult.getValue() )
   {
      cs.brcc1 = cs.brcc2 = ' ';
      cs.nmults += cs.nctry;
   }
   if ( districtMult.getValue() )
   {
      cs.brcc3 = cs.brcc4 = ' ';
      cs.nmults += cs.ndistrict;
   }
   if ( locMult.getValue() )
   {
      cs.brloc1 = cs.brloc2 = ' ';
      cs.nmults += cs.nlocs;
   }
   if (usesBonus.getValue())
   {
      cs.brbonus1 = cs.brbonus2 = ' ';
   }
   cs.nmults = qMax(cs.nmults, 1);

   cs.totalScore = (cs.contestScore + cs.bonus)*cs.nmults;

}
//============================================================
DupContact::DupContact(QSharedPointer<BaseContact> c ) : dct( c ), sct( nullptr )
{}
DupContact::DupContact( ScreenContact *c ) : dct( nullptr ), sct( c )
{}
DupContact::DupContact() : dct( nullptr ), sct( nullptr )
{}
DupContact::~DupContact()
{}
bool DupContact::operator<( const DupContact& rhs ) const
{
   Callsign * c1 = nullptr;    // search item
   Callsign *c2 = nullptr;    // collection item

   if ( dct )
   {
      c1 = &dct->cs;
   }
   else
      if ( sct )
      {
         c1 = &sct->cs;
      }
   if ( rhs.dct )
   {
      c2 = &rhs.dct->cs;
   }
   else
      if ( rhs.sct )
      {
         c2 = &rhs.sct->cs;
      }

   return (*c1 < *c2);
}
bool DupContact::operator==( const DupContact& rhs ) const
{
   Callsign * c1 = nullptr;    // search item
   Callsign *c2 = nullptr;    // collection item

   if ( dct )
   {
      c1 = &dct->cs;
   }
   else
      if ( sct )
      {
         c1 = &sct->cs;
      }
   if ( rhs.dct )
   {
      c2 = &rhs.dct->cs;
   }
   else
      if ( rhs.sct )
      {
         c2 = &rhs.sct->cs;
      }

   return (*c1 == *c2);
}
bool DupContact::operator!=( const DupContact& rhs ) const
{
   return !( *this == rhs );
}
dupsheet::dupsheet()
{}
dupsheet::~dupsheet()
{
   clear();
}
bool dupsheet::checkCurDup(ScreenContact *nct, unsigned long valpseq, bool insert )
{
   curdup.reset();
   if ( nct->cs.valRes == CS_OK )
   {
      QSharedPointer<DupContact> test( new DupContact(nct) );
      DupIterator c = ctList.find(test);
      if ( c!= ctList.end() )
      {
         if ( !( nct->contactFlags & VALID_DUPLICATE ) )
         {
            if ( valpseq != 0 && valpseq <= c->wt ->dct->getLogSequence() )
            {
               return false; // as val point earlier than current list item
            }

            if ( c != ctList.end() )
               curdup = c->wt;

            return true;
         }
      }
      else
         if ( insert )
         {
            MapWrapper<DupContact> ins( test);
            ctList.insert( ins, ins );
            return false;
         }
   }
   return false;
}
bool dupsheet::checkCurDup(BaseContestLog *contest, unsigned long nctseq, unsigned long valpseq, bool insert )
{
   curdup.reset();
   QSharedPointer<BaseContact> nct = contest->pcontactAtSeq(nctseq);
   if ( nct->cs.valRes == CS_OK )
   {
      QSharedPointer<DupContact> test( new DupContact(nct) );
      DupIterator c = ctList.find(test);
      if ( c != ctList.end() )
      {
         if ( !( nct->contactFlags.getValue() & VALID_DUPLICATE ) )
         {
            if ( valpseq != 0 && valpseq  <= c->wt ->dct->getLogSequence() )
            {
               return false; // as val point earlier than current list item
            }

            if ( c != ctList.end() )
               curdup = c->wt;

            return true;
         }
      }
      else
         if ( insert )
         {
            QSharedPointer<DupContact> ins(new DupContact( nct ));
            ctList.insert( ins, ins );
            return false;
         }
   }
   return false;
}

bool dupsheet::isCurDup( ScreenContact *nct ) const
{
   const DupContact test( nct );
   bool cd = ( curdup && ( *curdup == test ) ) ;

   if ( cd && !nct->isNextContact() )
   {
      if ( nct->getLogSequence() > curdup->dct->getLogSequence() )
      {
         return false;
      }
   }
   return cd;
}
bool dupsheet::isCurDup(QSharedPointer<BaseContact> nct ) const
{
   const DupContact test( nct );
   bool cd = curdup && ( *curdup == test ) ;

   if ( cd )
   {
      if ( nct->getLogSequence() > curdup->dct->getLogSequence() )
      {
         return false;
      }
   }
   return cd;
}
void dupsheet::clearCurDup()
{
   curdup.reset();
}
QSharedPointer<BaseContact> dupsheet::getCurDup()
{
   if ( curdup )
      return curdup->dct;
   return QSharedPointer<BaseContact>();
}
void dupsheet::clear()
{
   curdup.reset();
   ctList.clear();
}
//============================================================
void BaseContestLog::processMinosStanza( const QString &methodName, MinosTestImport * const mt )
{
   unsigned long logSequence = static_cast< unsigned long > (- 1);

   int itemp;
   if ( mt->getStructArgMemberValue( "lseq", itemp ) )
      logSequence = static_cast< unsigned long > (itemp);

   if ( methodName == "MinosLogContest" )
   {
      mt->getStructArgMemberValue( "name", name );
      mt->getStructArgMemberValue( "band", band );

      bool btemp;
      if ( mt->getStructArgMemberValue( "scoreKms", btemp ) )
         scoreMode.setInitialValue( btemp ? PPKM : PPQSO );

      mt->getStructArgMemberValue( "startTime", DTGStart );
      mt->getStructArgMemberValue( "endTime", DTGEnd );

      mt->getStructArgMemberValue( "districtMult", districtMult );
      mt->getStructArgMemberValue( "DXCCMult", countryMult );
      mt->getStructArgMemberValue( "NonGCtryMult", nonGCountryMult );
      mt->getStructArgMemberValue( "locMult", locMult );
      mt->getStructArgMemberValue( "GLocMult", GLocMult );
      mt->getStructArgMemberValue( "QTHReq", otherExchange );
      mt->getStructArgMemberValue( "AllowLoc4", allowLoc4 );
      mt->getStructArgMemberValue( "AllowLoc8", allowLoc8 );
      mt->getStructArgMemberValue( "currentMode", currentMode);

      mt->getStructArgMemberValue( "RSTField", RSTField);
      mt->getStructArgMemberValue( "serialField", serialField);
      mt->getStructArgMemberValue( "locatorField", locatorField);

      mt->getStructArgMemberValue( "UKACBonus", usesBonus );
      mt->getStructArgMemberValue("BonusType", bonusType);
      if (usesBonus.getValue())
      {
          if (bonusType.getValue().isEmpty())
              bonusType.setValue("B4"); // cope with old Minos files
          loadBonusList();
      }
      mt->getStructArgMemberValue( "M7Mults", M7Mults);

      if ( M7Mults.getValue())
      {
         NonUKloc_mult = true;
         NonUKloc_multiplier = 1;
         UKloc_mult = true;
         UKloc_multiplier = 2;
      }
      else
      {
         if (locMult.getValue())
         {
            UKloc_multiplier = 1;
            UKloc_mult = true;
            if (GLocMult.getValue())
            {
               NonUKloc_multiplier = 0;
               NonUKloc_mult = false;
            }
            else
            {
               NonUKloc_multiplier = 1;
               NonUKloc_mult = true;
            }
         }
         else
         {
            UKloc_multiplier = 0;
            UKloc_mult = false;
            NonUKloc_multiplier = 0;
            NonUKloc_mult = false;
         }
      }
      mt->getStructArgMemberValue("MGMContestRules", MGMContestRules);
   }
   else
      if ( methodName == "MinosLogMode" )
      {
         mt->getStructArgMemberValue( "protectedContest", protectedContest );
      }
      else
         if ( methodName == "MinosLogQTH" )
         {
            if ( mt->getStructArgMemberValue( "locator", myloc.loc ) )
               validateLoc();
            mt->getStructArgMemberValue( "district", location );
            mt->getStructArgMemberValue( "location", location ); // doubled up...
         }
         else
            if ( methodName == "MinosLogEntry" )
            {
               mt->getStructArgMemberValue( "call", mycall.fullCall );
            }
            else
               if ( methodName == "MinosLogStation" )
               {
                  mt->getStructArgMemberValue( "power", power );
               }
               else
                  if ( methodName == "MinosLogOperators" )
                  {}
                  else
                     if ( methodName == "MinosLogCurrent" )
                     {}
                     else
                        if ( methodName == "MinosLogBundles" )
                        {}
                        else
                           if ( methodName == "MinosLogComment" )
                           {
                             QSharedPointer<BaseContact> rct = pcontactAtSeq( logSequence );
                              if ( !rct )
                              {
                                 makeContact( false, rct );
                                 rct->setLogSequence( logSequence );
                                 MapWrapper<BaseContact> wrct(rct);
                                 ctList.insert( wrct, wrct );
                                 if (logSequence >> 16 >= nextBlock)
                                 {
                                    nextBlock = (logSequence >> 16) + 1;
                                 }
                              }
                              rct->processMinosStanza( methodName, mt );

                           }
                           else
                              if ( methodName == "MinosLogQSO" )
                              {
                                 QSharedPointer<BaseContact> rct = pcontactAtSeq( logSequence );
                                 if ( !rct )
                                 {
                                    makeContact( false, rct );
                                    rct->setLogSequence( logSequence );
                                    MapWrapper<BaseContact> wrct(rct);
                                    ctList.insert( wrct, wrct );
                                    // Was just nextBlock++ - no test
                                    if (logSequence >> 16 >= nextBlock)
                                    {
                                       nextBlock = (logSequence >> 16) + 1;
                                    }
                                 }
                                 rct->processMinosStanza( methodName, mt );

                              }
}

//====================================================================
void BaseContestLog::setStanza( unsigned int /*stanza*/, int /*stanzaStart*/ )
{
   // not used in base log
   // Used in Logger to give file position of stanza
}
//====================================================================
bool BaseContestLog::getStanza( unsigned int /*stanza*/, QString & /*stanzaData*/ )
{
   // not used in base log
   // used in Logger to get stanza direct from a file
   return false;
}
//====================================================================
QSharedPointer<BaseContact> BaseContestLog::findNextUnfilledContact()
{
   for ( LogIterator i = ctList.begin(); i != ctList.end(); i++ )
   {
      if ( i->wt ->contactFlags.getValue() & TO_BE_ENTERED )
      {
         return i->wt;
      }
   }
   return QSharedPointer<BaseContact>();
}
//====================================================================
bool BaseContestLog::checkTime(const QDateTime &check) const
{
      QString t1 = DTGStart.getValue();
      QDateTime start = CanonicalToTDT( t1 );
      t1 = DTGEnd.getValue();
      QDateTime end = CanonicalToTDT( t1 );

      if (check < start)
      {
         return false;
      }
      if (check >= end)
      {
         return false;
      }
      return true;
}
bool BaseContestLog::checkTime(const dtg &t) const
{
    QString dtgstr = t.getDate(DTGFULL) + t.getTime(DTGLOG);

    QDateTime check = CanonicalToTDT( dtgstr );

    return checkTime(check);
}
static bool loadCalYear ( Calendar &cal, int year )
{
    bool loaded = false;
    QVector<QSharedPointer<CalendarYear> > yearList;
    for ( int i = LOWYEAR; i <= HIGHYEAR; i++ )
    {
        yearList.push_back ( QSharedPointer<CalendarYear> ( new VHFCalendarYear ( i ) ) );
    }

    for ( int i = yearList.size() - 1; i >= 0; i-- )
    {
        if ( !loaded && FileExists ( yearList[ i ] ->getPath() ) && year >= calendarFormYear + yearList[ i ] ->yearOffset )
        {
            loaded = cal.parseFile ( yearList[ i ] ->getPath() );
        }
    }
    return loaded;
}
void BaseContestLog::loadBonusList()
{
    if (usesBonus.getValue() && (bonusType.getValue() == "B2" || bonusType.getValue() == "B4"))
    {
        QDateTime  contestStart = CanonicalToTDT(DTGStart.getValue());
        int year = contestStart.date().year();
        if (year != bonusYearLoaded || bonusType.getValue() != bonusTypeLoaded)
        {
            Calendar vhf(year, ectVHF);
            bool loaded = loadCalYear ( vhf, year );

            if (loaded)
            {
                bonusYearLoaded = year;
                bonusTypeLoaded = bonusType.getValue();
            }

            locBonuses.clear();
            if (bonusType.getValue() == "B2")
            {
                MultType B2 = vhf.mults["B2"];

                if (B2.bonuses.size() == 0)
                {
                    // load from ./Configuration/B2Mults.xml
                    vhf = Calendar(year, ectVHF);
                    loaded = vhf.parseFile ( "./Configuration/B2Mults.xml" );
                    B2 = vhf.mults["B2"];
                }
                for (QMap<QString, int>::iterator i = B2.bonuses.begin(); i != B2.bonuses.end(); i++)
                {
                    QString name = i.key().toUpper();
                    int value = i.value();

                    locBonuses[name] = value;
                }
            }
            if (bonusType.getValue() == "B4")
            {
                MultType B4 = vhf.mults["B4"];

                if (B4.bonuses.size() == 0)
                {
                    B4.bonuses["DEFAULT"] = 500;
                }
                for (QMap<QString, int>::iterator i = B4.bonuses.begin(); i != B4.bonuses.end(); i++)
                {
                    QString name = i.key().toUpper();
                    int value = i.value();

                    locBonuses[name] = value;
                }
            }
        }
    }
    else if (usesBonus.getValue() && bonusType.getValue() == "NAC")
    {
        bonusYearLoaded = 0;
        locBonuses.clear();
        locBonuses["DEFAULT"] = 500;
    }

}
int BaseContestLog::getSquareBonus(QString sloc)
{
    int bonus = 0;
    QMap<QString, int>::iterator l = locBonuses.find(sloc);

    if ( l != locBonuses.end())
    {
       // specific bonus for square allocated
       bonus = l.value();
    }
    else
    {
        QMap<QString, int>::iterator l = locBonuses.find("DEFAULT");
        if ( l != locBonuses.end())
        {
           // specific bonus for square allocated
           bonus = l.value();
        }
    }
    return bonus;
}

//====================================================================
ContestScore::ContestScore(BaseContestLog *ct)
{
   brcc1 = '(';
   brcc2 = ')';
   brcc3 = '(';
   brcc4 = ')';
   brloc1 = '(';
   brloc2 = ')';
   brbonus1 = '(';
   brbonus2 = ')';

   name = ct->publishedName;
   usesBonus = ct->usesBonus.getValue();
}
QString ContestScore::disp()
{
    QString buff;
    if (usesBonus == true)
    {
        buff = QString( "Score: Qsos: %1; %2 pts :%3%4 countries%5: bonuses %6(%7) = %8" )
            .arg(nqsos).arg(contestScore).arg(brcc1).arg(nctry).arg(brcc2)
            .arg(bonus) .arg(nbonus)
            .arg(totalScore );
    }
    else
    {
        buff = QString( "Score: Qsos: %1; %2 pts :%3%4 countries%5:%6%7 districts%8:%9%10(%11/%12) locators %13 = %14" )
            .arg(nqsos).arg(contestScore).arg(brcc1).arg(nctry).arg(brcc2).arg(brcc3).arg(ndistrict)
            .arg(brcc4).arg(brloc1).arg(nlocs).arg(nGlocs).arg(nonGlocs).arg(brloc2)
            .arg(totalScore );    }
   return buff;
}
//====================================================================

