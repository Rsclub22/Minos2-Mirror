#include "MatchCollection.h"

TMatchCollection::TMatchCollection( )
{
    qRegisterMetaType< SharedMatchCollection > ( "SharedMatchCollection" );
}
TMatchCollection::~TMatchCollection( )
{
}

int TMatchCollection::contactCount()
{
    int cc = 0;
    for (auto const &i: qAsConst(contestMatchList) )
    {
        cc += i.wt->contactMatchList.size();
    }
    return cc;
}

