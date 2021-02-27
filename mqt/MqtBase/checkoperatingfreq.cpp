#include "checkoperatingfreq.h"

CheckOperatingFreq::CheckOperatingFreq()
{

}


int CheckOperatingFreq::freqValid(const QString &band, const QString &mode, const Frequency &freq)
{
    QMap<QString, ModeFreqDetail<Frequency>> modeList;
    ModeFreqDetail<Frequency> freqs;


    if (bandModeFreqList.contains(band))
    {
        modeList = bandModeFreqList.value(band);
        if (modeList.contains(mode))
        {
            freqs = modeList.value(mode);
            for (auto const &freqLimits: qAsConst(freqs.freq))
            {
                if (freqLimits.count() == 0)
                    continue;
                if (freq >= (freqLimits[0]) && freq < (freqLimits[1]))
                {
                    return FREQ_NOT_OK;

                }
            }

            return FREQ_OK;
        }

        return MODE_MISSING;
    }

    return BAND_MISSING;
}

bool CheckOperatingFreq::modeExists(const QString &band, const QString &mode)
{
    QMap<QString, ModeFreqDetail<Frequency>> modeList;

    if (bandModeFreqList.contains(band))
    {
        modeList = bandModeFreqList.value(band);
        if (modeList.contains(mode))
        {
            return true;
        }
    }

    return false;
}


int CheckOperatingFreq::getFreqLimitsForDial(ModeFreqDetail<Frequency> &listOfFreqs, const QString &band, const QString &mode)
{
    QMap<QString, ModeFreqDetail<Frequency>> modeList;


    if (bandModeFreqList.contains(band))
    {
        modeList = bandModeFreqList.value(band);
        if (modeList.contains(mode))
        {
            listOfFreqs = modeList.value(mode);

            return FREQ_OK;
        }

        return MODE_MISSING;
    }

    return BAND_MISSING;

}
