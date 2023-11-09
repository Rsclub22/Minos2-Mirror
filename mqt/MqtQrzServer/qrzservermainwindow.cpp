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
#include "regsettings.h"
#include "AppStartup.h"
#include "MinosRPC.h"
#include "RPCCommandConstants.h"
#include "delayedaction.h"
#include "qrzdb.h"
#include "qrzservermainwindow.h"
#include "qrzserverrpc.h"
#include "LogEvents.h"
#include "MTrace.h"
#include "callsign.h"

#include "qrzconfiguredialog.h"
#include "ui_qrzservermainwindow.h"

// The QRZ XML format is described at
// https://www.qrz.com/page/current_spec.html

// SubExp response

const char * QRZURL = "https://xmldata.qrz.com/xml/current/?";
const char * AGENT = "Minos";
const char * NONSUBCRIBER = "non-subscriber";

QrzServerMainWindow::QrzServerMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QrzServerMainWindow)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    trace("Connect to stdinRead");  // This connect doesn't appear to work for some time!
    connect(stdinReader, &StdInReader::stdinLine, this, &QrzServerMainWindow::onStdInRead);

    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    QString fileName = "./Configuration/QRZServer.ini";
    QSettings config(fileName, QSettings::IniFormat);

    logonCallsign = config.value("logonCallsign", "").toString();
    password = config.value("password", "").toString();

    if (logonCallsign.isEmpty() && password.isEmpty())
    {
        QSettings qsettings;
        logonCallsign = qsettings.value("logonCallsign", "").toString();
        password = qsettings.value("password", "").toString();
        if (!logonCallsign.isEmpty() || !password.isEmpty())
        {
            config.setValue("logonCallsign", logonCallsign);
            config.setValue("password", password);
        }
    }

    createCloseEvent();

    connect(&LogTimer, &QTimer::timeout, this, &QrzServerMainWindow::LogTimerTimer);
    LogTimer.start(100);

    QString appName = getAppStartupName();
    trace(QString("AppName = %1").arg(appName));
    MinosRPC *rpc = MinosRPC::getMinosRPC(appName);
    Q_UNUSED(rpc)

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

    delayedAction(this, [=]()
    {
        trace("Starting logon");
        logon();
    }
    );

    sblabel0 = new QLabel( "" );
    statusBar() ->addWidget( sblabel0, 6 );
    sblabel1 = new QLabel( "" );
    statusBar() ->addWidget( sblabel1, 1 );
    sblabel2 = new QLabel( "" );
    statusBar() ->addWidget( sblabel2, 2 );

    qdb = new QRZDB(this);

    dbRecords = qdb->getRecordCount();

}

QrzServerMainWindow::~QrzServerMainWindow()
{
    delete ui;
}
void QrzServerMainWindow::resizeEvent(QResizeEvent * event)
{
    RegSettings settings;
    settings.getSettings().setValue("geometry", saveGeometry());
    QWidget::resizeEvent(event);
}
void QrzServerMainWindow::moveEvent(QMoveEvent * event)
{
    RegSettings settings;
    settings.getSettings().setValue("geometry", saveGeometry());
    QWidget::moveEvent(event);
}
void QrzServerMainWindow::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        RegSettings settings;
        settings.getSettings().setValue("geometry", saveGeometry());
    }
}
void QrzServerMainWindow::closeEvent(QCloseEvent *event)
{
    trace("QrzServerMainWindow::closeEvent");

    RegSettings settings;
    settings.getSettings().setValue("geometry", saveGeometry());

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
    trace(QString("onStdInRead %1").arg(cmd));
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

    QString logonQrz = QString(QRZURL) + "username=" + logonCallsign.trimmed() + ";password=" + password.trimmed() + ";agent=" + AGENT;
    addTextToLogWindow(tr("Logging on to QRZ.com with callsign: %1").arg(logonCallsign));
    sendUrl(logonQrz);
}

bool QrzServerMainWindow::askDBCallsignData(QString callsign)
{
    Callsign cs;
    cs.setFullCall(callsign);

    qrzCallsignData = qdb->getRecord(cs.realCall);

    if (cs.realCall == qrzCallsignData.getCallsign())
    {
        qrzCallsignData.setDataSource("DB|" + qrzCallsignData.getDataSource());
        dbRequests++;
        callsignDataReceived();
        return true;
    }
    return false;
}

