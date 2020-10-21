//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef MultsImplH
#define MultsImplH 
#include "base_pch.h"
#include "contest.h"
#include "cutils.h"

template < class itemtype >
class MultList : public QObject, public QMap < MapWrapper<itemtype>, MapWrapper<itemtype> >
{

public:
      virtual int getWorked( int /*item*/, BaseContestLog * const /*ct*/ )
      {
         return 0;
      }

      QString multfilename;
      virtual bool procLine( QStringList ) = 0;
      MultList()
      {}
      virtual ~MultList()
      {
      }
      void loadEntries( const QString &sfname, const QString &fmess )
      {
         QString fname = sfname;
         if ( multfilename.length() )
            fname = multfilename;

         QFile lf(fname);

         if (!lf.open(QIODevice::ReadOnly|QIODevice::Text))
         {
             QString ebuff = QString( "Failed to open %1 (%2)" ).arg(fmess).arg(fname );
             MinosParameters::getMinosParameters() ->mshowMessage( ebuff );
             return;
         }
         QTextStream istr(&lf);
         while (!istr.atEnd())
         {
         // loop through file, parsing each line into a_exp entries
         // for each line, call proc_line
         // ignore comment lines. (any non alpha/num char)

             QString buff = istr.readLine(255);
            QStringList a;
            if (buff.isEmpty() || (buff[ 0 ] != '/' && !buff[0].isLetterOrNumber() ) )     // allow '/' for suffixes
               continue;   // skip comment lines
            bool sep2seen;
            parseLine( buff, ',', a, 255, 0, sep2seen );

            if ( !procLine( a ) )
               break;

         }
      }
      virtual int slen( bool )
      {
         return -1;
      }

      int indexOf( itemtype item )
      {
          int i = 0;
          for (typename MultList::iterator m = this->begin(); m != this->end(); m++)
          {
            if (*m->wt.data() == item)
                return i;

            i++;
          }
          return -1;
      }

      QSharedPointer<itemtype> itemAt(int offset)
      {
          QSharedPointer<itemtype> ce = std::next(this->begin(), offset)->wt;
          return ce;
      }

      QString getText( int item, int Column, BaseContestLog *const ct )
      {
         QString dest;
         if ( !ct || item >= MultList::size())
            return dest;
         QSharedPointer<itemtype> ce = std::next(this->begin(), item)->wt;
         switch ( Column )
         {
            case ectCall:
               {
                  dest = ce->str( true );
                  break;
               }

            case ectWorked:
               {
                  dest = QString::number(getWorked( item, ct ));
                  break;
               }

            case ectLocator:
               {
                  dest = ce->central.getLoc().left( 6 );
                  break;
               }

            case ectBearing:
               {
                  double longitude = 0.0;
                  double latitude = 0.0;
                  double useScore = 0.0;
                  bool brgscoreValid = true;
                  int useBearing = 0;

                  QString brgbuff;
                  int valRes = lonlat( ce->central.getLoc(), longitude, latitude, MinosParameters::getMinosParameters() ->getAllowLoc4() );
                  if ( ( valRes == LOC_OK ) && ct ->locValid )
                     ct ->disbeara( longitude, latitude, useScore, useBearing );
                  else
                     brgscoreValid = false;
                  if ( brgscoreValid && ( valRes == LOC_OK ) )
                  {
                        const QChar degreeChar(0260); // octal value
                        brgbuff = QString( "%1%2").arg(varBrg( useBearing + ct->bearingOffset.getValue()), 3 ).arg(degreeChar);
                  }

                  dest = brgbuff;
                  break;
               }


            case ectName:
               {
                  // want to list synonyms after the real name, for both countries and districts
                  dest = ce->realName;
                  break;
               }

            case ectOtherCalls:
               {
                  ce->addSynonyms( dest );
                  break;
               }
         }
         return dest;
      }

