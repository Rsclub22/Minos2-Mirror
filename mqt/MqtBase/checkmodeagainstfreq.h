#ifndef CHECKMODEAGAINSTFREQ_H
#define CHECKMODEAGAINSTFREQ_H

#include "freqmodebandplan.h"

class checkModeAgainstFreq : public freqModeBandPlan
{
    Q_OBJECT
public:
    checkModeAgainstFreq();
    int confirmMode(QString &band, QString &mode, double freq);
    bool modeExists(QString &band, QString &mode);
    QString getMode(const QString &band, double freq);
};

#endif // CHECKMODEAGAINSTFREQ_H
