/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include <QRegularExpression>
#include "cutils.h"
#include "contacts.h"
#include "MultsImpl.h"


//============================================================================
// prefix/mult count
QVector<DistCount> GlistList::distCounts =
   {
      DistCount("G", 1),
      DistCount("GD", 1),
      DistCount("GI", 6),
      DistCount("GJ", 1),
      DistCount("GM", 3),
      DistCount("GU", 1),
      DistCount("GW", 1)
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
MultEntry::MultEntry( const QString &name, const QString &cloc )
{
   realName = name.trimmed();

   // set up central
   central.setLoc( cloc.left( 6 ) );
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
   MapWrapper<CountryEntry> test2(new CountryEntry(prefix2));
   MapWrapper<CountryEntry> test(new CountryEntry(prefix));
   MapWrapper<CountryEntry> res = MultListsImpl::getMultLists() ->ctryList.value(test2);
   if (res)
   {
       country2 = res.wt;
   }
   res = MultListsImpl::getMultLists() ->ctryList.value(test);
   if (res)
   {
       country1 = res.wt;
   }

}
DistrictEntry::DistrictEntry ( const QString &cd ) :
        MultEntry ( "", "" )
{
    // version for tests

    districtCode = cd;
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
         temp += country1->getBasePrefix();
      if ( country2 )
      {
         temp += "/" + country2->getBasePrefix();
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
   MapWrapper<DistrictEntry> test(new DistrictEntry(cd));
   MapWrapper<DistrictEntry> res = MultListsImpl::getMultLists() ->distList.value(test);
   if (res)
   {
       district = res.wt;
   }
}
DistrictSynonym::DistrictSynonym( const QString &syn ) :
      district( nullptr )
{
   synonym = syn;
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
}
bool DistrictList::procLine(QStringList a )
{
   QString cd = a[ 0 ];
   QString cname = a[ 1 ];
   QString prefix = a[ 2 ];
   QString prefix2 = a[ 3 ];
   QString cloc = (a.size() > 4)?a[ 4 ]:QString();

   MapWrapper<DistrictEntry >dte(new DistrictEntry ( cd, cname, prefix, prefix2, cloc ));
   if (!contains(dte))
       insert ( dte, dte );
   return true;
}
int DistrictList::getWorked( const QString &item, BaseContestLog *const ct, const QString &band )
{
   if ( ct )
      return ct->getDistrictsWorked( band, item );
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
   QString rn = ce->getRealName();
   int len = rn.indexOf('(');

   if ( strnicmp( ce->getRealName(), syn, len ) == 0 )
      return true;

   return false;
}
static QSharedPointer<DistrictEntry> searchDistrict( const QString &syn )
{
   // given a random string, look for an entry or a synonym
   for ( auto const &i: MultListsImpl::getMultLists() ->distList )
   {
      if ( i.wt->districtCode.compare( syn, Qt::CaseInsensitive ) == 0 )
      {
         return i.wt;
      }
   }

   for ( auto const &i: MultListsImpl::getMultLists() ->distSynList )
   {
      if ( i.wt->synonym.compare( syn, Qt::CaseInsensitive ) == 0 )
      {
         return i.wt ->district;
      }
   }

   for ( auto const &i: MultListsImpl::getMultLists() ->distList )
   {
      if ( compdistnames( i.wt.data(), syn ) )
      {
         return i.wt;
      }
   }

   return QSharedPointer<DistrictEntry>();
}
//======================================================================
CountryEntry::CountryEntry(const QString &continent, const QString &prefix,
                            const QString &name, const QString &cloc , int cq, int itu) :
    MultEntry( name, cloc )
  , continent( continent )
  , ITUZone(itu)
  , CQZone(cq)
{
   basePrefix = prefix.trimmed();
}
CountryEntry::CountryEntry( const QString &prefix ):MultEntry("", "")
{
   basePrefix = prefix.trimmed();
}
CountryEntry::~CountryEntry()
{}
int CountryEntry::districtLimit()
{
    if ( distLimit >= 0 )
        return distLimit;

    for (auto &dc: qAsConst(GlistList::distCounts))
    {
        if ( basePrefix.compare( dc.prefix, Qt::CaseSensitive ) == 0 )
        {
            distLimit = dc.dcount;
            return distLimit;
        }
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

   QStringList sl;
   for ( auto const &i: MultListsImpl::getMultLists() ->ctrySynList )
   {
       if ( i.wt ->getCountry() == this && i.wt->prefixType == stNormal )
      {
            sl.append(i.wt->getSynPrefix());
      }
   }
   sl.sort();
   s = sl.join(" ");
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

CountrySynonym::CountrySynonym(const QString &ssyn, const QString &sprefix
                              , const QString &cq, const QString &itu, const QString &ll
                              , const QString &cont, SynType ptype) :
      prefixType(ptype)
{
    //    (#) Override CQ Zone
    //    [#] Override ITU Zone
    //    <#/#> Override latitude/longitude
    //    {aa} Override Continent

    // none of these at the moment, so ignore them
    Q_UNUSED(ll)
    Q_UNUSED(cont)

    QString syn = ssyn.trimmed();
    QString prefix = sprefix.trimmed();


    if ( prefix.length() )   		// allow for stack based version to search by
    {
        synPrefix = syn;
        // search country list for the prefix
        for ( auto const &i: MultListsImpl::getMultLists() ->ctryList )
        {
            if (  i.wt->getBasePrefix().compare( prefix, Qt::CaseInsensitive ) == 0 )
            {
                country = i.wt;
                CQZone = i.wt->getCQZone();
                ITUZone = i.wt->getITUZone();
                continent = i.wt->getContinent();
                central = i.wt->getCentral();
                break;
            }
        }
    }
    else
    {
        synPrefix = syn;
    }
    if (!cq.isEmpty() && isPureNumeric(cq))
    {
        CQZone = cq.toInt();
    }
    if (!itu.isEmpty() && isPureNumeric(itu))
    {
        ITUZone = itu.toInt();
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
   for ( auto const &i: MultListsImpl::getMultLists() ->glist )
   {
      if ( i.wt->synPrefix.compare( prefix2, Qt::CaseInsensitive ) == 0 )
      {
         sprefix2 = i.wt->dupPrefix;
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

    static QRegularExpression ccOpen( "[\\(\\{\\[\\<]");
    static QRegularExpression ccClose("[\\)\\}\\]\\>]");

    QFile lf(fname);

    if (!lf.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QString ebuff = tr( "Failed to open %1 (%2)" ).arg(fmess, fname );
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

         int cq = a[1].toInt();
         int itu = a[2].toInt();

         MapWrapper<CountryEntry> cte(new CountryEntry ( a[ 3 ], a[ 7 ], a[ 0 ], gridref, cq, itu ));
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
         while ( !skip && i < 99 && !b[i].isEmpty() )   // = prefixes a full callsign
         {
//             (#) Override CQ Zone
//             [#] Override ITU Zone
//             <#/#> Override latitude/longitude
//             {aa} Override Continent

            QStringList overrides = {"", "", "", ""};
            QString synName;
            int openBracket = b[ i ].indexOf( ccOpen, 0 );
            if (openBracket >= 0)
            {
                // We can have ALL the bracket types in sequence

                synName = b[i].left(openBracket);   // chop off the brackets

                while (openBracket >= 0)
                {
                    QChar btype = b[i][openBracket];

                    int closeBracket = b[ i ].indexOf( ccClose, openBracket + 1 );
                    if (closeBracket >= 0)
                    {
                        QString bracketted = b[i].mid(openBracket + 1, closeBracket - openBracket - 1);
                        if (btype == '(')
                        {
                            overrides[0] = bracketted;
                        }
                        else if (btype == '[')
                        {
                            overrides[1] = bracketted;
                        }
                        else if (btype == '<')
                        {
                            // none at the moment
                            overrides[2] = bracketted;
                        }
                        else if (btype == '{')
                        {
                            // none at the moment
                            overrides[3] = bracketted;
                        }
                    }

                    openBracket = b[ i ].indexOf( ccOpen, closeBracket + 1 );
                }
            }
            else
            {
                synName = b[i];
            }
            if ( b[ i ][ 0 ] == '=')
            {
                // One off callsign - it all needs to match
                QString cs = QString(b[i]);
                cs = cs.mid(1);

                makeCountrySynonym( cs, mainPrefix, overrides[0], overrides[1], overrides[2], overrides[3], stCallsign );
            }
            else
            {
                makeCountrySynonym( synName, mainPrefix, overrides[0], overrides[1], overrides[2], overrides[3], stNormal );
            }
            i++;
         }
      }
   }
}
int CountryList::getWorked(const QString &item, BaseContestLog *const ct, const QString &band )
{
   if ( ct )
      return ct->getCountriesWorked( band, item );
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
        MultListsImpl::getMultLists()->ctryList.makeCountrySynonym( a[ i ], a[ 0 ]
                                                                  , "", "", "", ""
                                                                  , stNormal
                                                                  );
   }

   return true;
}
void CountryList::makeCountrySynonym(const QString &ssyn, const QString &sprefix
                                     , const QString &cq, const QString &itu, const QString &ll
                                     , const QString &cont, SynType prefixType)
{
   // search country list for the prefix

   QString syn = ssyn.trimmed();
   QString prefix = sprefix.trimmed();

   if ( syn.indexOf( '-' ) >= 0 )
   {
      MinosParameters::getMinosParameters() ->mshowMessage( ( tr( "Synonym ranges no longer allowed : %1 for %2" ).arg(ssyn, sprefix) ) );
      return ;
   }

   QSharedPointer<CountryEntry> ctry;
   MapWrapper<CountryEntry> test(new CountryEntry(prefix));
   MapWrapper<CountryEntry> res = value(test);
   if (res)
   {
       ctry = res.wt;
   }

   MapWrapper< CountrySynonym> cts(MultListsImpl::getMultLists()->searchCountrySynonym ( syn ));
   if ( cts.wt && ( cts.wt->getCountry().data() == ctry.data() ) )
      return ;		// as already there

   cts = MapWrapper<CountrySynonym >(new CountrySynonym ( syn, prefix
                                                       , cq, itu, ll, cont
                                                       , prefixType
                                                       ));

   if ( cts.wt->getCountry() )
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
    return loc.compare(rhs.loc) == 0;
}
bool LocSquare::operator!=( const LocSquare& rhs ) const
{
    return loc.compare(rhs.loc) != 0;
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
// /*
   QFile fos("c:/temp/multlist.txt");
   if (!fos.open(QIODevice::WriteOnly|QIODevice::Text))
      return false;

   QTextStream os(&fos);

   MultListsImpl *m = this;

   os << "================== country entries ========================\n";
   for (MultList < CountryEntry >::iterator i = m->ctryList.begin(); i != m->ctryList.end(); i++)
   {
      os << i->wt->getBasePrefix() + " " + i->wt->getRealName() << "\n";
   }
   os << QString("================== country synonyms ") + QString::number(m->ctrySynList.size()) + "========================\n";
   for (MultList < CountrySynonym  >::iterator i = m->ctrySynList.begin(); i != m->ctrySynList.end(); i++)
   {
      QString temp1 = i->wt->getSynPrefix();
      QSharedPointer<CountryEntry> country = i->wt->country;
      QString temp2 = country->getBasePrefix();
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
// */
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
   for ( auto const &i: MultListsImpl::getMultLists() ->ctryList )
   {
      if ( i.wt ->getBasePrefix().compare( forcedMult, Qt::CaseInsensitive ) == 0 )
      {
         ctryMult = i.wt;
         break;
      }
   }
   return ctryMult;
}

//void MultListsImpl::addCountry( bool addsyn );
QSharedPointer<CountrySynonym> MultListsImpl::searchCountrySynonym(const QString &syn )
{
    MapWrapper < CountrySynonym > test(new CountrySynonym( syn, "", "", "", "", "", stNormal ));
    MapWrapper < CountrySynonym > defVal(new CountrySynonym("", "", "", "", "", "", stNormal));
    MapWrapper < CountrySynonym > cs = MultListsImpl::getMultLists() ->ctrySynList.value(test, defVal);

    if (cs == defVal)
        return QSharedPointer<CountrySynonym>();
    else
        return cs.wt;
}
QSharedPointer<DistrictEntry> MultListsImpl::searchDistrict( const QString &syn )
{
   return ::searchDistrict( syn );
}
QString MultListsImpl::getCtryListText( const QString &item, int Column, BaseContestLog *const ct, const QString &band )
{
   return ctryList.getText( item, Column, ct, band );
}
QString MultListsImpl::getDistListText(const QString &item, int Column, BaseContestLog *const ct, const QString &band )
{
   return distList.getText( item, Column, ct, band );
}
bool MultListsImpl::isUKprefix(const Callsign &cs)
{
   QSharedPointer<CountryEntry> ctry = findCtryPrefix( cs );
   if (!ctry)
   {
      return false;
   }
   for ( auto &dc: qAsConst(GlistList::distCounts) )
   {
       if ( ctry->getBasePrefix().compare( dc.prefix, Qt::CaseSensitive ) == 0 )
       {
           return true;
       }
   }
    return false;
}

int MultListsImpl::getDistWorked(const QString & item, BaseContestLog * const ct, const QString &band)
{
    return distList.getWorked(item, ct, band);
}

int MultListsImpl::getCountryWorked(const QString & item, BaseContestLog * const ct, const QString &band)
{
    return ctryList.getWorked(item, ct, band);
}
QVector<QSharedPointer<DistrictEntry> > &MultListsImpl::getDistList()
{
    if (distVector.size() == 0)
    {
        for (auto &d: qAsConst(distList))
        {
            distVector.push_back(d.wt);
        }
        std::sort(distVector.begin(), distVector.end(),
        [=](const QSharedPointer<DistrictEntry> a, const QSharedPointer<DistrictEntry> b)->bool
          {
              return a->districtCode < b->districtCode;
          }
        );

    }
    return distVector;
}
QVector<QSharedPointer<CountryEntry> > &MultListsImpl::getCountryList()
{
    if (countryVector.size() == 0)
    {
        for (auto &d: qAsConst(ctryList))
        {
            countryVector.push_back(d.wt);
        }
        std::sort(countryVector.begin(), countryVector.end(),
        [=](const QSharedPointer<CountryEntry> a, const QSharedPointer<CountryEntry> b)->bool
          {
              return a->getBasePrefix() < b->getBasePrefix();
          }
        );
    }
    return countryVector;
}
