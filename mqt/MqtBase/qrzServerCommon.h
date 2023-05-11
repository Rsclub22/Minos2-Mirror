#ifndef QRZSERVERCOMMON_H
#define QRZSERVERCOMMON_H


#include <QString>
#include "PublishState.h"

const QString QRZ_BUTTON_ON_STYLE = QString("background-color: Sandybrown ; border-style: outset; border-width: 1px; border-color: black; min-width: 5em; padding: 3px;\n");
const QString QRZ_BUTTON_OFF_STYLE = QString("background-color: Gainsboro ; border-style: outset; border-width: 1px; border-color: black; min-width: 5em; padding: 3px;\n");

const QString QRA_LOOKUP_OK = "qraLookUpOK";
const QString QRA_NOT_FOUND = "qraNotFound";

class QrzServer
{
public:
    QString routerName;
    QString app;
    QString publisherProgram;
    PublishState state;

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

    void setDataSource(QString source_){dataSource = source_;}
    QString getDataSource() const {return dataSource;}

    void setCallsign(QString callsign_){callsign = callsign_;}
    QString getCallsign() const {return callsign;}

    void setFirstName(QString firstName_){firstName = firstName_;}
    QString getFirstName() const {return firstName;}

    void setName(QString name_){name = name_;}
    QString getName() const {return name;}

    void setAddr1(QString addr1_){addr1 = addr1_;}
    QString getAddr1() const {return addr1;}

    void setAddr2(QString addr2_){addr2 = addr2_;}
    QString getAddr2() const {return addr2;}

    void setCounty(QString county_){county = county_;}
    QString getCounty() const {return county;}

    void setCountry(QString country_){country = country_;}
    QString getCountry() const {return country;}

    void setLat(QString lat_){lat = lat_;}
    QString getLat() const {return lat;}

    void setLon(QString lon_){lon = lon_;}
    QString getLon() const {return lon;}

    void setQra(QString qra_){qra = qra_;}
    QString getQra() const {return qra;}

    void setCqZone(QString cqZone_){cqZone = cqZone_;}
    QString getCqZone() const {return cqZone;}

    void setItuZone(QString ituZone_){ituZone = ituZone_;}
    QString getItuZone() const {return ituZone;}

    void setModDate(QString moddate_){moddate = moddate_;}
    QString getModDate() const {return moddate;}

    void setDBDate(QString dbdate_){dbdate = dbdate_;}
    QString getDBDate() const {return dbdate;}

    int getDbRecords() const
    {
        return dbRecords;
    }

    void setDbRecords(int newDbRecords)
    {
        dbRecords = newDbRecords;
    }

    int getDbRecalls() const
    {
        return dbRecalls;
    }

    void setDbRecalls(int newDbRecalls)
    {
        dbRecalls = newDbRecalls;
    }

    int getQrzRecalls() const
    {
        return qrzRecalls;
    }

     void setQrzRecalls(int newQrzRecalls)
    {
        qrzRecalls = newQrzRecalls;
    }
    void clear()
    {
        dataSource.clear();
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
        moddate.clear();
        dbdate.clear();

        dbRecords = 0;
        dbRecalls = 0;
        qrzRecalls = 0;
    }

private:

    QString dataSource;
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
    QString moddate;
    QString dbdate;

    int dbRecords = 0;
    int dbRecalls = 0;
    int qrzRecalls = 0;

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

    void setFromStationName(QString fromStationName_){fromStationName = fromStationName_;}
    QString getFromStationName(){return fromStationName;}

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
    QString fromStationName;
    QString loggerUuid;
    bool loggerFlag;           // true from logger, false from cluster


};


#endif // QRZSERVERCOMMON_H
