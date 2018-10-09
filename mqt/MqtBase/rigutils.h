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

const QString FREQ_EDIT_ERR_MSG = QString("Frequency has invalid characters or missing periods.\r\n\r\nThe format required is (e.g.) 1296.3, 144.32 or 144.290123\r\n");
const QString RADIO_FREQ_EDIT_ERR_MSG = QString("Radio " + FREQ_EDIT_ERR_MSG);
const QString TARGET_FREQ_EDIT_ERR_MSG = QString("Target " + FREQ_EDIT_ERR_MSG);

QString convertFreqStrDisp(QString frequency);
QString convertFreqStrDispSingle(QString sfreq);
QString convertFreqStrDispSingleNoTrailZero(QString sfreq);

QString convertFreqToStr(double frequency);
QString convertFreqToStr(int frequency);

QString convertSinglePeriodFreqToFullDigit(QString f);

double convertStrToFreq(QString frequency);

QString removeTrailingZeroes(QString sfreq);

//QString validateFreqTxtInput(QString f, bool* ok);
bool validateFreqTxtInput(QString f);
bool valInputFreq(QString f, QString errMsg);

QString convertSinglePeriodFreqToMultiPeriod(QString f);
QString convertFreqToFullDigit(QString f);

QString convertRitFreqToStr(int freq);

QString convertKhzToMhz(QString f);
QString alignFreqRight(QString f);


#endif // RIGUTILS_H