void QrzServerMainWindow::askCallsignData(QString callsign)
{
    Callsign cs;
    cs.setFullCall(callsign);
    QString callsignUrl = QString(QRZURL) + "s=" + qrzSessionData.getKey() + ";callsign=" + callsign;
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
        QByteArray xml = reply->readAll();
        xmlData.addData( xml );
        if (raw == 200)
        {
            trace(QString("XML read %1").arg(QString(xml)));
            if (xmlData.readNextStartElement())
            {
                if (xmlData.name().contains(QString("QRZDatabase")))
                {
                    bool csRX = false;
                    while (xmlData.readNextStartElement())
                    {
                        if (xmlData.name().contains(QString("Session")))
                        {
                            parseSessionData(xmlData);
                            sessionDataReceived();
                        }
                        else if (xmlData.name().contains(QString("Callsign")))
                        {
                            csRX = true;
                            parseCallsignData(xmlData);

                        }
                        else if (xmlData.name().contains(QString("DXCC")))
                        {
                            parseDXCCData(xmlData);
                        }
                    }
                    if (csRX)
                    {
                        QString sessmess = qrzSessionData.getMessage();
                        if (!sessmess.isEmpty())
                        {
                            qrzCallsignData.setMessage(sessmess);
                        }
                        qrzCallsignData.setDBDate(QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss"));
                        qdb->createRecord(qrzCallsignData);
                        qrzRequests++;
                        dbRecords++;
                        callsignDataReceived();
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
            trace(QString("OpenSSSL version build is %1").arg(QSslSocket::sslLibraryBuildVersionString()));
            sslError = "\r\n" + tr("SSL not supported on this system.");
        }

        QString msg = QString( "HTTP Get of " ) + url_ + " failed: " + reply->errorString() + sslError;
        trace ( QString( "HTTP Get of " ) + url_ + " failed: " + reply->errorString()  + sslError );
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


    if (qrzServerStateFlags.getAskCallsignFlag())
    {
        QString stateMsg;
        if (!qrzSessionData.getError().isEmpty())
        {
            stateMsg = qrzSessionData.getError();
        }
//        else if (!qrzSessionData.getMessage().isEmpty())
//        {
//            stateMsg = qrzSessionData.getMessage();
//        }

        if (!stateMsg.isEmpty())
        {
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
            qrzServerStateFlags.setAskCallsignFlag(false);
        }
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

            qrzCallsignData.setDbRecords(dbRecords);
            qrzCallsignData.setDbRecalls(dbRequests);
            qrzCallsignData.setQrzRecalls(qrzRequests);
            QrzServerRpc::getQrzServerRpc()->sendQrzResponseToLoggerDisplay(qrzCallsignData, stateMsg, requestedStation.getFromStationName(), requestedStation.getLoggerUuid());

        }
        qrzServerStateFlags.setAskCallsignFlag(false);
    }
}


void QrzServerMainWindow::parseSessionData(QXmlStreamReader &xmlData)
{
    trace(QString("Parse Session Data"));
    qrzSessionData.clear();
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

    qrzCallsignData.setDataSource("QRZ.com");

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
            if (grid.size() == 6)
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
        else if (xmlData.name() == QString("moddate"))
        {
            qrzCallsignData.setModDate(xmlData.readElementText());
            trace(QString("Callsign Data: moddate = %1").arg(qrzCallsignData.getModDate()));
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

        QString fileName = "./Configuration/QRZServer.ini";
        QSettings config(fileName, QSettings::IniFormat);

        if (conf.logCallsign.trimmed() != config.value("logonCallsign", "").toString())
        {
            logonCallsign = conf.logCallsign.trimmed();
            config.setValue("logonCallsign", logonCallsign);
            callsignChanged = true;
        }

        if (conf.logPassword.trimmed() != config.value("pasword", "").toString())
        {
            password = conf.logPassword.trimmed();
            config.setValue("password", password);
            passwordChanged = true;
        }

        if (callsignChanged || passwordChanged || !logonCallsign.isEmpty() || !password.isEmpty())
        {
            qrzSessionData.clear();
        }

        if (conf.resetDB)
        {
            qdb->resetDB();
            delete qdb;
            qdb = new QRZDB(this);

            dbRecords = qdb->getRecordCount();
            dbRequests = 0;
            qrzRequests = 0;
        }
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
                    addTextToLogWindow(tr("Ask QRZ DB for callsign - %1").arg(askQrzCallsign));
                    trace(QString("handleQrzRequests: ask qrz database for callsign %1").arg(askQrzCallsign));

                    qrzServerStateFlags.setAskCallsignFlag(true);
                    if (!askDBCallsignData(askQrzCallsign))
                    {
                        addTextToLogWindow(tr("Ask QRZ for callsign - %1").arg(askQrzCallsign));
                        trace(QString("handleQrzRequests: ask qrz for callsign %1").arg(askQrzCallsign));
                        askCallsignData(askQrzCallsign);
                    }
                    else
                    {
                        dbRequests++;
                    }
                    sblabel0->setText(QString("qrz %1").arg(qrzRequests));
                    sblabel1->setText(QString("db %1").arg(dbRequests));
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
