////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Cluster Mode/Bandplan
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
///
//
//
/////////////////////////////////////////////////////////////////////////////




#ifndef CLUSTERMODEBANDPLAN_H
#define CLUSTERMODEBANDPLAN_H

#include "base_pch.h"
#include "clustercommon.h"

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
    bool loadFile(QString filename);

    int confirmMode(QString &band, QString &mode, double freq);
    bool modeExists(QString &band, QString &mode);
private:

    bool readFile(QString f);
    QMap<QString, QMap<QString, ModeFreqDetail> > bandModeFreqList;
    QMap<QString, ModeFreqDetail> modeFreqList;


};

#endif // CLUSTERMODEBANDPLAN_H
