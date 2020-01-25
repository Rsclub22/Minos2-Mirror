/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"
#include "mults.h"
#include "cutils.h"

const double dtg::daySecs = 86400.0;	// 24 * 60 * 60

//---------------------------------------------------------------------------
BaseLogList::BaseLogList():QObject()
{}
BaseLogList::BaseLogList(const BaseLogList &):QObject()
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
Locator::Locator(const QString & l): valRes(ERR_NOLOC)
{
    loc.setInitialValue(l);
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
   valRes = lonlat( loc.getValue(), lon, lat, MinosParameters::getMinosParameters() ->getAllowLoc4() );
   return valRes;
}

Locator & Locator::operator =(const Locator &rhs)
{
    loc.setValue(rhs.loc);
    validate();
    return *this;
}
char Locator::validate( )
{
   double longitude = 0.0;
   double latitude = 0.0;

   valRes = lonlat( loc.getValue(), longitude, latitude, MinosParameters::getMinosParameters() ->getAllowLoc4() );

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
   sdate.setValue(rhs.sdate);
   stime.setValue(rhs.stime);
   baddtg = false;
}
dtg& dtg::operator =(const dtg&rhs)
{
   sdate.setValue(rhs.sdate);
   stime.setValue(rhs.stime);
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
Callsign& Callsign::operator =(const Callsign& rhs)
{
    fullCall.setValue(rhs.fullCall);
    valRes = CS_NOT_VALIDATED;
    validate();
    return *this;
}
Callsign::~Callsign()
{
   // nothing needed
}

static bool isMobileString ( QString p )
{
    return ( p == "P" || p == "M" || p == "MM" || p == "A" );
}
static bool isMobileNumeric ( QString p )
{
    return ( p.length() == 1 &&  p[ 0 ].isNumber() );
}
static bool isNumeric ( const QString &s )
{
    int slen = s.length();
    if ( slen == 0 )
    {
        return false;
    }
    for ( int i = 0; i < slen; i++ )
    {
        if ( !s[ i ].isNumber() )
        {
            return false;
        }
    }
    return true;
}
static QString getPrefix ( QString p, QSharedPointer<CountrySynonym> &csyn )
{
    QString testpart = p;

    while ( testpart.length() >= 1 )
    {
        // we need to stop when we get to the basic prefix...
        // otherwise RVI6ABC ends up matching R, which is UA

        csyn = MultLists::getMultLists()->searchCountrySynonym ( testpart );

        if ( csyn )
        {
            break;
        }

        testpart = testpart.left (testpart.length() - 1 );
    }
    return testpart;
}

static int extraTail ( QString p, QSharedPointer<CountrySynonym> &csyn )
{

    QString testpart = getPrefix ( p, csyn );
    if ( testpart.length() == 0 )
    {
        return -1;   // we didn't find a synonym
    }
    int tail =  p.length() - testpart.length();
    return tail;
}

bool Callsign::isValidStructure()
{
    return isValidStructure( dupPrefix, number, body );
}
/*static*/ bool Callsign::isValidStructure( const QString &prefix,  const QString &number,  const QString &body )
{
    if ( prefix.length() > 0 && number.length() > 0 )
    {
        if ( body.length() > 0)// || ( body.length() == 0 && prefix == "LM" && number == "1814" ) )
        {
            return true;
        }
        return false;
    }
    return false;
}

int Callsign::validate( )
{
    if ( valRes != CS_NOT_VALIDATED )
        return valRes;

    QString cs = fullCall.getValue();

    valRes = ERR_NOCS;
    int cslen = cs.length();
    if ( cslen == 0 )    			// all spaces
        return valRes;

    // first check no funny characters; only alphanum and / allowed
    // (spaces are now invalid)

    valRes = ERR_INVCS;

    for ( int i = 0; i < cslen; i++ )
    {
        if ( ( cs[ i ] != '/' ) && ( !cs[ i ].isLetterOrNumber() ) )
        {
            return valRes;
        }
    }

    locCtryPrefix.clear();
    dupPrefix.clear();
    suffix.clear();
    number.clear();
    body.clear();

    // Break cs into it's 3 possible parts as divided by "/"
    QString p1;
    QString p2;
    QString p3;
    int spos = cs.indexOf( "/" );
    if ( spos != -1 )
    {
        p1 = cs.left(spos );
        cs = cs.mid ( spos + 1 );
        spos = cs.indexOf ( "/" );
        if ( spos != -1 )
        {
            p2 = cs.left ( spos );
            p3 = cs.mid ( spos + 1 );
        }
        else
        {
            p2 = cs;
        }
    }
    else
    {
        p1 = cs;
    }

    // We now need to analyse what we have

    QString countryPrefix;
    QString call;
    QString mobileSuffix;

    int p1size = p1.length();
    int p2size = p2.length();

    if ( p1size == 0 )
    {
        valRes = ERR_INVCS;
        return valRes;
    }
    if ( p1size != 0 && p2size == 0 )
    {
        call = p1;  // just a single part
    }
    else
    {
        // at least two parts
        if ( isMobileString ( p3 ) )
        {
            mobileSuffix = p3;
        }
        else
            if ( isMobileNumeric ( p3 ) )
            {
                mobileSuffix = p3;
            }

        if ( isMobileString ( p2 ) )
        {
            mobileSuffix = p2;
            call = p1;
        }
        else
            if ( isMobileNumeric ( p2 ) )
            {
                mobileSuffix = p2;
                call = p1;
            }
        if ( call.isEmpty() )
        {
            // decide which is call and which is prefix - or is the whole thing broken

            QSharedPointer<CountrySynonym> csyn1;
            QSharedPointer<CountrySynonym> csyn2;
            int p1t = extraTail ( p1, csyn1 );
            int p2t = extraTail ( p2, csyn2 );

            if ( p1t == -1 || p2t == -1 )
            {
                if ( p1size <= p2size )
                {
                    countryPrefix = p1;
                    call = p2;
                }
                else
                {
                    countryPrefix = p2;
                    call = p1;
                }
            }
            else
                if ( p1t < p2t )
                {
                    // We hope that the shortest tail will be the country of location
                    countryPrefix = p1;
                    call = p2;
                }
                else
                    if ( p1t == p2t )
                    {
                        if ( p1size <= p2size )
                        {
                            countryPrefix = p1;
                            call = p2;
                        }
                        else
                        {
                            countryPrefix = p2;
                            call = p1;
                        }
                    }
                    else
                    {
                        countryPrefix = p2;
                        call = p1;
                    }
        }
    }

    locCtryPrefix = countryPrefix;
    suffix = mobileSuffix;

    // NB that leading spaces may be genuine wildcards, and so may
    // actually be significant; we should reject as invalid
    // calls with leading spaces, even though this isn't very friendly

    // first check no funny characters; only alphanum and / allowed
    // (spaces are now invalid)

    valRes = ERR_INVCS;

    QString leadPart;
    int callOffset = 0;

    while (  call[callOffset].isDigit() )    // leading digits (2 of 2E0...)
    {
        leadPart += call[callOffset++];
    }

    int csize = call.length();
    while ( callOffset < csize )            // prefix letters (E of 2E0)
    {
        if ( call[callOffset].isDigit() )
        {
            break ;
        }
        else
        {
            leadPart += call[callOffset++];
        }
    }

    // with e.g. RT7T leadpart is RT but prefix3 is R, as cty.dat doesn't have RT
    // so it gets reduced to R. country of location is R but we want dupprefix to be RT
    // With GW4ALG location is GW but dupPrefix needs to be G

    QSharedPointer<CountrySynonym> csyn;
    QString prefix3 = getPrefix ( call, csyn );  // strip call back until we find it in the control file
    if ( prefix3.length() )
    {
        if ( locCtryPrefix.length() == 0 )
        {
            // locCtryPrefix is country of location
            // dupPrefix is country of issue
            // prefix3 is based on the issue country - but we have no xx/ leadin
            // so loc and issue are the same

            locCtryPrefix = prefix3;

            // What should we do with e.g. NH6T/W4
            if ( isNumeric ( suffix ))    //eg for w3abc/6 US call areas changes w3 to w6
            {
                locCtryPrefix += suffix;
            }
        }
    }

    if ( dupPrefix.isEmpty() )  // it SHOULD be empty!
    {
        dupPrefix = leadPart;
    }


    while ( call[callOffset].isDigit() )     // central number (0 of 2E0)
        number += call[callOffset++];


    // main CS letters
    body = call.mid(callOffset);

    realCall = dupPrefix + number + body;

    for (int i = 0; i < body.count(); i++)
    {
        if (body[i].isDigit())
        {
            valRes = ERR_INVCS;
            return valRes;
        }
    }

    valRes = ERR_CSDIGIT;
    if ( number.length() == 0 )
        return valRes;

    // Replace real dupPrefix with the dup check dupPrefix - for dup checking

    QSharedPointer<CountrySynonym> syn = MultLists::getMultLists()->searchCountrySynonym ( dupPrefix );
    if ( syn )
    {
        syn->getDupPrefix ( dupPrefix );
    }

    valRes = CS_OK;
    if (!isValidStructure() ||  locCtryPrefix.length() == 0 )   	// suffix can be null
    {
        valRes = ERR_INVCS;
    }
    if (fullCall.getValue().right(1) == "/" )
    {
        valRes = ERR_INVCS;
    }

    return valRes;

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

   int ret = c1->dupPrefix.compare(c2->dupPrefix, Qt::CaseInsensitive );
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
    int ret = c1->dupPrefix.compare(c2->dupPrefix, Qt::CaseInsensitive );
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
