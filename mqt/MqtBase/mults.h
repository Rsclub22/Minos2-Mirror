/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef MultsH
#define MultsH 
#include <QString>
#include <QMap>

#include "MapWrapper.h"
#include "locator.h"
#include "callsign.h"
//----------------------------------------------------------------------------
class DistrictEntry;
class DistrictSynonym;
class DistrictList;
class CountrySynonymList;
class CountryEntry;
class CountrySynonym;
class CountryList;
class MultEntry;
class BaseContestLog;

class ContList
{
public:
    ContList(const QString &c, bool a): continent(c), allow(a){}
    ContList(){}
   QString continent;
   bool allow = false;
};

extern QVector<ContList> contlist;

enum eMultGridCols {ectCall, ectWorked, ectLocator, ectBearing, ectName,
                    ectCQZone, ectITUZone, ectOtherCalls,
                    ectMultMaxCol
                   };

#define GLIST_PREFIX_LEN 5

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define qHashRet size_t
#else
#define qHashRet uint
#endif

class GlistEntry
{
   public:
      GlistEntry( const QString &cd, const QString &syn );
      virtual ~GlistEntry();

      bool operator<( const GlistEntry& rhs ) const;
      bool operator==( const GlistEntry& rhs ) const;
      bool operator!=( const GlistEntry& rhs ) const;

      QString synPrefix;
      QString dupPrefix;
      qHashRet qHash() const
      {
          return ::qHash(synPrefix);
      }
};
class MultEntry
{
    Locator central;	// central point to take bearings to
    QString realName;

public:
      MultEntry( const QString &name, const QString &cloc );
      virtual ~MultEntry();

      virtual QString str( bool ) = 0;

      virtual void addSynonyms( QString & );
      Locator getCentral() const;
      QString getRealName() const;
      virtual int getITUZone() const {return 0;}
      virtual int getCQZone() const {return 0;};
};
#define DISTRICT_CODE_LENGTH 2
class DistrictEntry : public MultEntry
{
   public:

      QString districtCode; // RSGB code
      QSharedPointer<CountryEntry> country1; // country containing district
      QSharedPointer<CountryEntry> country2; // country containing district

      DistrictEntry( const QString &cd, const QString &name, const QString &prefix, const QString &prefix2, const QString &cloc );
      DistrictEntry( const QString &cd );
      virtual ~DistrictEntry();
      bool operator<( const DistrictEntry& rhs ) const;
      bool operator==( const DistrictEntry& rhs ) const;
      bool operator!=( const DistrictEntry& rhs ) const;

      virtual QString str( bool );
      virtual void addSynonyms( QString & );
      qHashRet qHash() const
      {
          return ::qHash(districtCode);
      }
};

class DistrictSynonym
{
   public:
      DistrictSynonym( const QString &cd, const QString &syn );
      DistrictSynonym( const QString &syn );
      virtual ~DistrictSynonym();

      QString synonym;
      QSharedPointer<DistrictEntry> district;
      bool operator<( const DistrictSynonym& rhs ) const;
      bool operator==( const DistrictSynonym& rhs ) const;
      bool operator!=( const DistrictSynonym& rhs ) const;
      qHashRet qHash() const
      {
          return ::qHash(synonym);
      }
};

class CountryEntry : public MultEntry
{
      int distLimit;
      QString basePrefix;
      QString continent;

      int ITUZone = 0;
      int CQZone = 0;
   public:

      int districtLimit( );
      bool hasDistricts( );

      CountryEntry(const QString &continent, const QString &prefix, const QString &name, const QString &cloc, int cq, int itu );
      CountryEntry( const QString &prefix );
      virtual ~CountryEntry();
      virtual QString str( bool ) override;
      virtual void addSynonyms( QString & ) override;
      bool operator<( const CountryEntry& rhs ) const;
      bool operator==( const CountryEntry& rhs ) const;
      bool operator!=( const CountryEntry& rhs ) const;
      qHashRet qHash() const
      {
          return ::qHash(basePrefix);
      }
      QString getBasePrefix() const;
      QString getContinent() const;
      int getITUZone() const override;
      int getCQZone() const override;
};

class CountrySynonym
{
    QString synPrefix;
    QSharedPointer<CountryEntry> country;
    Locator central;	// central point to take bearings to

    QString continent;

    int ITUZone = 0;
    int CQZone = 0;
public:

      CountrySynonym(const QString &syn, const QString &prefix, const QString &cq, const QString &itu, const QString &ll, const QString &cont );
      virtual ~CountrySynonym();

      void getDupPrefix( QString & );
      virtual void synCat( QString &add_buff );

      virtual int compare( const CountrySynonym & ) const;

      bool operator<( const CountrySynonym& rhs ) const;
      bool operator==( const CountrySynonym& rhs ) const;
      bool operator!=( const CountrySynonym& rhs ) const;
      qHashRet qHash() const
      {
          return ::qHash(synPrefix);
      }
      QString getSynPrefix() const;

      QSharedPointer<CountryEntry> getCountry() const;

      QString getBasePrefix() const;
      QString getRealName() const;
      QString getContinent() const;
      int getITUZone() const;
      int getCQZone() const;
      Locator getCentral() const;
};

class LocCount
{
public:
    unsigned short UKLocCount;
      unsigned short nonUKLocCount;
      bool UKMultGiven;
      LocCount():UKLocCount(0), nonUKLocCount(0), UKMultGiven(false){}
};
class LocSquare
{
   public:
      LocSquare( const QString &loc );
      LocCount *map( int num ); // give count char for loc num
      LocCount *map(const QString &num ); // give count char for loc num
      void clear();

      QString loc; // two letter main square

      LocCount numbers[ 10 ][ 10 ]; // map of parts worked
      bool operator<( const LocSquare& rhs ) const;
      bool operator==( const LocSquare& rhs ) const;
      bool operator!=( const LocSquare& rhs ) const;

      bool isClear();
};

typedef QMap < MapWrapper<LocSquare>, MapWrapper<LocSquare> > LocSquareList;
class LocList
{
   public:
      LocSquareList llist;
      LocList();
      virtual ~LocList();
      QSharedPointer<LocSquare> itemAt(int offset)
      {
          QSharedPointer<LocSquare> ce = std::next(llist.begin(), offset)->wt;
          return ce;
      }
};

class MultLists
{
   public:
      static MultLists *getMultLists();
      MultLists();
      virtual ~MultLists();

      virtual QSharedPointer<CountrySynonym> searchCountrySynonym( const QString &syn ) = 0;
      virtual QSharedPointer<DistrictEntry> searchDistrict( const QString &syn ) = 0;

      virtual int getCtryListSize() = 0;
      virtual int getDistListSize() = 0;
      virtual QSharedPointer<CountryEntry> getCtryForPrefix( const QString &forcedMult ) = 0;
      virtual QString getCtryListText( const QString & item, int Column, BaseContestLog *const ct, const QString &band ) = 0;
      virtual QString getDistListText( const QString & item, int Column, BaseContestLog *const ct, const QString &band ) = 0;
      virtual bool isUKprefix(const Callsign &cs) = 0;
      virtual int getDistWorked(const QString & item, BaseContestLog *const ct, const QString &band ) = 0;
      virtual int getCountryWorked(const QString & item, BaseContestLog *const ct, const QString &band ) = 0;

      virtual QVector<QSharedPointer<DistrictEntry> > &getDistList() = 0;
      virtual QVector<QSharedPointer<CountryEntry> > &getCountryList() = 0;
};
#endif
