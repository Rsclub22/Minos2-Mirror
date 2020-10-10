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


#ifndef SPOTDATABASE_H
#define SPOTDATABASE_H



namespace bandmapSpotType {

enum SPOT_TYPE {NONE, CLUSTER, CLUSTER_MARKED, LOGGED, MARKED, SAVED, CQ};

}

class SpotdataBase
{
public:
    SpotdataBase();

    SpotdataBase(const SpotdataBase &sdp);

    void clear();

    void setRxTime(const qint64 rxTime_){rxTime = rxTime_;}
    qint64 getRxTime() const {return rxTime;}

    void setSpotDateTime(const QDateTime spotDateTime_) {spotDateTime = spotDateTime_;}
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
        dxCall = Callsign(dxCall_.trimmed());

        dxCallValidateCode = dxCall.validate();
    }

    Callsign getDxCall() const {return dxCall;}
    QString getDxCallStr() const {return dxCall.fullCall.getValue();}
    int getDxCallValidateCode() const {return dxCallValidateCode;}


    void setFreq(const Frequency freq_){freq = freq_;}
    Frequency getFreq() const {return freq;}

    void setBand(const QString band_){band = band_.trimmed();}
    QString getBand() const {return band;}

    void setBandMask(const QString bandMask_){bandMask = bandMask_.trimmed();}
    QString getBandMask() const {return bandMask;}

    void setMode(const QString mode_){mode = mode_.trimmed();}
    QString getMode() const {return mode;}

    void setModeMask(const QString modeMask_){modeMask = modeMask_.trimmed();}
    QString getModeMask() const {return modeMask;}

    void setDxLocator(const QString dxLocator_){dxLocator = dxLocator_.trimmed();}
    QString getDxLocator() const {return dxLocator;}

    void setSpotterLocator(const QString spotterLocator_){spotterLocator = spotterLocator_.trimmed();}
    QString getSpotterLocator()const{return spotterLocator;}

    void setDxPropMode(const QString dxPropMode_){dxPropMode = dxPropMode_.trimmed();}
    QString getDxPropMode()const{return dxPropMode;}

    void setSpotterCall(const QString spotterCall_)
    {
        spotterCall = Callsign(spotterCall_.trimmed());
        spotterCallValidateCode = spotterCall.validate();
    }
    Callsign getSpotterCall()const {return spotterCall;}
    QString getSpotterCallStr() const {return spotterCall.fullCall.getValue();}

    int getSpotterCallValidateCode() const {return spotterCallValidateCode;}

    void setSpotComment(const QString spotComment_){spotComment = spotComment_.trimmed();}
    QString getSpotComment() const {return spotComment;}



protected:

    qint64 rxTime;
    QDateTime spotDateTime;
    QString band;
    QString bandMask;
    QString mode;
    QString modeMask;
    Callsign dxCall;
    int dxCallValidateCode;
    Frequency freq;
    QString dxLocator;
    Callsign spotterCall;
    int spotterCallValidateCode;
    QString spotterLocator;
    QString dxPropMode;
    QString spotComment;


};

//---------------------------------------------------------------------

class ClusterSpotDataBase
{

public:
    ClusterSpotDataBase();
    ClusterSpotDataBase(const ClusterSpotDataBase &sdp);

    ClusterSpotDataBase &operator =(const ClusterSpotDataBase &cpd);


    void setDxLocatorIsFromNode(const bool dxLocatorIsFromNode_){dxLocatorIsFromNode = dxLocatorIsFromNode_;}
    bool getDxLocatorIsFromNode() const {return dxLocatorIsFromNode;}

    void setAskQrzFailed(const bool askQrzFailed_){askQrzFailed = askQrzFailed_;}
    bool getAskQrzFailed() const {return askQrzFailed;}

    void setDxDist(const QString dxDist_){dxDist = dxDist_.trimmed();}
    QString getDxDist() const {return dxDist;}

    void setDxBrg(const QString dxBrg_){dxBrg = dxBrg_.trimmed();}
    QString getDxBrg()const{return dxBrg;}

    void setDxCallWorked(const bool dxCallWorked_){dxCallWorked = dxCallWorked_;}
    bool getDxCallWorked()const{return dxCallWorked;}

    void setDxLocatorWorked(const bool dxLocatorWorked_){dxLocatorWorked = dxLocatorWorked_;}
    bool getDxLocatorWorked()const{return dxLocatorWorked;}

    void setSentToMemory(const bool sentToMemory_){sentToMemory = sentToMemory_;}
    bool getSentToMemory()const {return sentToMemory;}



    void clear();


protected:


    bool dxLocatorIsFromNode;
    bool askQrzFailed;
    QString dxDist;
    QString dxBrg;
    bool dxCallWorked;
    bool dxLocatorWorked;
    bool sentToMemory;

};


//-------------------------------------------------------------------------


class ClusterSpotData: public SpotdataBase, public ClusterSpotDataBase
{
public:

    ClusterSpotData();
    ClusterSpotData(const ClusterSpotData &csd);

    ClusterSpotData &operator =(const ClusterSpotData &csd);
    bool operator ==(const ClusterSpotData &csd) const;
    bool operator !=(const ClusterSpotData &csd) const;

    void clear();


};

//---------------------------------------------------------------------


class BandmapSpotData: public SpotdataBase, public ClusterSpotDataBase
{
public:
    BandmapSpotData(bandmapSpotType::SPOT_TYPE spotType_);
    BandmapSpotData(const BandmapSpotData &sdp);

    bool operator ==(const BandmapSpotData &bsd) const;
    bool operator !=(const BandmapSpotData &bsd) const;

    void clear();

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



private:

    bool runModeOn;
    bool offRunFreq;
    QString district;
    bool districtWorked;
    QString rotBrg;
    bool rotConnected;
    bool isSelected;
    bandmapSpotType::SPOT_TYPE spotType;

};



#endif // SPOTDATABASE_H
