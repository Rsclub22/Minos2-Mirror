/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include <QHostInfo>
#include <cmath>
#include "QtUtils.h"
#include "calcs.h"
#include "contacts.h"
#include "cutils.h"
#include "Calendar.h"
#include "CalendarList.h"
#include "MinosTestImport.h"
#include "BandList.h"
#include "MinosParameters.h"
#include "contest.h"
#include "fileutils.h"
#include "rigcontrolcommonconstants.h"
#include "MTrace.h"

void BaseContestLog::addCountryWorked(QString band, const QString &basePrefix)
{
    countryWorked[band][ basePrefix ]++;
}

void BaseContestLog::addDistrictWorked(QString band, const QString &cd)
{
    districtWorked[band][ cd ]++;
}

BaseContestLog::BaseContestLog()
{}
BaseContestLog::BaseContestLog(bool hf)
{
    hfContest.setValue(hf);

    static int inst = 0;
    QString h = QHostInfo::localHostName();
   uuid = /*makeUuid()*/ h + "_" + QString::number(inst++);
   bearingOffset.setValue(0);
   currentMode.setValue( isHF()?PH:hamlibData::USB );

  protectedContest.setValue( false );
  allowLoc8.setValue( false );
  allowLoc4.setValue ( false );
  RSTMandatoryField.setValue( true );
  serialMandatoryField.setValue( true );
  locatorMandatoryField.setValue( !hf );
  exchangeRequired.setValue( false );
  exchangeDashAllowed.setValue( false );
  countryMult.setValue( false );
  districtMult.setValue( false );
  districtBonus.setValue( false );
  locMult.setValue( false );
  otherMult.setValue(0);
  asymmetricMult.setValue(false);
  usesBonus.setValue(false);
  scoreMode.setValue( PPKM );
  powerWatts.setValue( true );
  MGMContestRules.setValue(false);
  bandPointsMultiplier.setValue(1);

}
BaseContestLog::~BaseContestLog()
{
}
void BaseContestLog::setVersion(QString v)
{
    appVersion.setValue(v);
}
QString BaseContestLog::getCabrilloName(QString contestName, QString cabrilloName, int calType)
{
    if (cabrilloName.isEmpty())
    {
        QString calTypeString;

        switch (calType)
        {
        case ectVHF:
        case ectHF:
        case ectMwave:
            calTypeString = "RSGB";
            break;
        case ectHFBARTG:
            calTypeString = "BARTG";
            break;
        case ectUKSMG:
            calTypeString = "UKSMG";
        default:
        case ectVHFOther:
        case ectHFOther:
            break;      // leave empty
        };

        cabrilloName = contestName;
        if (!calTypeString.isEmpty())
        {
            if (!cabrilloName.startsWith(calTypeString))
            {
                cabrilloName = calTypeString + " " + cabrilloName;
            }
            cabrilloName .replace(" ", "-");
        }

    }
    return cabrilloName;
}


void BaseContestLog::clearCache()
{
    ctIndexCache.clear();
    ctPointerIndexMap.clear();
}

void BaseContestLog::refreshCache()
{
    if (cacheRefreshNeeded)
    {
        clearCache();
        int offset = 0;
        for ( auto const &c: QASCONST(ctList ))
        {
            ctPointerIndexMap[c.wt.data()] = offset;
            ctIndexCache.push_back(c.wt);
            offset++;
        }
        cacheRefreshNeeded = false;
    }
}

void BaseContestLog::addToContestList(QSharedPointer<BaseContact> rct )
{
    MapWrapper<BaseContact> wrct(rct);
    ctList.insert( wrct, wrct );

    MapWrapper<BaseContact> last = *(std::prev(ctList.end()));
    if (last != rct)
    {
        cacheRefreshNeeded = true;
        lastInserted = indexOf(rct);
    }
    else
    {
        int s = ctList.size() - 1;
        ctPointerIndexMap[rct.data()] = s;
        ctIndexCache.push_back(rct);
        lastInserted = s;
    }
}
int BaseContestLog::indexOf(QSharedPointer<BaseContact> item )
{
    refreshCache();
    int offset = -1;
    auto ci = ctPointerIndexMap.find(item.data());
    if (ci != ctPointerIndexMap.end())
    {
        offset = *ci;
    }
    return offset;
}
int BaseContestLog::getContactCount( )
{
   return ctList.size();
}

QSharedPointer<BaseContact> BaseContestLog::pcontactAt( int i )
{
    refreshCache();
    QSharedPointer<BaseContact> bc;
    if (i < ctIndexCache.size())
    {
        bc = ctIndexCache[i];
    }
    return bc;
}

QSharedPointer<BaseContact> BaseContestLog::pcontactAtSeq( unsigned long logSequence ) const
{
    QSharedPointer<BaseContact> test(new BaseContact(nullptr, false));
    test->setLogSequence(logSequence);
    auto res = ctList.find(test);
    if (res != ctList.end())
    {
        return (*res).wt;
    }

   return QSharedPointer<BaseContact>();
}
double BaseContestLog::getAdifFreqBand(Frequency txfreq, QString &cb) const
{
    // get a tx freq, even when we don't have
    // rig control, and the proper ADIF name of the band

    double freq = txfreq;
    BandList &blist = BandList::getBandList();
    QSharedPointer<BandInfo>  bi;
    bool bandOK = false;

    if (freq >= 100)
    {
        bandOK = blist.findBand(freq, bi);
        if (bandOK)
        {
            cb = bi->adif;
            return freq;
        }

    }

    //currentBand is irrelevant (might not be the same as the QSO) - we have to work from frequency

    QString cband = currentBand.getValue();

    QString tcb = cband.trimmed();
    bandOK = blist.findBand(tcb, bi);
    if (bandOK)
    {
        tcb = bi->adif;
        if (freq < 100)
        {
            cb = tcb;
            freq = bi->bandfreq;
            return freq;
        }
    }
    return freq;
}
QString BaseContestLog::getCabrilloFreqBand(Frequency txfreq ) const
{
    // get a tx freq, even when we don't have
    // rig control, and the proper Cabrillo name of the band

    QString cband = currentBand.getValue();

    // currentBand is irrelevant (might not be the same as the QSO) - we have to work from frequency
    // but I think this still works

    QString cb = cband.trimmed();
    BandList &blist = BandList::getBandList();
    QSharedPointer<BandInfo>  bi;
    bool bandOK = blist.findBand(cb, bi);
    if (bandOK)
    {
        cb = bi->cabrillo;
        qint64 f = qint64(txfreq);
        if (f  < 100)
        {
            return cb;
        }

        if (bi->getType() != HF_BANDTYPE)
        {
            return cb;
        }
        f = f/1000.0;
        return QString::number(f, 'f', 0);
    }
    return "XXX";
}

