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



class freqModeBandPlan
{
    Q_DECLARE_TR_FUNCTIONS(freqModeBandPlan)

public:
    freqModeBandPlan();

    bool loadFile(QString filename);

    int confirmMode(QString &band, QString &mode, double freq);
    bool modeExists(QString &band, QString &mode);
    bool checkLoadedOk();

protected:
    QMap<QString, QMap<QString, ModeFreqDetail<double>> > bandModeFreqList;
    QMap<QString, ModeFreqDetail<double>> modeFreqList;

private:

    bool readFile(QString f);
    bool loadedOk;




};

#endif // FREQMODEBANDPLAN_H
