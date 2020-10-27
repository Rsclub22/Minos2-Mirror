#include "clusterminosparameters.h"

static ClusterMinosParameters mp;

bool ClusterMinosParameters::getAllowLoc4()
{
    return true;
}
bool ClusterMinosParameters::getAllowLoc8()
{
    return true;
}

bool ClusterMinosParameters::yesNoMessage( QWidget* Owner, const QString &mess )
{
   return mShowYesNoMessage( Owner, mess );
}
void ClusterMinosParameters::mshowMessage( const QString &mess, QWidget* Owner )
{
   mShowMessage( mess, Owner );
}

