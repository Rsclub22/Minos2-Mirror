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
#include <QSharedPointer>
#include <QSettings>
#include <QProcessEnvironment>
#include "AppStartup.h"
#include "MinosRPC.h"
#include "RPCCommandConstants.h"
#include "SecondInstall.h"
#include "qrzservermainwindow.h"
#include "qrzserverrpc.h"
#include "LogEvents.h"
#include "MTrace.h"
#include "callsign.h"

#include "qrzconfiguredialog.h"
#include "ui_qrzservermainwindow.h"

QrzServerMainWindow::QrzServerMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QrzServerMainWindow)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);


    connect(stdinReader, &StdInReader::stdinLine, this, &QrzServerMainWindow::onStdInRead);

    QSettings settings;
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);



    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    appName = env.value("MQTRPCNAME", "") ;

    // this is only needed for standalone to store to registry - appname might not be correct here!
    if (appName.isEmpty())
    {
        QApplication::setOrganizationName( SecondInstall::getOrgName() );
        QApplication::setOrganizationDomain( "g0gjv.org.uk" );
        QApplication::QCoreApplication::setApplicationName( "mqtQrzServer" );

    }

    logonCallsign = settings.value("logonCallsign", "").toString();
    password = settings.value("password", "").toString();

    createCloseEvent();

    connect(&LogTimer, &QTimer::timeout, this, &QrzServerMainWindow::LogTimerTimer);
    LogTimer.start(100);

    trace(QString("AppName = %1").arg(appName));
    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName());
    Q_UNUSED(rpc)

    QStringList sv{
            rpcConstants::clusterApp, rpcConstants::qrzDisplayApp
        };

    connect (QrzServerRpc::getQrzServerRpc(), &QrzServerRpc::clusterQrzMsg, this, &QrzServerMainWindow::onClusterQrzMessage);
    connect (QrzServerRpc::getQrzServerRpc(), &QrzServerRpc::loggerQrzMsg, this, &QrzServerMainWindow::onLoggerQrzMsg);



    connect(ui->actionSetup_QRZ, &QAction::triggered, this, [=](){onConfigure();});

    connect(ui->connectPb, &QPushButton::clicked, this, [=](){onConnectPushButtonClicked();});

    checkQrzRequestsTimer = new QTimer(this);
    connect(checkQrzRequestsTimer, &QTimer::timeout, this, [=](){handleQrzRequests();});
    checkQrzRequestsTimer->start(500);

    pingStateTimer = new QTimer(this);
    connect(pingStateTimer, &QTimer::timeout, this, [=](){onPingStateTimerTimeout();});
    pingStateTimer->start(5000);

    ui->messageTextWindow->isReadOnly();
    addTextToLogWindow(tr("Note! An xml subscription is required to look up QRA data on QRZ.com"));


    logon();


}

QrzServerMainWindow::~QrzServerMainWindow()
{
    delete ui;
}
void QrzServerMainWindow::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    QWidget::resizeEvent(event);
}
void QrzServerMainWindow::moveEvent(QMoveEvent * event)
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    QWidget::moveEvent(event);
}
void QrzServerMainWindow::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("geometry", saveGeometry());
    }
}
void QrzServerMainWindow::closeEvent(QCloseEvent *event)
{
    trace("QrzServerMainWindow::closeEvent");

    QSettings settings;
    settings.setValue("geometry", saveGeometry());

    QWidget::closeEvent(event);
}

