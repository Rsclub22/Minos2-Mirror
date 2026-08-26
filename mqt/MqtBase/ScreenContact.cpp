/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "QtUtils.h"
#include "contest.h"
#include "cutils.h"
#include "BandList.h"
#include "calcs.h"
#include "MinosParameters.h"
#include "ScreenContact.h"
#include "rigcontrolcommonconstants.h"

ScreenContact::ScreenContact():CheckableContact()
{}
ScreenContact::~ScreenContact()
{}
ScreenContact::ScreenContact(const ScreenContact &ct) : CheckableContact()
{
    // probably never used
    *this = ct;
}
ScreenContact &ScreenContact::operator =(const ScreenContact &)
{
    return *this;
}
void ScreenContact::initialise(BaseContestLog *ct , bool initReport)
{
    contest = ct;
    setLogSequence(static_cast<unsigned long> (- 1));
    cs = Callsign();
    loc = Locator();
    timeOn = dtg( false );
    timeOff = dtg( false );

    if (!ct)
    {
        return;
    }

    BaseContestLog * clp = ct;

    int ms = 0;
    if ( clp )
    {
        mode = clp->currentMode;
        if (mode.getValue() == MGM)
        {
            // don't clear submode - keep the old one
        }
        else
        {
            mgmSubmode.clear();
        }
        ms = clp->maxSerial + 1;
    }

    if (mode.getValue() != MGM)
    {
        QString cb = clp->currentBand.getValue().trimmed();
        BandList &blist = BandList::getBandList();
        QSharedPointer<BandInfo>  bi;
        bool bandOK = blist.findBand(cb, bi);
        bool hf = false;
        if (bandOK)
        {
           hf = bi->getType() == HF_BANDTYPE;
        }
        else
        {
            if (cb == allHF)
            {
                hf = true;
            }
        }
        if (hf)
        {
            QString m = mode.getValue();
             if (m == hamlibData::CW || m == RY || m == PSK)
             {
                 repr = "599" ;
                 reps = "599" ;
             }
             else
             {
                 repr = "59 " ;
                 reps = "59 " ;
             }
        }
        else
        {
            if (initReport)
            {
                repr = "5  " ;
                reps = "5  " ;
            }
            else
            {
                repr = "   " ;
                reps = "   " ;
            }
        }
    }
    else
    {
        repr = "   " ;
        reps = "   " ;
    }


    QString temp = QString("%1").arg(ms, 3, 10, QChar('0'));  //leading zeros
    serials = temp;
    serialr = QString( SERIALLENGTH, ' ' );
    extraText.setValue(QString());
    comments.setValue(QString());
    contactFlags.setValue(0);
    forcedMult.setValue(QString());
    sentExchange.setValue(QString());
    setFrequency(Frequency(), QString());
    markOffset.clear();
    rotatorHeading = "";
    rigName = "";
    QSOValid = false;
    newCtry = false;
    newDistrict = false;
    locMultCount = 0 ;
    newLoc = false ;
    op1 = "" ;
    op2 = "" ;

    districtMult = QSharedPointer<DistrictEntry>();
    ctryMult = QSharedPointer<CountryEntry>();

    contactScore.setValue(-1);
    bearing = 0;

    multCount = 0;
    locBonus = 0;
    distBonus = 0;
    countryBonus = 0;
    newBonus = 0;
    cqResponse = false;

}
void ScreenContact::copyFromArg( QSharedPointer<BaseContact> cct )
{
    setLogSequence(cct->getLogSequence());
    loc = cct->loc;
    loc.clearDirty();

    extraText = cct->extraText;

    cs = cct->cs;
    cs.clearDirty();

    timeOn = cct->timeOff;
    timeOn.clearDirty();
    timeOff = cct->timeOff;
    timeOff.clearDirty();

    reps = cct->reps.getValue();
    serials = cct->serials.getValue();
    repr = cct->repr.getValue();
    serialr = cct->serialr.getValue();

    QSOValid = cct->QSOValid;

    districtMult = cct->districtMult;
    ctryMult = cct->ctryMult;
    multCount = cct->multCount;
    forcedMult = cct->forcedMult;
    sentExchange = cct->sentExchange;
    setFrequency(cct->getFrequency(), cct->band);
    markOffset = cct->markOffset;
    rotatorHeading = cct->rotatorHeading.getValue();
    rigName = cct->rigName.getValue();
    locBonus = cct->locBonus;
    countryBonus = cct->countryBonus;
    distBonus = cct->distBonus;
    newBonus = cct->newBonus;

    op1 = cct->op1.getValue();
    op2 = cct->op2.getValue();

    locMultCount = cct->locMultCount;
    newLoc = cct->newLoc;
    newDistrict = cct->newDistrict;
    newCtry = cct->newCtry;

    comments = cct->comments;

    contactFlags = cct->contactFlags;

    contactScore = cct->contactScore;
    bearing = cct->bearing;
    mode = cct->mode;
    mgmSubmode = cct->mgmSubmode.getValue();
    cqResponse = cct->cqResponse.getValue();
}
void ScreenContact::copyFromArg( ScreenContact &cct )
{
    //   logSequence = cct.logSequence; // don't copy between partial and screen contacts.
    loc = cct.loc;
    loc.clearDirty();

    extraText = cct.extraText;

    cs = cct.cs;
    cs.clearDirty();

    timeOn = cct.timeOn;
    timeOn.clearDirty();
    timeOff = cct.timeOff;
    timeOff.clearDirty();

    reps = cct.reps;
    serials = cct.serials;
    repr = cct.repr;
    serialr = cct.serialr;

    QSOValid = cct.QSOValid;

    districtMult = cct.districtMult;
    ctryMult = cct.ctryMult;
    multCount = cct.multCount;
    forcedMult = cct.forcedMult;
    sentExchange = cct.sentExchange;
    setFrequency(cct.getFrequency(), cct.band);
    markOffset = cct.markOffset;
    rotatorHeading = cct.rotatorHeading;
    rigName = cct.rigName;
    locBonus = cct.locBonus;
    distBonus = cct.distBonus;
    countryBonus = cct.countryBonus;
    newBonus = cct.newBonus;

    op1 = cct.op1;
    op2 = cct.op2;

    locMultCount = cct.locMultCount;
    newLoc = cct.newLoc;
    newDistrict = cct.newDistrict;
    newCtry = cct.newCtry;

    comments = cct.comments;

    contactFlags = cct.contactFlags;

    contactScore = cct.contactScore;
    bearing = cct.bearing;
    mode = cct.mode;
    mgmSubmode = cct.mgmSubmode;
    cqResponse = cct.cqResponse;
}
void ScreenContact::checkScreenContact( )
{
    checkContact(false);

    multCount = 0;
    newDistrict = false;
    newCtry = false;
    locMultCount = 0;
    newLoc = false;
    locBonus = 0;
    distBonus = 0;
    countryBonus = 0;
    newBonus = 0;

    score();

}

