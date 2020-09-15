#ifndef FREQMODEBANDPLAN_H
#define FREQMODEBANDPLAN_H

#include "base_pch.h"
#include "clustercommon.h"


template <class T>
class ModeFreqDetail
{
    //QString band;
    //QString mode;
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

    int confirmMode(QString &band, QString &mode, double freq);
    bool modeExists(QString &band, QString &mode);
    bool checkLoadedOk();

protected:
    QMap<QString, QMap<QString, ModeFreqDetail<double>> > bandModeFreqList;

private:

//    bool readFile(QString f);
    bool loadedOk;




};

#endif // FREQMODEBANDPLAN_H
