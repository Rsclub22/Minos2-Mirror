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

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    appName = env.value("MQTRPCNAME", "") ;

    // this is only needed for standalone to store to registry - appname might not be correct here!
    if (appName.isEmpty())
    {
        QApplication::setOrganizationName( "Minos2Qt" );
        QApplication::setOrganizationDomain( "g0gjv.org.uk" );
        QApplication::QCoreApplication::setApplicationName( "mqtQrzServer" );

    }

    QSettings settings;
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    createCloseEvent();

    connect(&LogTimer, &QTimer::timeout, this, [=](){LogTimerTimer();});
    LogTimer.start(100);

    connect(ui->actionSetup_QRZ, &QAction::triggered, this, [=]{onConfigure();});

    logonCallsign = settings.value("logonCallsign", "").toString().trimmed();
    password = settings.value("password", "").toString().trimmed();

    QString qrzURL = "https://xmldata.qrz.com/xml/current/?";

    QString agent = "Minos";


    if (logonCallsign.isEmpty() || password.isEmpty())
    {
        onConfigure();
    }

    QString logonQrz = qrzURL + "username=" + logonCallsign + ";password=" + password + ";agent=" + agent;

    sendUrl(logonQrz);


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
/*
    bool doClose = false;
    if (cmd.indexOf("Shutdown", 0, Qt::CaseInsensitive) >= 0)
    {
//        closeApp = true;
        doClose = true;
    }
    executeStdIn(cmd);
    if (doClose)
        close();
*/
}


void QrzServerMainWindow::quit()
{

}



void QrzServerMainWindow::sendUrl(QString url)
{

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
                        }
                        else if (xmlData.name().contains("Callsign"))
                        {

                            parseCallsignData(xmlData);

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
        // error
        //trace ( QString( "HTPP Get of " ) + calendarURL + " failed: " + reply->errorString() );

    }








}


void QrzServerMainWindow::parseSessionData(QXmlStreamReader &xmlData)
{
    while(xmlData.readNextStartElement())
    {
        if (xmlData.name() == "Error")
        {
           qrzSessionData.setError(xmlData.readElementText());
           qDebug() << "Error = " << qrzSessionData.getError();
        }
        else if(xmlData.name() == "Key")
        {
            qrzSessionData.setKey( xmlData.readElementText() );
            qDebug() << "Key = " << qrzSessionData.getKey();
        }
        else if (xmlData.name() == "SubExp")
        {
            qrzSessionData.setSubExp(xmlData.readElementText());
            qDebug() << "SubExp = " <<  qrzSessionData.getSubExp();
        }
        else
        {
           xmlData.skipCurrentElement();
        }
    }
}



void QrzServerMainWindow::parseCallsignData(QXmlStreamReader &xmlData)
{
    while(xmlData.readNextStartElement())
    {
        if(xmlData.name() == "call")
        {
            qrzCallsignData.setCallsign(xmlData.readElementText());
            qDebug() << "callsign = " << qrzCallsignData.getCallsign();
        }
        else if (xmlData.name() == "fname")
        {
            qrzCallsignData.setFirstName(xmlData.readElementText());
            qDebug() << "first name = " <<  qrzCallsignData.getFirstName();
        }
        else if (xmlData.name() == "name")
        {
            qrzCallsignData.setName(xmlData.readElementText());
            qDebug() << "name = " << qrzCallsignData.getName();
        }
        else if (xmlData.name() == "addr2")
        {
            qrzCallsignData.setQth(xmlData.readElementText());
            qDebug() << "qth = " << qrzCallsignData.getQth();
        }
        else if (xmlData.name() == "county")
        {
            qrzCallsignData.setCounty(xmlData.readElementText());
            qDebug() << "county = " << qrzCallsignData.getCounty();
        }
        else if (xmlData.name() == "country")
        {
            qrzCallsignData.setCountry(xmlData.readElementText());
            qDebug() << "county = " << qrzCallsignData.getCountry();
        }
        else if (xmlData.name() == "lat")
        {
            qrzCallsignData.setLat(xmlData.readElementText());
            qDebug() << "lat = " << qrzCallsignData.getLat();
        }
        else if (xmlData.name() == "lon")
        {
            qrzCallsignData.setLon(xmlData.readElementText());
            qDebug() << "lon = " << qrzCallsignData.getLon();
        }
        else if (xmlData.name() == "grid")
        {
            qrzCallsignData.setQra(xmlData.readElementText());
            qDebug() << "grid = " << qrzCallsignData.getQra();
        }
        else if (xmlData.name() == "cqzone")
        {
            qrzCallsignData.setCqZone(xmlData.readElementText());
            qDebug() << "cqZone = " << qrzCallsignData.getCqZone();
        }
        else if (xmlData.name() == "ituzone")
        {
            qrzCallsignData.setItuZone(xmlData.readElementText());
            qDebug() << "ituZone = " <<  qrzCallsignData.getItuZone();
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
    QFile file("C:/Qt_Projects/build-qrzServer-Desktop_Qt_5_12_10_MinGW_32_bit-Debug/debug/qrzTest.xml");
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
                                qDebug() << "county = " << qPrintable(country);
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


