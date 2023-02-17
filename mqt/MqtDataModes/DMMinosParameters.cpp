#include "MShowMessageDlg.h"
#include "MMessageDialog.h"
#include "DMMinosParameters.h"

static DMMinosParameters mp;

bool DMMinosParameters::getAllowLoc4()
{
    return true;
}
bool DMMinosParameters::getAllowLoc8()
{
    return true;
}

bool DMMinosParameters::yesNoMessage( QWidget* Owner, const QString &mess )
{
   return mShowYesNoMessage( Owner, mess );
}
void DMMinosParameters::mshowMessage( const QString &mess, QWidget* Owner )
{
    mShowMessage( mess, Owner );
}

