#include "QtUtils.h"
#include "checkmodeagainstfreq.h"
#include "clustercommon.h"
#include "rigcontrolcommonconstants.h"

checkModeAgainstFreq::checkModeAgainstFreq()
{

}

QString checkModeAgainstFreq::getMode(const QString &band, Frequency freq)
{
    if (bandModeFreqList.contains(band))
    {
        QMap<QString, ModeFreqDetail<Frequency>> modeList = bandModeFreqList.value(band);

        for (QMap<QString, ModeFreqDetail<Frequency> >::const_iterator i = modeList.constBegin(); i != modeList.constEnd(); i++)
        {
            ModeFreqDetail<Frequency> freqs = i.value();
            for (auto const &freqLimits: QASCONST(freqs.freq))
            {
                if (freqLimits.count() == 0)
                    continue;
                if (freq >= freqLimits[0] && freq <= freqLimits[1])
                {
                    if (i.key() == hamlibData::MGM)
                    {
                        continue;
                    }
                    return i.key();       // found mode
                }
            }
        }
    }

    return NONE_MODE;
}
