/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
//
// COPYRIGHT         (c) D.G. Balharrie 2024
//
/////////////////////////////////////////////////////////////////////////////

#include "checkHamlibVersionIsValid.h"




int checkHamlibVersionIsValid(bool &ok, QString installedVersionNumber, const QString requiredVersionNumber)
{

    ok = false;
    int installedVerNum = 0;
    int miniMumVerNum = 0;

    installedVerNum = extractNumberFromString(installedVersionNumber);
    miniMumVerNum = extractNumberFromString(requiredVersionNumber);

    if (installedVerNum == 0 || miniMumVerNum == 0)
    {
        ok = false;
        return -1;

    }

    if (installedVerNum < miniMumVerNum)
    {
        ok = false;
        return -2;

    }
    else
    {

        ok = true;
        return 0;

    }




    return -3;
}

int extractNumberFromString(const QString str)
{

    QString numberFromString;
    int n = 0;

    for (const QChar& digit : str)
    {
        if (digit >= '0' && digit <= '9')
        {
            numberFromString.append(digit);
        }
    }

    if (!numberFromString.isEmpty())
    {
        if (numberFromString.length() == 1)
        {
            numberFromString.append("00");
        }
        else if (numberFromString.length() == 2)
        {
            numberFromString.append("0");
        }

        n = numberFromString.toInt();
    }

    return n;


}
