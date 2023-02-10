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
#include "callsign.h"
#include "dtg.h"
#include "frequency.h"
#include "gjvparams.h"
#include "mults.h"
#include <QString>
class MinosTestImport;
class ScreenContact;

// This header covers the structure of contests, logs, etc

class ContactBuffs
{
public:
   QString scorebuff;
   QString brgbuff;
   QString buff2;
   QString qthbuff;
   QString srbuff;
   QString ssbuff;
   QString buff;

   void clear()
   {
       scorebuff.clear();
       brgbuff.clear();
       buff2.clear();
       qthbuff.clear();
       srbuff.clear();
       ssbuff.clear();
       buff.clear();
   }
};

extern ContactBuffs contactBuffs;

QSharedPointer<CountryEntry> findCtryPrefix( const Callsign &cs );

// This is a contact, either in the log or the current screen contact
// Various parts may be missing to allow "comment" records in the log.

// NB we still have the FF00 flags available!
// And these are in the GJV file, so don't change the values!

//const short LOCAL_COMMENT = 0x0800;		// Local comment only
const short VALID_DISTRICT = 0x0400;		// Treat district as valid when checking
const short COUNTRY_FORCED = 0x0200;		// take the country forced in
const short UNKNOWN_COUNTRY = 0x0100;		// country not in control files
const short NON_SCORING = 0x0080;     // general non scoring
//const short COMMENT_ONLY = 0x0040;     // only a (long) comment
const short MANUAL_SCORE = 0x0020;     // scored manually
const short DONT_PRINT = 0x0010;     // Not to be printed (deleted)
const short VALID_DUPLICATE = 0x0008;		// e.g. backpacker, second time
const short TO_BE_ENTERED = 0x0004;		// auto spaced, to be gone back to later
const short XBAND = 0x0002;		// Cross band - half score
const short FORCE_LOG = 0x0001;		// Force logged into log


class CheckableContact: public QObject
{
    Q_OBJECT
    unsigned long logSequence = 0L; // sparse sequence, used to provide

public:
    BaseContestLog *contest;
    Callsign cs;   //CONTAIN MinosItem
    Locator loc;   //CONTAIN MinosItem

    double lon = 0.0;
    double lat = 0.0;

    dtg timeOn = dtg(false);    //CONTAIN MinosItem
    dtg timeOff = dtg(false);   //CONTAIN MinosItem

    MinosItem<int> contactScore;

    MinosStringItem<QString> extraText;
    MinosStringItem<QString> comments;
    MinosItem<unsigned short> contactFlags;
    MinosStringItem<QString> forcedMult;

    QSharedPointer<DistrictEntry> districtMult;
    QSharedPointer<CountryEntry> ctryMult;

    MinosFrequencyItem<Frequency> frequency;
    Frequency markOffset;
    MinosStringItem<QString> mode;

    int bearing = -1;
    bool QSOValid =false;

    // these are used in the derived classes
    bool newCtry = false;
    bool newDistrict = false;
    int locCount = 0;  // was newLocs, now is mult from locs
    bool newGLoc = false;
    bool newNonGLoc = false;

    char multCount = 0;
    int bonus = 0;
    bool newBonus = 0;


protected:
    CheckableContact();
    CheckableContact( const CheckableContact & ct);
    CheckableContact(BaseContestLog * contest, dtg time_now );
    CheckableContact& operator =(const CheckableContact &);;
    virtual ~CheckableContact(){};

public:
    void setLogSequence( unsigned long ul )
    {
       logSequence = ul;
    }
    unsigned long getLogSequence() const
    {
       return logSequence;
    }
    bool isNextContact() const
    {
        return ( getLogSequence() == static_cast< unsigned long > (- 1L) ) ? true : false;
    }
    virtual void getText(QString &/*dest*/, const BaseContestLog * const /*curcon*/ , bool /*forHistory*/) const
    {}

    virtual QString getField( int /*ACol*/, const BaseContestLog * const /*curcon*/ ) const
    {
        return QString();
    }
    virtual int getContactScore() const
    {
        return contactScore.getValue();
    }

    virtual int getModificationCount() const
    {
       return 0;
    }
    virtual int checkContact(bool adddup );
    bool notValidContact();
    void calcDisBear();
    int checkDistrict(int checkret);
};

class BaseContact: public CheckableContact
{
    // we don't make null functions pure virtual as there are places we want to create BaseContact objects
    Q_OBJECT
      QVector < QSharedPointer<BaseContact> > history;
   protected:
   public:
      BaseContact( const BaseContact & );
      BaseContact(BaseContestLog *contest, dtg time_now );
      BaseContact& operator =( const BaseContact & );
      bool operator==(const BaseContact& rhs) const;
      bool operator<( const BaseContact& rhs ) const;
      bool operator!=(const BaseContact& rhs) const;
      virtual ~BaseContact() override {}
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
      virtual void getCabrilloText( QString & )
      {}
      virtual void getPrintFileText(QString &, short )
      {}
      virtual QString getADIFLine()
      {
          return QString();
      }

      // These CONTAIN minositems

      dtg updtime;      //CONTAIN MinosItem

      MinosStringItem<QString> mgmSubmode;
      MinosStringItem<QString> reps;
      MinosStringItem<QString> serials;
      MinosStringItem<QString> repr;
      MinosStringItem<QString> serialr;
      MinosStringItem<QString> rigName;
      MinosStringItem<QString> rotatorHeading;

      MinosStringItem<QString> op1;         // current main op - derived from contacts
      MinosStringItem<QString> op2;         // current second op - derived from contacts

      MinosItem<bool> cqResponse;           // QSO was in response to a CQ call (or at least, on CQ frequency)

      virtual void clearDirty();
      virtual void setDirty();

      virtual void makestrings( bool serialFields ) const;
      virtual void getText(QString &dest, const BaseContestLog * const curcon , bool forHistory) const override;

      // silly implementations that are needed to keep things isolated
      virtual QString getField( int /*ACol*/, const BaseContestLog * const /*curcon*/ ) const override
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
      virtual void copyFromArg(QSharedPointer<BaseContact> )
      {}
      virtual void copyFromArg( ScreenContact & )
      {}

};

#endif
