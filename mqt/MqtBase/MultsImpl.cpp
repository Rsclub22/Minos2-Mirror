/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"

#include "contacts.h"
#include "MultsImpl.h"


//============================================================================
// prefix/mult count
struct DistCount
{
   char prefix[ 3 ];
   char dcount;
};
static DistCount distCounts[] =
   {
      {"G", 1},
      {"GD", 1},
      {"GI", 6},
      {"GJ", 1},
      {"GM", 3},
      {"GU", 1},
      {"GW", 1},
      {"", 0}
   };
//============================================================================
GlistEntry::GlistEntry( const QString &syn, const QString &dup )
{
   synPrefix = syn.trimmed();
   dupPrefix = dup.trimmed();
}
GlistEntry::~GlistEntry()
{}

bool GlistEntry::operator<( const GlistEntry& rhs ) const
{
   // p1 is from list; p2 is the one being searched for

   int res = synPrefix.compare(rhs.synPrefix, Qt::CaseInsensitive );
   return res < 0;
}
bool GlistEntry::operator==( const GlistEntry& rhs ) const
{
   // p1 is from list; p2 is the one being searched for

    int res = synPrefix.compare(rhs.synPrefix, Qt::CaseInsensitive );
   return res == 0;
}
bool GlistEntry::operator!=( const GlistEntry& rhs ) const
{
   // p1 is from list; p2 is the one being searched for

    int res = synPrefix.compare(rhs.synPrefix, Qt::CaseInsensitive );
   return res != 0;
}

GlistList::GlistList( )
{}
GlistList::~GlistList()
{}
void GlistList::load( )
{
   loadEntries( "./Configuration/prefix.syn", tr("prefix synonyms file") );
}
bool GlistList::procLine( QStringList a )
{
   QString syn = a[ 0 ];
   QString dup = a[ 1 ];

   MapWrapper<GlistEntry>gle(new GlistEntry ( syn, dup ));
   if (!contains(gle))
       insert ( gle, gle );
   return true;
}

//======================================================================
MultEntry::MultEntry( const QString &name, const QString &cloc ) : listOffset( -1 )
{
   realName = name.trimmed();

   // set up central
   central.loc.setValue( cloc.trimmed().left( 6 ) );
}
MultEntry::~MultEntry()
{}
void MultEntry::addSynonyms( QString &s )
{
   s = "";
}
//======================================================================
DistrictEntry::DistrictEntry( const QString &cd, const QString &name, const QString &prefix, const QString &prefix2, const QString &cloc ) :
      MultEntry( name, cloc ), country1( nullptr ), country2( nullptr )
{
   // set district code to cd

   districtCode = cd;

   // search country list for the prefix

   for ( MultList < CountryEntry>::iterator i = MultListsImpl::getMultLists() ->ctryList.begin(); i != MultListsImpl::getMultLists() ->ctryList.end(); i++ )
   {
      if ( i->wt->basePrefix.compare( prefix2, Qt::CaseInsensitive ) == 0 )
      {
         country2 = i->wt;
         break;
      }
   }
   for ( MultList < CountryEntry>::iterator i = MultListsImpl::getMultLists() ->ctryList.begin(); i != MultListsImpl::getMultLists() ->ctryList.end(); i++ )
   {
      if ( i->wt->basePrefix.compare( prefix, Qt::CaseInsensitive ) == 0 )
      {
         country1 = i->wt;
         break;
      }
   }
}
DistrictEntry::~DistrictEntry()
{
   // no need to delete any components
}
QString DistrictEntry::str( bool longdisp )
{
   QString temp;
   temp = districtCode;
   if ( longdisp )
   {
      temp += "(";
      if ( country1 )
         temp += country1->basePrefix;
      if ( country2 )
      {
         temp += "/" + country2->basePrefix;
      }
      temp += ")";
   }
   return temp;
}
void DistrictEntry::addSynonyms( QString &s )
{
   s = "";
}
bool DistrictEntry::operator<( const DistrictEntry& rhs ) const
{
   int res =  districtCode.compare( rhs.districtCode, Qt::CaseInsensitive );
   return res < 0;
}
bool DistrictEntry::operator==( const DistrictEntry& rhs ) const
{
    int res =  districtCode.compare( rhs.districtCode, Qt::CaseInsensitive );
   return res == 0;
}
bool DistrictEntry::operator!=( const DistrictEntry& rhs ) const
{
    int res =  districtCode.compare( rhs.districtCode, Qt::CaseInsensitive );
   return res != 0;
}


