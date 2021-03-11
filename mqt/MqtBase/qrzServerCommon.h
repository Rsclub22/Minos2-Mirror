#ifndef QRZSERVERCOMMON_H
#define QRZSERVERCOMMON_H

#include "base_pch.h"


class QrzServer
{
public:
    QString routerName;
    QString app;
    QString publisherProgram;
    PublishState state;
    void sendQrzResponseToClusterServer(QString dxCall, QString dxQra, QString dxCallStatus, QString spotterCall, QString spotterQra, QString spotterCallStatus);
};




class QrzCallsignData
{

public:
    QrzCallsignData()
    {
        clear();
    }
    ~QrzCallsignData()
    {

    }

    void setCallsign(QString callsign_){callsign = callsign_;}
    QString getCallsign(){return callsign;}

    void setFirstName(QString firstName_){firstName = firstName_;}
    QString getFirstName(){return firstName;}

    void setName(QString name_){name = name_;}
    QString getName(){return name;}

    void setAddr1(QString addr1_){addr1 = addr1_;}
    QString getAddr1(){return addr1;}

    void setAddr2(QString addr2_){addr2 = addr2_;}
    QString getAddr2(){return addr2;}

    void setCounty(QString county_){county = county_;}
    QString getCounty(){return county;}

    void setCountry(QString country_){country = country_;}
    QString getCountry(){return country;}

    void setLat(QString lat_){lat = lat_;}
    QString getLat(){return lat;}

    void setLon(QString lon_){lon = lon_;}
    QString getLon(){return lon;}

    void setQra(QString qra_){qra = qra_;}
    QString getQra(){return qra;}

    void setCqZone(QString cqZone_){cqZone = cqZone_;}
    QString getCqZone(){return cqZone;}

    void setItuZone(QString ituZone_){ituZone = ituZone_;}
    QString getItuZone(){return ituZone;}

    void clear()
    {
        callsign.clear();
        firstName.clear();
        name.clear();
        addr1.clear();
        addr2.clear();
        county.clear();
        country.clear();
        lat.clear();
        lon.clear();
        qra.clear();
        cqZone.clear();
        ituZone.clear();
    }



private:

    QString callsign;
    QString firstName;
    QString name;
    QString addr1;
    QString addr2;
    QString county;
    QString country;
    QString lat;
    QString lon;
    QString qra;
    QString cqZone;
    QString ituZone;

};


class QrzServerMessage
{

public:
    QrzServerMessage(){clear();}

    void setDxCall(QString dxCall_){dxCall = dxCall_;}
    QString getDxCall(){return dxCall;}

    void setSpotterCall(QString spotterCall_){spotterCall = spotterCall_;}
    QString getSpotterCall(){return spotterCall;}

    void setLoggerFlag(bool state){loggerFlag = state;}
    bool getLoggerFlag(){return loggerFlag;}

    void setLoggerUuid(QString loggerUuid_){loggerUuid = loggerUuid_;}
    QString getLoggerUuid(){return loggerUuid;}

    void clear(){
        dxCall.clear();
        spotterCall.clear();
        loggerUuid.clear();
        loggerFlag = false;

    }

private:

    QString dxCall;
    QString spotterCall;
    QString loggerUuid;
    bool loggerFlag;           // true from logger, false from cluster


};


#endif // QRZSERVERCOMMON_H
