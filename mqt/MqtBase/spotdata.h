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

    SpotData(qint64 _rxTime, QString _spotTime, QString _spotDate,
             QString _dxFreq, QString _dxBandStr, QString _dxBandMask,
             QString _dxMode, QString _dxModeMaskStr,
             QString _dxCall, bool _dxCallWorked, QString _dxLocator,
             bool _dxLocatorWorked, QString distance,
             QString bearing, QString _spotterCall,
             QString _spotterLocator, QString _dxPropMode, QString comment);

    SpotData(const SpotData &spd);

    void operator = (const SpotData& spd);

    void clear();

    void setRxTime(qint64 rxTime_){rxTime = rxTime_;}
    qint64 getRxTime()const {return rxTime;}

    void setDxCall(const QString dxCall_){dxCall = dxCall_.trimmed();}
    QString getDxCall()const {return dxCall;}

    void setDxFreq(const QString dxFreq_){dxFreq = dxFreq_.trimmed();}
    QString getDxFreq()const {return dxFreq;}

    void setDxBandStr(const QString dxBandStr_){dxBandStr = dxBandStr_.trimmed();}
    QString getDxBandStr()const {return dxBandStr;}

    void setDxBandMask(const QString dxBandMask_){dxBandMask = dxBandMask_.trimmed();}
    QString getDxBandMask()const {return dxBandMask;}

    void setDxModeStr(const QString dxModeStr_){dxModeStr = dxModeStr_.trimmed();}
    QString getDxModeStr()const {return dxModeStr;}

    void setDxModeMaskStr(const QString dxModeMaskStr_){dxModeMaskStr = dxModeMaskStr_.trimmed();}
    QString getDxModeMaskStr()const {return dxModeMaskStr;}

    void setSpotterCall(const QString spotterCall_){spotterCall = spotterCall_.trimmed();}
    QString getSpotterCall()const {return spotterCall;}

    void setSpotComment(const QString spotComment_){spotComment = spotComment_.trimmed();}
    QString getSpotComment()const {return spotComment;}

    void setSpotTime(const QString spotTime_){spotTime = spotTime_.trimmed();}
    QString getSpotTime()const {return spotTime;}

    void setSpotDate(const QString spotDate_){spotDate = spotDate_.trimmed();}
    QString getSpotDate()const {return spotDate;}


    void setDxLocator(const QString dxLocator_){dxLocator = dxLocator_.trimmed();}
    QString getDxLocator() const {return dxLocator;}

    void setDxLocatorFromQrz(const bool dxLocatorFromQrz_){dxLocatorFromQrz = dxLocatorFromQrz_;}
    bool getDxLocatorFromQrz()const {return dxLocatorFromQrz;}

    void setAskQrz(const bool askQrz_){askQrz = askQrz_;}
    bool getAskQrz()const {return askQrz;}


    void setSpotterLocator(const QString spotterLocator_){spotterLocator = spotterLocator_.trimmed();}
    QString getSpotterLocator() const {return spotterLocator;}

    void setDxPropMode(const QString dxPropMode_){dxPropMode = dxPropMode_.trimmed();}
    QString getDxPropMode() const {return dxPropMode;}

    void setDxDist(const QString dxDist_){dxDist = dxDist_.trimmed();}
    QString getDxDist() const {return dxDist;}

    void setDxBrg(const QString dxBrg_){dxBrg = dxBrg_.trimmed();}
    QString getDxBrg() const {return dxBrg;}

    void setDxCallWorked(const bool dxCallWorked_){dxCallWorked = dxCallWorked_;}
    bool getDxCallWorked() const {return dxCallWorked;}

    void setDxLocatorWorked(const bool dxLocatorWorked_){dxLocatorWorked = dxLocatorWorked_;}
    bool getDxLocatorWorked()const {return dxLocatorWorked;}

    void setSentToMemory(const bool sentToMemory_){sentToMemory = sentToMemory_;}
    bool getSentToMemory() const {return sentToMemory;}



private:

    qint64 rxTime;
    QString spotTime;
    QString spotDate;
    QString dxFreq;
    QString dxBandStr;
    QString dxBandMask;
    QString dxModeStr;
    QString dxModeMaskStr;
    QString dxCall;
    QString dxLocator;
    bool dxLocatorFromQrz;
    bool askQrz;
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