Frequency BaseContestLog::getTxFreqBand(Frequency txfreq, QString &cb) const
{
    // we now want to get the band associated with the current freq
    // so we can get the correct map value for mults etc

    BandList &blist = BandList::getBandList();
    QSharedPointer<BandInfo>  bi;
    bool bandOK;

    Frequency freq = txfreq;

    if (qint64(txfreq) < 100)
    {
        // we are only using currentBand here as a last resort
        QString cband = currentBand.getValue().trimmed();
        bandOK = blist.findBand(cband, bi);
        if (bandOK)
        {
            cb = bi->uk;
            freq = bi->bandfreq;
        }
        else
        {
            cb = "XXX";
        }
        return freq;
    }

    bandOK = blist.findBand(freq, bi);
    if (bandOK)
    {
        cb = bi->uk;
    }
    else
    {
        cb = "XXX";     // a standard value for no known band
    }

    return freq;
}

void BaseContestLog::setCurrentBand(QString cb)
{
    currentBand.setValue(cb);
    commonSave(false);
}

//==========================================================================

void BaseContestLog::clearDirty()
{
   appVersion.clearDirty();
   protectedContest.clearDirty();
   mycall.clearDirty();

   name.clearDirty();
   cabrilloName.clearDirty();
   calType.clearDirty();
   location.clearDirty();
   myloc.clearDirty();
   allowLoc4.clearDirty();
   allowLoc8.clearDirty();
   RSTMandatoryField.clearDirty();
   serialMandatoryField.clearDirty();
   locatorMandatoryField.clearDirty();
   power.clearDirty();
   currentMode.clearDirty();
   modeList.clearDirty();
   contestBands.clearDirty();
   currentBand.clearDirty();
   bandsList.clearDirty();
   hfContest.clearDirty();
   exchangeRequired.clearDirty();
   exchangeDashAllowed.clearDirty();
   countryMult.clearDirty();
   locMult.clearDirty();
   districtMult.clearDirty();
   districtBonus.clearDirty();
   otherMult.clearDirty();
   asymmetricMult.clearDirty();

   usesBonus.clearDirty();
   bonusType.clearDirty();
   MGMContestRules.clearDirty();

   bandPointsMultiplier.clearDirty();

   powerWatts.clearDirty();
   scoreMode.clearDirty();
   DTGStart.clearDirty();
   DTGEnd.clearDirty();
   currentOp1.clearDirty();
   currentOp2.clearDirty();
   for ( auto const &i: QASCONST(ctList) )
   {
      i.wt->clearDirty();
   }
   QSOMapFilterSettings.clearDirty();
}
void BaseContestLog::setDirty()
{
   appVersion.setDirty();
   protectedContest.setDirty();
   mycall.setDirty();
   name.setDirty();
   cabrilloName.setDirty();
   calType.setDirty();
   location.setDirty();
   myloc.setDirty();
   allowLoc4.setDirty();
   allowLoc8.setDirty();
   RSTMandatoryField.setDirty();
   serialMandatoryField.setDirty();
   locatorMandatoryField.setDirty();
   power.setDirty();
   currentMode.setDirty();
   modeList.setDirty();
   contestBands.setDirty();
   currentBand.setDirty();
   bandsList.setDirty();
   hfContest.setDirty();
   exchangeRequired.setDirty();
   exchangeDashAllowed.setDirty();
   countryMult.setDirty();
   locMult.setDirty();
   districtMult.setDirty();
   districtBonus.setDirty();
   otherMult.setDirty();
   asymmetricMult.setDirty();

   usesBonus.setDirty();
   bonusType.setDirty();
   MGMContestRules.setDirty();

   bandPointsMultiplier.setDirty();

   powerWatts.setDirty();
   scoreMode.setDirty();
   DTGStart.setDirty();
   DTGEnd.setDirty();
   currentOp1.setDirty();
   currentOp2.setDirty();

   for ( auto const &i: QASCONST(ctList) )
   {
      i.wt->setDirty();
   }
   QSOMapFilterSettings.setDirty();
}

