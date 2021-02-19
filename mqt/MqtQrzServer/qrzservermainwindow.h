#ifndef QRZSERVERMAINWINDOW_H
#define QRZSERVERMAINWINDOW_H

#include <QMainWindow>
#include <QXmlStreamReader>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

#include "mqtUtils_pch.h"

QT_BEGIN_NAMESPACE
namespace Ui { class QrzServerMainWindow; }
QT_END_NAMESPACE

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

    void setLon(QString lon_){lat = lon_;}
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
        key.clear();
        subExp.clear();
    }

    void setError(QString error_){error = error_;}
    QString getError(){return error;}

    void setKey(QString key_){key = key_;}
    QString getKey(){return key;}

    void setSubExp(QString subExp_){subExp = subExp_;}
    QString getSubExp(){return subExp;}

private:

    QString error;
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
private:
    Ui::QrzServerMainWindow *ui;

    StdInReader stdinReader;
    QString appName = "";

    QTimer LogTimer;

    QString logonCallsign;
    QString password;

    int parseTest();

    QrzCallsignData qrzCallsignData;
    QrzSessionData qrzSessionData;

    bool loggedOn = false;

    QString key;

    void sendUrl(QString url);
    void parseCallsignData(QXmlStreamReader &xmlData);
    void parseSessionData(QXmlStreamReader &xmlData);
    void parseDXCCData(QXmlStreamReader &xmlData);
    void getData(QNetworkReply *netReply);

};
#endif // QRZSERVERMAINWINDOW_H
