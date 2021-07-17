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


#include <QFile>
#include <QDebug>
#include <QSharedPointer>
#include <QSettings>
#include <QProcessEnvironment>

#include "qrzservermainwindow.h"
#include "qrzconfiguredialog.h"
#include "ui_qrzservermainwindow.h"

QrzServerMainWindow::QrzServerMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QrzServerMainWindow)
{
    ui->setupUi(this);


    connect(&stdinReader, &StdInReader::stdinLine, this, &QrzServerMainWindow::onStdInRead);
    stdinReader.start();

    QSettings settings;
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);



    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    appName = env.value("MQTRPCNAME", "") ;

    // this is only needed for standalone to store to registry - appname might not be correct here!
    if (appName.isEmpty())
    {
        QApplication::setOrganizationName( "Minos2Qt" );
        QApplication::setOrganizationDomain( "g0gjv.org.uk" );
        QApplication::QCoreApplication::setApplicationName( "mqtQrzServer" );

    }

    logonCallsign = settings.value("logonCallsign", "").toString();
    password = settings.value("password", "").toString();

    createCloseEvent();

    //connect(&LogTimer, &QTimer::timeout, this, [=](){LogTimerTimer();});
    //LogTimer.start(100);

    trace(QString("AppName = %1").arg(appName));
    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName());
    Q_UNUSED(rpc)

    QStringList sv{
            rpcConstants::clusterApp, rpcConstants::qrzDisplayApp
        };

    connect (QrzServerRpc::getQrzServerRpc(), &QrzServerRpc::clusterQrzMsg, this, &QrzServerMainWindow::onClusterQrzMessage);
    connect (QrzServerRpc::getQrzServerRpc(), &QrzServerRpc::loggerQrzMsg, this, &QrzServerMainWindow::onLoggerQrzMsg);



    connect(ui->actionSetup_QRZ, &QAction::triggered, this, [=](){onConfigure();});

    checkQrzRequestsTimer = new QTimer(this);
    connect(checkQrzRequestsTimer, &QTimer::timeout, this, [=](){handleQrzRequests();});
    checkQrzRequestsTimer->start(500);

    pingStateTimer = new QTimer(this);
    connect(pingStateTimer, &QTimer::timeout, this, [=](){onPingStateTimerTimeout();});
    pingStateTimer->start(3000);

    ui->messageTextWindow->isReadOnly();

    logon();


}

QrzServerMainWindow::~QrzServerMainWindow()
{
    delete ui;
}

void QrzServerMainWindow::LogTimerTimer()
{
    bool show = getShowApp();
    if ( !isVisible() && show )
    {
        setVisible(true);
    }
    if ( isVisible() && !show )
    {
        setVisible(false);
    }

    static bool closed = false;
    if ( !closed )
    {
        if ( checkCloseEvent() )
        {
            trace("close event seen");
            closed = true;
            close();
        }
    }
}



void QrzServerMainWindow::onPingStateTimerTimeout()
{
        QrzServerRpc::getQrzServerRpc()->sendQrzLoggedState(qrzServerStateFlags.getQrzLoggedOnFlag(), stateErrorMessage);

}

void QrzServerMainWindow::onStdInRead(QString cmd)
{

    bool doClose = false;
    if (cmd.indexOf("Shutdown", 0, Qt::CaseInsensitive) >= 0)
    {
//        closeApp = true;
        doClose = true;
    }
    executeStdIn(cmd);
    if (doClose)
        close();

}


void QrzServerMainWindow::quit()
{

}


void QrzServerMainWindow::logon()
{
    if (qrzServerStateFlags.getAskCallsignFlag() || qrzServerStateFlags.getAskLogonFlag())
    {
        return;
    }

    qrzSessionData.clear();

    qrzServerStateFlags.setAskLogonFlag(true);
    qrzServerStateFlags.setQrzLoggedOnFlag(false);

    if (logonCallsign.isEmpty() || password.isEmpty())
    {
        onConfigure();
    }

    QString logonQrz = QRZURL + "username=" + logonCallsign.trimmed() + ";password=" + password.trimmed() + ";agent=" + AGENT;
    addTextToLogWindow(tr("Logging on to QRZ.com with callsign: %1").arg(logonCallsign));
    sendUrl(logonQrz);
}

