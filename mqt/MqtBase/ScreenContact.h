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
      unsigned long logSequence = 0L; // sparse sequence, used to provide
   protected:
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

      void score();

      BaseContestLog *contest =nullptr;

      Callsign cs;   //CONTAIN MinosItem
      Locator loc;   //CONTAIN MinosItem
      dtg time;      //CONTAIN MinosItem

      QString mode;
      QString mgmSubmode;
      QString reps;
      QString serials;
      QString repr;
      QString serialr;
      QString extraText;
      QString comments;
      unsigned short contactFlags = 0;
      QString forcedMult;
      Frequency frequency;
      QString rotatorHeading;
      QString rigName;

      QString op1;
      QString op2;
      //------------------

      bool screenQSOValid =false;
      bool newCtry = false;
      bool newDistrict = false;
      int locCount = 0;  // was newLocs, now is mult from locs
      bool newGLoc = false;
      bool newNonGLoc = false;

      QSharedPointer<DistrictEntry> districtMult;
      QSharedPointer<CountryEntry> ctryMult;

      int contactScore = 0;
      int bearing = 0;
      char multCount = 0;
      int bonus = 0;
      bool newBonus = 0;
};

#endif