//======================================================================
DistrictSynonym::DistrictSynonym( const QString &cd, const QString &syn ) :
      district( nullptr )
{
   synonym = syn;

   // find district entry from cd code

   for ( MultList < DistrictEntry>::iterator i = MultListsImpl::getMultLists() ->distList.begin(); i != MultListsImpl::getMultLists() ->distList.end(); i++ )
   {
      if ( i->wt->districtCode.compare( cd, Qt::CaseInsensitive ) == 0 )
      {
         district = i->wt;
         break;
      }
   }
}
DistrictSynonym::~DistrictSynonym()
{}
bool DistrictSynonym::operator<( const DistrictSynonym& rhs ) const
{
   int res = synonym.compare( rhs.synonym, Qt::CaseInsensitive );
   return res < 0;
}
bool DistrictSynonym::operator==( const DistrictSynonym& rhs ) const
{
    int res = synonym.compare( rhs.synonym, Qt::CaseInsensitive );
   return res == 0;
}
bool DistrictSynonym::operator!=( const DistrictSynonym& rhs ) const
{
    int res = synonym.compare( rhs.synonym, Qt::CaseInsensitive );
   return res != 0;
}
//======================================================================
DistrictList::DistrictList( )
{}
DistrictList::~DistrictList()
{
   // nothing to delete
}
int DistrictList::slen( bool longver )
{
   return longver ? 10 : 2;
}
void DistrictList::load( )
{
   loadEntries( "./Configuration/district.ctl", "District Control File" );

   int i = 0;
   for(  MultList<DistrictEntry>::iterator ce = begin(); ce != end(); ce++)
   {
       ce->wt ->listOffset = i++;
   }
}
bool DistrictList::procLine(QStringList a )
{
   QString cd = a[ 0 ];
   QString cname = a[ 1 ];
   QString prefix = a[ 2 ];
   QString prefix2 = a[ 3 ];
   QString cloc = a[ 4 ];

   MapWrapper<DistrictEntry >dte(new DistrictEntry ( cd, cname, prefix, prefix2, cloc ));
   if (!contains(dte))
       insert ( dte, dte );
   return true;
}
int DistrictList::getWorked( int item, BaseContestLog *const ct )
{
   if ( ct )
      return ct->getDistrictsWorked( item );
   else
      return 0;
}
//======================================================================
DistrictSynonymList::DistrictSynonymList( )
{}
DistrictSynonymList::~DistrictSynonymList()
{
   // nothing to delete
}

