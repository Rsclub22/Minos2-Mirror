/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef ContactsH
#define ContactsH 
//----------------------------------------------------------------------------
#include "mwin.h"
class  BaseContestLog;
class MinosTestImport;

// This header covers the structure of contests, logs, etc

namespace ContactBuffs
{
   extern QString scorebuff;
   extern QString brgbuff;
   extern QString buff2;
   extern QString qthbuff;
   extern QString srbuff;
   extern QString ssbuff;
   extern QString buff;
}

class MatchContact;

class TMatchCollection;
class dupsheet;

template < class itemtype >
class MultList;
class DistrictEntry;
class DistrictSynonym;
class DistrictList;
class CountrySynonymList;
class CountryEntry;
class CountrySynonym;
class CountryList;
class ScreenContact;

QSharedPointer<CountryEntry> findCtryPrefix( const callsign &cs );

// This is a contact, either in the log or the current screen contact
// Various parts may be missing to allow "comment" records in the log.

// NB we still have the FF00 flags available!
// And these are in the GJV file, so don't change the values!

const short LOCAL_COMMENT = 0x0800;		// Local comment only
const short VALID_DISTRICT = 0x0400;		// Treat district as valid when checking
const short COUNTRY_FORCED = 0x0200;		// take the country forced in
const short UNKNOWN_COUNTRY = 0x0100;		// country not in control files
const short NON_SCORING = 0x0080;     // general non scoring
const short COMMENT_ONLY = 0x0040;     // only a (long) comment
const short MANUAL_SCORE = 0x0020;     // scored manually
const short DONT_PRINT = 0x0010;     // Not to be printed (deleted)
const short VALID_DUPLICATE = 0x0008;		// e.g. backpacker, second time
const short TO_BE_ENTERED = 0x0004;		// auto spaced, to be gone back to later
const short XBAND = 0x0002;		// Cross band - half score
const short FORCE_LOG = 0x0001;		// Force logged into log

class BaseContact
{
      QVector < QSharedPointer<BaseContact> > history;
   protected:
      BaseContestLog *contest;
   public:
      BaseContact( const BaseContact & );
      BaseContact(BaseContestLog *contest, dtg time_now );
      BaseContact& operator =( const BaseContact & );
      bool operator<( const BaseContact& rhs ) const;
      virtual ~BaseContact(){}
      virtual void setLogSequence( unsigned long /*ul*/ )
      {}
      virtual unsigned long getLogSequence() const
      {
         return 0;
      }
      virtual int getModificationCount() const
      {
         return 0;
      }
      virtual QVector < QSharedPointer<BaseContact> > &getHistory()
      {
         return history;
      }
      virtual bool GJVload( int /*diskBlock*/ )
      {
          return false;
      }

      virtual bool GJVsave( GJVParams & )
      {
          return false;
      }
      virtual void addReg1TestComment(QStringList & )
      {}
      virtual void getReg1TestText( QString &, bool /*noSerials*/ )
      {}
      virtual void getPrintFileText(QString &, short )
      {}
      virtual QString getADIFLine()
      {
          return QString();
      }

      // These CONTAIN minositems

      dtg updtime;      //CONTAIN MinosItem

      callsign cs;   //CONTAIN MinosItem
      locator loc;   //CONTAIN MinosItem
      dtg time;      //CONTAIN MinosItem

      MinosStringItem<QString> extraText;
      MinosStringItem<QString> mode;
      MinosStringItem<QString> reps;
      MinosStringItem<QString> serials;
      MinosStringItem<QString> repr;
      MinosStringItem<QString> serialr;
      MinosStringItem<QString> comments;
      MinosItem<unsigned short> contactFlags;
      MinosStringItem<QString> forcedMult;
      MinosStringItem<QString> rigName;
      MinosStringItem<QString> frequency;
      MinosStringItem<QString> rotatorHeading;

      MinosStringItem<QString> op1;         // current main op - derived from contacts
      MinosStringItem<QString> op2;         // current second op - derived from contacts

      MinosItem<int> contactScore;

      virtual void clearDirty();
      virtual void setDirty();

      int bearing;

      bool QSOValid;
      bool newDistrict;
      bool newCtry;
      
      int locCount;  // was newLocs, now is mult from locs
      bool newGLoc;
      bool newNonGLoc;

      int bonus;
      bool newBonus;

      QSharedPointer<DistrictEntry> districtMult;
      QSharedPointer<CountryEntry> ctryMult;
      virtual void makestrings( bool serialFields ) const;
      virtual void getText(QString &dest, const BaseContestLog * const curcon ) const;
      char multCount;

      // silly implementations that are needed to keep things isolated
      virtual QString getField( int /*ACol*/, const BaseContestLog * const /*curcon*/ ) const
      {
         return "no field";
      }
      virtual bool ne( const ScreenContact& ) const
      {
         return false;
      }
      virtual bool commonSave(QSharedPointer<BaseContact> )
      {
         return false;
      }
      virtual void processMinosStanza( const QString &/*methodName*/, MinosTestImport * const /*mt*/ )
      {}
      virtual void checkContact( )
      {}
      virtual void copyFromArg(QSharedPointer<BaseContact> )
      {}
      virtual void copyFromArg( ScreenContact & )
      {}

}
;

#endif