      QString getText( int item, BaseContestLog *const ct )
      {
         // Only called for fullMultDisp
         QString dest;
         if ( item >= MultList::size())
            return dest;
         itemtype ce = MultList::at( item );

         double longitude = 0.0;
         double latitude = 0.0;
         double useScore = 0.0;
         bool brgscoreValid = true;
         int useBearing = 0;

         QString brgbuff;
         QString buff;

         int valRes = lonlat( ce->central.loc.getValue(), longitude, latitude );
         if ( ( valRes == LOC_OK ) && ct ->locValid )
            ct ->disbeara( longitude, latitude, useScore, useBearing );
         else
            brgscoreValid = false;

         if ( brgscoreValid && ( valRes == LOC_OK ) )
         {
             const QChar degreeChar(0260); // octal value
             brgbuff = QString("%1%2").arg( varBrg( useBearing), 3 ).arg(degreeChar) ;
         }

         int next = 0;
         next = placestr( buff, ce->str( true ), next, slen( true ) );

         QString wcount;
         wcount = QString("(%1)").arg( getWorked( item, ct ) );
         next = placestr( buff, wcount, next + 1, 5 );

         next = placestr( buff, ce->central.loc.getValue(), next + 1, 6 );
         next = placestr( buff, brgbuff, next + 1, 4 );

         // want to list synonyms after the real name, for both countries and districts
         next = placestr( buff, ce->realName, next + 1, ce->realName.length() );

         QString namebuff;
         ce->addSynonyms( namebuff );

         placestr( buff, namebuff, next + 1, 254 - next );

         dest = buff;
         return dest;
      }


};
class GlistList : public MultList < GlistEntry >
{
    Q_OBJECT

      // list of DistrictEntry
   public:
      GlistList( );
      virtual ~GlistList();
      void load( );
      virtual bool procLine(QStringList );

};
class DistrictList : public MultList < DistrictEntry >
{
      // list of DistrictEntry
   public:
      DistrictList( );
      virtual ~DistrictList();
      void load( );
      virtual bool procLine( QStringList );
      virtual int slen( bool );
      virtual int getWorked( int item, BaseContestLog *const ct );
};

class DistrictSynonymList : public MultList < DistrictSynonym >
{
      // list of DistrictSynonym
   public:
      DistrictSynonymList( );
      virtual ~DistrictSynonymList();
      void load( );
      virtual bool procLine(QStringList );
};


class CountryList : public MultList < CountryEntry >
{
    Q_OBJECT

      // list of CountryEntry
   public:
      CountryList( );
      virtual ~CountryList();
      void load( );
      virtual bool procLine( QStringList );
      virtual int slen( bool );
      void loadEntries( const QString &fname, const QString &fmess );
      virtual int getWorked( int item, BaseContestLog *const ct );
};

class CountrySynonymList : public MultList < CountrySynonym >
{
    Q_OBJECT

      // list of CountrySynonym
   public:
      CountrySynonymList( );
      virtual ~CountrySynonymList();
      void load( );
      virtual bool procLine(QStringList );
      static void makeCountrySynonym(const QString &ssyn, const QString &sprefix);
};


class MultListsImpl: public MultLists
{
      static MultListsImpl *multLists;
      bool loadMultFiles ( );
      MultListsImpl();
   public:
      CountryList ctryList;
      CountrySynonymList ctrySynList;
      DistrictList distList;
      DistrictSynonymList distSynList;
      GlistList glist;

      static MultListsImpl *getMultLists();
      ~MultListsImpl();
      //      void addCountry( bool addsyn );
      virtual QSharedPointer<CountrySynonym> searchCountrySynonym( const QString &syn );
      virtual QSharedPointer<DistrictEntry> searchDistrict( const QString &syn );
      virtual int getCtryListSize();
      virtual int getDistListSize();
      virtual QSharedPointer<CountryEntry> getCtryForPrefix( const QString &forcedMult );
      virtual QString getCtryListText( int item, int Column, BaseContestLog *const ct );
      virtual QString getDistListText( int item, int Column, BaseContestLog *const ct );
      virtual QSharedPointer<CountryEntry>  getCtryListAt( int index );
      virtual int getCtryListIndexOf(QSharedPointer<CountryEntry> );
      virtual int getDistListIndexOf( QSharedPointer<DistrictEntry> );
      virtual bool isUKprefix(const Callsign &cs);
//      virtual DistrictEntry *getDistrictEntry(int item);
//      virtual CountryEntry *getCountryEntry(int item);
      virtual int getDistWorked(int item, BaseContestLog *const ct );
      virtual int getCountryWorked(int item, BaseContestLog *const ct );

};
#endif
