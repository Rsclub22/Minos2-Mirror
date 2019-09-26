#include "checkoperatingfreq.h"

CheckOperatingFreq::CheckOperatingFreq()
{

}


int CheckOperatingFreq::freqValid(const QString &band, const QString &mode, const double freq)
{
    QMap<QString, ModeFreqDetail> modeList;
    ModeFreqDetail freqs;


    if (bandModeFreqList.contains(band))
    {
        modeList = bandModeFreqList.value(band);
        if (modeList.contains(mode))
        {
            freqs = modeList.value(mode);
            for (int i = 0; i < freqs.freq.count(); i++)
            {
                QList< double > freqLimits = freqs.freq[i];
                if (freq >= freqLimits[0] && freq <= freqLimits[1])
                {
                    return FREQ_OK;

                }
            }

            return FREQ_NO_MATCH;
        }

        return MODE_MISSING;
    }

    return BAND_MISSING;
}

bool CheckOperatingFreq::modeExists(const QString &band, const QString &mode)
{
    QMap<QString, ModeFreqDetail> modeList;

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