void DistrictSynonymList::load( )
{
   loadEntries( "./Configuration/DISTRICT.SYN", "District Synonym File" );
}
bool DistrictSynonymList::procLine( QStringList a )
{
   QString cd = a[ 0 ];
   QString cname = a[ 1 ];
   MapWrapper<DistrictSynonym> dse(new DistrictSynonym ( cd, cname ));

   if ( dse.wt->district )
       insert ( dse, dse );
   return true;
}
static bool compdistnames( DistrictEntry *ce, const QString &syn )
{
   int len = ce->realName.length();
   for ( int i = 0; i < len; i++ )
   {
      if ( ce->realName[ i ] == '(' )
      {
         len = i;
         break;
      }
   }

   if ( strnicmp( ce->realName, syn, len ) == 0 )
      return true;

   return false;
}
static QSharedPointer<DistrictEntry> searchDistrict( const QString &syn )
{
   // given a random string, look for an entry or a synonym
   for ( MultList < DistrictEntry>::iterator i = MultListsImpl::getMultLists() ->distList.begin(); i != MultListsImpl::getMultLists() ->distList.end(); i++ )
   {
      if ( i->wt->districtCode.compare( syn, Qt::CaseInsensitive ) == 0 )
      {
         return i->wt;
      }
   }

   for ( MultList < DistrictSynonym>::iterator i = MultListsImpl::getMultLists() ->distSynList.begin(); i != MultListsImpl::getMultLists() ->distSynList.end(); i++ )
   {
      if ( i->wt->synonym.compare( syn, Qt::CaseInsensitive ) == 0 )
      {
         return i->wt ->district;
      }
   }

   for ( MultList < DistrictEntry>::iterator i = MultListsImpl::getMultLists() ->distList.begin(); i != MultListsImpl::getMultLists() ->distList.end(); i++ )
   {
      if ( compdistnames( i->wt.data(), syn ) )
      {
         return i->wt;
      }
   }

   return QSharedPointer<DistrictEntry>();
}
//======================================================================
CountryEntry::CountryEntry( const QString &continent, const QString &prefix,
                            const QString &name, const QString &cloc ) :
      MultEntry( name, cloc ), distLimit( -1 ), continent( continent )
{
   basePrefix = prefix.trimmed();
}
CountryEntry::~CountryEntry()
{}
int CountryEntry::districtLimit()
{
   //	search distCounts
   if ( distLimit >= 0 )
      return distLimit;

   DistCount *dc = &distCounts[ 0 ];
   while ( dc->dcount )
   {
      if ( basePrefix.compare( dc->prefix, Qt::CaseInsensitive ) == 0 )
      {
         distLimit = dc->dcount;
         return distLimit;
      }
      dc++;
   }
   return 0;
}
bool CountryEntry::hasDistricts()
{
   if ( districtLimit() > 0 )
      return true;
   return false;
}
QString CountryEntry::str( bool )
{
   return basePrefix;
}
void CountryEntry::addSynonyms( QString &s )
{
   // add list of synonyms to the display buffer
   s = QString();
   for ( MultList < CountrySynonym >::iterator i = MultListsImpl::getMultLists() ->ctrySynList.begin(); i != MultListsImpl::getMultLists() ->ctrySynList.end(); i++ )
   {
      if ( i->wt ->country == this )
      {
         if ( s.length() < 180 )   		// should really use the correct value here!
         {
            s += " ";
            i->wt ->synCat( s );
         }
      }
   }
}
bool CountryEntry::operator<( const CountryEntry& rhs ) const
{
   int res = basePrefix.compare( rhs.basePrefix, Qt::CaseInsensitive );
   return res < 0;
}
bool CountryEntry::operator==( const CountryEntry& rhs ) const
{
    int res = basePrefix.compare( rhs.basePrefix, Qt::CaseInsensitive );
   return res == 0;
}
bool CountryEntry::operator!=( const CountryEntry& rhs ) const
{
    int res = basePrefix.compare( rhs.basePrefix, Qt::CaseInsensitive );
   return res != 0;
}
//======================================================================
static QSharedPointer<CountrySynonym> searchCountrySynonym( const QString &syn )
{
    MapWrapper <CountrySynonym> test(new CountrySynonym( syn, "" ));

   MultList < CountrySynonym > ::iterator cs = std::lower_bound( MultListsImpl::getMultLists() ->ctrySynList.begin(),
         MultListsImpl::getMultLists() ->ctrySynList.end(),
         test);

   if ( cs == MultListsImpl::getMultLists() ->ctrySynList.end() || !( ( *cs->wt.data() ) == *test.wt.data() ) )
   {
      cs = MultListsImpl::getMultLists() ->ctrySynList.end();
   }
   if ( cs == MultListsImpl::getMultLists() ->ctrySynList.end() )
      return QSharedPointer<CountrySynonym>();
   else
      return cs->wt;

}
CountrySynonym::CountrySynonym( const QString &ssyn, const QString &sprefix ) :
      country( nullptr )
{
   QString syn = ssyn.trimmed();
   QString prefix = sprefix.trimmed();

   if ( prefix.length() )   		// allow for stack based version to search by
   {
      synPrefix = syn;
      // search country list for the prefix
      for ( MultList < CountryEntry>::iterator i = MultListsImpl::getMultLists() ->ctryList.begin(); i != MultListsImpl::getMultLists() ->ctryList.end(); i++ )
      {
         if (  i->wt->basePrefix.compare( prefix, Qt::CaseInsensitive ) == 0 )
            country = i->wt;
      }
   }
   else
   {
      synPrefix = syn;
   }
}
CountrySynonym::~CountrySynonym()
{}
void CountrySynonym::getDupPrefix( QString &sprefix2 )
{
   QString prefix2 = sprefix2.trimmed();
   //	search Glist
   // dup_prefix_offset was used to speed this up. We may need something similar
   // None found, then don't change prefix2
   for ( MultList < GlistEntry >::iterator i = MultListsImpl::getMultLists() ->glist.begin(); i != MultListsImpl::getMultLists() ->glist.end(); i++ )
   {
      if ( i->wt->synPrefix.compare( prefix2, Qt::CaseInsensitive ) == 0 )
      {
         sprefix2 = i->wt->dupPrefix;
         break;
      }
   }
   return ;
}
bool CountrySynonym::operator<( const CountrySynonym& rhs ) const
{
   int res = compare( rhs );
   return res < 0;
}
bool CountrySynonym::operator==( const CountrySynonym& rhs ) const
{
   int res = compare( rhs );
   return res == 0;
}
bool CountrySynonym::operator!=( const CountrySynonym& rhs ) const
{
   int res = compare( rhs );
   return res != 0;
}
int CountrySynonym::compare( const CountrySynonym &cs ) const
{
   // p1 is from list; p2 is the one being searched for
   int res;

   res = synPrefix.compare(cs.synPrefix, Qt::CaseInsensitive );
   if ( res < 0 )
      return -1;
   else
      if ( res == 0 )
         return 0;
      else
         return 1;
}
void CountrySynonym::synCat( QString &add_buff )
{
   add_buff += synPrefix;
}
//======================================================================
CountryList::CountryList( )
{}
CountryList::~CountryList()
{
   // nothing to delete
}
int CountryList::slen( bool )
{
   return 9;
}
void CountryList::load( )
{
   loadEntries( "./Configuration/cty.dat", tr("CT9 Country File" ));

   int i = 0;
   for(  MultList<CountryEntry>::iterator ce = begin(); ce != end(); ce++)
   {
       ce->wt ->listOffset = i++;
   }
}
bool CountryList::procLine(QStringList )
{
   return true;
}
// lat, longi to be in degrees, -ve for W or S
extern int geotoloc( double lat, double longi, QString &gridref );
//==============================================================================

