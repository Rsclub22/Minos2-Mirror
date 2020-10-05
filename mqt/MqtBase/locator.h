#ifndef LOCATOR_H
#define LOCATOR_H

#include "XMPP_pch.h"

#define LOC_NOT_VALIDATED -1
#define LOC_OK 0
#define LOC_SHORT 1
#define LOC_PARTIAL 2
#define ERR_NOLOC 30
#define ERR_LOC_RANGE 31

class Locator
{
   public:
      MinosStringItem<QString> loc;

      mutable char valRes;

      Locator();
      Locator(const QString &);
      //    locator(const locator&);
      //    locator& operator =(const locator&);
      ~Locator();
      char validate( );
      char validate( double &lon, double &lat );
      Locator &operator =( const Locator& );


};

#endif // LOCATOR_H
