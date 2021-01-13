#include "checkmodeagainstfreq.h"

checkModeAgainstFreq::checkModeAgainstFreq()
{

}



QString checkModeAgainstFreq::getMode(const QString &band, Frequency freq)
{
    if (bandModeFreqList.contains(band))
    {
        QMap<QString, ModeFreqDetail<Frequency>> modeList = bandModeFreqList.value(band);
        QMapIterator<QString, ModeFreqDetail<Frequency>> i(modeList);
        while (i.hasNext())
        {
            i.next();
            ModeFreqDetail<Frequency> freqs = i.value();
            for (auto const &freqLimits: freqs.freq)
            {
                if (freqLimits.count() == 0)
                    continue;
                if (freq >= freqLimits[0] && freq <= freqLimits[1])
                {
                    return i.key();       // found mode
                }
            }
        }

    }

    return tr("None");
}


int checkModeAgainstFreq::confirmMode(QString &band, QString &mode, Frequency freq)
{

    QMap<QString, ModeFreqDetail<Frequency>> modeList;
    ModeFreqDetail<Frequency> freqs;


    if (bandModeFreqList.contains(band))
    {
        modeList = bandModeFreqList.value(band);
        if (modeList.contains(mode))
        {
            freqs = modeList.value(mode);
            for (auto const &freqLimits: freqs.freq)
            {
                if (freqLimits.count() == 0)
                    continue;
                if (freq >= freqLimits[0] && freq <= freqLimits[1])
                {
                    return MODE_FREQ_MATCH;

                }
            }

            return NO_MODE_FREQ_MATCH;
        }

        return MODE_NOT_FOUND;
    }

    return BAND_NOT_FOUND;
}



// return false if band or mode doesn't exist - true if mode and band exists

bool checkModeAgainstFreq::modeExists(QString &band, QString &mode)
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