// Parse the CT9 CTY.DAT format
/*
------------------------------------------------------------------------
 
 
      CTY.DAT file format
 
CTY.DAT is a new file format for CT Version 9. It includes more
information than the .CTY files for previous versions of CT. This allows
CT to calculate, in real time, beam heading and sun times. It also means
that a single file can be used for all DX contests. The format is as
follows. Note that the fields are aligned in columns and spaced out for
readability only. It is the ":" at the end of each field that acts as a
delimiter for that field:
 
Column Length Description
1 26 Country Name
27 5 CQ Zone
32 5 ITU Zone
37 5 2-letter continent abbreviation
42 9 Latitude in degrees, + for North
51 10 Longitude in degrees, + for West
61 9 Local time offset from GMT
70 6 Primary DXCC Prefix (A "*" preceding this prefix indicates that the
country is on the DARC WAEDC list, and counts in CQ-sponsored contests,
but not ARRL-sponsored contests).
 
Alias DXCC prefixes (including the primary one) follow on consecutive
lines, separated by ",". If there is more than one line, subsequent
lines begin with the "&" continuation character. A ";" terminates the
last prefix in the list.
 
The following special characters can be applied to an alias prefix:
 
(#) Override CQ Zone
[#] Override ITU Zone
<#/#> Override latitude/longitude
{aa} Override Continent
 
See the examples on page 27 in the CT9 manual.
 
------------------------------------------------------------------------
// Some samples:
Chesterfield Is.:         32:  56:  OC:  -19.90:  -158.30:   -11.0:  TX0:
    TX0;
Benin:                    35:  46:  AF:    6.50:    -2.60:    -1.0:  TY:
    TY;
Mali:                     35:  46:  AF:   12.70:     8.00:     0.0:  TZ:
    TZ;
European Russia:          16:  29:  EU:   55.80:   -37.60:    -3.0:  UA:
    R,U;
Kaliningradsk:            15:  29:  EU:   55.00:   -20.50:    -3.0:  UA2:
    R2,RA2,RB2,RC2,RD2,RE2,RF2,RG2,RH2,RI2,RJ2,RK2,RL2,RM2,RN2,RP2,RQ2,RR2,
    RS2,RT2,RU2,RV2,RW2,RX2,RY2,RZ2,U2,UA2,UB2,UC2,UD2,UF2,UG2,UH2,UI2;
Asiatic Russia:           17:  30:  AS:   55.00:   -83.00:    -7.0:  UA9:
    R0,R3F/9,R7(17),R8,R8T(18)[32],R8V(18)[32],R9,R9I(18),R9M(17),R9S(17),R9W(17),
    RA0,RA7(17),RA8,RA8T(18)[32],RA8V(18)[32],RA9,RA9I(18),RA9M(17),RA9S(17),
    .
    .
    .
    UI9,UI9I(18),UI9M(17),UI9S(17),UI9W(17);
------------------------------------------------------------------------
*/
void CountryList::loadEntries( const QString &fname, const QString &fmess )
{
   // load a CT9 formatted list

    QFile lf(fname);

    if (!lf.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QString ebuff = tr( "Failed to open %1 (%2)" ).arg(fmess).arg(fname );
        MinosParameters::getMinosParameters() ->mshowMessage( ebuff );
        return;
    }
    QTextStream istr(&lf);
    while (!istr.atEnd())
    {

      QString countrybuff = istr.readLine(255);

      if ( countrybuff.isEmpty() || countrybuff[ 0 ] == '#' )      // only allow # comments
         continue;   // skip comment lines

      QStringList a;
      bool sep2seen;
      parseLine( countrybuff, ':', a, 9, 0, sep2seen );

      QString mainPrefix( a[ 7 ] );
      bool skip = ( mainPrefix[ 0 ] == '*' );
      if ( !skip )
      {
         // lat, longi to be in degrees, -ve for W or S

         // name: continent: lat: long: DXCC Id:
         // BUT CT uses + for N, + for W
         double lat = 0.0;
         double longi = 0.0;

         lat = a[ 4 ].toDouble();
         longi = a[ 5 ].toDouble();

         QString gridref;
         geotoloc( lat, -longi, gridref );	// kill temporary warning

         MapWrapper<CountryEntry> cte(new CountryEntry ( a[ 3 ], a[ 7 ], a[ 0 ], gridref ));
         if (!contains(cte))
             insert ( cte, cte );
      }
   /*
      CTY.dat cty-1805 1/6/2008

      IMPORTANT: This release represents a change in the file format.
      Starting with this release, a '=' character will prefix full callsigns
      in CTY.DAT, CTY_WT.DAT, CTY_WT_MOD.DAT and WL_CTY.DAT.
      This is necessary to differentiate a full callsign like K7A
      in Alaska from the prefix K7A (i.e. K7ABC should still be
      United States, not Alaska).
   */
      // now we go through following lines up to a semicolon terminator
      sep2seen = false;
      while ( !sep2seen && !istr.atEnd())
      {
          countrybuff = istr.readLine(255);
         // elements are comma separated, including the end of line
         // may be white space around
         // elements may be complete callsigns
         // each one wants to be added to the CountrySynonymList

         if ( countrybuff.isEmpty() || countrybuff[ 0 ] == '#' )      // only allow # comments
            continue;   // skip comment lines

         QStringList b;
         parseLine( countrybuff, ',', b, 99, ';', sep2seen );
         int i = 0;
         QString part = b[i];
         while ( !skip && i < 99 && !part.isEmpty()  && part[ 0 ] != '=')
         {
            int bracket = strcspn( b[ i ], "({[<" );
            if ( bracket >= 0 )
               b[ i ] = b[i].left(bracket);   // chop off the brackets
            CountrySynonymList::makeCountrySynonym( b[ i ], mainPrefix );
            i++;
            part = b[i];
         }
      }
   }
}
int CountryList::getWorked( int item, BaseContestLog *const ct )
{
   if ( ct )
      return ct->getCountriesWorked( item );
   else
      return 0;
}

