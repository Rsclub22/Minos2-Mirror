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
#include "qrzServerCommon.h"

QT_BEGIN_NAMESPACE
namespace Ui { class QrzServerMainWindow; }
QT_END_NAMESPACE

const QString QRZURL = "https://xmldata.qrz.com/xml/current/?";
const QString AGENT = "Minos";

const int QUERYTIMEOUT = 2000;



// SubExp response

const QString NONSUBCRIBER = "non-subscriber";



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

class QrzServerStateFlags
{

public:

    QrzServerStateFlags()
    {
        clear();
    }

    void clear()
    {
        askLogonFlag = false;
        askCallsignFlag = false;
        qrzLoggedOnFlag = false;
    }

    bool getAskLogonFlag(){return askLogonFlag;}
    void setAskLogonFlag(bool state){askLogonFlag = state;}

    bool getAskCallsignFlag(){return askCallsignFlag;}
    void setAskCallsignFlag(bool state){askCallsignFlag = state;}

    bool getQrzLoggedOnFlag(){return qrzLoggedOnFlag;}
    void setQrzLoggedOnFlag(bool state){qrzLoggedOnFlag = state;}



private:

    bool askLogonFlag = false;
    bool askCallsignFlag = false;
    bool qrzLoggedOnFlag = false;




};





class QrzServerMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QrzServerMainWindow(QWidget *parent = nullptr);
    ~QrzServerMainWindow();

    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;


private slots:
    void quit();
    void LogTimerTimer();
    void onConfigure();
    void onStdInRead(QString cmd);

    void clusterClientServerList(QVector<ClusterServer> serverList);


    void onClusterQrzMessage(QrzServerMessage qrzRequest);
    void handleQrzRequests();

    void onLoggerQrzMsg(QrzServerMessage qrzRequest);

    void onPingStateTimerTimeout();
private:
    Ui::QrzServerMainWindow *ui;


    StdInReader stdinReader;
    QString appName = "";



    QTimer LogTimer;
    QTimer *pingStateTimer;

    QString logonCallsign;
    QString password;

    QVector<QrzServerMessage> qrzRequestQueue;
    QrzServerMessage requestedStation;

    int parseTest();

    QrzCallsignData qrzCallsignData;
    QrzSessionData qrzSessionData;

    QString askQrzCallsign;

    QTimer *checkQrzRequestsTimer;

    QrzServerStateFlags qrzServerStateFlags;

    QString stateErrorMessage;


    QString key;

    void closeEvent(QCloseEvent *event) override;

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
