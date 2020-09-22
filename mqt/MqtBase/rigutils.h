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


#include "base_pch.h"

extern const char * FREQ_EDIT_ERR_MSG;
extern const char * RADIO_FREQ_EDIT_ERR_MSG;
extern const char * TARGET_FREQ_EDIT_ERR_MSG;

QString convertSinglePeriodFreqToFullDigit(QString f);


bool validateFreqTxtInput(QString f);
bool valInputFreq(QString f, QString errMsg);

QString convertSinglePeriodFreqToMultiPeriod(QString f);
QString convertFreqToFullDigit(QString f);

QString convertRitFreqToStr(const ShortFreq &freq, bool ritKHzFlag);

QString removeHundredHzAndHzDigits(Frequency f);

#endif // RIGUTILS_H
