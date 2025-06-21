/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2020
//
///
//
//
/////////////////////////////////////////////////////////////////////////////


#ifndef SPOTBASEDATA_H
#define SPOTBASEDATA_H
#include <QCoreApplication>
#include <QDateTime>

#include "callsign.h"
#include "frequency.h"

class BaseContestLog;

namespace bandmapSpotType {

enum SPOT_TYPE {NONE, CLUSTER, CLUSTER_MARKED, LOGGED, MARKED, SAVED, CQ, DELETED};

}
namespace clusterSpotType {

inline const QString DXSPOT_TYPE = "dxSpotType";
inline const QString SHOW_DXSPOT_TYPE = "showDxSpotType";

}

class ClusterSpotData
{
    Q_DECLARE_TR_FUNCTIONS(ClusterSpotData)
public:
    ClusterSpotData();

    ClusterSpotData(bandmapSpotType::SPOT_TYPE spotType_);

    bool sameSpotAs(QSharedPointer<ClusterSpotData> cpd);

    void setShowSpotType(bool sst){showSpotType = sst;}
    bool isShowSpotType()const {return showSpotType;}

    void setRxTime(const qint64 rxTime_){rxTime = rxTime_;}
    qint64 getRxTime() const {return rxTime;}

    void setSpotDateTime(const QDateTime spotDateTime_)
    {
        spotDateTime = spotDateTime_;
    }
    QDateTime getSpotDateTime() const {return spotDateTime;}

    QString getSpotTime() const
    {
        return spotDateTime.time().toString("hh:mm");
    }

    // format of spotTime either hh:mm or hhmm
    void setSpotTime(const QString spotTime_)
    {
        QString spotTime = spotTime_;
        QString time = spotTime.remove(':');
        bool okH, okM;
        int h = time.left(2).toInt(&okH);
        int m = time.right(2).toInt(&okM);
        if (okH && okM)
        {
            spotDateTime.setTime(QTime(h,m));
        }

    }

    // format spotDate either dd:mm:yyyy or ddmmyyyy
    void setSpotDate(const QString spotDate_)
    {
        QString spotDate = spotDate_;
        QString date = spotDate.remove(':');
        bool okD, okM, okY;
        int d = date.left(2).toInt(&okD);
        int m = date.mid(2,2).toInt(&okM);
        int y = date.right(4).toInt(&okY);
        if (okD && okM && okY)
        {
           spotDateTime.setDate(QDate(y, m, d));
        }
    }

    QString getSpotDate() const
    {
        return spotDateTime.date().toString("dd:mm:yyyy");
    }

    void setDxCall(const QString dxCall_)
    {
        dxCall.setFullCall(dxCall_);
    }

    void setCallsign(const Callsign &cs){dxCall = cs;}

    const Callsign &getDxCall() const {return dxCall;}
    QString getDxCallStr() const {return dxCall.getFullCall();}


    void setFreq(const Frequency freq_){freq = freq_;}
    Frequency getFreq() const {return freq;}

    void setBand(const QString band_){band = band_.trimmed();}
    QString getBand() const {return band;}

    void setBandType(const QString bandType_){bandType = bandType_;}
    QString getBandType() const {return bandType;}

    void setMode(const QString mode_){mode = mode_.trimmed();}
    QString getMode() const {return mode;}

    void setDxLocator(const QString dxLocator_){dxLocator = dxLocator_.trimmed();}
    QString getDxLocator() const {return dxLocator;}

    void setSpotterLocator(const QString spotterLocator_){spotterLocator = spotterLocator_.trimmed();}
    QString getSpotterLocator()const{return spotterLocator;}

    void setDxPropMode(const QString dxPropMode_){dxPropMode = dxPropMode_.trimmed();}
    QString getDxPropMode()const{return dxPropMode;}

    void setSpotterCall(const QString spotterCall_)
    {
        spotterCall.setFullCall(spotterCall_);
    }
    Callsign getSpotterCall()const {return spotterCall;}
    QString getSpotterCallStr() const {return spotterCall.getFullCall();}

