/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) D G Balharrie M0DGB/G8FKH
//
/////////////////////////////////////////////////////////////////////////////

#ifndef RIGUTILS_H
#define RIGUTILS_H

#include <QString>

QString convertFreqStrDisp(QString frequency);

QString convertFreqToStr(double frequency);

double convertStrToFreq(QString frequency);

#endif // RIGUTILS_H