QSharedPointer<BaseContact> BaseContestLog::findContact(CheckableContact *cct) const
{
    for ( auto const &i: QASCONST(ctList) )
    {
       // linear search
       if (i.wt.data() == cct)
       {
           return i.wt;
       }
    }
    return QSharedPointer<BaseContact>();
}
void BaseContestLog::makeContact( bool timeNow, QSharedPointer<BaseContact>&lct )
{
   lct = QSharedPointer<BaseContact>(new BaseContact( this, timeNow ));
}
void BaseContestLog::validateLoc( )
{
    if ( myloc.validate( odea, odna ) == LOC_OK )
    {
        cosodna = cos( odna );
        sinodna = sin( odna );
        locValid = true;
    }
    else
        locValid = false;

    Locator nloc;
    nloc.setLoc(myloc.getLoc().left(4) + "MM");
    if ( nloc.validate( odec, odnc ) == LOC_OK )
    {
        cosodnc = cos( odnc );
        sinodnc = sin( odnc );
        locValid = true;
        myloc.setValRes(LOC_OK);
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
void BaseContestLog::disbeara( double lon, double lat, double &dist, int &brg ) const
//double lon ;                    /* other stations longtitude */
//double lat ;                    /* other stations latitude */
//int *dist ;                     /* resulting distance */
//int *brg ;                      /* resulting bearing */

{
   if ( myloc.getValRes() != LOC_OK )
   {
      dist = 1;
      brg = 0;
      return ;
   }
   if ( almost_equal(lon, odea, 2) && almost_equal(lat, odna, 2) )        /* same square ! */ /* testing doubles for equality! */
   {
      // same square - commenced Kilometre, so always 1
      dist = 1;
      brg = 0 ;                       /* or bearing */
      return ;                         /* just exit */
   }

   double coslat = cos( lat );                    /* pre-calculate values */
   double sinlat = sin( lat );
   double coscos = cosodna * coslat ;

   // first distance into dx

   double co = cos( odea - lon ) * coscos + sinodna * sinlat ;
   double ca = atan( fabs( sqrt( 1.0 - co * co ) / co ) );
   if ( co < 0.0 )
      ca = pi - ca ;
   double dx = 6371.291 * ca ;       /* 6371.291 is approved radius of earth */

   // and then the bearing

   double si = sin( lon - odea ) * coscos ;
   co = sinlat - sinodna * cos( ca );
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
void BaseContestLog::disbearc( double lon, double lat, double &dist, int &brg ) const
//double lon ;                    /* other stations longtitude */
//double lat ;                    /* other stations latitude */
//int *dist ;                     /* resulting distance */
//int *brg ;                      /* resulting bearing */

{
   if ( myloc.getValRes() != LOC_OK )
   {
      dist = 1;
      brg = 0;
      return ;
   }
   if ( almost_equal(lon, odec, 2) && almost_equal(lat, odnc, 2) )        /* same square ! */ /* testing doubles for equality! */
   {
      // same square - commenced Kilometre, so always 1
      dist = 1;
      brg = 0 ;                       /* or bearing */
      return ;                         /* just exit */
   }

   double coslat = cos( lat );                    /* pre-calculate values */
   double sinlat = sin( lat );
   double coscos = cosodnc * coslat ;

   // first distance into dx

   double co = cos( odec - lon ) * coscos + sinodnc * sinlat ;
   double ca = atan( fabs( sqrt( 1.0 - co * co ) / co ) );
   if ( co < 0.0 )
      ca = pi - ca ;
   double dx = 6371.291 * ca ;       /* 6371.291 is approved radius of earth */

   // and then the bearing - but use the full loc for us!

   double si = sin( lon - odea ) * coscos ;
   co = sinlat - sinodna * cos( ca );
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
bool BaseContestLog::getsdist( const QString &loc, QString &minloc, double &mindist ) const
{
   int brg;
   double dist = 0.0;
   double lon = 0.0;
   double lat = 0.0;

   int lres = lonlat( loc, lon, lat, MinosParameters::getMinosParameters() ->getAllowLoc4() );
   if ( lres == LOC_OK || lres == LOC_SHORT )
   {
      disbeara( lon, lat, dist, brg );
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
int BaseContestLog::CalcNearest( const QString &qscalcloc ) const
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
int BaseContestLog::CalcCentres( const QString &qscalcloc, int &brg ) const
{
   if ( qscalcloc.length() < 4 )
      return 0;	// only valid 4 or more fig locs

   // calculate the nearest point of loc2 from loc1

   QString temploc = qscalcloc.left(4) + "MM";

   double dist = 0.0;
   double lon = 0.0;
   double lat = 0.0;

   int lres = lonlat( temploc, lon, lat, MinosParameters::getMinosParameters() ->getAllowLoc4() );
   if (lres == LOC_OK)
   {
        disbearc(lon, lat, dist, brg);
   }
   return static_cast<int>(dist);
}
void BaseContestLog::calcDistanceBearing(const QString& _locator, double* distance, int* bearing)
{
    bool locValid = true;
    QString locator = _locator;
    double latitude;
    double longitude;
    double dist;
    int brg = 0;

    if (!locator.isEmpty())
    {
        if (locator.size() == 4)
        {
            locator.append("MM");
        }

        int locValres = lonlat( locator, longitude, latitude, MinosParameters::getMinosParameters() ->getAllowLoc4() );
        if ( ( locValres ) != LOC_OK )
        {
            locValid = false;
        }
        if (locValid)
        {
            disbeara(longitude, latitude, dist, brg);
            *distance = dist;
            *bearing = brg;
        }
    }
}

void BaseContestLog::getMatchText( CheckableContact *pct, QString &disp, const BaseContestLog *const ct ) const
{
   if ( DupSheet.isCurDup( pct ) )
   {
      disp = tr("DUPLICATE OF ");
   }
   QString temp;
   pct->getText( temp, ct, false );
   disp += temp;

   disp = disp.trimmed();
}
bool BaseContestLog::isCurDup( CheckableContact *pct) const
{
   return pct && DupSheet.isCurDup( pct );
}
void BaseContestLog::getMatchField(CheckableContact *pct, int col, QString &disp, const BaseContestLog *const ct ) const
{
   if ( col ==0 && isCurDup( pct ) )
   {
      disp = tr("DUP OF");
      return ;
   }
   QString temp;
   if ( pct )
      temp = pct->getField( col, ct );

   disp = temp.trimmed();
}
bool BaseContestLog::updateStat( CheckableContact *cct, int sp1, int sp2 )
{
   // need to check if a valid DTG
   bool acted = false;

   QDateTime cttime;

   if ( ( cct->getContactScore() <= 0 ) || !cct->timeOff.getDtg( cttime ) )
      return true;

   QDateTime t = QDateTime::currentDateTimeUtc().addSecs( MinosParameters::getMinosParameters() ->getBigClockCorrection());

   qint64 tdiff = cttime.secsTo(t);
   if ( tdiff < 0 )
      return true;

   long thisscore = cct->getContactScore();
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
      bonus1 += cct->locBonus;
      bonus1 += cct->distBonus;
      bonus1 += cct->countryBonus;
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
         bonus1p += cct->locBonus;
         bonus1p += cct->distBonus;
         bonus1p += cct->countryBonus;
         acted = true;
      }

   if ( tdiff < sp2 )
   {
      // need a common routine
      // this period
      QSO2++;
      kms2 += thisscore;
      mults2 += cct->multCount;
      bonus2 += cct->locBonus;
      bonus2 += cct->distBonus;
      bonus2 += cct->countryBonus;
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
         bonus2p += cct->locBonus;
         bonus2p += cct->distBonus;
         bonus2p += cct->countryBonus;
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
       if ( !updateStat( it.value().wt.data(), p1, p2 ) )
          break;
   }

}
int BaseContestLog::getValidQSOs()
{
   int nvalid = 0;
   for(auto const &i: QASCONST(ctList))
   {
      QSharedPointer<BaseContact> dct = i.wt;

      if ( dct->notValidContact() )
      {
         continue;
      }

      if (dct->cs.getValRes() != CS_OK)    // duplicate?
      {
          continue;
      }

      if ( dct->contactScore.getValue() > 0 )
         nvalid++;
   }
   return nvalid;
}

static void isBestDX( QSharedPointer<BaseContact> cct, QSharedPointer<BaseContact> *bestDX )
{

   if ( cct->notValidContact() )
      return ;

   if ( cct->cs.getValRes() != CS_OK )
      return ;

   if ( ( !*bestDX ) || ( ( cct->contactScore.getValue() > ( *bestDX ) ->contactScore.getValue() ) ) )
      * bestDX = cct;
}
QSharedPointer<BaseContact> BaseContestLog::getBestDX( )
{
   QSharedPointer<BaseContact> bestDX;
   for ( auto const &i: QASCONST(ctList) )
      isBestDX( i.wt, &bestDX );
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
         QString qsodate = i->wt ->timeOff.getDate( DTGLOG );
         if ( qsodate < date1 )
         {
            low = i;
            date1 = i->wt ->timeOff.getDate( DTGLOG );
         }
         if ( qsodate > date2 )
         {
            high = i;
            date2 = i->wt ->timeOff.getDate( DTGLOG );
         }
      }
   }
   if ( low == ctList.end() || high == ctList.end() )
   {
      return "";
   }
   return low->wt->timeOff.getDate( dstyle ) + ";" + high->wt->timeOff.getDate( dstyle );
}

void BaseContestLog::setScore( QString &buff )
{
   ContestScore cs(this);
   getScoresTo(cs, QDateTime::currentDateTime());
   buff = cs.disp();
}
// and we need to do this a bit more often to pick up unfilled properly
QString BaseContestLog::scanContact(QSharedPointer<BaseContact> nct, QDateTime  contestStart, QDateTime  contestEnd)
{
    if (nct->contactFlags.getValue() & TO_BE_ENTERED)
    {
       unfilledCount++;
    }
    QString temp = nct->op1.getValue();
    QString curop1 = temp;
    if (temp.size())
    {
       oplist.insert( temp, temp );
    }
    temp = nct->op2.getValue();
    if (temp.size())
    {
        oplist.insert( temp, temp );
    }

    if ( nct->notValidContact() )
    {
       nct->contactScore.setValue( -1 );		// force it!
       return curop1;
    }

    QDateTime contactTime;
    bool dirty = false;
    nct->timeOff.getDtg(contactTime, dirty);
    if (contactTime >= contestStart && contactTime < contestEnd)
    {
        int qoffset = contestStart.secsTo(contactTime)/60;
        qsoTimeMap[qoffset].count++;
    }

    validationPoint = nct->getLogSequence();

    // check for duplicates; accumulate the current points score


    if ( DupSheet.checkCurDup( nct.data(), nct->getLogSequence(), true ) )    // check for dup, insert it if required
       nct->cs.setValRes( ERR_DUPCS);

    nct->bearing = -1;		// force a recalc

    nct->checkContact( false);   // in scanContest

    if (nct->timeOff.notEntered() == 0 && !(nct->contactFlags.getValue() & TO_BE_ENTERED))
    {
       nct->timeOff = nct->getHistory()[0]->updtime;
       nct->timeOff.clearDirty();
       nct->timeOff.setBadDtg();
    }
    return curop1;
}

void BaseContestLog::scanContest( )
{
   DupSheet.clear();

   locs.clear();

   districtWorked.clear();
   countryWorked.clear();

   // set up for the idle loop scan
   // NB we may need to clear e.g. the accumulated score

   // we will need to clear the multiplier work counts

   contestScore = 0;
   ndistrict.clear();
   nctry.clear();
   nlocs.clear();

   nextScan = -1;
   unfilledCount = 0;

//   oplist.clear();
   QString curop1 = currentOp1.getValue();
   oplist.insert( curop1, curop1 );
   QString curop2 = currentOp2.getValue();
   oplist.insert( curop2, curop2 );

   QDateTime  contestStart = CanonicalToTDT(DTGStart.getValue());
   QDateTime  contestEnd = CanonicalToTDT(DTGEnd.getValue());

   int contestMinutes =  static_cast<int>(contestStart.secsTo(contestEnd)/60);
   if (contestMinutes <= 0)
   {
       contestMinutes = 1;
   }

   if (qsoTimeMap.size() != contestMinutes)
   {
        qsoTimeMap.resize(contestMinutes);
   }
   for(auto &m:qsoTimeMap)
   {
       m.count = 0;
   }

   validateLoc();

   for(auto const &wnct: QASCONST(ctList))
   {
      // get the next contact in sequence and do any required scan checks
      QSharedPointer<BaseContact> nct = wnct.wt;

      nct->cs.reValidate();

      curop1 = scanContact(nct, contestStart, contestEnd);
   }
   //validationPoint = 0;
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
   cs.nqsos = 0;
   cs.contestScore = 0;
   cs.bonus = 0;
   cs.nbonus = 0;

   for(auto const &i: QASCONST(ctList))
   {
       // get the next contact in sequence and do any required scan checks
      QSharedPointer<BaseContact> nct = i.wt;

// NB this doesn't cope with crazy times from test contests and QSOs

      QDateTime ncheck =nct->timeOff.getQDT();

      if (ncheck > limit)
      {
         break;
      }


      if ( nct->notValidContact() )
      {
         continue;
      }
      if (nct->cs.getValRes() != CS_OK)
      {
         continue;
      }

     if ( nct->contactScore.getValue() >= 0 )   		// don't add -1 scores in, but DO add zero km
                                                                                // as it is 1 point.
      {
         if ( locatorMandatoryField.getValue())
         {
             cs.nlocs += (nct->newLoc?1:0);
         }
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
//         cs.nlocs += (nct->newGLoc || nct->newNonGLoc)?1:0;
         cs.nqsos++;

         cs.bonus += nct->locBonus;
         cs.bonus += nct->countryBonus;
         cs.bonus += nct->distBonus;
         cs.nbonus += nct->newBonus;

      }
      else
      {
         trace(QString("neg score ") + nct->cs.getFullCall() + " " + nct->serials.getValue());
      }
   }
   cs.nmults = 0;
   QString bt = bonusType.getValue();
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
   if ( bt == "B6" )
   {
       cs.brcc1 = cs.brcc2 = ' ';
   }
   if ( bt == "B6" )
   {
       cs.brcc3 = cs.brcc4 = ' ';
   }
   if ( bt == "B4" || bt == "B6" )
   {
       cs.brloc1 = cs.brloc2 = ' ';
   }
   if (usesBonus.getValue())
   {
      cs.brbonus1 = cs.brbonus2 = ' ';
   }
   cs.nmults = qMax(cs.nmults, 1);

   cs.totalScore = (cs.contestScore + cs.bonus)*cs.nmults;

}
class Period
{
    public:
        bool isGap = false;
        int length = 0;
        int startMinute = -1;

        Period()
        {}
};

void BaseContestLog::getOpTime(QString &otBuff, SHOWOPERATINGTIME sot)
{
    // scan qsoTimeMap

    if (sot == otNone)
    {
        otBuff.clear();
        return;
    }
    QVector<Period> periods;

    int minGap = 60;
    if (sot == otIARU)
    {
        minGap = 120;
    }

    QDateTime  contestStart = CanonicalToTDT(DTGStart.getValue());
    QDateTime  contestEnd = CanonicalToTDT(DTGEnd.getValue());
 //   qint64 fromContestStart = contestStart.secsTo(QDateTime::currentDateTime());

    QDateTime now = QDateTime::currentDateTimeUtc();
    if(now < contestStart || now > contestEnd)
    {
        now = contestEnd;
    }
    const int contestMinutes =  static_cast<int>((contestStart.secsTo(now) + 59)/60);    // round up so we include this minute

    int gap = 0;
    int cur = 0;
    int curStart = -1;
    int gapStart = -1;
    int maxGapLen = 0;
    int maxGapOffset = 0;

    int i = 0;

    while ( i < contestMinutes )
    {

        while ( i < contestMinutes && qsoTimeMap[ i ].count )
        {
            if ( curStart < 0 )
            {
                curStart = i;
            }
            cur++;

            i++;
        }
        while ( i < contestMinutes && !qsoTimeMap[ i ].count )
        {
            if ( gapStart < 0 )
            {
                gapStart = i;
            }
            gap++;
            i++;
        }

        if ( gapStart > 0 && gap > 0 && gap < minGap && i < contestMinutes )
        {
            // include gap into operate time
            gapStart = -1;
            cur += gap;

            gap = 0;
            if ( i < contestMinutes )
                continue;
        }

        if ( cur )
        {
            Period p;
            p.isGap = false;
            p.length = cur;
            p.startMinute = curStart;

            periods.push_back ( p );

            if (cur > maxGapLen)
            {
                maxGapLen = cur;
                maxGapOffset = periods.length() - 1;
            }
        }
        if ( gap )
        {
            Period p;
            p.isGap = true;
            p.length = gap;
            p.startMinute = gapStart;
            periods.push_back ( p );
        }
        gap = 0;
        cur = 0;
        curStart = -1;
        gapStart = -1;
    }

    int operatingTime = 0;
    int lastGapLength = 0;
    for ( QVector<Period>::iterator i = periods.begin(); i != periods.end(); i++ )
    {
        int startMinute = ( *i ).startMinute;
        int length = ( *i ).length;

        if ( ( *i ).isGap && startMinute == 0 )
        {
            continue;   // don't show leading gap
        }
        if ( ( *i ).isGap && i == ( periods.end() - 1 ) )
        {
            lastGapLength = (*i).length;
            if (now != contestEnd)
            {
                lastGapLength -= 1;    // as the current minute shouldn't be included when in contest
            }
            continue;   // don't show trailing gap
        }

        // if IARU we have to look for the largest gap >= 120 mins, and disallow the rest
        if ( ! ( *i ).isGap || (sot == otIARU && (i - periods.begin()) != maxGapOffset))
        {
            operatingTime += length;
        }
    }
    int oh = operatingTime/60;
    int om = operatingTime%60;
    int gh = lastGapLength/60;
    int gm = lastGapLength%60;
    otBuff = QString("op time %1:%2 gap %3:%4").arg(oh).arg(om, 2, 10, QLatin1Char('0')).arg(gh).arg(gm, 2, 10, QLatin1Char('0'));


}

//============================================================
DupContact::DupContact(CheckableContact *c ) : dct( c )
{}
DupContact::DupContact()
{}
DupContact::~DupContact()
{}
uint qHash(const DupContact &dup)
{
    return dup.qHash();
}
//If HF we need to include mode and band - VHF should be single band
bool DupContact::operator<( const DupContact& rhs ) const
{
   Callsign * c1 = &dct->cs;  // search item
   QString b1 = dct->band;
   QString m1 = dct->mode.getValue();

   Callsign *c2 = &rhs.dct->cs;  // collection item
   QString b2 = rhs.dct->band;
   QString m2 = rhs.dct->mode.getValue();

   if (b2.isEmpty())
   {
       b2 = b1;
   }

   if (!c1 || !c2)
   {
       return false;
   }

   if (*c1 == *c2)
   {
       if (b1.isEmpty() || m1.isEmpty())
       {
           return false;
       }
       if (b1 == b2)
       {
           if (dct->contest->isHF())
           {
               return m1 < m2;
           }
           return false;
       }
       return b1 < b2;
   }

   return (*c1 < *c2);
}
bool DupContact::operator==( const DupContact& rhs ) const
{
    Callsign * c1 = &dct->cs;  // search item
    QString b1 = dct->band;

    Callsign *c2 = &rhs.dct->cs;  // collection item
    QString b2 = rhs.dct->band;

    if (b2.isEmpty())
    {
        b2 = b1;
    }
    if (dct->contest->isHF())
    {
        QString m1 = dct->mode.getValue();
        QString m2 = rhs.dct->mode.getValue();
        if (b1.isEmpty() || m1.isEmpty())
        {
            return *c1 == *c2;
        }
        return (c1 && c2 && *c1 == *c2 && b1 == b2 && m1 == m2);
    }
    else
    {
        if (b1.isEmpty())
        {
            return *c1 == *c2;
        }
        return (c1 && c2 && *c1 == *c2 && b1 == b2);
    }
}
bool DupContact::operator!=( const DupContact& rhs ) const
{
   return !( *this == rhs );
}

qHashRet DupContact::qHash() const
{
    // find in set works off hash values, NOT the equality operator
    // and they aren't the same (KST has no band/mode, monitored contact has band/mode)

    // BUT hash just gives a bucket to be scanned, and THEN equality should be used

    return ::qHash(dct->cs.realCall);
}
dupsheet::dupsheet()
{}
dupsheet::~dupsheet()
{
    clear();
}
bool dupsheet::checkCurDup(CheckableContact *nct, unsigned long valpseq, bool insert )
{
    curdup.reset();
    if ( nct->cs.getValRes() == CS_OK )
    {
        QSharedPointer<DupContact> test( new DupContact(nct) );
      DupIterator c = dupList.find(test);
      if ( c!= dupList.end() )
      {
         if ( !( nct->contactFlags.getValue() & VALID_DUPLICATE ) )
         {
            if ( valpseq != 0 && valpseq <= c->wt ->dct->getLogSequence() )
            {
               return false; // as val point earlier than current list item
            }

            if ( c != dupList.end() )
               curdup = c->wt;

            return true;
         }
      }
      else
         if ( insert )
         {
            MapWrapper<DupContact> ins( test);
            dupList.insert( ins, ins );
            return false;
         }
   }
   return false;
}

bool dupsheet::checkCurDup(BaseContestLog *contest, unsigned long nctseq, unsigned long valpseq, bool insert )
{
   curdup.reset();
   QSharedPointer<BaseContact> nct = contest->pcontactAtSeq(nctseq);
   if ( nct && nct->cs.getValRes() == CS_OK )
   {
      QSharedPointer<DupContact> test( new DupContact(nct.data()) );
      DupIterator c = dupList.find(test);
      if ( c != dupList.end() )
      {
         if ( !( nct->contactFlags.getValue() & VALID_DUPLICATE ) )
         {
            if ( valpseq != 0 && valpseq  <= c->wt ->dct->getLogSequence() )
            {
               return false; // as val point earlier than current list item
            }

            if ( c != dupList.end() )
               curdup = c->wt;

            return true;
         }
      }
      else
         if ( insert )
         {
            QSharedPointer<DupContact> ins(new DupContact( nct.data() ));
            dupList.insert( ins, ins );
            return false;
         }
   }
   return false;
}


bool dupsheet::isCurDup(CheckableContact *nct ) const
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
CheckableContact *dupsheet::getCurDup()
{
   if ( curdup )
      return curdup->dct;
   return nullptr;
}
void dupsheet::clear()
{
   curdup.reset();
   dupList.clear();
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
      mt->getStructArgMemberValue( "version", appVersion );
      mt->getStructArgMemberValue( "name", name );
      mt->getStructArgMemberValue( "cabrilloName", cabrilloName);
      mt->getStructArgMemberValue( "calType", calType);
      mt->getStructArgMemberValue( "band", contestBands );
      mt->getStructArgMemberValue( "currentBand", currentBand );
      if (currentBand.getValue().isEmpty())
      {
          currentBand.setValue(contestBands);
      }
      mt->getStructArgMemberValue( "bandsList", bandsList );

      bool isHfContest = isHF();
      mt->getStructArgMemberValue( "hf", hfContest);
      if (isHF() != isHfContest)
      {
          locatorMandatoryField.setValue(!isHF());
      }

      if (currentBand.getValue() == allHF)
      {
          currentBand.setValue("3.5 MHz");
      }

      BandList &blist = BandList::getBandList();
      QSharedPointer<BandInfo>  bi;
      bool bandOK = false;
      bandOK = blist.findBand(currentBand.getValue(), bi);
      if (bandOK && currentBand.getValue() != bi->uk)
      {
          currentBand.setValue(bi->uk);
      }


      bool btemp;
      if ( mt->getStructArgMemberValue( "scoreKms", btemp ) )
         scoreMode.setInitialValue( btemp ? PPKM : PPQSO );

      mt->getStructArgMemberValue( "startTime", DTGStart );
      mt->getStructArgMemberValue( "endTime", DTGEnd );

      mt->getStructArgMemberValue( "districtMult", districtMult );
      mt->getStructArgMemberValue( "districtBonus", districtBonus );
      mt->getStructArgMemberValue( "DXCCMult", countryMult );
      mt->getStructArgMemberValue( "locMult", locMult );
      mt->getStructArgMemberValue( "OtherMultType", otherMult);
      mt->getStructArgMemberValue( "AsymmetricMult", asymmetricMult);

      mt->getStructArgMemberValue( "QTHReq", exchangeRequired );
      mt->getStructArgMemberValue( "QTHOpt", exchangeDashAllowed );
      if (exchangeDashAllowed.getValue())
      {
         exchangeRequired.setValue(true);
      }
      mt->getStructArgMemberValue( "AllowLoc4", allowLoc4 );
      mt->getStructArgMemberValue( "AllowLoc8", allowLoc8 );
      mt->getStructArgMemberValue( "currentMode", currentMode);

      mt->getStructArgMemberValue( "modeList", modeList);
      if (modeList.getValue().isEmpty())
      {
          QString modeString = hamlibData::CW
                       + "|" + (isHF()?PH:hamlibData::USB)
                      + "|" + hamlibData::FM
                      + "|" + MGM
                      + "|" + RY
                      + "|" + PSK
                      ;

          modeList.setValue( modeString);
      }

      mt->getStructArgMemberValue( "RSTField", RSTMandatoryField);
      mt->getStructArgMemberValue( "serialField", serialMandatoryField);
      mt->getStructArgMemberValue( "locatorField", locatorMandatoryField);

      mt->getStructArgMemberValue( "bandPointsMultiplier", bandPointsMultiplier);

      mt->getStructArgMemberValue( "UKACBonus", usesBonus );
      mt->getStructArgMemberValue("BonusType", bonusType);
      if (usesBonus.getValue())
      {
          if (bonusType.getValue().isEmpty())
          {
              bonusType.setValue("B4"); // cope with old Minos files
          }
          loadBonusList();
      }

     if (locMult.getValue())
     {
        loc_multiplier = 1;
     }
     else
     {
        loc_multiplier = 0;
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
            QString temp;
            if ( mt->getStructArgMemberValue( "locator", temp ) )
            {
                myloc.setLoc(temp);
               validateLoc();
            }
            mt->getStructArgMemberValue( "district", location );
            mt->getStructArgMemberValue( "location", location ); // doubled up...
         }
         else
            if ( methodName == "MinosLogEntry" )
            {
                QString temp;
               if (mt->getStructArgMemberValue( "call", temp ))
               {
                   mycall.setFullCall(temp);
               }
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
                                 addToContestList(rct);
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
                                    addToContestList(rct);
                                    // Was just nextBlock++ - no test
                                    if (logSequence >> 16 >= nextBlock)
                                    {
                                       nextBlock = (logSequence >> 16) + 1;
                                    }
                                 }
                                 rct->processMinosStanza( methodName, mt );

                              }
                              else
                               if (methodName == "MinosQSOMapFilter")
                               {

                                   setQSOMapFilterSettingsExist(true);
                                   BandmapClientFilterSettings bcfs;
                                   bool filterFlag = false;
                                   bool filterFlag1 = false;


                                   mt->getStructArgMemberValue("modeFilterCW", filterFlag);
                                   bcfs.setModeFilter("CW", filterFlag);
                                   mt->getStructArgMemberValue("modeFilterLSBMODE", filterFlag);
                                   bcfs.setModeFilter("LSB", filterFlag);
                                   mt->getStructArgMemberValue("modeFilterUSBMODE", filterFlag);
                                   bcfs.setModeFilter("USB", filterFlag);
                                   mt->getStructArgMemberValue("modeFilterFMMODE", filterFlag);
                                   bcfs.setModeFilter("FM", filterFlag);
                                   mt->getStructArgMemberValue("modeFilterRTTYMODE", filterFlag);
                                   bcfs.setModeFilter("RTTY", filterFlag);
                                   mt->getStructArgMemberValue("modeFilterPSK31MODE", filterFlag);
                                   bcfs.setModeFilter("PSK31", filterFlag);
                                   mt->getStructArgMemberValue("modeFilterFT4MODE", filterFlag);
                                   bcfs.setModeFilter("FT4", filterFlag);
                                   mt->getStructArgMemberValue("modeFilterFT8MODE", filterFlag);
                                   bcfs.setModeFilter("FT8", filterFlag);
                                   mt->getStructArgMemberValue("modeFilterMSK144MODE", filterFlag);
                                   bcfs.setModeFilter("MSK144", filterFlag);
                                   mt->getStructArgMemberValue("modeFilterJT65MODE", filterFlag);
                                   bcfs.setModeFilter("JT65", filterFlag);
                                   mt->getStructArgMemberValue("modeFilterNONEMODE", filterFlag);
                                   bcfs.setModeFilter("NONE", filterFlag);
                                   int distance = 0;
                                   mt->getStructArgMemberValue("distanceFilter", distance);
                                   bcfs.setDistanceFilter(distance);
                                   mt->getStructArgMemberValue("ignoreDistanceFlag", filterFlag);
                                   bcfs.setIgnoreDistanceFlag(filterFlag);
                                   mt->getStructArgMemberValue("ignoreEmptyDistanceFlag", filterFlag1);
                                   bcfs.setIgnoreEmptyDistanceFlag(filterFlag1);


                                   saveInitialQSOMapFilter(bcfs);

                               }
}

//====================================================================
void BaseContestLog::setStanza(unsigned int /*stanza*/, int /*stanzaStart*/ , int /*stanzaEnd*/)
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
    for ( auto const &i: QASCONST( ctList) )
   {
      if ( i.wt ->contactFlags.getValue() & TO_BE_ENTERED )
      {
         return i.wt;
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
    if (usesBonus.getValue() && (bonusType.getValue() == "B6" || bonusType.getValue() == "B4"))
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
            postcodeBonuses.clear();
            dxccBonuses.clear();

            if (bonusType.getValue() == "B6")
            {
                MultType B6 = vhf.mults["B6"];
                
                if (B6.locBonuses.size() == 0)
                {
                    B6.locBonuses["DEFAULT"] = 200;
                }
                for (QMap<QString, int>::iterator i = B6.locBonuses.begin(); i != B6.locBonuses.end(); i++)
                {
                    QString name = i.key().toUpper();
                    int value = i.value();

                    locBonuses[name] = value;
                }

                if (B6.postcodeBonuses.size() == 0)
                {
                    B6.postcodeBonuses["DEFAULT"] = 200;
                }
                for (QMap<QString, int>::iterator i = B6.postcodeBonuses.begin(); i != B6.postcodeBonuses.end(); i++)
                {
                    QString name = i.key().toUpper();
                    int value = i.value();

                    postcodeBonuses[name] = value;
                }

                if (B6.dxccBonuses.size() == 0)
                {
                    B6.dxccBonuses["DEFAULT"] = 200;
                }
                for (QMap<QString, int>::iterator i = B6.dxccBonuses.begin(); i != B6.dxccBonuses.end(); i++)
                {
                    QString name = i.key().toUpper();
                    int value = i.value();

                    dxccBonuses[name] = value;
                }
            }
            if (bonusType.getValue() == "B4")
            {
                MultType B4 = vhf.mults["B4"];
                
                if (B4.locBonuses.size() == 0)
                {
                    B4.locBonuses["DEFAULT"] = 500;
                }
                for (QMap<QString, int>::iterator i = B4.locBonuses.begin(); i != B4.locBonuses.end(); i++)
                {
                    QString name = i.key().toUpper();
                    int value = i.value();

                    locBonuses[name] = value;
                }
                postcodeBonuses["DEFAULT"] = 0;
                dxccBonuses["DEFAULT"] = 0;
            }
        }
    }
    else if (usesBonus.getValue() && bonusType.getValue() == "NAC")
    {
        bonusYearLoaded = 0;
        locBonuses["DEFAULT"] = 500;
        postcodeBonuses["DEFAULT"] = 0;
        dxccBonuses["DEFAULT"] = 0;
    }

}
int BaseContestLog::getSquareBonus(QString sloc) const
{
    int bonus = 0;
    QMap<QString, int>::const_iterator l = locBonuses.find(sloc);

    if ( l != locBonuses.end())
    {
       // specific bonus for square allocated
       bonus = l.value();
    }
    else
    {
        QMap<QString, int>::const_iterator l = locBonuses.find("DEFAULT");
        if ( l != locBonuses.end())
        {
           // specific bonus for square allocated
           bonus = l.value();
        }
    }
    return bonus;
}
int BaseContestLog::getCountryBonus(QString c) const
{
    int bonus = 0;
    QMap<QString, int>::const_iterator l = dxccBonuses.find(c);

    if ( l != dxccBonuses.end())
    {
        // specific bonus for square allocated
        bonus = l.value();
    }
    else
    {
        QMap<QString, int>::const_iterator l = dxccBonuses.find("DEFAULT");
        if ( l != dxccBonuses.end())
        {
            // specific bonus for country allocated
            bonus = l.value();
        }
    }
    return bonus;
}
int BaseContestLog::getDistBonus(QString d) const
{
    int bonus = 0;
    QMap<QString, int>::const_iterator l = postcodeBonuses.find(d);

    if ( l != postcodeBonuses.end())
    {
        // specific bonus for district allocated
        bonus = l.value();
    }
    else
    {
        QMap<QString, int>::const_iterator l = postcodeBonuses.find("DEFAULT");
        if ( l != postcodeBonuses.end())
        {
            // specific bonus for square allocated
            bonus = l.value();
        }
    }
    return bonus;
}

int BaseContestLog::getBonus() const
{
    int tot = 0;
    for(auto const &n: bonus)
    {
        tot += n;
    }
    return tot;
}

int BaseContestLog::getNlocs() const
{
    int tot = 0;
    for(auto const &n: nlocs)
    {
        tot += n;
    }
    return tot;
}

int BaseContestLog::getNdistrict() const
{
    int tot = 0;
    for(auto const &n: ndistrict)
    {
        tot += n;
    }
    return tot;
}

int BaseContestLog::getNctry() const
{
    int tot = 0;
    for(auto const &n: nctry)
    {
        tot += n;
    }
    return tot;
}
int BaseContestLog::getDistrictsWorked( const QString &item )
{
    int n = 0;
    for(auto &dc: QASCONST(districtWorked))
    {
        if (dc.contains(item))
        {
            auto nc = dc.value(item);
            n += nc;
        }
    }
    return n;
}
int BaseContestLog::getCountriesWorked( const QString &item )
{
    int n = 0;
    for(auto &dc: QASCONST(countryWorked))
    {
        if (dc.contains(item))
        {
            auto nc = dc.value(item);
            n += nc;
        }
    }
    return n;
}
int BaseContestLog::getDistrictsWorked( const QString &pband, const QString &item )
{
    QString cband = pband;
    if (districtWorked.count() == 1 && districtWorked.contains(""))
    {
        cband = "";
    }
    if (cband.isEmpty() || districtWorked.contains(cband))
    {
        int n = districtWorked[cband][item];
        if (n > 0)
        {
            return n;
        }
    }
   return 0;
}
int BaseContestLog::getCountriesWorked(const QString &pband, const QString &item )
{
   QString cband = pband;
   if (countryWorked.count() == 1 && countryWorked.contains(""))
   {
        cband = "";
   }
    if (cband.isEmpty() || countryWorked.contains(cband))
    {
        return countryWorked[cband][item];
    }
   return 0;
}
QSharedPointer<BandInfo> BaseContestLog::checkBandChange(Frequency targetFreq, Frequency refFreq)
{
    QSharedPointer<BandInfo>  nb;
    if (isHF())
    {
        BandList &bl = BandList::getBandList();
        QSharedPointer<BandInfo>  b1;
        bool b1Ok = bl.findBand(targetFreq, b1);
        QSharedPointer<BandInfo>  b2;
        /*bool b2Ok =*/ bl.findBand(refFreq, b2);

        if (b1Ok && b1 != b2 && b1->enabled && b1->getType() == HF_BANDTYPE)
        {
            return b1;
        }
    }
    return nb;
}
void BaseContestLog::checkSpotWorked(const Callsign &mcs, const QString &locator, const Frequency &freq, bool* callWorked, bool* locatorWorked)
{
    bool callfound = false;
    bool locfound = false;
    if (!isReadOnly())
    {
        for ( LogIterator i = ctList.begin(); i != ctList.end(); i++ )
        {
            if ((*i).wt->notValidContact() )
            {
                continue;
            }

            if (isHF())
            {
                QSharedPointer<BandInfo> bandChanged = checkBandChange(freq, (*i).wt->getFrequency().getValue().str());
                if (bandChanged)
                {
                    // i.e. freq isn't same band as the searched QSO
                    continue;
                }
            }
            if (!callfound)
            {
                if ((*i).wt->cs == mcs)
                {
                    *callWorked = true;
                    callfound = true;
                }
            }

            if (!locfound && !locator.isEmpty())
            {
                QString loc = locator.mid(0,4);
                if ((*i).wt->loc.getLoc().mid(0,4) == loc)
                {
                    *locatorWorked = true;
                    locfound = true;
                }
            }

            if (callfound && locfound)
            {
                return;
            }
        }
    }
}
QString BaseContestLog::getLocForCall(const Callsign &mcs)
{
    for ( LogIterator i = ctList.begin(); i != ctList.end(); i++ )
    {
        if ((*i).wt->notValidContact() )
        {
            continue;
        }

        if ((*i).wt->cs == mcs)
        {
            return (*i).wt->loc.getLoc();
        }
    }
    return QString();
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
   bonusType = ct->bonusType.getValue();
}
QString ContestScore::disp()
{
    QString buff;
    if (usesBonus == true)
    {
        if (bonusType == "B4")
        {
            buff = tr("Score: %1 Qsos; %2 pts; (%3 countries); %4 locs; bonuses %5(%6) = %7")
                       .arg(nqsos).arg(contestScore)
                       .arg(nctry).arg(nlocs)
                       .arg(bonus) .arg(nbonus)
                       .arg(totalScore );
        }
        else if (bonusType == "B6")
        {
            buff = tr("Score: %1 Qsos; %2 pts; %3 countries; %4 districts; %5 locs; bonuses %6(%7) = %8")
                       .arg(nqsos).arg(contestScore)
                       .arg(nctry).arg(ndistrict).arg(nlocs)
                       .arg(bonus) .arg(nbonus)
                       .arg(totalScore );
        }
        else
        {
            buff = tr("Score: %1 Qsos; %2 pts; %3%4 countries%5; bonuses %6(%7) = %8")
                       .arg(nqsos).arg(contestScore).arg(brcc1).arg(nctry).arg(brcc2)
                       .arg(bonus) .arg(nbonus)
                       .arg(totalScore );
        }
    }
    else
    {
         buff = tr( "Score: %1 Qsos; %2 pts;%3%4 countries%5;%6%7 districts%8; %9%10 locators %13 = %14" )
            .arg(nqsos).arg(contestScore).arg(brcc1).arg(nctry).arg(brcc2).arg(brcc3).arg(ndistrict)
            .arg(brcc4).arg(brloc1).arg(nlocs).arg(brloc2)
            .arg(totalScore );
    }
   return buff;
}
//==========================================================================

bool BaseContestLog::getQSOMapFilterSettingsExist() const
{
    return QSOMapFilterSettingsExist;
}

void BaseContestLog::setQSOMapFilterSettingsExist(bool newQSOMapFilterSettingsExist)
{
    QSOMapFilterSettingsExist = newQSOMapFilterSettingsExist;
}
void BaseContestLog::saveQSOMapFilter(const BandmapClientFilterSettings &bcfs)
{
    QSOMapFilterSettings.setValue(bcfs);
    setQSOMapFilterSettingsExist(true);

    trace("BaseContestLog::saveQSOMapFilter: ");
    trace(QSOMapFilterSettings.getValue().print());

    commonSave(false);
}
void BaseContestLog::saveInitialQSOMapFilter(const BandmapClientFilterSettings &bcfs)
{
    QSOMapFilterSettings.setInitialValue(bcfs);
    setQSOMapFilterSettingsExist(true);

    trace("BaseContestLog::saveInitialQSOMapFilter: ");
    trace(QSOMapFilterSettings.getValue().print());
}
BandmapClientFilterSettings BaseContestLog::getQSOMapFilter()
{
    trace("BaseContestLog::getQSOMapFilter: ");
    trace(QSOMapFilterSettings.getValue().print());
    return QSOMapFilterSettings.getValue();
}
//====================================================================
