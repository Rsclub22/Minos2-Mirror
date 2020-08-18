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
    SpotData(const SpotData &spd);

    void operator = (const SpotData& spd);

    void clear();

    void setRxTime(qint64 rxTime_){rxTime = rxTime_;}
    qint64 getRxTime()const {return rxTime;}

    void setDxCall(QString dxCall_){dxCall = dxCall_;}
    QString getDxCall()const {return dxCall;}

    void setDxFreq(QString dxFreq_){dxFreq = dxFreq_;}
    QString getDxFreq()const {return dxFreq;}

    void setDxBandStr(QString dxBandStr_){dxBandStr = dxBandStr_;}
    QString getDxBandStr()const {return dxBandStr;}

    void setDxBandMask(QString dxBandMask_){dxBandMask = dxBandMask_;}
    QString getDxBandMask()const {return dxBandMask;}

    void setDxModeStr(QString dxModeStr_){dxModeStr = dxModeStr_;}
    QString getDxModeStr()const {return dxModeStr;}

    void setDxModeMask(QString dxModeMask_){dxModeMask = dxModeMask_;}
    QString getDxModeMask()const {return dxModeMask;}

    void setSpotterCall(QString spotterCall_){spotterCall = spotterCall_;}
    QString getSpotterCall()const {return spotterCall;}

    void setSpotComment(QString spotComment_){spotComment = spotComment_;}
    QString getSpotComment()const {return spotComment;}

    void setSpotTime(QString spotTime_){spotTime = spotTime_;}
    QString getSpotTime()const {return spotTime;}

    void setSpotDate(QString spotDate_){spotDate = spotDate_;}
    QString getSpotDate()const {return spotDate;}

    void setSpotDateTime(QDateTime spotDateTime_){spotDateTime = spotDateTime_;}
    QDateTime getSpotDateTime()const {return spotDateTime;}


    void setDxLocator(QString dxLocator_){spotDate = dxLocator_;}
    QString getDxLocator() const {return dxLocator;}

    void setAskQrz(const bool askQrz_){askQrz = askQrz_;}
    bool getAskQrz()const {return askQrz;}


    void setSpotterLocator(QString spotterLocator_){spotterLocator = spotterLocator_;}
    QString getSpotterLocator() const {return spotterLocator;}

    void setDxPropMode(QString dxPropMode_){dxPropMode = dxPropMode_;}
    QString getDxPropMode() const {return dxPropMode;}

    void setDxDist(QString dxDist_){dxDist = dxDist_;}
    QString getDxDist() const {return dxDist;}

    void setDxBrg(QString dxBrg_){dxBrg = dxBrg_;}
    QString getDxBrg() const {return dxBrg;}

    void setDxCallWorked(bool dxCallWorked_){dxCallWorked = dxCallWorked_;}
    bool getDxCallWorked() const {return dxCallWorked;}

    void setDxLocatorWorked(bool dxLocatorWorked_){dxLocatorWorked = dxLocatorWorked_;}
    bool getDxLocatorWorked()const {return dxLocatorWorked;}

    void setSentToMemory(bool sentToMemory_){sentToMemory = sentToMemory_;}
    bool getSentToMemory() const {return sentToMemory;}



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




