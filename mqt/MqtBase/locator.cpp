#include "locator.h"

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

