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
#include "contacts.h"
//----------------------------------------------------------------------------
class ScreenContact: public CheckableContact
{
   protected:
   public:
      ScreenContact();
      virtual ~ScreenContact();
      ScreenContact(const ScreenContact &ct);   // probably never used
      virtual void copyFromArg(QSharedPointer<BaseContact> );   // this MIGHT just get used for dummy, for ops
      virtual void copyFromArg( ScreenContact & );    // used for partialSave
      void checkScreenContact( );

      void initialise(BaseContestLog *ct, bool initReport );

      void score();

      QString mgmSubmode;
      QString reps;
      QString serials;
      QString repr;
      QString serialr;
      QString rotatorHeading;
      QString rigName;

      QString op1;
      QString op2;

      bool cqResponse;
};

#endif
