#include "checkoperatingfreq.h"

CheckOperatingFreq::CheckOperatingFreq()
{

}


int CheckOperatingFreq::freqValid(const QString &band, const QString &mode, const Frequency &freq)
{
    QMap<QString, ModeFreqDetail<Frequency>> modeList;
    ModeFreqDetail<Frequency> freqs;

    QString m = mode;
    m = m.remove(':');

    if (bandModeFreqList.contains(band))
    {
        modeList = bandModeFreqList.value(band);
        if (modeList.contains(m))
        {
            freqs = modeList.value(m);
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


int CheckOperatingFreq::getFreqLimitsForDial(ModeFreqDetail<Frequency> &listOfFreqs, const QString &band, const QString &pmode)
{
    QMap<QString, ModeFreqDetail<Frequency>> modeList;

    QString mode(pmode);
    if (bandModeFreqList.contains(band))
    {
        if (mode == "PH")
        {
            auto modeTestFreq = bandModeFreqList.value(band).begin()->freq.at(0).at(0);
            if (modeTestFreq > Frequency(10000000))
            {
                mode = "USB";
            }
            else
            {
                mode = "LSB";
            }
        }

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
