/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef LatlongH
#define LatlongH 

#include "base_pch.h"
//----------------------------------------------------------------------------
#define GRIDOK 10000
#define INVALIDGREF 10001

enum gridstyle {NUL, GEO, OWN, NGR, LOC};

struct Location
{
   enum gridstyle gridstyle;
   QString datastring;
   double northing, easting, centremeridian;
};

void dms( double, int *, int *, double * );
int transform( Location *, Location * );
double raddeg( double );
double degrad( double );
QString txgeoloc( double *n, double *e, int f, char t);
int geotoloc( double lat, double longi, QString &gridref );

#endif
