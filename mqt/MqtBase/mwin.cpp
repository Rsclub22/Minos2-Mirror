/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"

const double dtg::daySecs = 86400.0;	// 24 * 60 * 60

//---------------------------------------------------------------------------
BaseLogList::BaseLogList()
{}
BaseLogList::~BaseLogList()
{}
//---------------------------------------------------------------------------
GJVParams::GJVParams(QSharedPointer<QFile> f )
      : diskBlock( 1 ), fd( f ), count( 0 )
{}
GJVParams::~GJVParams()
{}
//============================================================
Locator::Locator( ) : valRes( ERR_NOLOC )
{
   loc.setInitialValue( "        " );
}
// default versions are good enough for now!
//locator::locator(const locator&)
//{
//}
//locator& locator::operator =(const locator&)
//{
//}
Locator::~Locator()
{}
char Locator::validate( double &lon, double &lat )
{
   valRes = lonlat( loc.getValue(), lon, lat );
   return valRes;
}
char Locator::validate( )
{
   double longitude = 0.0;
   double latitude = 0.0;

   valRes = lonlat( loc.getValue(), longitude, latitude );

   return valRes;
}
//============================================================
dtg::dtg( bool now ): baddtg(false)
{
    if ( now )
    {
        QDateTime tdt = QDateTime::currentDateTimeUtc();
        qint64 correction = MinosParameters::getMinosParameters() ->getBigClockCorrection();
        if ( correction )
        {
            tdt = tdt.addSecs( correction );
        }

        QString d = tdt.toString("yyMMdd");
        sdate.setValue(d);
        QString t = tdt.toString("HHmmss");
        stime.setValue(t);
//        setDate( tdt.toString( "dd/MM/yy" ), DTGDISP );
//        setTime( tdt.toString(hh:mm:ss" ), DTGDISP );
    }
    else
    {
        setDate( "", DTGDISP );
        setTime( "", DTGDISP );
        baddtg = true;
    }
}
void dtg::setIsoDTG(const QString &d )
{
   // Untested! No errror handling!
   QString curDate = d.mid( 2, 6 );
   QString curTime = d.mid( 9, 8 );
   setDate( curDate, DTGLOG );
   setTime( curTime, DTGDISP );
}
QString dtg::getIsoDTG( bool &d ) const
{
   // Untested! No error handling!
   QString temp_date;
   QString prefix = "20";

   bool dateDirty = false;
   bool timeDirty = false;

   QString dateValue = sdate.getValue( dateDirty );
   dateValue += "            ";

   QString timeValue = stime.getValue( timeDirty );
   timeValue += "            ";

   if ( dateValue [ 0 ] >= '8' )
      prefix = "19";

   temp_date = prefix + sdate.getValue();
   temp_date += "T";
   temp_date += timeValue [ 0 ];
   temp_date += timeValue [ 1 ];
   temp_date += ':';
   temp_date += timeValue [ 2 ];
   temp_date += timeValue [ 3 ];
   temp_date += ':';
   temp_date += timeValue [ 4 ];
   temp_date += timeValue [ 5 ];

   for ( int i = 0; i < 17; i++ )
      if ( temp_date[ i ].unicode() == 0 )
         temp_date[ i ] = ' ';

   d = dateDirty || timeDirty;
   return QString( temp_date );
}
QString dtg::getIsoDTG( ) const
{
   bool dirty;
   return getIsoDTG( dirty );
}
QString dtg::getDate( DTG dstyle, bool &d ) const
{
   QString temp_date;
   QString dateValue = sdate.getValue( d );
   dateValue += "            ";
   if ( dstyle == DTGFULL )
   {
      QString prefix = "20";

      if ( dateValue [ 0 ] >= '8' )
         prefix = "19";
      temp_date = prefix + dateValue;
      temp_date = temp_date.left( 8);
   }
   else
      if ( dstyle == DTGLOG )
      {
         temp_date = dateValue;
         temp_date = temp_date.left( 8);
      }
      else
         if ( dstyle == DTGReg1Test )
         {
            temp_date = dateValue;
            temp_date = temp_date.left( 6);
         }
         else
            if (dstyle == DTGPRINT)
            {
               QString prefix = "20";

               if ( dateValue [ 0 ] >= '8' )
                  prefix = "19";
               temp_date += dateValue [ 4 ];
               temp_date += dateValue [ 5 ];
               temp_date += '/';
               temp_date += dateValue [ 2 ];
               temp_date += dateValue [ 3 ];
               temp_date += '/';
               temp_date += prefix;
               temp_date += dateValue [ 0 ];
               temp_date += dateValue [ 1 ];

               for ( int i = 0; i < 10; i++ )
                  if ( temp_date[ i ].unicode() == 0 )
                     temp_date[ i ] = ' ';
               temp_date = temp_date.left(10);
            }
            else
            {
               temp_date += dateValue [ 4 ];
               temp_date += dateValue [ 5 ];
               temp_date += '/';
               temp_date += dateValue [ 2 ];
               temp_date += dateValue [ 3 ];
               temp_date += '/';
               temp_date += dateValue [ 0 ];
               temp_date += dateValue [ 1 ];

               for ( int i = 0; i < 8; i++ )
                  if ( temp_date[ i ].unicode() == 0 )
                     temp_date[ i ] = ' ';
               temp_date = temp_date.left( 8);
            }
   return temp_date;
}
QString dtg::getDate( DTG dstyle ) const
{
   bool dirty;
   return getDate( dstyle, dirty );
}
QString dtg::getTime( DTG dstyle, bool &d ) const
{
   QString temp_time;
   QString timeValue = stime.getValue( d );
   timeValue += "            ";

   if ( dstyle == DTGLOG )
   {
      temp_time = timeValue.left( 4);
   }
   else
      if ( dstyle == DTGReg1Test )
      {
         temp_time += timeValue [ 0 ];
         temp_time += timeValue [ 1 ];
         temp_time += timeValue [ 2 ];
         temp_time += timeValue [ 3 ];
      }
      else if (dstyle == DTGACCURATE)
      {
         temp_time += timeValue [ 0 ];
         temp_time += timeValue [ 1 ];
         temp_time += ':';
         temp_time += timeValue [ 2 ];
         temp_time += timeValue [ 3 ];
         temp_time += ':';
         temp_time += timeValue [ 4 ];
         temp_time += timeValue [ 5 ];

         for ( int i = 0; i < 8; i++ )
            if ( temp_time[ i ].unicode() == 0 )
               temp_time[ i ] = ' ';
      }
         else
         {
            temp_time += timeValue [ 0 ];
            temp_time += timeValue [ 1 ];
            temp_time += ':';
            temp_time += timeValue [ 2 ];
            temp_time += timeValue [ 3 ];

            for ( int i = 0; i < 5; i++ )
               if ( temp_time[ i ].unicode() == 0 )
                  temp_time[ i ] = ' ';
         }
   return temp_time;
}
QString dtg::getTime( DTG dstyle ) const
{
   bool dirty;
   return getTime( dstyle, dirty );
}
bool dtg::getDtg( QDateTime &cttime, bool &d ) const
{
   QString dateValue = sdate.getValue( d );
   dateValue += "            ";
   dateValue = dateValue.left(6);

   QString timeValue = stime.getValue( d );
   timeValue += "            ";
   timeValue = timeValue.left(6);

   for ( int i = 0; i < 6; i++ )
      if ( !dateValue [ i ].isDigit() )
         return false;
   for ( int i = 0; i < 4; i++ )
      if ( !timeValue [ i ].isDigit() )
         return false;

   dateValue  = "20" + dateValue;

   QTime tm = QTime::fromString(timeValue, "HHmmss");

   QDate dt = QDate::fromString(dateValue, "yyyyMMdd");

   cttime = QDateTime(dt, tm, Qt::UTC);

   return true;
}
bool dtg::getDtg(QDateTime &cttime ) const
{
   bool dirty;
   return getDtg( cttime, dirty );
}
void dtg::setDate(const QString &d, DTG dstyle )
{
   QString temp;
   if ( d.length() == 0 || ( d[ 0 ] == ' ' ) || ( d[ 0 ] == '/' )
        || ( ( dstyle != DTGLOG ) && ( dstyle != DTGReg1Test ) && ( ( d[ 2 ] != '/' ) || ( d[ 5 ] != '/' )
              || d.length() != 8 ) ) )
   {
      temp = "      ";
   }
   else
      if ( dstyle == DTGDISP )
      {
         temp = d[ 6 ];
         temp += d[ 7 ];
         temp += d[ 3 ];
         temp += d[ 4 ];
         temp += d[ 0 ];
         temp += d[ 1 ];
      }
      else // LOG or Reg1Test
      {
         temp = d.left( 6 );
      }
   sdate.setValue( temp );
   baddtg = false;
}

