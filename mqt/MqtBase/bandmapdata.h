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
#include "frequency.h"

namespace bandmapSpotType {

enum SPOT_TYPE {NONE, CLUSTER, CLUSTER_MARKED, LOGGED, MARKED, SAVED, CQ};

}


class BandmapData
{


public:
    BandmapData();
    BandmapData(qint64 _rxTime, QString _spotTime, QString _spotDate,
                       Frequency _dxFreq, QString _dxBand, QString _dxBandMask,
                       QString _dxMode,  QString _dxModeMask,
                       QString _dxCall, bool _dxCallWorked,
                       QString _dxLocator, bool _dxLocatorIsFromNode, bool _dxLocatorWorked,
                       QString _dxDistrict, bool _dxDistrictWorked,
                       QString _distance, QString _bearing, QString _rotBrg, bool _rotConnected,
                       QString _spotterCall, QString _spotterLocator, QString _dxPropMode,
                       bool _runModeOn, bool _offRunFreq,
                       QString comment, bandmapSpotType::SPOT_TYPE _spotType);

    void operator = (const BandmapData& bmd);
    void operator = (const BandmapData* bmd);
    void clear();

    void setRxTime(qint64 rxTime_){rxTime = rxTime_;}
    qint64 getRxTime()const {return rxTime;}

    void setDxCall(const QString dxCall_){dxCall = dxCall_.trimmed();}
    QString getDxCall()const {return dxCall;}

    void setDxFreq(const Frequency dxFreq_){dxFreq = dxFreq_;}
    Frequency getDxFreq()const {return dxFreq;}

    void setDxBand(const QString dxBand_){dxBand = dxBand_.trimmed();}
    QString getDxBand()const {return dxBand;}

    void setDxBandMask(const QString dxBandMask_){dxBandMask = dxBandMask_.trimmed();}
    QString getDxBandMask()const {return dxBandMask;}

    void setDxMode(const QString dxMode_){dxMode = dxMode_.trimmed();}
    QString getDxMode()const {return dxMode;}

    void setDxModeMask(const QString dxModeMask_){dxModeMask = dxModeMask_.trimmed();}
    QString getDxModeMask()const {return dxModeMask;}

    void setSpotterCall(const QString spotterCall_){spotterCall = spotterCall_.trimmed();}
    QString getSpotterCall()const {return spotterCall;}

    void setSpotComment(const QString spotComment_){spotComment = spotComment_.trimmed();}
    QString getSpotComment()const {return spotComment;}

    void setSpotTime(const QString spotTime_){spotTime = spotTime_.trimmed();}
    QString getSpotTime()const {return spotTime;}

    void setSpotDate(const QString spotDate_){spotDate = spotDate_.trimmed();}
    QString getSpotDate()const {return spotDate;}

    void setSpotDateTime(const QDateTime spotDateTime_){spotDateTime = spotDateTime_;}
    QDateTime getSpotDateTime() const {return spotDateTime;}


    void setDxLocator(const QString dxLocator_){dxLocator = dxLocator_.trimmed();}
    QString getDxLocator() const {return dxLocator;}

    void setDxLocatorIsFromNode(const bool dxLocatorIsFromNode_){dxLocatorIsFromNode = dxLocatorIsFromNode_;}
    bool getDxLocatorIsFromNode()const {return dxLocatorIsFromNode;}

    void setSpotterLocator(const QString spotterLocator_){spotterLocator = spotterLocator_.trimmed();}
    QString getSpotterLocator() const {return spotterLocator;}

    void setDxPropMode(const QString dxPropMode_){dxPropMode = dxPropMode_.trimmed();}
    QString getDxPropMode() const {return dxPropMode;}

    void setDxDist(const QString dxDist_){dxDist = dxDist_.trimmed();}
    QString getDxDist() const {return dxDist;}

    void setDxDistrict(const QString dxDistrict_) {dxDistrict = dxDistrict_;}
    QString getDxDistrict() const {return dxDistrict;}

    void setDxDistrictWorked(const bool dxDistrictWorked_) {dxDistrictWorked = dxDistrictWorked_;}
    bool getDxDistrictWorked() const {return dxDistrictWorked;}

    void setDxBrg(const QString dxBrg_){dxBrg = dxBrg_.trimmed();}
    QString getDxBrg() const {return dxBrg;}

    void setRotBrg(const QString rotBrg_) {rotBrg = rotBrg_.trimmed();}
    QString getRotBrg()const {return rotBrg;}

    void setRotConnected(const bool rotConnected_) {rotConnected = rotConnected_;}
    bool getRotConnected() const {return rotConnected;}

    void setDxCallWorked(const bool dxCallWorked_){dxCallWorked = dxCallWorked_;}
    bool getDxCallWorked() const {return dxCallWorked;}

    void setDxLocatorWorked(const bool dxLocatorWorked_){dxLocatorWorked = dxLocatorWorked_;}
    bool getDxLocatorWorked()const {return dxLocatorWorked;}

    void setSentToMemory(const bool sentToMemory_){sentToMemory = sentToMemory_;}
    bool getSentToMemory() const {return sentToMemory;}

    void setRunModeOn(const bool runModeOn_){runModeOn = runModeOn_;};
    bool getRunModeOn() const {return  runModeOn;}

    void setOffRunFreq(const bool offRunFreq_){offRunFreq = offRunFreq_;};
    bool getOffRunFreq() const {return  offRunFreq;}

    void setSpotType(const bandmapSpotType::SPOT_TYPE spotType_){spotType = spotType_;}
    bandmapSpotType::SPOT_TYPE getSpotType() const {return spotType;}

    void setIsSelected (const bool isSelected_){isSelected = isSelected_;}
    bool getIsSelected() const {return isSelected;}

private:


    qint64 rxTime;
    QString spotTime;
    QString spotDate;
    QDateTime spotDateTime;
    Frequency dxFreq;
    QString dxBand;
    QString dxBandMask;
    QString dxMode;
    QString dxModeMask;
    QString dxCall;
    QString dxLocator;
    bool dxLocatorIsFromNode;
    QString dxDist;
    QString dxBrg;
    QString rotBrg;
    bool rotConnected;
    bool dxCallWorked;
    bool dxLocatorWorked;
    QString dxDistrict;
    bool dxDistrictWorked;
    bool sentToMemory;
    QString spotterCall;
    QString spotterLocator;
    QString dxPropMode;
    bool runModeOn;
    bool offRunFreq;
    QString spotComment;
    bandmapSpotType::SPOT_TYPE spotType;
    bool isSelected;





};



#endif // BANDMAPDATA_H
