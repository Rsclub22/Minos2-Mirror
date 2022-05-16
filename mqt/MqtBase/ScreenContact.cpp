/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"
#include "contest.h"
#include "cutils.h"
#include "BandList.h"

#include "ScreenContact.h"

ScreenContact::ScreenContact():CheckableContact()
{}
ScreenContact::~ScreenContact()
{}
void ScreenContact::initialise(BaseContestLog *ct , bool rInit)
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
        mode = clp->currentMode.getValue();
        if (mode == hamlibData::MGM)
        {
            // don't clear submode - keep the old one
        }
        else
        {
            mgmSubmode.clear();
        }
        ms = clp->maxSerial + 1;
    }

    if (mode != hamlibData::MGM)
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
             if (mode == hamlibData::CW)
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
            if (rInit)
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
    extraText = "";
    comments = "";
    contactFlags = 0;
    forcedMult = "";
    frequency.clear();
    rotatorHeading = "";
    rigName = "";
    QSOValid = false;
    newCtry = false;
    newDistrict = false;
    locCount = 0 ;
    newGLoc = false ;
    newNonGLoc = false ;
    op1 = "" ;
    op2 = "" ;

    districtMult = QSharedPointer<DistrictEntry>();
    ctryMult = QSharedPointer<CountryEntry>();

    contactScore = -1;
    bearing = 0;

    multCount = 0;
    bonus = 0;
    newBonus = false;
    cqResponse = false;

}
void ScreenContact::copyFromArg( QSharedPointer<BaseContact> cct )
{
    setLogSequence(cct->getLogSequence());
    loc = cct->loc;
    loc.clearDirty();

    extraText = cct->extraText.getValue();

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
    forcedMult = cct->forcedMult.getValue();
    frequency = cct->frequency.getValue();
    rotatorHeading = cct->rotatorHeading.getValue();
    rigName = cct->rigName.getValue();
    bonus = cct->bonus;
    newBonus = cct->newBonus;

    op1 = cct->op1.getValue();
    op2 = cct->op2.getValue();

    locCount = cct->locCount;
    newGLoc = cct->newGLoc;
    newNonGLoc = cct->newNonGLoc;
    newDistrict = cct->newDistrict;
    newCtry = cct->newCtry;

    comments = cct->comments.getValue();

    contactFlags = cct->contactFlags.getValue();

    contactScore = cct->contactScore.getValue();
    bearing = cct->bearing;
    mode = cct->mode.getValue();
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
    frequency = cct.frequency;
    rotatorHeading = cct.rotatorHeading;
    rigName = cct.rigName;
    bonus = cct.bonus;
    newBonus = cct.newBonus;

    op1 = cct.op1;
    op2 = cct.op2;

    locCount = cct.locCount;
    newGLoc = cct.newGLoc;
    newNonGLoc = cct.newNonGLoc;
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
    checkContact();
}

void ScreenContact::score()
{
    // check should already have run

    QString gridref = loc.getLoc().trimmed();
    if (gridref.isEmpty())
        return;

    double latitude;
    double longitude;
    /*int locValres =*/ lonlat( gridref, longitude, latitude, MinosParameters::getMinosParameters() ->getAllowLoc4() );

    if ( !( contactFlags & MANUAL_SCORE ) || ( contactFlags & DONT_PRINT ) )
    {

        // now we want to look for mults and bonuses

        QString band;
        contest->getTxFreqBand(frequency, band);

        if ( districtMult && districtMult->country1)
        {
           int n = contest->getDistrictsWorked(band, districtMult->districtCode) + 1;
           if ( n < districtMult->country1->districtLimit() )
          if ( contest->districtMult.getValue() )
          {
             multCount++;
          }
          newDistrict = true;
       }

        if ( ctryMult)
        {
            int n = contest->getCountriesWorked(band, ctryMult->getBasePrefix());
            if ( n == 1 )
            {
                if (!contest->nonGCountryMult.getValue() || !cs.isUK())
                {
                   if ( contest->countryMult.getValue() )
                   {
                       multCount++;
                   }
                   newCtry = true;
                }
            }
        }

        if ( !( contactFlags & ( MANUAL_SCORE | NON_SCORING | DONT_PRINT ) ) )
        {
            double dist;
            int brg = 0;
            if (contest->MGMContestRules.getValue())
            {
                 dist = contest->CalcCentres ( loc.getLoc(), brg );
                 if ( almost_equal(dist, 1.0, 2))
                     dist = 50;  // MGM same square == 50 points
            }
            else if ( loc.getLoc().size() == 4 && contest->allowLoc4.getValue() )
            {
               dist = contest->CalcNearest( loc.getLoc() ); // deal with 4 char locs
            }
            else
            {
                contest->disbeara( longitude, latitude, dist, brg );
            }
            contactScore = static_cast<int>(dist);
            bearing = brg;
        }

        if ( !contest->locatorMandatoryField.getValue() || contactScore >= 0 )   		// don't add -1 scores in, but DO add zero km
           // as it is 1 point.
        {
           switch ( contest->scoreMode.getValue() )
           {
              case PPKM:
                 {
                    if ( contactFlags & XBAND )
                    {
                       contactScore = ( contactScore + 1 ) / 2;
                    }
                 }
                 break;

              case PPQSO:
               if ( contactScore > 0 )
                  contactScore = 1;
               else
                  contactScore = 0;
               break;
           }
        }

        {
           // now look at the locator list
           QString letters;
           QString numbers;

           QString sloc = loc.getLoc().mid(0, 4);

           letters = sloc.left(2);
           numbers = sloc.mid(2, 2);

           LocSquare *ls = nullptr;

           for ( auto const &i: qAsConst(contest->locs[band].llist) )
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
                 ls = new LocSquare ( letters );
                 MapWrapper<LocSquare> wls(ls);
                 if (!contest->locs[band].llist.contains(wls))
                     contest->locs[band].llist.insert ( wls, wls );
              }
           }

           int oldMultCount = multCount;
           if ( ls )
           {
              bool UKcall = cs.isUK();
              LocCount * npt = ls->map ( numbers );
              if ( npt )
              {
                 if (contest->usesBonus.getValue())
                 {
                    if (npt->UKLocCount == 0 &&  npt->nonUKLocCount == 0)
                    {
                       bonus += contest->getSquareBonus(sloc);
                       newBonus = true;
                    }
                 }
                 if (UKcall)
                 {
                    if (!npt->UKMultGiven)
                    {
                       newGLoc = true;
                       if (npt->UKLocCount + npt->nonUKLocCount == 0)
                       {
                          // hasn't been worked at all
                          multCount += contest->UKloc_multiplier;
                       }
                       else
                       {
                          // has already been worked - must have been non-uk, so that
                          // bit of the mult has already happened.
                          multCount += contest->UKloc_multiplier - contest->NonUKloc_multiplier;
                       }
                    }
                 }
                 else
                 {
                    if ( npt->UKLocCount + npt->nonUKLocCount == 0 )
                    {
                       multCount += contest->NonUKloc_multiplier;
                    }
                    if (npt->nonUKLocCount == 0)
                    {
                       newNonGLoc = true;
                    }
                 }
              }
           }
           locCount = multCount - oldMultCount;
        }

    }
}