void dtg::setTime( const QString &t, DTG dstyle )
{
   QString temp;
   if ( t.length() == 0  || ( t[ 0 ] == ' ' ) || ( t[ 0 ] == ':' ) )
   {
      temp = "    ";
   }
   else
      if ( dstyle == DTGDISP )
      {
         QString t2 = t + ":00:00:00";
         temp = t2[ 0 ];
         temp += t2[ 1 ];

         temp += t2[ 3 ];
         temp += t2[ 4 ];

         temp += t2[ 6 ];
         temp += t2[ 7 ];
      }
      else   // Log or Reg1Test (which should be a 4 char time)
      {
         QString t2 = t + "000000";
         temp = t2.left( 6 );
      }
   stime.setValue( temp );
   baddtg = false;
}
int dtg::notEntered( )
{
   int i;
   bool te = false;
   bool de = false;
   QString temp_date = getDate( DTGDISP );
   QString temp_time = getTime( DTGDISP );

   for ( i = 0; i < DATELENGTH; i++ )
   {
      if ( i >= temp_date.length() )
         break;

      if ( temp_date[ i ] != ' ' && temp_date[ i ] != '/' )
      {
         de = true;
         break;
      }
   }
   for ( i = 0; i < TIMELENGTH; i++ )
   {
      if ( i >= temp_time.length() )
         break;

      if ( temp_time[ i ] != ' ' && temp_time[ i ] != ':' )
      {
         te = true;
         break;
      }
   }
   if ( !de && !te )
      return 0;   // neither entered, will fill in when cs entered

   if ( !de )
      return 1;   // time, no date

   if ( !te )
      return 2;   // date, no time

   return -1;     // both entered
}
// default versions are good enough for now!
dtg::dtg(const dtg&rhs)
{
   sdate = rhs.sdate;
   stime = rhs.stime;
   baddtg = false;
}
dtg& dtg::operator =(const dtg&rhs)
{
   sdate = rhs.sdate;
   stime = rhs.stime;
   baddtg = false;
   return *this;
}
dtg::~dtg()
{}
QDate dtg::getDate()
{
    QString dtgstr = getDate(DTGFULL) + getTime(DTGLOG);
    QDateTime check = CanonicalToTDT( dtgstr );

    return check.date();

}
QTime dtg::getTime()
{
    QString dtgstr = getDate(DTGFULL) + getTime(DTGLOG);
    QDateTime check = CanonicalToTDT( dtgstr );

    return check.time();
}
void dtg::setDate(QDate tdt)
{
    QString d = tdt.toString("yyMMdd");
    sdate.setValue(d);
}
void dtg::setTime(QTime tdt)
{
    QString t = tdt.toString("HHmmss");
    stime.setValue(t);
}

