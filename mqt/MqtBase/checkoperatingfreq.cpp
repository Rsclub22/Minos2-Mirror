#include "checkoperatingfreq.h"

CheckOperatingFreq::CheckOperatingFreq()
{

}


int CheckOperatingFreq::freqValid(const QString &band, const QString &mode, const double freq)
{
    QMap<QString, ModeFreqDetail<double>> modeList;
    ModeFreqDetail<double> freqs;


    if (bandModeFreqList.contains(band))
    {
        modeList = bandModeFreqList.value(band);
        if (modeList.contains(mode))
        {
            freqs = modeList.value(mode);
            for (int i = 0; i < freqs.freq.count(); i++)
            {
                QList< double > freqLimits = freqs.freq[i];
                if (freq >= (freqLimits[0] * 1000) && freq < (freqLimits[1] * 1000))
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
    QMap<QString, ModeFreqDetail<double>> modeList;

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


int CheckOperatingFreq::getFreqLimitsForDial(ModeFreqDetail<double> &listOfFreqs, const QString &band, const QString &mode)
{
    QMap<QString, ModeFreqDetail<double>> modeList;


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
