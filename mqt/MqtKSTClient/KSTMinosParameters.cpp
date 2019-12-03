#include "KSTMinosParameters.h"

static KSTMinosParameters mp;

bool KSTMinosParameters::getAllowLoc4()
{
    return true;
}
bool KSTMinosParameters::getAllowLoc8()
{
    return true;
}

bool KSTMinosParameters::yesNoMessage( QWidget* Owner, const QString &mess )
{
   return mShowYesNoMessage( Owner, mess );
}
void KSTMinosParameters::mshowMessage( const QString &mess, QWidget* Owner )
{
   mShowMessage( mess, Owner );
}

