#ifndef FREQMODEBANDPLAN_H
#define FREQMODEBANDPLAN_H

#include "base_pch.h"
#include "clustercommon.h"


template <class T>
class ModeFreqDetail
{
public:

    QList<QList<T>> freq;

    int count() {return freq.count();}
};

class freqModeBandPlan: public QObject
{
public:
    freqModeBandPlan();

    bool loadBandsFromBandList();
    bool loadExclusionsFromBandList();

    int confirmMode(QString &band, QString &mode, Frequency freq);
    bool modeExists(QString &band, QString &mode);
    bool checkLoadedOk();

protected:
    QMap<QString, QMap<QString, ModeFreqDetail<Frequency>> > bandModeFreqList;

private:

    bool loadedOk;
    void addPair(ModeFreqDetail<Frequency> &mfl, Frequency fLow, Frequency fHigh);
};

#endif // FREQMODEBANDPLAN_H
