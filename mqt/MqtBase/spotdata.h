////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Cluster Client
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
///
//
//
/////////////////////////////////////////////////////////////////////////////



#ifndef SPOTDATA_H
#define SPOTDATA_H

#include <QString>
#include "clustercommon.h"

class SpotData
{
public:
    SpotData();
    SpotData(SpotData &spd);
    SpotData(qint64 _rxTime, QString _spotTime,
             QString _dxFreq, QString _dxBandStr, QString _dxBandMask,
             QString _dxMode, QString _dxModeMaskStr,
             QString _dxCall, bool _dxCallWorked, QString _dxLocator,
             bool _dxLocatorWorked, QString distance,
             QString bearing, QString _spotterCall,
             QString _spotterLocator, QString _dxPropMode, QString comment);

    void operator = (const SpotData& spd);

    void clear();

    void setRxTime(qint64 rxTime_){rxTime = rxTime_;}
    qint64 getRxTime(){return rxTime;}

    void setDxCall(QString dxCall_){dxCall = dxCall_;}
    QString getDxCall(){return dxCall;}

    void setDxFreq(QString dxFreq_){dxFreq = dxFreq_;}
    QString getDxFreq(){return dxFreq;}

    void setDxBandStr(QString dxBandStr_){dxBandStr = dxBandStr_;}
    QString getDxBandStr(){return dxBandStr;}

    void setDxBandMask(QString dxBandMask_){dxBandMask = dxBandMask_;}
    QString getDxBandMask(){return dxBandMask;}

    void setDxModeStr(QString dxModeStr_){dxModeStr = dxModeStr_;}
    QString getDxModeStr(){return dxModeStr;}

    void setDxModeMask(QString dxModeMask_){dxModeMask = dxModeMask_;}
    QString getDxModeMask(){return dxModeMask;}

    void setSpotterCall(QString spotterCall_){spotterCall = spotterCall_;}
    QString getSpotterCall(){return spotterCall;}

    void setSpotComment(QString spotComment_){spotComment = spotComment_;}
    QString getSpotComment(){return spotComment;}

    void setSpotTime(QString spotTime_){spotTime = spotTime_;}
    QString getSpotTime(){return spotTime;}

    void setSpotDate(QString spotDate_){spotDate = spotDate_;}
    QString getSpotDate(){return spotDate;}

    void setSpotDateTime(QDateTime spotDateTime_){spotDateTime = spotDateTime_;}
    QDateTime getSpotDateTime(){return spotDateTime;}


    void setDxLocator(QString dxLocator_){spotDate = dxLocator_;}
    QString getDxLocator(){return dxLocator;}

    void setSpotterLocator(QString spotterLocator_){spotterLocator = spotterLocator_;}
    QString getSpotterLocator(){return spotterLocator;}

    void setDxPropMode(QString dxPropMode_){dxPropMode = dxPropMode_;}
    QString getDxPropMode(){return dxPropMode;}

    void setDxDist(QString dxDist_){dxDist = dxDist_;}
    QString getDxDist(){return dxDist;}

    void setDxBrg(QString dxBrg_){dxBrg = dxBrg_;}
    QString getDxBrg(){return dxBrg;}

    void setDxCallWorked(bool dxCallWorked_){dxCallWorked = dxCallWorked_;}
    bool getDxCallWorked(){return dxCallWorked;}

    void setDxLocatorWorked(bool dxLocatorWorked_){dxLocatorWorked = dxLocatorWorked_;}
    bool getDxLocatorWorked(){return dxLocatorWorked;}

    void setSentToMemory(bool sentToMemory_){sentToMemory = sentToMemory_;}
    bool getSentToMemory(){return sentToMemory;}



private:

    qint64 rxTime = 0;
    QString spotTime;
    QString spotDate;
    QDateTime spotDateTime;
    QString dxFreq;
    QString dxBandStr;
    QString dxBandMask;
    QString dxModeStr;
    QString dxModeMask;
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









};

#endif // SPOTDATA_H