    void setSpotComment(const QString spotComment_){spotComment = spotComment_.trimmed();}
    QString getSpotComment() const {return spotComment;}


    void setDxLocatorIsFromNode(const bool dxLocatorIsFromNode_){dxLocatorIsFromNode = dxLocatorIsFromNode_;}
    bool getDxLocatorIsFromNode() const {return dxLocatorIsFromNode;}

    void setDxDist(const QString dxDist_){dxDist = dxDist_.trimmed();}
    QString getDxDist() const {return dxDist;}

    void setDxBrg(const QString dxBrg_){dxBrg = dxBrg_.trimmed();}
    QString getDxBrg()const{return dxBrg;}

    void setDxCallWorked(const bool dxCallWorked_)
    {
        dxCallWorked = dxCallWorked_;
    }
    bool getDxCallWorked()const{return dxCallWorked;}

    void setDxLocatorWorked(const bool dxLocatorWorked_){dxLocatorWorked = dxLocatorWorked_;}
    bool getDxLocatorWorked()const{return dxLocatorWorked;}

    void setSentToMemory(const bool sentToMemory_){sentToMemory = sentToMemory_;}
    bool getSentToMemory()const {return sentToMemory;}

    bool getRunModeOn()const{return runModeOn;}
    void setRunModeOn(bool _runModeOn){runModeOn = _runModeOn;}


    bool getOffRunFreq()const{return offRunFreq;}
    void setOffRunFreq(bool _offRunFreq){offRunFreq = _offRunFreq;}

    QString getDistrict()const{return district;}
    void setDistrict(const QString district_){district = district_;}

    bool getDistrictWorked()const{return districtWorked;}
    void setDistrictWorked(const bool districtWorked_){districtWorked = districtWorked_;}

    QString getRotBrg()const{return rotBrg;}
    void setRotBrg(const QString rotBrg_){rotBrg = rotBrg_;}

    bool getRotConnected()const{return rotConnected;}
    void setRotConnected(const bool rotConnected_){rotConnected = rotConnected_;}

    bool getIsSelected()const{return isSelected;}
    void setIsSelected(const bool isSelected_){isSelected = isSelected_;}

    bandmapSpotType::SPOT_TYPE getSpotType()const{return spotType;}
    void setSpotType(bandmapSpotType::SPOT_TYPE _spotType){spotType = _spotType;}
    QString spotName();
    static QString spotName(bandmapSpotType::SPOT_TYPE _spotType);

    bool getCqResponse() const {return cqResponse;}
    void setCqResponse(bool value) {cqResponse = value;}

    int getRecNo() const {return recno;}
    void setRecNo(int value) {recno = value;}
    bool spotMatchesDxCallsignFreqAndTime(const QSharedPointer<ClusterSpotData> &other, int freqTolerance = 200, int timeToleranceMins = 10) const;
private:

    int recno = -1;  // for spots DB

    // from SpotBaseData

    qint64 rxTime = 0;
    QDateTime spotDateTime;
    bool showSpotType;
    QString band;
    QString bandType;
    QString mode;
    Callsign dxCall;
    Frequency freq;
    QString dxLocator;
    Callsign spotterCall;
    QString spotterLocator;
    QString dxPropMode;
    QString spotComment;

    // from ClusterSpotBaseData
    bool dxLocatorIsFromNode = false;
    QString dxDist;
    QString dxBrg;
    bool dxCallWorked = false;
    bool dxLocatorWorked = false;
    bool sentToMemory = false;

    // from BandmapSpotData, to allow QSOs to be modelled as spots

    bool runModeOn = false;
    bool offRunFreq = false;
    bool cqResponse = false;
    QString district;
    bool districtWorked = false;
    QString rotBrg;
    bool rotConnected = false;
    bool isSelected = false;
    bandmapSpotType::SPOT_TYPE spotType = bandmapSpotType::SPOT_TYPE::NONE;
};
Q_DECLARE_METATYPE(ClusterSpotData *)

extern QSharedPointer<ClusterSpotData> stringToDxSpot(QString spot, BaseContestLog *ct, qlonglong &timeToLive);

#endif // SPOTBASEDATA_H
