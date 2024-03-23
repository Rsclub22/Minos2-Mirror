#include "QtUtils.h"
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
    for (auto const &i: QASCONST(contestMatchList) )
    {
        cc += i.wt->contactMatchList.size();
    }
    return cc;
}