//======================================================================
CountrySynonymList::CountrySynonymList( )
{}
CountrySynonymList::~CountrySynonymList()
{
   // nothing to delete
}
void CountrySynonymList::load( )
{
   loadEntries( "./Configuration/cty.syn", tr("Country Synonym File") );
}
bool CountrySynonymList::procLine( QStringList a )
{
   for ( int i = 1; i < a.length() && !a[ i ].isEmpty() ; i++ )
   {
      makeCountrySynonym( a[ i ], a[ 0 ] );
   }

   return true;
}
void CountrySynonymList::makeCountrySynonym( const QString &ssyn, const QString &sprefix )
{
   // search country list for the prefix

   QString syn = ssyn.trimmed();
   QString prefix = sprefix.trimmed();

   if ( syn.indexOf( '-' ) >= 0 )
   {
      MinosParameters::getMinosParameters() ->mshowMessage( ( tr( "Synonym ranges no longer allowed : %1 for %2" ).arg(ssyn).arg(sprefix) ) );
      return ;
   }

   QSharedPointer<CountryEntry> ctry;
   for ( MultList < CountryEntry>::iterator i = MultListsImpl::getMultLists() ->ctryList.begin(); i != MultListsImpl::getMultLists() ->ctryList.end(); i++ )
   {
      if ( i->wt->basePrefix.compare( prefix, Qt::CaseInsensitive ) == 0 )
      {
         ctry = i->wt;
         break;
      }
   }
   if ( !ctry )
      return ;		// as it will be unsuccessfull anyway

   MapWrapper< CountrySynonym> cts(searchCountrySynonym ( syn ));
   if ( cts.wt && ( cts.wt->country.data() == ctry.data() ) )
      return ;		// as already there

   cts = MapWrapper<CountrySynonym >(new CountrySynonym ( syn, prefix ));

   if ( cts.wt->country )
   {
       if (!MultListsImpl::getMultLists() ->ctrySynList.contains(cts))
           MultListsImpl::getMultLists() ->ctrySynList.insert ( cts, cts );   // must add to the syn list...
   }
}

