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


const int CONTINENTS = 6;
struct ContList
{
   char continent[ 3 ];
   bool allow;
};

const int UKREGIONS = 10;

enum eMultGridCols {ectCall, ectWorked, ectLocator, ectBearing, ectName, ectOtherCalls,
                    ectMultMaxCol
                   };

#define GLIST_PREFIX_LEN 5
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
      uint qHash()
      {
          return ::qHash(synPrefix);
      }
};
class MultEntry
{
   public:
      int listOffset;
      Locator central;	// central point to take bearings to
      QString realName;

      MultEntry( const QString &name, const QString &cloc );
      virtual ~MultEntry();

      virtual QString str( bool ) = 0;

      virtual void addSynonyms( QString & );
};
#define DISTRICT_CODE_LENGTH 2
class DistrictEntry : public MultEntry
{
   public:

      QString districtCode; // RSGB code
      QSharedPointer<CountryEntry> country1; // country containing district
      QSharedPointer<CountryEntry> country2; // country containing district

      DistrictEntry( const QString &cd, const QString &name, const QString &prefix, const QString &prefix2, const QString &cloc );
      virtual ~DistrictEntry();
      bool operator<( const DistrictEntry& rhs ) const;
      bool operator==( const DistrictEntry& rhs ) const;
      bool operator!=( const DistrictEntry& rhs ) const;

      virtual QString str( bool );
      virtual void addSynonyms( QString & );
      uint qHash()
      {
          return ::qHash(districtCode);
      }
};

class DistrictSynonym
{
   public:
      DistrictSynonym( const QString &cd, const QString &syn );
      virtual ~DistrictSynonym();

      QString synonym;
      QSharedPointer<DistrictEntry> district;
      bool operator<( const DistrictSynonym& rhs ) const;
      bool operator==( const DistrictSynonym& rhs ) const;
      bool operator!=( const DistrictSynonym& rhs ) const;
      uint qHash()
      {
          return ::qHash(synonym);
      }
};

class CountryEntry : public MultEntry
{
      int distLimit;
   public:
      QString basePrefix;
      QString continent;

      int districtLimit( );
      bool hasDistricts( );

      CountryEntry( const QString &continent, const QString &prefix, const QString &name, const QString &cloc );
      virtual ~CountryEntry();
      virtual QString str( bool );
      virtual void addSynonyms( QString & );
      bool operator<( const CountryEntry& rhs ) const;
      bool operator==( const CountryEntry& rhs ) const;
      bool operator!=( const CountryEntry& rhs ) const;
      uint qHash()
      {
          return ::qHash(basePrefix);
      }
};

class CountrySynonym
{
   public:
      QString synPrefix;
      QSharedPointer<CountryEntry> country;

      CountrySynonym( const QString &syn, const QString &prefix );
      virtual ~CountrySynonym();

      void getDupPrefix( QString & );
      virtual void synCat( QString &add_buff );

      virtual int compare( const CountrySynonym & ) const;

      bool operator<( const CountrySynonym& rhs ) const;
      bool operator==( const CountrySynonym& rhs ) const;
      bool operator!=( const CountrySynonym& rhs ) const;
      uint qHash()
      {
          return ::qHash(synPrefix);
      }
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
typedef LocSquareList::iterator LocSquareIterator;
typedef LocSquareList::const_iterator ConstLocSquareIterator;
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

      //      void addCountry( bool addsyn ) = 0;
      virtual QSharedPointer<CountrySynonym> searchCountrySynonym( const QString &syn ) = 0;
      virtual QSharedPointer<DistrictEntry> searchDistrict( const QString &syn ) = 0;

      virtual int getCtryListSize() = 0;
      virtual int getDistListSize() = 0;
      virtual QSharedPointer<CountryEntry> getCtryForPrefix( const QString &forcedMult ) = 0;
      virtual QString getCtryListText( int item, int Column, BaseContestLog *const ct ) = 0;
      virtual QString getDistListText( int item, int Column, BaseContestLog *const ct ) = 0;
      virtual QSharedPointer<CountryEntry> getCtryListAt( int index ) = 0;
      virtual int getCtryListIndexOf( QSharedPointer<CountryEntry> ) = 0;
      virtual int getDistListIndexOf( QSharedPointer<DistrictEntry> ) = 0;
      virtual bool isUKprefix(const Callsign &cs) = 0;
//      virtual DistrictEntry *getDistrictEntry(int item) = 0;
//      virtual CountryEntry *getCountryEntry(int item) = 0;
      virtual int getDistWorked(int item, BaseContestLog *const ct ) = 0;
      virtual int getCountryWorked(int item, BaseContestLog *const ct ) = 0;
};
#endif
