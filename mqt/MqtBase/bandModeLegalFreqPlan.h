////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      General Band/Mode/Frequency Plan Handler
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
///
//
//
/////////////////////////////////////////////////////////////////////////////




#ifndef BANDMODELEGALFREQPLAN_H
#define BANDMODELEGALFREQPLAN_H

#include "base_pch.h"
#include "clustercommon.h"

class ModeFreqDetail
{
    //QString band;
    //QString mode;
public:

    QList<QList<double>> freq;

};






class BandModeLegalFreqPlan
{
public:
    BandModeLegalFreqPlan();
    bool loadFile(QString f);

    int confirmMode(QString &band, QString &mode, double freq);
    bool modeExists(QString &band, QString &mode);
private:

    bool readFile(QString f);
    QMap<QString, QMap<QString, ModeFreqDetail> > bandModeFreqList;
    QMap<QString, ModeFreqDetail> modeFreqList;


};

#endif // BANDMODELEGALFREQPLAN_H
