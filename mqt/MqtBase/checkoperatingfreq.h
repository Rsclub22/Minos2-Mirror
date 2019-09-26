#ifndef CHECKOPERATINGFREQ_H
#define CHECKOPERATINGFREQ_H

#include "freqmodebandplan.h"


enum checkOperatingFreqError {NO_FILE, FREQ_OK, FREQ_NO_MATCH, MODE_MISSING, BAND_MISSING};


class CheckOperatingFreq : public freqModeBandPlan
{
public:
    CheckOperatingFreq();

    int freqValid(const QString &band, const QString &mode, const double freq);
    bool modeExists(const QString &band, const QString &mode);
};

#endif // CHECKOPERATINGFREQ_H
