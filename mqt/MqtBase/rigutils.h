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

QString convertFreqStrDisp(const Frequency &frequency);
QString convertFreqStrDispSingle(const Frequency &frequency);
QString convertFreqStrDispSingleNoTrailZero(const Frequency &frequency);

QString convertFreqToStr(const Frequency &frequency);

QString convertSinglePeriodFreqToFullDigit(QString f);

Frequency convertStrToFreq(QString frequency);

QString removeTrailingZeroes(QString sfreq);

//QString validateFreqTxtInput(QString f, bool* ok);
bool validateFreqTxtInput(QString f);
bool valInputFreq(QString f, QString errMsg);

QString convertSinglePeriodFreqToMultiPeriod(QString f);
QString convertFreqToFullDigit(QString f);

QString convertRitFreqToStr(const ShortFreq &freq, bool ritKHzFlag);

QString convertKhzToMhz(QString f);
QString alignFreqRight(QString f);

QString removeHundredHzAndHzDigits(QString f);

QString extractKhz(Frequency f);

#endif // RIGUTILS_H