void QrzServerMainWindow::LogTimerTimer()
{
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

void QrzServerMainWindow::onConnectPushButtonClicked()
{
    logon();

}

void QrzServerMainWindow::onPingStateTimerTimeout()
{
        QrzServerRpc::getQrzServerRpc()->sendQrzLoggedState(qrzServerStateFlags.getQrzLoggedOnFlag(), stateErrorMessage);

}

void QrzServerMainWindow::onStdInRead(QString cmd)
{
    if (cmd.indexOf("Shutdown", 0, Qt::CaseInsensitive) >= 0)
    {
        close();
    }
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
                if (xmlData.name().contains(QString("QRZDatabase")))
                {
                    if (xmlData.readNextStartElement())
                    {
                        if (xmlData.name().contains(QString("Session")))
                        {
                            parseSessionData(xmlData);
                            sessionDataReceived();
                        }
                        else if (xmlData.name().contains(QString("Callsign")))
                        {

                            parseCallsignData(xmlData);
                            callsignDataReceived();

                        }
                        else if (xmlData.name().contains(QString("DXCC")))
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
        QString sslError;
        if (!QSslSocket::supportsSsl())
        {
            sslError = "\r\n" + tr("SSL not supported on this system.");
        }

        QString msg = QString( "HTPP Get of " ) + url_ + " failed: " + reply->errorString() + sslError;
        trace ( QString( "HTPP Get of " ) + url_ + " failed: " + reply->errorString()  + sslError );
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
        //if (qrzServerStateFlags.getAskCallsignFlag())
        //{
        //    qrzServerStateFlags.setAskCallsignFlag(false);
        //}

        //if (qrzServerStateFlags.getAskLogonFlag())
        //{
        //    qrzServerStateFlags.setAskLogonFlag(false);
        //}

    }




    if (qrzServerStateFlags.getAskLogonFlag())
    {
        if (!qrzSessionData.getKey().isEmpty() && qrzSessionData.getError().isEmpty())
        {
            // logon succesfull
            qrzServerStateFlags.setQrzLoggedOnFlag(true);
            qrzServerStateFlags.setAskLogonFlag(false);
            QString msg = QString("Qrz Logged on Ok with call %1").arg(logonCallsign);
            trace(msg);
            addTextToLogWindow(tr("Qrz logged on Ok with call %1").arg(logonCallsign));
            addToErrorTextLabel("");
            addToMessageTextLabel("");
            setQrzStatusConnected(true);
        }
        else
        {
            // send error message to client
            QString errorMsg = qrzSessionData.getError();
            trace(errorMsg);
            addTextToLogWindow(tr("Logon failed to Qrz.com, logon callsign = %1, error = %2").arg(logonCallsign, errorMsg));
            setQrzStatusConnected(false);
            qrzServerStateFlags.clear();

        }

    }


    if (qrzServerStateFlags.getAskCallsignFlag() /*&& requestedStation.getLoggerFlag()*/)
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

        if (qrzServerStateFlags.getAskCallsignFlag())
        {
            qrzServerStateFlags.setAskCallsignFlag(false);
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

        //QrzServerRpc::getQrzServerRpc()->sendQrzResponseToLoggerDisplay(qrzCallsignData, stateMsg, requestedStation.getFromStationName(),requestedStation.getLoggerUuid());

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
            addTextToLogWindow(tr("Cluster Qrz Callsign Data received for call = %1, Qra = %2 - Send to Cluster Server").arg(requestedStation.getDxCall(), qrzCallsignData.getQra()));

            QrzServerRpc::getQrzServerRpc()->sendQrzResponseToClusterServer(requestedStation.getDxCall(), qrzCallsignData.getQra(), QRA_LOOKUP_OK, requestedStation.getSpotterCall(), "", rpcConstants::qrzServerCallOK);

        }
        else
        {
            // a request from logger
            QString msg = QString(QString("Logger Qrz Callsign Data received for call = %1, Send to Qrz Display in Logger Server").arg(requestedStation.getDxCall()));
            trace(msg);
            addTextToLogWindow(tr("Logger Qrz Callsign Data received for call = %1, Send to Qrz Display in Logger Server").arg(requestedStation.getDxCall()));
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
        if (xmlData.name() == QString("Error"))
        {
           qrzSessionData.setError(xmlData.readElementText());
           trace(QString("Session Data: Error = %1").arg(qrzSessionData.getError()));
        }
        else if(xmlData.name() == QString("Key"))
        {
            qrzSessionData.setKey( xmlData.readElementText() );
            trace(QString("Session Data: Key = %1").arg(qrzSessionData.getKey()));
        }
        else if (xmlData.name() == QString("SubExp"))
        {
            qrzSessionData.setSubExp(xmlData.readElementText());
            trace(QString("Session Data: SubExp = %1").arg(qrzSessionData.getSubExp()));
        }
        else if (xmlData.name() == QString("Message"))
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
        if(xmlData.name() == QString("call"))
        {
            qrzCallsignData.setCallsign(xmlData.readElementText());
            trace(QString("Callsign Data: callsign = %1").arg(qrzCallsignData.getCallsign()));
        }
        else if (xmlData.name() == QString("fname"))
        {
            qrzCallsignData.setFirstName(xmlData.readElementText());
            trace(QString("Callsign Data: first name = %1").arg(qrzCallsignData.getFirstName()));
        }
        else if (xmlData.name() == QString("name"))
        {
            qrzCallsignData.setName(xmlData.readElementText());
            trace(QString("Callsign Data: name = %1").arg(qrzCallsignData.getName()));
        }
        else if (xmlData.name() == QString("addr1"))
        {
            qrzCallsignData.setAddr1(xmlData.readElementText());
            trace(QString("Callsign Data: addr1 = %1").arg(qrzCallsignData.getAddr1()));
        }
        else if (xmlData.name() == QString("addr2"))
        {
            qrzCallsignData.setAddr2(xmlData.readElementText());
            trace(QString("Callsign Data: addr2 = %1").arg(qrzCallsignData.getAddr2()));
        }
        else if (xmlData.name() == QString("county"))
        {
            qrzCallsignData.setCounty(xmlData.readElementText());
            trace(QString("Callsign Data: county = %1").arg(qrzCallsignData.getCounty()));
        }
        else if (xmlData.name() == QString("country"))
        {
            qrzCallsignData.setCountry(xmlData.readElementText());
            trace(QString("Callsign Data: country = %1").arg(qrzCallsignData.getCountry()));
        }
        else if (xmlData.name() == QString("lat"))
        {
            qrzCallsignData.setLat(xmlData.readElementText());
            trace(QString("Callsign Data: lat = %1").arg(qrzCallsignData.getLat()));
        }
        else if (xmlData.name() == QString("lon"))
        {
            qrzCallsignData.setLon(xmlData.readElementText());
            trace(QString("Callsign Data: lon = %1").arg(qrzCallsignData.getLon()));
        }
        else if (xmlData.name() == QString("grid"))
        {
            QString grid = xmlData.readElementText();
            if (grid.count() == 6)
            {
                grid = grid.replace(4, 2, grid.right(2).toUpper());
            }
            qrzCallsignData.setQra(grid);
            trace(QString("Callsign Data: grid = %1").arg(qrzCallsignData.getQra()));
        }
        else if (xmlData.name() == QString("cqzone"))
        {
            qrzCallsignData.setCqZone(xmlData.readElementText());
            trace(QString("Callsign Data: cqZone = %1").arg(qrzCallsignData.getCqZone()));
        }
        else if (xmlData.name() == QString("ituzone"))
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

        bool callsignChanged = false;
        bool passwordChanged = false;


        QSettings settings;

        if (conf.logCallsign.trimmed() != settings.value("logonCallsign", "").toString())
        {
            logonCallsign = conf.logCallsign.trimmed();
            settings.setValue("logonCallsign", logonCallsign);
            callsignChanged = true;

        }

        if (conf.logPassword.trimmed() != settings.value("pasword", "").toString())
        {
            password = conf.logPassword.trimmed();
            settings.setValue("password", password);
            passwordChanged = true;
        }

        if (callsignChanged || passwordChanged || !logonCallsign.isEmpty() || !password.isEmpty())
        {
            qrzSessionData.clear();
            //logon();
        }
        //else if (logonCallsign.isEmpty() || password.isEmpty())
        //{
        //    qrzSessionData.clear();
        //}

    }





}

void QrzServerMainWindow::clusterClientServerList(QVector<ClusterServer> serverList)
{

    for ( auto const &s:qAsConst(serverList) )
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

        trace(QString("handQrzRequests: number of callsigns in queue = %1").arg(qrzRequestQueue.count()));
        if (qrzServerStateFlags.getQrzLoggedOnFlag())
        {
            trace(QString("handQrzRequests: logged on to qrz"));

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

    //ui->messageTextWindow-> appendPlainText(QTime::currentTime().toString("hh:mm:ss.z") + " " + message);
    ui->messageTextWindow-> appendPlainText(QDateTime::currentDateTimeUtc().time().toString("hh:mm:ss.z") + " UTC - " + message);
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
        ui->connectPb->setText("");
        ui->connectPb->setText(tr("Connected"));
        ui->connectPb->setStyleSheet(QRZ_BUTTON_ON_STYLE);
    }
    else
    {
        ui->connectPb->setText("");
        //ui->connectPb->setText(tr(Disconnected"));
        ui->connectPb->setStyleSheet(QRZ_BUTTON_OFF_STYLE);
    }
}
