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

enum SPOT_TYPE {NONE, CLUSTER, CLUSTER_MARKED, LOGGED, MARKED, SAVED, CQ};

}


class BandmapData
{


public:
    BandmapData();
    BandmapData(qint64 _rxTime, QString _spotTime,
             QString _dxFreqStr, qint64 dxFreq, QString _dxBand, QString _dxBandMaskStr,
             QString _dxMode, QString _dxModeMaskStr, QString _dxCall,
             bool _dxCallWorked, QString _dxLocator,
             bool _dxLocatorWorked, QString _distance,
             QString _bearing, QString _rotBrg, bool _rotConnected, QString _spotterCall,
             QString _spotterLocator, QString _dxPropMode,
             bool _runModeOn, bool _offRunFreq,
             QString comment, bandmapSpotType::SPOT_TYPE _spotType);

    void operator = (const BandmapData& bmd);
    void operator = (const BandmapData* bmd);

    qint64 rxTime;
    QString spotTime;
    QString dxFreqStr;
    qint64 dxFreq;
    QString dxBand;
    QString dxBandMaskStr;
    QString dxMode;
    QString dxModeMaskStr;
    QString dxCall;
    QString dxLocator;
    QString dxDist;
    QString dxBrg;
    QString rotBrg;
    bool rotConnected;
    bool dxCallWorked;
    bool dxLocatorWorked;
    bool sentToMemory;
    QString spotterCall;
    QString spotterLocator;
    QString dxPropMode;
    bool runModeOn;
    bool offRunFreq;
    QString spotComment;
    bandmapSpotType::SPOT_TYPE spotType;
    bool isSelected;




    void clear();
};



#endif // BANDMAPDATA_H
