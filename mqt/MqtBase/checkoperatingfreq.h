#ifndef CHECKOPERATINGFREQ_H
#define CHECKOPERATINGFREQ_H

#include "freqmodebandplan.h"


enum checkOperatingFreqError {NO_FILE, FREQ_OK, FREQ_NOT_OK, MODE_MISSING, BAND_MISSING};


class CheckOperatingFreq : public freqModeBandPlan
{
public:
    CheckOperatingFreq();

    int freqValid(const QString &band, const QString &mode, const double freq);
    bool modeExists(const QString &band, const QString &mode);
    int getFreqLimitsForDial(ModeFreqDetail<double> &listOfFreqs, const QString &band, const QString &mode);
};

#endif // CHECKOPERATINGFREQ_H