void ScreenContact::score()
{
    // This only affects the screen contact, not the contest

    // checkContact should already have run

    double latitude = 0.0;
    double longitude = 0.0;

    QString gridref = loc.getLoc();
    if (gridref.length() >= 4)
    {
        lonlat( gridref, longitude, latitude, MinosParameters::getMinosParameters() ->getAllowLoc4() );
    }
    if ( !( contactFlags.getValue() & MANUAL_SCORE ) || ( contactFlags.getValue() & DONT_PRINT ) )
    {

        // now we want to look for mults and bonuses

        if ( districtMult && (districtMult->country1 || cs.getFullCall().isEmpty()))
        {
           int n = contest->getDistrictsWorked(band, districtMult->districtCode);
           if ( n < districtMult->country1->districtLimit() )
           {
                if ( contest->districtMult.getValue() )
                {
                    multCount++;
                }
                newDistrict = true;
                if (newDistrict && contest->usesBonus.getValue())
                {
                    int db = contest->getDistBonus(districtMult->districtCode);
                    if (db)
                    {
                        distBonus += db;
                        newBonus++;
                    }
                }

           }
       }

        if ( ctryMult)
        {
            int n = contest->getCountriesWorked(band, ctryMult->getBasePrefix());
            if ( n == 0 )
            {
               if ( contest->countryMult.getValue() )
               {
                   multCount++;
               }
               newCtry = true;
               if (newCtry && contest->usesBonus.getValue())
               {
                   int cb = contest->getCountryBonus(ctryMult->getBasePrefix());
                   if (cb)
                   {
                       countryBonus += cb;
                       newBonus++;
                   }
               }
            }
        }

        if ( !notValidContact() )
        {
            double dist = 0.0;
            int brg = 0;
            if (contest->MGMContestRules.getValue())
            {
                 dist = contest->CalcCentres ( gridref, brg );
                 if ( almost_equal(dist, 1.0, 2))
                     dist = 50;  // MGM same square == 50 points
            }
            else if ( gridref.size() == 4 && contest->allowLoc4.getValue() )
            {
               dist = contest->CalcNearest( loc.getLoc() ); // deal with 4 char locs
            }
            else if (gridref.size() >= 4)
            {
                contest->disbeara( longitude, latitude, dist, brg );
            }
            contactScore.setValue(static_cast<int>(dist));
            bearing = brg;
        }

        if ( !contest->locatorMandatoryField.getValue() || contactScore.getValue() >= 0 )   		// don't add -1 scores in, but DO add zero km
           // as it is 1 point.
        {
           switch ( contest->scoreMode.getValue() )
           {
              case PPKM:
                 {
                    if ( contactFlags.getValue() & XBAND )
                    {
                       contactScore .setValue( ( contactScore.getValue() + 1 ) / 2);
                    }
                 }
                 break;

              case PPQSO:
               if ( contactScore.getValue() > 0 )
                  contactScore.setValue(1);
               else
                   contactScore.setValue(0);
               break;
           }
        }

        if (gridref.length() >= 4)
        {
           // now look at the locator list
           QString letters;
           QString numbers;

           QString sloc = loc.getLoc().mid(0, 4);

           letters = sloc.left(2);
           numbers = sloc.mid(2, 2);

           LocSquare *ls = nullptr;

           for ( auto const &i: QASCONST(contest->locs[band].llist) )
           {
               LocSquare *locsq = i.wt.data();
               if ( strnicmp ( locsq ->loc, letters, 2 ) == 0 )
               {
                   ls = locsq;
                   break;
               }

           }

           if ( !ls )
           {
              if ( letters[ 0 ].isLetter() && letters[ 1 ].isLetter() )
              {
                 LocSquare lst ( letters );
                 MapKeyWrapper<LocSquare> wlsk(&lst);
                 if (!contest->locs[band].llist.contains(wlsk))
                 {
                     LocSquare *lsi = new LocSquare ( letters );
                     MapKeyWrapper<LocSquare> wlsk(lsi);
                     MapWrapper<LocSquare> wls(ls);
                     contest->locs[band].llist.insert ( wlsk, wls );
                 }
              }
           }

           int oldMultCount = multCount;
           if ( ls )
           {
              LocCount * npt = ls->map ( numbers );
              if ( npt && npt->locCount == 0)
              {
                 if (contest->usesBonus.getValue())
                 {
                    int lb = contest->getSquareBonus(sloc);
                    if (lb)
                    {
                        locBonus += lb;
                        newBonus++;
                    }
                 }

                 multCount += contest->loc_multiplier;  // will be 0 if no loc mults
                 newLoc = true;
              }
           }
           locMultCount = multCount - oldMultCount;
        }

    }
}
