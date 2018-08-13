/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"
#include "cutils.h"
/******************************************************/
/*                                                    */
/* convert Maidenhead locator to longtitude and       */
/* latitude                                           */
/*                                                    */
/* returns 0 if OK, else 1 if source locator in error */
/*                                                    */
/* Latitude is NS, Longtitude is EW                   */
/*                                                    */
/******************************************************/
char lonlat( const QString &inqra, double &lon, double &lat )
{
   // defaulted to square centres - LL55LL55
   double ft[ 8 ] = {0.0, 0.0, 5.0, 5.0, 11.0, 11.0, 5.0, 5.0} ; /* work values for locator positions */
   double *fp1 ;

   int l ;

   for ( l = inqra.length() - 1; l >= 0; l-- )
   {
      if ( inqra[ l ] != ' ' )
         break;
   }
   if ( l <= 0 )
	  return ( ERR_NOLOC );

   char locres = LOC_OK;

   l++;

   /* Process Maidenhead locator */
   if ( l <= 8 && l >= 2 )      /* maidenhead locator */
   {
      const QString lowval = "AA00AA00";   // make sure this is all after the
      const QString hival = "RR99XX99";    // length check

      const QString qra = inqra.left(l).toUpper();


      /* set up for conversion */
      fp1 = ft ;                       /* set to front of converted table */

      /* validate */

      for (int i = 0; i < l; i++)
      {
         QChar qi = qra[i];
         if ( qi == ' ' )
         {
            if ( i <= 2 )
               return ERR_LOC_RANGE;
            locres = LOC_PARTIAL;

			fp1++;	// already set to centre
         }
         else
         {
            if ( qi < lowval[i] || qi > hival[i] )     /* out of range */
               return ( ERR_LOC_RANGE );                 /* invalid locator */
            else
               *( fp1++ ) = qi.unicode() - lowval[i].unicode(); /* convert to double */
		 }

      }

      // NB for short locs we have already set up ft

      /* calculate longtitude and latitude from maidenhead locator*/
      double wfe1 = ft[ 0 ] * 20.0 + ft[ 2 ] * 2.0 - 180.0 ;
      double wfe2 = ft[ 4 ] / 12.0 + ft[ 6 ] / 120.0;
      double wfe = wfe1 + wfe2 ;

      double wfn1 = ft[ 1 ] *  10.0 + ft[ 3 ] - 90.0 ;
      double wfn2 = ft[ 5 ] / 24.0 + ft[ 7 ] / 240.0;
      double wfn = wfn1 + wfn2;

      lat = wfn ;                  /* set up latitude */
      lon = wfe ;                  /* set up longtitude */
   }
   else                                /* invalid locator */
      return ( ERR_LOC_RANGE );                       /* so bug out */

   /* clean exit */
   lon = lon * dr ;                /* hold ready for calculation */
   lat = lat * dr ;

   bool allowLoc4 = MinosParameters::getMinosParameters() ->getAllowLoc4();

   if ( l == 4 && allowLoc4 )
   {
      return locres;
   }
   if ( l == 6 || l == 8 )
   {
	  return locres;
   }
   return LOC_PARTIAL;
}

QString WtodbW(const QString &W )
{
   QString res;
   double watts =W.toDouble();
   if ( watts >= 0.00000001 )
   {
      double dbWatts = 10.0 * log10( watts );
      res = QString::number( dbWatts );
   }
   return res;
}

QString dbWtoW(const QString &dbW )
{
   double dbWatts = dbW.toDouble();
   double watts = pow( 10.0, dbWatts / 10.0 ) + 0.05;
   QString res = QString::number(watts );
   return res;
}

int normBrg( int mbear )
{
   if ( mbear < 0 )
      mbear += 360;
   if ( mbear >= 360 )
      mbear -= 360;
   return mbear;
}
int varBrg( int mbear )
{
   mbear += MinosParameters::getMinosParameters() ->getMagneticVariation();
   return normBrg( mbear );
}

