////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Bandmap
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
///
//
//
/////////////////////////////////////////////////////////////////////////////



#ifndef BANDMAPDATA_H
#define BANDMAPDATA_H

#include <QString>

namespace bandmapSpotType {

enum SPOT_TYPE {NONE, CLUSTER, LOGGED, MARKED, SAVED};

}


class BandmapData
{


public:
    BandmapData();
    BandmapData(qint64 _rxTime, QString _spotTime,
             QString _dxFreqStr, qint64 dxFreq, QString _dxFreqMaskStr,
             QString _dxMode, QString _dxModeMaskStr, QString _dxCall,
             bool _dxCallWorked, QString _dxLocator,
             bool _dxLocatorWorked, QString distance,
             QString bearing, QString _spotterCall,
             QString _spotterLocator, QString _dxPropMode,
             QString comment, bandmapSpotType::SPOT_TYPE _spotType);

    void operator = (const BandmapData& bmd);

    qint64 rxTime;
    QString spotTime;
    QString dxFreqStr;
    qint64 dxFreq;
    QString dxFreqMaskStr;
    QString dxMode;
    QString dxModeMaskStr;
    QString dxCall;
    QString dxLocator;
    QString dxDist;
    QString dxBrg;
    bool dxCallWorked;
    bool dxLocatorWorked;
    bool sentToMemory;
    QString spotterCall;
    QString spotterLocator;
    QString dxPropMode;
    QString spotComment;
    bandmapSpotType::SPOT_TYPE spotType;
    bool isSelected;




};



#endif // BANDMAPDATA_H
