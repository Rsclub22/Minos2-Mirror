/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"
#include "contest.h"
#include "ListContact.h"
QString ListContact::getField( int ACol, const BaseContestLog *const curcon ) const
{
   // only used to get fields for main log display
   QString res;
   if ( !curcon )
      return res;

   switch ( ACol )
   {
      case egCall:
         res = cs.fullCall.getValue();
         break;
      case egLoc:
         res = loc.loc.getValue();
         break;
      case egBrg:
         {
            QString brgbuff;
            double lon = 0.0;
            double lat = 0.0;

            int lres = lonlat( loc.loc.getValue(), lon, lat );
            if ( lres == LOC_OK )
            {
               int brg;
               double dist;

               curcon->disbear( lon, lat, dist, brg );

			   int offset = curcon->bearingOffset.getValue();
               const QChar degreeChar(0260); // octal value
               brgbuff = QString( "%1%2").arg( varBrg(brg + offset), 3).arg(degreeChar);
            }
            res = brgbuff;
         }
         break;

      case egScore:
         {
            QString scorebuff;

            // we don't have it worked out already...
            double lon = 0.0;
            double lat = 0.0;
            int brg;
            double dist = 0.0;

            if ( lonlat( loc.loc.getValue(), lon, lat ) == LOC_OK )
            {
               curcon->disbear( lon, lat, dist, brg );
            }
            scorebuff = QString( "%1").arg(static_cast<int>(dist) );
            res = scorebuff;
         }
         break;

      case egExchange:
         {
            res = extraText;
         }
         break;

      case egComments:
         {
            res = comments;
         }
         break;
   }
   return res;
}

