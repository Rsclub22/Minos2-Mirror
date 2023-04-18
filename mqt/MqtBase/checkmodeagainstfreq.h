#ifndef CHECKMODEAGAINSTFREQ_H
#define CHECKMODEAGAINSTFREQ_H

#include "freqmodebandplan.h"

class checkModeAgainstFreq : public freqModeBandPlan
{
    Q_OBJECT
public:
    checkModeAgainstFreq();
    QString getMode(const QString &band, Frequency freq);
};

#endif // CHECKMODEAGAINSTFREQ_H
