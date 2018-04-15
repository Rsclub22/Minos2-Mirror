/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef ContestH
#define ContestH 
//----------------------------------------------------------------------------
// This header covers the structure of contests, logs, etc

//class DisplayContestContact;
class BaseContact;
class ContestContact;
class ScreenContact;
class DisplayContestContact;
class ContestScore;
class MinosTestImport;

class DupContact
{
   public:
      QSharedPointer<BaseContact> dct;
      ScreenContact *sct;
      bool operator<( const DupContact& rhs ) const;
      bool operator==( const DupContact& rhs ) const;
      bool operator!=( const DupContact& rhs ) const;

      DupContact( QSharedPointer<BaseContact> c );
      DupContact( ScreenContact *c );
      DupContact();
      ~DupContact();
};
typedef QMap < MapWrapper<DupContact>, MapWrapper<DupContact> > DupList;
typedef DupList::iterator DupIterator;
class dupsheet
{
      // a dupsheet is a sorted collection of (full LoggerContestLog) ContestContact records,
      // sorted by the callsign and logSequence number

   private:
      DupList ctList;
      QSharedPointer<DupContact> curdup; // points into dupsheet

   public:
      bool checkCurDup(BaseContestLog *contest, unsigned long nctseq, unsigned long valpseq, bool insert );
      bool checkCurDup(ScreenContact *nct, unsigned long valpseq, bool insert );
      bool isCurDup(QSharedPointer<BaseContact> nct ) const;
      bool isCurDup(ScreenContact *nct ) const;
      void clearCurDup();
      void clear();
      QSharedPointer<BaseContact> getCurDup();
      dupsheet();
      ~dupsheet();
};

enum SCOREMODE {PPKM, PPQSO};

typedef QMap < MapWrapper<BaseContact>, MapWrapper<BaseContact> > LogList;
typedef LogList::iterator LogIterator;

typedef QMap < QString, QString > OperatorList;
typedef OperatorList::iterator OperatorIterator;

class BaseContestLog: public BaseLogList
{
      friend class MinosTestImport;
      friend class MonitoredLog;
      // This is the basis behind all variants - currently we have Logger and Monitor
      // which hold slightly different info, and more importantly handle backing store
      // totally differently


   protected:
      int ct_stanzaCount;
      unsigned long nextBlock;
      bool suppressProtected;
      bool unwriteable;

      virtual bool minosSaveFile( bool /*newfile*/ )
      {
         return false;
      }

   public:
      int cslotno;
      int unfilledCount;

      BaseContestLog(const BaseContestLog &);   // I hope a copy constructor
      BaseContestLog();
      virtual ~BaseContestLog();

      int getCtStanzaCount()
      {
         return ct_stanzaCount;
      }

      // The contest details

      // "Real" basic contest data that needs monitoring
      // and provide the "front sheet" data

      MinosItem<bool> protectedContest;

      MinosStringItem<QString> name;         // name of contest
      MinosStringItem<QString> location;

      Callsign mycall;  // CONTAINS MinosItem
      Locator myloc;  // CONTAINS MinosItem

      MinosItem<bool> allowLoc4;
      MinosItem<bool> allowLoc8;

      MinosItem<bool> RSTField;
      MinosItem<bool> serialField;
      MinosItem<bool> locatorField;

      MinosStringItem<QString> power;
      MinosStringItem<QString> currentMode;

      MinosStringItem<QString> band;
      MinosItem<bool> otherExchange;
      MinosItem<bool> countryMult;
      MinosItem<bool> nonGCountryMult;
      MinosItem<bool> locMult;
      MinosItem<bool> GLocMult;
      MinosItem<bool> districtMult;

      MinosItem<bool> M7Mults;

      MinosItem<bool> usesBonus;
      MinosStringItem<QString> bonusType;

      MinosItem<bool> MGMContestRules;

      MinosItem<bool> powerWatts;
      MinosItem<int> scoreMode;
      MinosStringItem<QString> DTGStart;
      MinosStringItem<QString> DTGEnd;

	  MinosItem<int> bearingOffset;

      // dirty info is only relevant when it is being editted
      // but needs to stay with the data

      virtual void clearDirty();
      virtual void setDirty();
      bool isReadOnly( void )
      {
         return (protectedContest.getValue() && ! suppressProtected) || unwriteable;
      }
      bool isProtected( void )
      {
         return protectedContest.getValue();
      }
      void setProtected( bool s  )
      {
         if (protectedContest.getValue() && !s)
         {
            suppressProtected = true;
         }
         else if (s)
         {
            suppressProtected = false;
            protectedContest.setValue( true );
         }
      }
      bool isProtectedSuppressed( void )
      {
         return suppressProtected;
      }
      void setProtectedSuppressed( bool s)
      {
         suppressProtected = s;
      }
      void setUnwriteable( bool s )
      {
         unwriteable = s ;
      }
      bool isUnwriteable( )
      {
         return unwriteable;
      }
      unsigned long getNextBlock()
      {
          return nextBlock;
      }
      // end of contest details

