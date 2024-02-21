/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
//
// COPYRIGHT         (c) D.G. Balharrie 2024
//
/////////////////////////////////////////////////////////////////////////////



#ifndef CHECKHAMLIBVERSIONISVALID_H
#define CHECKHAMLIBVERSIONISVALID_H

#include <QString>

const QString MINIMUM_HAMLIB_VERSION = "4.6";

int checkHamlibVersionIsValid(bool &ok, QString installedVersionNumber, const QString requiredVersionNumber);

int extractNumberFromString(const QString str);






#endif // CHECKHAMLIBVERSIONISVALID_H
