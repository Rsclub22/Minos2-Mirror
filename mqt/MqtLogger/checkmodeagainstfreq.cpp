#include "checkmodeagainstfreq.h"

checkModeAgainstFreq::checkModeAgainstFreq()
{

}



QString checkModeAgainstFreq::getMode(QString &band, double freq)
{
    QMap<QString, ModeFreqDetail> modeList;
    ModeFreqDetail freqs;

    if (bandModeFreqList.contains(band))
    {

        modeList = bandModeFreqList.value(band);
        QMapIterator<QString, ModeFreqDetail> i(modeList);
        while (i.hasNext())
        {
            i.next();
            freqs = i.value();
            for (int fi = 0; fi < freqs.freq.count(); fi++)
            {
                QList<double> freqLimits = freqs.freq[fi];
                if (freq >= freqLimits[0] && freq <= freqLimits[1])
                {
                    return i.key();       // found mode
                }
            }
        }

    }

    return "";
}


int checkModeAgainstFreq::confirmMode(QString &band, QString &mode, double freq)
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