      // The log itself

      LogList ctList;

      // Ancilliary variables

      MinosStringItem<QString> currentOp1;         // current main op - derived from contacts
      MinosStringItem<QString> currentOp2;         // current second op - derived from contacts

      OperatorList oplist;

      QString opsQSO1;
      QString opsQSO2;

      int maxSerial;
      long contestScore;

      bool locValid;
      double ode;
      double odn;
      double cosodn ;            /* cos of odn */
      double sinodn ;            /* sin of odn */

      QString cfileName;
      QString publishedName;

      // duplicate sheet

      unsigned long validationPoint;   // key of contact from log list to be treated
      dupsheet DupSheet;
      int nextScan;

      bool NonUKloc_mult;
      int NonUKloc_multiplier;
      bool UKloc_mult;
      int UKloc_multiplier;
      
      int nctry;
      int ndistrict;
      int nlocs;

      int nbonus;
      int bonus;

      int multsAsBonuses;
      int bonusYearLoaded;
      QString bonusTypeLoaded;
      QMap<QString, int> locBonuses;
      void loadBonusList();
      int getSquareBonus(QString sloc);

      int *districtWorked;
      int *countryWorked;
      LocList locs;

      int getDistrictsWorked( int item )
      {
         return districtWorked[ item ];
      }
      int getCountriesWorked( int item )
      {
         return countryWorked[ item ];
      }
      int getValidQSOs();

      // stats data

      int QSO1, QSO2, QSO1p, QSO2p;
      long kms1, kms2, kms1p, kms2p;
      int mults1, mults2, mults1p, mults2p;
      int bonus1, bonus2, bonus1p, bonus2p;
      bool updateStat(QSharedPointer<BaseContact> cct , int sp1, int sp2);
      void updateStats(int p1, int p2);
      char lasttchar;

      // methods

      // common file stuff
      virtual bool commonSave( bool /*newfile*/ )
      {
         return false;
      }
      virtual bool minosSaveContestContact( const QSharedPointer<BaseContact> /*lct*/ )
      {
         return false;
      }
      virtual void closeFile( void )
      {}
      virtual bool GJVload( void )
      {
         return false;
      }

      // Log monitoring

      virtual void processMinosStanza( const QString &methodName, MinosTestImport * const mt );
      virtual bool getStanza( unsigned int stanza, QString &stanzaData );
      virtual void setStanza( unsigned int stanza, int stanzaStart );

      // general

      void validateLoc( void );
      void getMatchText(QSharedPointer<BaseContact>, QString &, const BaseContestLog *const ct ) const;
      void getMatchField( QSharedPointer<BaseContact> pct, int col, QString &disp, const BaseContestLog *const ct ) const;
      void scanContest( void );
      void setScore( QString & );
      bool isCurDup(QSharedPointer<BaseContact>) const;

      virtual void getScoresTo(ContestScore &cs, QDateTime limit);

      // manipulation of contact list

      int getContactCount( void );
      int indexOf( QSharedPointer<BaseContact> item );
      QSharedPointer<BaseContact> pcontactAtSeq( unsigned long logSequence );
      QSharedPointer<BaseContact> pcontactAt(int offset );

      //      virtual void makeContact( bool time_now, DisplayContestContact *&){}
      virtual void makeContact(bool time_now, QSharedPointer<BaseContact> & );
      QSharedPointer<BaseContact> findNextUnfilledContact();

      // calcs

      void disbear( double lon, double lat, double &dist, int &brg ) const;
      int CalcNearest( const QString &scalcloc );
      int CalcCentres( const QString &scalcloc );
      bool getsdist(const QString &loc, QString &minloc, double &mindist );
      QSharedPointer<BaseContact> getBestDX( void );
      QString dateRange( DTG dstyle );
      bool checkTime(const dtg &t) const;
      bool checkTime(const QDateTime &t) const;

};
class ContestScore
{
   public:
      char brcc1;
      char brcc2;
      char brcc3;
      char brcc4;
      char brloc1;
      char brloc2;
      char brbonus1;
      char brbonus2;

      QString name;
      bool usesBonus;

      int nqsos;
      int contestScore;
      int nctry;
      int ndistrict;
      int nlocs;
      int nGlocs;
      int nonGlocs;
      int nmults;
      int bonus;
      int nbonus;
      int totalScore;

      ContestScore(BaseContestLog *);
      QString disp();
};
Q_DECLARE_METATYPE(BaseContestLog)
#endif