//============================================================
Callsign::Callsign( ) : valRes( CS_NOT_VALIDATED )
{
    fullCall.setValue( "" );
}
Callsign::Callsign(const QString &pcs ) : valRes( CS_NOT_VALIDATED )
{
   fullCall.setValue( pcs );
}
// default versions are good enough for now!
//callsign::callsign(const callsign&)
//{
//}
//callsign& callsign::operator =(const callsign&)
//{
//}
Callsign::~Callsign()
{
   // nothing needed
}
int Callsign::validate( )
{
   if ( valRes != CS_NOT_VALIDATED )
      return valRes;

   // NB that leading spaces may be genuine wildcards, and so may
   // actually be significant; we should reject as invalid
   // calls with leading spaces, even though this isn't very friendly
   char csv[CALLSIGNLENGTH + 1];
   strcpy(csv, trimr(fullCall.getValue()).toStdString().c_str());          // set up a modifiable buffer

   csv[CALLSIGNLENGTH] = 0;

   valRes = ERR_NOCS;
   if ( fullCall.getValue().isEmpty() )   			// all spaces
      return valRes;

   // first check no funny characters; only alphanum and / allowed
   // (spaces are now invalid)

   valRes = ERR_INVCS;

   size_t cslen = strlen(csv);
   for ( size_t i = 0; i < cslen; i++ )
   {
      if ( ( csv[ i ] != '/' ) && ( !isalnum(csv[ i ]) ) )
      {
         return valRes;
      }
   }

   prefix.clear();
   prefix2.clear();
   suffix.clear();
   number.clear();
   body.clear();

   const char *f = csv;

   char *spt = nullptr;          // CEPT separator
   char *spt2 = nullptr;         // suffix separator

   char *cse = &csv[ cslen ];  // zero termination

   if ( cse - f < 3 )
   {
      return valRes;   // cs too short
   }

   // find any separators

   spt = strchr( csv, '/' );
   if ( spt )
   {
      *spt = 0;
      spt++;
      spt2 = strchr( spt, '/' );
      if ( cse - spt <= 0 )
         return valRes; // nothing after end
      if ( spt == f )
         return valRes; // nothing before first /

      if ( spt2 )
      {
         *spt2 = 0;
         spt2++;

         if ( strchr( spt2, '/' ) )
            return valRes; // too many separators

         if ( spt2 >= cse )
            return valRes; // nothing after end

         if ( spt2 - spt < 3 )
            return valRes;   // too little in the middle
      }

   }
   if ( spt && spt2 )
   {
      // two separators.
      // Usually F/G0GJV/P
      //What about F0ABC/RVI/P The /RVI is a country...

      if ( spt2 - spt <= TRAILBITLENGTH + 1 )   	// assume the RVI case, allow for the '/'
      {
         prefix = "/";
         prefix += QString( spt ).mid( 1, BITLENGTH );
         spt = csv;			// main body is at the start
      }
      else
      {
         // current country prefix at the start
         prefix = QString( spt ).left( BITLENGTH );
      }
      suffix = QString( spt2 ).left(TRAILBITLENGTH );
   }
   else
      if ( spt && ( cse - spt <= TRAILBITLENGTH ) )
      {
         // tail shorter than 4 chars, therefor e.g. /p or /mm - or /RVI
         // we will eventually search for mults on suffix first then prefix
         suffix = QString( spt ).left( TRAILBITLENGTH );
         *spt = 0;
         spt = csv;
      }
      else
         if ( spt )
         {
            // long tail, must be normal prefix only, no suffix

            prefix = QString( csv ).left(BITLENGTH );
         }
         else
         {
            spt = csv;
         }

   // spt now points to main callsign, no CEPT/, no /P
   // suffix and prefix are set up

   f = spt;

   while ( isdigit( *f ) && ( prefix2.length() < BITLENGTH ) )     // leading digits (2 of 2E0...)
   {
      prefix2 += *f++;
   }

   while ( *f && ( prefix2.length() < BITLENGTH ) )           // prefix letters (E of 2E0)
   {
      if ( isdigit( *f ) )
      {
         break ;
      }
      else
      {
         prefix2 += *f++;
      }
   }

   while ( isdigit( *f ) && ( number.length() < NUMBITLENGTH ) )     // central number (0 of 2E0)
      number += *f++;

   // main CS letters
   body = QString( f ).left( BITLENGTH );

   if ( prefix.length() == 0 )
   {
      prefix = prefix2.left( BITLENGTH );   // prefix is country of location
      // prefix2 is country of issue
   }

   valRes = ERR_CSDIGIT;
   int numlen = number.length();

   if ( numlen == 0 )
      return valRes;

   // Replace real prefix2 with the dup check prefix2 - for dup checking
   // Need to implement the G list here...
   QSharedPointer<CountrySynonym> syn = MultLists::getMultLists() ->searchCountrySynonym( prefix2 );
   if ( syn )
   {
      syn->getDupPrefix( prefix2 );
   }
   else
   {
       valRes = ERR_INVCS;
       return valRes;
   }

   // CountryEntry *findCtryPrefix (in contacts.cpp) does get the country by
   // stripping back the full call (or prefix/suffix as appropriate)
   // one by one...

   valRes = CS_OK;
   if ( ( prefix.length() == 0 ) || ( prefix2.length() == 0 ) || ( body.length() == 0 ) )   	// suffix can be null
   {
      valRes = ERR_INVCS;
      return valRes;
   }

   // And also want to look for
   //     trailing number in body
   //
   for (int i= 0; i < body.length(); i++)
   {
      if (!body[i].isLetter())
      {
         valRes = ERR_INVCS;
         return valRes;
      }
   }
   return valRes ;
}
//============================================================
bool Callsign::isUK() const
{
   if (fullCall.getValue().size() == 0)
   {
      return false;
   }
   // is this callsign in one of the UK areas?
   // prefix is country of location
   // prefix2 is country of issue
   return MultLists::getMultLists()->isUKprefix(*this);
}
//============================================================
bool Callsign::operator==( const Callsign& rhs ) const
{
   const Callsign * c1 = this;    // search item
   const Callsign *c2 = &rhs;    // collection item

   int ret = c1->prefix2.compare(c2->prefix2, Qt::CaseInsensitive );
   if ( ret != 0 )
      return false;

   ret = c1->number.compare(c2->number, Qt::CaseInsensitive );
   if ( ret != 0 )
      return false;

   ret = c1->body.compare( c2->body, Qt::CaseInsensitive );
   if ( ret != 0 )
      return false;

   return true;
}

bool Callsign::operator<( const Callsign& rhs ) const
{
   const Callsign * c1 = this;    // search item
   const Callsign *c2 = &rhs;    // collection item
    int ret = c1->prefix2.compare(c2->prefix2, Qt::CaseInsensitive );
    if ( ret == 0 )
    {
       ret = c1->number.compare(c2->number, Qt::CaseInsensitive );
       if ( ret == 0 )
       {
          ret = c1->body.compare( c2->body, Qt::CaseInsensitive );
          if ( ret == 0 )
             return false;
       }
    }
    return ret < 0;
}
