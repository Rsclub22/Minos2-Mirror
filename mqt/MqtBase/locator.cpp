#include "base_pch.h"
#include "locator.h"

Locator::Locator( )
{
   loc.setInitialValue( "        " );
}
//Locator::Locator(const QString & l)
//{
//    loc.setValue(l);
//    validate();
//}
// default versions are good enough for now!
Locator::Locator(const Locator& rhs)
{
    valRes = rhs.valRes;
    loc = rhs.loc;
}
Locator& Locator::operator =(const Locator& rhs)
{
    valRes = rhs.valRes;
    loc = rhs.loc;

    return *this;
}
Locator::~Locator()
{}

int Locator::validate( double &lon, double &lat )
{
   valRes = lonlat( loc.getValue(), lon, lat, MinosParameters::getMinosParameters() ->getAllowLoc4() );
   return valRes;
}


int Locator::validate( )
{
   double longitude = 0.0;
   double latitude = 0.0;

   valRes = lonlat( loc.getValue(), longitude, latitude, MinosParameters::getMinosParameters() ->getAllowLoc4() );

   return valRes;
}