//======================================================================
LocList::LocList( )

{}
LocList::~LocList()
{
}
//======================================================================
LocSquare::LocSquare( const QString &locId )
{
   clear();
   loc = locId.left(2).toUpper();
}

LocCount *LocSquare::map( const QString &num )
{
   if ( !num[ 0 ].isDigit() || !num[ 1 ].isDigit() )
      return nullptr;

   return &numbers[ num[ 0 ].toLatin1() - '0' ][ num[ 1 ].toLatin1() - '0' ];
}

LocCount *LocSquare::map( int num )
{
   if ( ( num < 0 ) || ( num >= 100 ) )
      return nullptr;
   int dig2 = num % 100;
   int dig1 = ( num - dig2 ) / 100;

   return &numbers[ dig1 ][ dig2 ];
}

void LocSquare::clear( )
{
   for ( int i = 0; i < 10; i++ )
      for ( int j = 0; j < 10; j++ )
      {
         numbers[ i ][ j ].UKMultGiven = false;
         numbers[ i ][ j ].UKLocCount = 0;
         numbers[ i ][ j ].nonUKLocCount = 0;
      }
}
bool LocSquare::isClear()
{
    for ( int i = 0; i < 10; i++ )
    {
       for ( int j = 0; j < 10; j++ )
       {
          if (numbers[ i ][ j ].UKLocCount > 0)
              return false;
          if (numbers[ i ][ j ].nonUKLocCount > 0)
              return false;
       }
    }
    return true;
}
bool LocSquare::operator<( const LocSquare& rhs ) const
{
   return loc.compare(rhs.loc) < 0;
}
bool LocSquare::operator==( const LocSquare& rhs ) const
{
    return loc.compare(rhs.loc) < 0;
}
bool LocSquare::operator!=( const LocSquare& rhs ) const
{
    return loc.compare(rhs.loc) < 0;
}

//======================================================================
MultListsImpl *MultListsImpl::multLists = nullptr;

