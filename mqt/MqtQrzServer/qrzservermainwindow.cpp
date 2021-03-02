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


    connect(&stdinReader, SIGNAL(stdinLine(QString)), this, SLOT(onStdInRead(QString)));
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


    connect (QrzServerRpc::getQrzServerRpc(), SIGNAL(clusterQrzMsg(QrzServerMessage)), this, SLOT(onClusterQrzMessage(QrzServerMessage)));


    connect(ui->actionSetup_QRZ, &QAction::triggered, this, [=]{onConfigure();});

    logon();

    //askCallsignData("M0SAT");

}

QrzServerMainWindow::~QrzServerMainWindow()
{
    delete ui;
}

void QrzServerMainWindow::LogTimerTimer()
{
    bool show = getShowServers();
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
    QSettings settings;

    qrzSessionData.clear();


    if (logonCallsign.isEmpty() || password.isEmpty())
    {
        onConfigure();
    }

    QString logonQrz = QRZURL + "username=" + logonCallsign.trimmed() + ";password=" + password.trimmed() + ";agent=" + AGENT;

    sendUrl(logonQrz);
}

void QrzServerMainWindow::askCallsignData(QString callsign)
{
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
    QObject::connect(&m_NetworkMngr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    eventLoop.exec(); // blocks stack until "finished()" has been called


    if ( reply->error() == QNetworkReply::NoError )
    {
        int raw = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (raw == 301)
        {
            QUrl redirect =  reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

            QNetworkRequest qnr1( redirect );
            qnr1.setRawHeader( "User-Agent" , "Mozilla/4.0 (compatible;Minos2)" );

            reply = QSharedPointer<QNetworkReply>(m_NetworkMngr.get( qnr1 ));
            QEventLoop loop;
            QObject::connect( reply.data(), SIGNAL( finished() ), &loop, SLOT( quit() ) );
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
        trace ( QString( "HTPP Get of " ) + url_ + " failed: " + reply->errorString() );

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

}


void QrzServerMainWindow::callsignDataReceived()
{

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
        else
        {
           xmlData.skipCurrentElement();
        }
    }
}



void QrzServerMainWindow::parseCallsignData(QXmlStreamReader &xmlData)
{
    trace(QString("Parse Callsign Data"));
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
        else if (xmlData.name() == "addr2")
        {
            qrzCallsignData.setQth(xmlData.readElementText());
            trace(QString("Callsign Data: qth = %1").arg(qrzCallsignData.getQth()));
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
            qrzCallsignData.setQra(xmlData.readElementText());
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


int QrzServerMainWindow::parseTest()
{
    QFile file("C:/Qt_Projects/minos-minos/runtime/Configuration/qrzTest.xml");
        if(!file.open(QFile::ReadOnly | QFile::Text)){
            qDebug() << "Cannot read file" << file.errorString();
            exit(0);
        }

        QXmlStreamReader reader(&file);

        if (reader.readNextStartElement())
        {
            if (reader.name().contains("QRZDatabase"))
            {
                if (reader.readNextStartElement())
                {
                    if (reader.name().contains("Callsign"))
                    {
                        while(reader.readNextStartElement())
                        {
                            if(reader.name() == "call")
                            {
                                QString callsign = reader.readElementText();
                                qDebug() << "callsign = " << qPrintable(callsign);
                            }
                            else if (reader.name() == "fname")
                            {
                                QString firstName = reader.readElementText();
                                qDebug() << "first name = " << qPrintable(firstName);
                            }
                            else if (reader.name() == "name")
                            {
                                QString name = reader.readElementText();
                                qDebug() << "name = " << qPrintable(name);
                            }
                            else if (reader.name() == "addr2")
                            {
                                QString qth = reader.readElementText();
                                qDebug() << "qth = " << qPrintable(qth);
                            }
                            else if (reader.name() == "county")
                            {
                                QString county = reader.readElementText();
                                qDebug() << "county = " << qPrintable(county);
                            }
                            else if (reader.name() == "country")
                            {
                                QString country = reader.readElementText();
                                qDebug() << "country = " << qPrintable(country);
                            }
                            else if (reader.name() == "lat")
                            {
                                QString lat = reader.readElementText();
                                qDebug() << "lat = " << qPrintable(lat);
                            }
                            else if (reader.name() == "lon")
                            {
                                QString lon = reader.readElementText();
                                qDebug() << "lon = " << qPrintable(lon);
                            }
                            else if (reader.name() == "grid")
                            {
                                QString qra = reader.readElementText();
                                qDebug() << "grid = " << qPrintable(qra);
                            }
                            else if (reader.name() == "cqzone")
                            {
                                QString cqZone = reader.readElementText();
                                qDebug() << "cqZone = " << qPrintable(cqZone);
                            }
                            else if (reader.name() == "ituzone")
                            {
                                QString ituZone = reader.readElementText();
                                qDebug() << "ituZone = " <<  qPrintable(ituZone);
                            }
                            else
                            {
                               reader.skipCurrentElement();
                            }
                        }


                    }
                }



            }
            else
            {
                reader.raiseError(QObject::tr("Incorrect file"));
            }
        }




        return 0;
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

    qrzRequestQueue += qrzRequest;
}


void QrzServerMainWindow::handleQrzRequests()
{
    if (!qrzRequestQueue.isEmpty())
    {
        requestedStation.clear();

        requestedStation = qrzRequestQueue[0];
        qrzRequestQueue.remove(0);
    }
}
