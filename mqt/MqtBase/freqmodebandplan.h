#ifndef FREQMODEBANDPLAN_H
#define FREQMODEBANDPLAN_H

#include "base_pch.h"
#include "clustercommon.h"



class ModeFreqDetail
{
    //QString band;
    //QString mode;
public:

    QList<QList<double>> freq;

};



class freqModeBandPlan
{
public:
    freqModeBandPlan();

    bool loadFile(QString filename);

    int confirmMode(QString &band, QString &mode, double freq);
    bool modeExists(QString &band, QString &mode);

protected:
    QMap<QString, QMap<QString, ModeFreqDetail> > bandModeFreqList;
    QMap<QString, ModeFreqDetail> modeFreqList;

private:

    bool readFile(QString f);




};

#endif // FREQMODEBANDPLAN_H