void QrzServerMainWindow::askCallsignData(QString callsign)
{
    Callsign cs;
    cs.setFullCall(callsign);
    QString callsignUrl = QRZURL + "s=" + qrzSessionData.getKey() + ";callsign=" + callsign;
    sendUrl(callsignUrl);
}



void QrzServerMainWindow::sendUrl(QString url)
{
    trace(QString("sendUrl - %1").arg(stripPasswordFromUrl(url)));

    QNetworkAccessManager m_NetworkMngr;

    QUrl qurl( url );
    QNetworkRequest qnr( qurl );

    qnr.setRawHeader( "User-Agent" , "Mozilla/4.0 (compatible;Minos2)" );

    QSharedPointer<QNetworkReply> reply = QSharedPointer<QNetworkReply>(m_NetworkMngr.get( QNetworkRequest( QUrl( url ) ) ));

    QEventLoop eventLoop;
        // "quit()" the event-loop, when the network request "finished()"
    QObject::connect(&m_NetworkMngr, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(); // blocks stack until "finished()" has been called


    if ( reply->error() == QNetworkReply::NoError )
    {
        stateErrorMessage.clear();

        int raw = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (raw == 301)
        {
            QUrl redirect =  reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

            QNetworkRequest qnr1( redirect );
            qnr1.setRawHeader( "User-Agent" , "Mozilla/4.0 (compatible;Minos2)" );

            reply = QSharedPointer<QNetworkReply>(m_NetworkMngr.get( qnr1 ));
            QEventLoop loop;
            QObject::connect( reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
            loop.exec();
            raw = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        }

        QXmlStreamReader xmlData;
        xmlData.addData( reply->readAll() );
        if (raw == 200)
        {

            if (xmlData.readNextStartElement())
            {
                if (xmlData.name().contains("QRZDatabase"))
                {
                    if (xmlData.readNextStartElement())
                    {
                        if (xmlData.name().contains("Session"))
                        {
                            parseSessionData(xmlData);
                            sessionDataReceived();
                        }
                        else if (xmlData.name().contains("Callsign"))
                        {

                            parseCallsignData(xmlData);
                            callsignDataReceived();

                        }
                        else if (xmlData.name().contains("DXCC"))
                        {
                            parseDXCCData(xmlData);
                        }
                    }
                }
             }
         }
    }
    else
    {
        QString url_ = stripPasswordFromUrl(url);

        // error
        QString msg = QString( "HTPP Get of " ) + url_ + " failed: " + reply->errorString();
        trace ( QString( "HTPP Get of " ) + url_ + " failed: " + reply->errorString() );
        stateErrorMessage = reply->errorString();
        addToErrorTextLabel(msg);
        addTextToLogWindow(msg);
        qrzServerStateFlags.clear();
        QrzServerRpc::getQrzServerRpc()->sendQrzLoggedState(qrzServerStateFlags.getQrzLoggedOnFlag(), reply->errorString());

    }

}


QString QrzServerMainWindow::stripPasswordFromUrl(QString url)
{
    if (url.contains(";password="))
    {
        QStringList ul = url.split(";password=");
        if (ul.count() == 2)
        {
            return ul[0];
        }

    }

    return url;

}


void QrzServerMainWindow::sessionDataReceived()
{

    if (!qrzSessionData.getError().isEmpty())
    {
        trace(QString("Qrz Error: %1").arg(qrzSessionData.getError()));
        addToErrorTextLabel(qrzSessionData.getError());
        addTextToLogWindow(qrzSessionData.getError());
        if (qrzServerStateFlags.getAskCallsignFlag())
        {
            qrzServerStateFlags.setAskCallsignFlag(false);
        }
        if (qrzServerStateFlags.getAskLogonFlag())
        {
            qrzServerStateFlags.setAskLogonFlag(false);
        }

    }

    if (!qrzSessionData.getMessage().isEmpty())
    {
        trace(QString("Qrz Message: %1").arg(qrzSessionData.getMessage()));
        addToErrorTextLabel(qrzSessionData.getMessage());
        addTextToLogWindow(qrzSessionData.getMessage());
        if (qrzServerStateFlags.getAskCallsignFlag())
        {
            qrzServerStateFlags.setAskCallsignFlag(false);
        }
        if (qrzServerStateFlags.getAskLogonFlag())
        {
            qrzServerStateFlags.setAskLogonFlag(false);
        }

    }

    if (qrzServerStateFlags.getAskLogonFlag() && !qrzSessionData.getKey().isEmpty() && qrzSessionData.getError().isEmpty())
    {
        // logon succesfull
        qrzServerStateFlags.setQrzLoggedOnFlag(true);
        qrzServerStateFlags.setAskLogonFlag(false);
        QString msg = QString("Qrz Logged on Ok with call %1").arg(logonCallsign);
        trace(msg);
        addTextToLogWindow(msg);
        addToErrorTextLabel("");
        addToMessageTextLabel("");
        setQrzStatusConnected(true);

    }
    else
    {
        qrzServerStateFlags.setAskLogonFlag(false);
    }

    if (qrzServerStateFlags.getAskCallsignFlag() && !qrzSessionData.getError().isEmpty() && requestedStation.getLoggerFlag())
    {
        QString stateMsg;
        if (!qrzSessionData.getError().isEmpty())
        {
            stateMsg = qrzSessionData.getError();
        }
        else if (!qrzSessionData.getMessage().isEmpty())
        {
            stateMsg = qrzSessionData.getMessage();
        }

        qrzCallsignData.clear();
        qrzCallsignData.setCallsign(requestedStation.getDxCall());
        if (requestedStation.getLoggerFlag())
        {
            QrzServerRpc::getQrzServerRpc()->sendQrzResponseToLoggerDisplay(qrzCallsignData, stateMsg, requestedStation.getFromStationName(),requestedStation.getLoggerUuid());

        }
        else
        {
            QrzServerRpc::getQrzServerRpc()->sendQrzResponseToClusterServer(qrzCallsignData.getCallsign(), "", stateMsg, "", "", "");
        }

        QrzServerRpc::getQrzServerRpc()->sendQrzResponseToLoggerDisplay(qrzCallsignData, stateMsg, requestedStation.getFromStationName(),requestedStation.getLoggerUuid());

        qrzServerStateFlags.setAskCallsignFlag(false);
    }

}


void QrzServerMainWindow::callsignDataReceived()
{
    if (qrzServerStateFlags.getAskCallsignFlag())
    {
        if (!requestedStation.getLoggerFlag())
        {
            // a request from cluster Server

            QString msg = QString("Cluster Qrz Callsign Data received for call = %1, Qra = %2 - Send to Cluster Server").arg(requestedStation.getDxCall(), qrzCallsignData.getQra());
            trace(msg);
            addTextToLogWindow(msg);

            QrzServerRpc::getQrzServerRpc()->sendQrzResponseToClusterServer(requestedStation.getDxCall(), qrzCallsignData.getQra(), QRA_LOOKUP_OK, requestedStation.getSpotterCall(), "", rpcConstants::qrzServerCallOK);

        }
        else
        {
            // a request from logger
            QString msg = QString(QString("Logger Qrz Callsign Data received for call = %1, Send to Qrz Display in Logger Server").arg(requestedStation.getDxCall()));
            trace(msg);
            addTextToLogWindow(msg);
            QString stateMsg = "";
            QrzServerRpc::getQrzServerRpc()->sendQrzResponseToLoggerDisplay(qrzCallsignData, stateMsg, requestedStation.getFromStationName(), requestedStation.getLoggerUuid());

        }

        qrzServerStateFlags.setAskCallsignFlag(false);

    }
}


void QrzServerMainWindow::parseSessionData(QXmlStreamReader &xmlData)
{
    QString("Parse Session Data");
    while(xmlData.readNextStartElement())
    {
        if (xmlData.name() == "Error")
        {
           qrzSessionData.setError(xmlData.readElementText());
           trace(QString("Session Data: Error = %1").arg(qrzSessionData.getError()));
        }
        else if(xmlData.name() == "Key")
        {
            qrzSessionData.setKey( xmlData.readElementText() );
            trace(QString("Session Data: Key = %1").arg(qrzSessionData.getKey()));
        }
        else if (xmlData.name() == "SubExp")
        {
            qrzSessionData.setSubExp(xmlData.readElementText());
            trace(QString("Session Data: SubExp = %1").arg(qrzSessionData.getSubExp()));
        }
        else if (xmlData.name() == "Message")
        {
            qrzSessionData.setMessage(xmlData.readElementText());
            trace(QString("Session Data: Message = %1").arg(qrzSessionData.getMessage()));
        }
        else
        {
           xmlData.skipCurrentElement();
        }
    }
}



void QrzServerMainWindow::parseCallsignData(QXmlStreamReader &xmlData)
{
    trace(QString("Parse Callsign Data"));

    qrzCallsignData.clear();

    while(xmlData.readNextStartElement())
    {
        if(xmlData.name() == "call")
        {
            qrzCallsignData.setCallsign(xmlData.readElementText());
            trace(QString("Callsign Data: callsign = %1").arg(qrzCallsignData.getCallsign()));
        }
        else if (xmlData.name() == "fname")
        {
            qrzCallsignData.setFirstName(xmlData.readElementText());
            trace(QString("Callsign Data: first name = %1").arg(qrzCallsignData.getFirstName()));
        }
        else if (xmlData.name() == "name")
        {
            qrzCallsignData.setName(xmlData.readElementText());
            trace(QString("Callsign Data: name = %1").arg(qrzCallsignData.getName()));
        }
        else if (xmlData.name() == "addr1")
        {
            qrzCallsignData.setAddr1(xmlData.readElementText());
            trace(QString("Callsign Data: addr1 = %1").arg(qrzCallsignData.getAddr1()));
        }
        else if (xmlData.name() == "addr2")
        {
            qrzCallsignData.setAddr2(xmlData.readElementText());
            trace(QString("Callsign Data: addr2 = %1").arg(qrzCallsignData.getAddr2()));
        }
        else if (xmlData.name() == "county")
        {
            qrzCallsignData.setCounty(xmlData.readElementText());
            trace(QString("Callsign Data: county = %1").arg(qrzCallsignData.getCounty()));
        }
        else if (xmlData.name() == "country")
        {
            qrzCallsignData.setCountry(xmlData.readElementText());
            trace(QString("Callsign Data: country = %1").arg(qrzCallsignData.getCountry()));
        }
        else if (xmlData.name() == "lat")
        {
            qrzCallsignData.setLat(xmlData.readElementText());
            trace(QString("Callsign Data: lat = %1").arg(qrzCallsignData.getLat()));
        }
        else if (xmlData.name() == "lon")
        {
            qrzCallsignData.setLon(xmlData.readElementText());
            trace(QString("Callsign Data: lon = %1").arg(qrzCallsignData.getLon()));
        }
        else if (xmlData.name() == "grid")
        {
            QString grid = xmlData.readElementText();
            if (grid.count() == 6)
            {
                grid = grid.replace(4, 2, grid.right(2).toUpper());
            }
            qrzCallsignData.setQra(grid);
            trace(QString("Callsign Data: grid = %1").arg(qrzCallsignData.getQra()));
        }
        else if (xmlData.name() == "cqzone")
        {
            qrzCallsignData.setCqZone(xmlData.readElementText());
            trace(QString("Callsign Data: cqZone = %1").arg(qrzCallsignData.getCqZone()));
        }
        else if (xmlData.name() == "ituzone")
        {
            qrzCallsignData.setItuZone(xmlData.readElementText());
            trace(QString("Callsign Data: ituZone = %1").arg(qrzCallsignData.getItuZone()));
        }
        else
        {
           xmlData.skipCurrentElement();
        }
    }
}


void QrzServerMainWindow::parseDXCCData(QXmlStreamReader &xmlData)
{
    Q_UNUSED(xmlData)
}




void QrzServerMainWindow::onConfigure()
{

    QrzConfigureDialog conf;

    conf.logCallsign = logonCallsign;
    conf.logPassword = password;

    int ret = conf.exec();
    if (ret == QDialog::Accepted)
    {
        logonCallsign = conf.logCallsign.trimmed();
        password = conf.logPassword.trimmed();

        QSettings settings;

        settings.setValue("logonCallsign", logonCallsign);
        settings.setValue("password", password);
    }



}

void QrzServerMainWindow::clusterClientServerList(QVector<ClusterServer> serverList)
{

    foreach ( auto const &s, serverList )
    {
        QString state = QString(clusterStateList[s.state]) + " " + s.app + "\r\n";
        trace(QString("clusterClientServerList - state = %1").arg(state));

    }
}



void QrzServerMainWindow::onClusterQrzMessage(QrzServerMessage qrzRequest)
{

    trace(QString("onClusterQrzMessage: add message to queue, callsign %1").arg(qrzRequest.getDxCall()));
    qrzRequestQueue += qrzRequest;
}

void QrzServerMainWindow::onLoggerQrzMsg(QrzServerMessage qrzRequest)
{
    trace(QString("onLoggerQrzMessage: add message to queue, callsign %1").arg(qrzRequest.getDxCall()));
    qrzRequestQueue += qrzRequest;
}


void QrzServerMainWindow::handleQrzRequests()
{
    if (!qrzRequestQueue.isEmpty())
    {
        if (qrzServerStateFlags.getQrzLoggedOnFlag())
        {
            if (!qrzServerStateFlags.getAskCallsignFlag())
            {
                requestedStation.clear();

                requestedStation = qrzRequestQueue[0];
                qrzRequestQueue.remove(0);

                ui->errorText->clear();
                ui->messageText->clear();
                askQrzCallsign.clear();

                if (requestedStation.getLoggerFlag())
                {
                    addTextToLogWindow(tr("Callsign received from logger - %1").arg(requestedStation.getDxCall()));
                    trace(QString("handleQrzRequests: Callsign received from logger - %1").arg(requestedStation.getDxCall()));
                }
                else
                {
                    addTextToLogWindow(tr("Callsign received from cluster - %1").arg(requestedStation.getDxCall()));
                    trace(QString("handleQrzRequests: Callsign received from cluster - %1").arg(requestedStation.getDxCall()));
                }



                Callsign callsign;
                callsign.setFullCall(requestedStation.getDxCall());
                if (callsign.getValRes() == CS_OK)
                {
                    addTextToLogWindow(tr("Callsign is valid - %1").arg(requestedStation.getDxCall()));
                    trace(QString("handleQrzRequests: callsign is valid - %1").arg(requestedStation.getDxCall()));

                    askQrzCallsign = callsign.realCall;

                    // ask for qra locator
                    addTextToLogWindow(tr("Ask QRZ for callsign - %1").arg(askQrzCallsign));
                    trace(QString("handleQrzRequests: ask qrz data for callsign %1").arg(askQrzCallsign));

                    qrzServerStateFlags.setAskCallsignFlag(true);
                    askCallsignData(askQrzCallsign);
                }
                else
                {
                    addTextToLogWindow(tr("Callsign is invalid - %1").arg(requestedStation.getDxCall()));
                    trace(QString("handleQrzRequests: callsign is invalid - %1").arg(requestedStation.getDxCall()));

                }


            }

        }
        else
        {

            QrzServerRpc::getQrzServerRpc()->sendQrzResponseToClusterServer(requestedStation.getDxCall(), "", rpcConstants::qrzServerLoggedOut, requestedStation.getSpotterCall(), "", rpcConstants::qrzServerLoggedOut);

        }

    }
}



void QrzServerMainWindow::addTextToLogWindow(QString message)
{
    ui->messageTextWindow-> appendPlainText(QTime::currentTime().toString("hh:mm:ss.z") + " " + message);
}

void QrzServerMainWindow::addToErrorTextLabel(QString message)
{
    ui->errorText->clear();
    ui->errorText->setText(message);
}

void QrzServerMainWindow::addToMessageTextLabel(QString message)
{
    ui->messageText->clear();
    ui->messageText->setText(message);
}

void QrzServerMainWindow::setQrzStatusConnected(bool state)
{
    if (state)
    {
        ui->statusText->clear();
        ui->statusText->setText("Connected");
        ui->connectPb->setStyleSheet(QRZ_BUTTON_ON_STYLE);
    }
    else
    {
        ui->statusText->clear();
        ui->statusText->setText("Disconnected");
        ui->connectPb->setStyleSheet(QRZ_BUTTON_OFF_STYLE);
    }
}
