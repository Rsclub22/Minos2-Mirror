#include "cutils.h"
#include "dtg.h"
const double dtg::daySecs = 86400.0;	// 24 * 60 * 60

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
   return temp_date;
}
QString dtg::getIsoDTG( ) const
{
   bool dirty;
   return getIsoDTG( dirty );
}
QString dtg::getN1mmDTG()
{
    // 2016-04-10 16:17:41
    QString temp_date;
    QString prefix = "20";

    bool dateDirty = false;
    bool timeDirty = false;

    QString dateValue = sdate.getValue( dateDirty );    //yyMMdd
    dateValue += "            ";

    QString timeValue = stime.getValue( timeDirty );    //HHmmss
    timeValue += "            ";

    if ( dateValue [ 0 ] >= '8' )
       prefix = "19";

    dateValue = prefix + dateValue;

    temp_date += dateValue[0];
    temp_date += dateValue[1];
    temp_date += dateValue[2];
    temp_date += dateValue[3];
    temp_date += "-";
    temp_date += dateValue[4];
    temp_date += dateValue[5];
    temp_date += "-";
    temp_date += dateValue[6];
    temp_date += dateValue[7];
    temp_date += " ";
    temp_date += timeValue [ 0 ];
    temp_date += timeValue [ 1 ];
    temp_date += ':';
    temp_date += timeValue [ 2 ];
    temp_date += timeValue [ 3 ];
    temp_date += ':';
    temp_date += timeValue [ 4 ];
    temp_date += timeValue [ 5 ];

    for ( int i = 0; i < temp_date.size(); i++ )
       if ( temp_date[ i ].unicode() == 0 )
          temp_date[ i ] = ' ';

    return temp_date;

}
QString dtg::getCabrilloDTG()
{
    // 2016-04-10 1617
    QString temp_date;
    QString prefix = "20";

    bool dateDirty = false;
    bool timeDirty = false;

    QString dateValue = sdate.getValue( dateDirty );    //yyMMdd
    dateValue += "            ";

    QString timeValue = stime.getValue( timeDirty );    //HHmmss
    timeValue += "            ";

    if ( dateValue [ 0 ] >= '8' )
       prefix = "19";

    dateValue = prefix + dateValue;

    temp_date += dateValue[0];
    temp_date += dateValue[1];
    temp_date += dateValue[2];
    temp_date += dateValue[3];
    temp_date += "-";
    temp_date += dateValue[4];
    temp_date += dateValue[5];
    temp_date += "-";
    temp_date += dateValue[6];
    temp_date += dateValue[7];
    temp_date += " ";
    temp_date += timeValue [ 0 ];
    temp_date += timeValue [ 1 ];
    temp_date += timeValue [ 2 ];
    temp_date += timeValue [ 3 ];

    for ( int i = 0; i < temp_date.size(); i++ )
       if ( temp_date[ i ].unicode() == 0 )
          temp_date[ i ] = ' ';

    return temp_date;

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
   bool dirty = false;
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


