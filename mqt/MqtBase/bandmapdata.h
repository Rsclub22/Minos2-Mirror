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




/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                       Bandmap
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
///
//
//
/////////////////////////////////////////////////////////////////////////////


#ifndef BANDMAPDATA_H
#define BANDMAPDATA_H

#include <QString>

class BandmapData
{


public:
    BandmapData();
    BandmapData(qint64 _rxTime, QString _spotTime,
             QString _dxFreq, QString _dxFreqMaskStr,
             QString _dxModeMaskStr, QString _dxCall,
             bool _dxCallWorked, QString _dxLocator,
             bool _dxLocatorWorked, QString distance,
             QString bearing, QString _spotterCall,
             QString _spotterLocator, QString comment);

    void operator = (const BandmapData& bmd);

    qint64 rxTime;
    QString spotTime;
    QString dxFreq;
    QString dxFreqMaskStr;
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
    QString spotComment;









};



#endif // BANDMAPDATA_H
