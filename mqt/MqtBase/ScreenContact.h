/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef ScreenContactH
#define ScreenContactH 
#include "base_pch.h"
#include "contacts.h"
//----------------------------------------------------------------------------
class ScreenContact
{
      unsigned long logSequence; // sparse sequence, used to provide
   protected:
      BaseContestLog *contest;
   public:
      ScreenContact();
      virtual ~ScreenContact();
      virtual void copyFromArg(QSharedPointer<BaseContact> );   // this MIGHT just get used for dummy, for ops
      virtual void copyFromArg( ScreenContact & );    // used for partialSave
      void checkScreenContact( );
      virtual bool isNextContact() const;
      virtual void setLogSequence( unsigned long ul )
      {
         logSequence = ul;
      }
      virtual unsigned long getLogSequence() const
      {
         return logSequence;
      }

      void initialise( BaseContestLog *ct );


      Callsign cs;   //CONTAIN MinosItem
      Locator loc;   //CONTAIN MinosItem
      dtg time;      //CONTAIN MinosItem

      QString mode;
      QString reps;
      QString serials;
      QString repr;
      QString serialr;
      QString extraText;
      QString comments;
      unsigned short contactFlags;
      QString forcedMult;
      QString frequency;
      QString rotatorHeading;
      QString rigName;

      QString op1;
      QString op2;
      //------------------

      bool screenQSOValid;
      bool newCtry;
      bool newDistrict;
      int locCount;  // was newLocs, now is mult from locs
      bool newGLoc;
      bool newNonGLoc;

      QSharedPointer<DistrictEntry> districtMult;
      QSharedPointer<CountryEntry> ctryMult;

      int contactScore;
      int bearing;
      char multCount;
      int bonus;
      bool newBonus;
};

#endif
