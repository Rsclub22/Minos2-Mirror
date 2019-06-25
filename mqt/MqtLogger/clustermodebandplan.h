#ifndef CLUSTERMODEBANDPLAN_H
#define CLUSTERMODEBANDPLAN_H

#include "base_pch.h"

class ModeFreqDetail
{
    //QString band;
    //QString mode;
public:

    QList<QList<double>> freq;

};






class ClusterModeBandPlan
{
public:
    ClusterModeBandPlan();
    bool loadFile();

    //QMap<QString, QVector<ModeFreqDetail>*> bandModeFreqList; //list of bands and modes/freq
    // QMap<Band, QMap<Mode, list of freq>
    //QMap<QString, QMap<QString, ModeFreqDetail*>*> bandModeFreqList;
private:

    bool readFile(QString f);
    QMap<QString, QMap<QString, ModeFreqDetail>> bandModeFreqList;
    QMap<QString, ModeFreqDetail> modeFreqList;


};

#endif // CLUSTERMODEBANDPLAN_H
