/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Qrz Server
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2021
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////



#ifndef QRZSERVERMAINWINDOW_H
#define QRZSERVERMAINWINDOW_H

#include <QMainWindow>
#include <QXmlStreamReader>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

#include "mqtUtils_pch.h"
#include "qrzserverrpc.h"

QT_BEGIN_NAMESPACE
namespace Ui { class QrzServerMainWindow; }
QT_END_NAMESPACE

const QString QRZURL = "https://xmldata.qrz.com/xml/current/?";
const QString AGENT = "Minos";

const int QUERYTIMEOUT = 2000;

const QString QRZ_BUTTON_ON_STYLE = QString("background-color: Sandybrown ; border-style: outset; border-width: 1px; border-color: black; min-width: 5em; padding: 3px;\n");
const QString QRZ_BUTTON_OFF_STYLE = QString("background-color: Gainsboro ; border-style: outset; border-width: 1px; border-color: black; min-width: 5em; padding: 3px;\n");


// SubExp response

const QString NONSUBCRIBER = "non-subscriber";

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

    void setQth(QString qth_){qth = qth_;}
    QString getQth(){return qth;}

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
        qth.clear();
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
    QString qth;
    QString county;
    QString country;
    QString lat;
    QString lon;
    QString qra;
    QString cqZone;
    QString ituZone;

};


class QrzSessionData
{
public:
    QrzSessionData(){}

    void clear()
    {
        error.clear();
        message.clear();
        key.clear();
        subExp.clear();
    }

    void setError(QString error_){error = error_;}
    QString getError(){return error;}

    void setMessage(QString message_){message = message_;}
    QString getMessage(){return message;}

    void setKey(QString key_){key = key_;}
    QString getKey(){return key;}

    void setSubExp(QString subExp_){subExp = subExp_;}
    QString getSubExp(){return subExp;}

private:

    QString error;
    QString message;
    QString key;
    QString subExp;

};





class QrzServerMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QrzServerMainWindow(QWidget *parent = nullptr);
    ~QrzServerMainWindow();

private slots:
    void quit();
    void LogTimerTimer();
    void onConfigure();
    void onStdInRead(QString cmd);

    void clusterClientServerList(QVector<ClusterServer> serverList);


    void onClusterQrzMessage(QrzServerMessage qrzRequest);
    void handleQrzRequests();
    void onQueryTimeout();
private:
    Ui::QrzServerMainWindow *ui;

    StdInReader stdinReader;
    QString appName = "";



    QTimer LogTimer;

    QString logonCallsign;
    QString password;

    QVector<QrzServerMessage> qrzRequestQueue;
    QrzServerMessage requestedStation;

    int parseTest();

    QrzCallsignData qrzCallsignData;
    QrzSessionData qrzSessionData;


    QTimer *queryTimer;
    QTimer *checkQrzRequestsTimer;

    bool askLogonFlag = false;
    bool askCallsignFlag = false;

    bool qrzLoggedOn = false;


    QString key;

    void sendUrl(QString url);
    void parseCallsignData(QXmlStreamReader &xmlData);
    void parseSessionData(QXmlStreamReader &xmlData);
    void parseDXCCData(QXmlStreamReader &xmlData);
    void getData(QNetworkReply *netReply);

    void logon();
    QString stripPasswordFromUrl(QString url);
    void askCallsignData(QString callsign);
    void sessionDataReceived();
    void callsignDataReceived();
    void addTextToLogWindow(QString message);
    void addToErrorTextLabel(QString message);
    void addToMessageTextLabel(QString message);
    void setQrzStatusConnected(bool state);
};
#endif // QRZSERVERMAINWINDOW_H
