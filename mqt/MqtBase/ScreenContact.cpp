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
#include "ScreenContact.h"
ScreenContact::ScreenContact() :
    logSequence( 0 ),
    time( false )
{}
ScreenContact::~ScreenContact()
{}
void ScreenContact::initialise( BaseContestLog *ct )
{
   contest = ct;
   logSequence = static_cast<unsigned long> (- 1);
   cs = Callsign();
   loc = Locator();
   time = dtg( false );

   BaseContestLog * clp = ct;

   int ms = 0;
   if ( clp )
   {
      mode = clp->currentMode.getValue();
      ms = clp->maxSerial + 1;
   }

   if (mode != hamlibData::MGM)
   {
       reps = "5  ";
       repr = "5  ";
   }
   else
   {
       reps = "   ";
       repr = "   ";
   }

   QString temp = QString("%1").arg(ms, 3, 10, QChar('0'));  //leading zeros
   serials = temp;
   serialr = QString( SERIALLENGTH, ' ' );
   extraText = "";
   comments = "";
   contactFlags = 0;
   forcedMult = "";
   frequency = "";
   rotatorHeading = "";
   rigName = "";
   screenQSOValid = false;
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

}
void ScreenContact::copyFromArg( QSharedPointer<BaseContact> cct )
{
   logSequence = cct->getLogSequence();
   loc = cct->loc;
   loc.validate();
   loc.loc.clearDirty();

   extraText = cct->extraText.getValue();

   cs = cct->cs;
   cs.fullCall.clearDirty();
   cs.validate();

   time = cct->time;
   time.clearDirty();

   reps = cct->reps.getValue();
   serials = cct->serials.getValue();
   repr = cct->repr.getValue();
   serialr = cct->serialr.getValue();

   screenQSOValid = cct->QSOValid;

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
}
void ScreenContact::copyFromArg( ScreenContact &cct )
{
   //   logSequence = cct.logSequence; // don't copy between partial and screen contacts.
   loc = cct.loc;
   loc.validate();
   loc.loc.clearDirty();

   extraText = cct.extraText;

   cs = cct.cs;
   cs.fullCall.clearDirty();
   cs.validate();

   time = cct.time;
   time.clearDirty();

   reps = cct.reps;
   serials = cct.serials;
   repr = cct.repr;
   serialr = cct.serialr;

   screenQSOValid = cct.screenQSOValid;

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
}
void ScreenContact::checkScreenContact( )
{
   // check on country and district. If valid, return true,
   // having mapped any synonyms to their parents and
   // saved the pointers.

   int checkret = 0;
   BaseContestLog * clp = contest;

   if ( contactFlags & ( LOCAL_COMMENT | COMMENT_ONLY ) )
      return ;
   if ( contactFlags & NON_SCORING )
      return ;

   screenQSOValid = false;             // initially, anyway
   // cs HAS been validated in TGJVEditFrame::contactValid
   int csret = cs.validate( );
   if ( csret != CS_OK && csret != ERR_DUPCS)
      checkret = ERR_13;

   // AND it has been dup checked
   if ( !checkret )
   {
      unsigned long valp = clp->validationPoint;
      if ( clp->DupSheet.checkCurDup( this, valp, false ) )
      {
         cs.valRes = ERR_DUPCS;
         checkret = ERR_12;
      }
   }

   // search for prefix in country synonym list. Have to allow for e.g. HB0 as a mult

   if ( contactFlags & COUNTRY_FORCED )
   {
      ctryMult = MultLists::getMultLists() ->getCtryForPrefix( forcedMult );
   }
   else
   {
      ctryMult = findCtryPrefix( cs );
   }


   unsigned short cf = contactFlags;
   cf &= ~UNKNOWN_COUNTRY;
   if ( !checkret && ( clp->countryMult.getValue() || clp->districtMult.getValue() ) && !ctryMult )    // need at least a valid country
   {
      cf |= UNKNOWN_COUNTRY;
   }
   contactFlags = cf;
   if ( clp->districtMult.getValue() && ctryMult )
   {
      // if CC_mult and country "has districts" search for the "extra" in the county synonym list

      // check that the district and country agree

      // if the correct parts don't exist, not a valid contact!
      // NB that the rest of the contact has to be valid as well!

      if ( ctryMult->hasDistricts() )    // continentals dont have counties
      {
         districtMult = MultLists::getMultLists() ->searchDistrict( extraText );
         if ( !districtMult && !( cf & VALID_DISTRICT ) )
         {
            checkret = ERR_8;
         }

         if (
            !checkret &&       						// no errors
            !( cf & VALID_DISTRICT ) &&      // ? district forced OK
            districtMult &&
            ( districtMult->country1 != ctryMult ) &&     // check district in country
            ( districtMult->country2 != ctryMult )
         )
         {
            checkret = ERR_8;
         }
      }
      // so all seems OK, or checkret is set to the first error
   }
   else
      if ( !checkret )
      {
         districtMult.reset();						// just in case we have changed the type...
         if ( clp->otherExchange.getValue() )
         {
            if ( clp->districtMult.getValue() )
            {
               if ( !comments.trimmed().size() )
                  checkret = ERR_21;
            }
            else
               if ( !extraText.trimmed().size() )
                  checkret = ERR_21;
         }
      }

   if ( checkret )
      return ;

   screenQSOValid = true;        // for now

}
bool ScreenContact::isNextContact( ) const
{
   return ( logSequence == static_cast< unsigned long > (- 1L) ) ? true : false;
}