MultListsImpl *MultListsImpl::getMultLists()
{
   static bool firstTime = true;
   if ( !multLists && firstTime )
   {
      firstTime = false;
      multLists = new MultListsImpl();
      multLists->loadMultFiles();
   }
   return multLists;
}
bool MultListsImpl::loadMultFiles( )
{
   ctryList.load();
   ctrySynList.load();
   distList.load();
   distSynList.load();
   glist.load();
/*
   QFile fos("c:/temp/multlist.txt");
   if (!fos.open(QIODevice::WriteOnly|QIODevice::Text))
      return false;

   QTextStream os(&fos);

   MultListsImpl *m = this;

   os << "================== country entries ========================\n";
   for (MultList < CountryEntry >::iterator i = m->ctryList.begin(); i != m->ctryList.end(); i++)
   {
      os << i->wt->basePrefix + " " + i->wt->realName << "\n";
   }
   os << QString("================== country synonyms ") + QString::number(m->ctrySynList.size()) + "========================\n";
   for (MultList < CountrySynonym  >::iterator i = m->ctrySynList.begin(); i != m->ctrySynList.end(); i++)
   {
      QString temp1 = i->wt->synPrefix;
      QSharedPointer<CountryEntry> country = i->wt->country;
      QString temp2 = country->basePrefix;
      os << (temp1 + " : " + temp2) << "\n";
   }
   os << "================== district entries ========================\n";
   for (MultList < DistrictEntry >::iterator i = m->distList.begin(); i != m->distList.end(); i++)
   {
      os << i->wt->districtCode << "\n";
   }
   os << "================== district synonyms ========================\n";
   for (MultList < DistrictSynonym >::iterator i = m->distSynList.begin(); i != m->distSynList.end(); i++)
   {
      os << i->wt->synonym + " : " + (i->wt->district)->districtCode  + "\n";
   }
   os << "================== Glist ========================\n";
   for (MultList < GlistEntry >::iterator i = m->glist.begin(); i != m->glist.end(); i++)
   {
      os << i->wt->synPrefix + " : " + i->wt->dupPrefix + "\n";
   }
*/
   return true;
}
MultListsImpl::MultListsImpl()
{
}
MultListsImpl::~MultListsImpl()
{
   multLists = nullptr;
}

int MultListsImpl::getCtryListSize()
{
   return ctryList.size();
}
int MultListsImpl::getDistListSize()
{
   return distList.size();
}
QSharedPointer<CountryEntry> MultListsImpl::getCtryForPrefix( const QString &forcedMult )
{
   QSharedPointer<CountryEntry> ctryMult;
   for ( MultList < CountryEntry >::iterator i = MultListsImpl::getMultLists() ->ctryList.begin(); i != MultListsImpl::getMultLists() ->ctryList.end(); i++ )
   {
      if ( i ->wt ->basePrefix.compare( forcedMult, Qt::CaseInsensitive ) == 0 )
      {
         ctryMult = i->wt;
         break;
      }
   }
   return ctryMult;
}

//void MultListsImpl::addCountry( bool addsyn );
QSharedPointer<CountrySynonym> MultListsImpl::searchCountrySynonym( const QString &syn )
{
   return ::searchCountrySynonym( syn );
}
QSharedPointer<DistrictEntry> MultListsImpl::searchDistrict( const QString &syn )
{
   return ::searchDistrict( syn );
}
QString MultListsImpl::getCtryListText( int item, int Column, BaseContestLog *const ct )
{
   return ctryList.getText( item, Column, ct );
}
QString MultListsImpl::getDistListText( int item, int Column, BaseContestLog *const ct )
{
   return distList.getText( item, Column, ct );
}
QSharedPointer<CountryEntry> MultListsImpl::getCtryListAt( int index )
{
    return std::next(ctryList.begin(), index)->wt;
//   return ctryList.at( index );
}

int MultListsImpl::getCtryListIndexOf( QSharedPointer<CountryEntry> e )
{
   return ctryList.indexOf( *e.data() );
}
int MultListsImpl::getDistListIndexOf( QSharedPointer<DistrictEntry> e )
{
   return distList.indexOf( *e.data() );
}
bool MultListsImpl::isUKprefix(const Callsign &cs)
{
   QSharedPointer<CountryEntry> ctry = findCtryPrefix( cs );
   if (!ctry)
   {
      return false;
   }
   for (unsigned int i = 0; i < sizeof(distCounts)/sizeof(DistCount); i++)
   {
      if (ctry->basePrefix.compare( distCounts[i].prefix, Qt::CaseInsensitive ) == 0)
      {
         return true;
      }
   }
	return false;
}

int MultListsImpl::getDistWorked(int item, BaseContestLog * const ct)
{
    return distList.getWorked(item, ct);
}

int MultListsImpl::getCountryWorked(int item, BaseContestLog * const ct)
{
    return ctryList.getWorked(item, ct);
}
