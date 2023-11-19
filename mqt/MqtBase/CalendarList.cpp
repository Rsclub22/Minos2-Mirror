#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>

#include "AppStartup.h"
#include "Calendar.h"
#include "CalendarList.h"
#include "MShowMessageDlg.h"
#include "MTrace.h"

QMap<QString, QString> contestNameMap;
int calendarFormYear = 0;

QString calendarNameString[] =
    {
        "vhfcontests",
        "hfcontests",
        "microcontests",
        "VHFContestsOther",
        "HFContestsOther",
        "bartgcontests"
    };

//---------------------------------------------------------------------------
QString CalendarYear::getPath()
{
    QString path =  getDirectoryLocation(dlConfiguration) + "/" + calendarNameString[ type ] + yearString() + ".xml";
    return path;
}
QString CalendarYear::getURL()
{
    QString url = getSite() + calendarNameString[ type ] + yearString() + ".xml";
    return url;

}
//---------------------------------------------------------------------------
QString VHFCalendarYear::getSite()
{
    return "https://www.rsgbcc.org/vhf/";
}
//---------------------------------------------------------------------------
QString HFCalendarYear::getSite()
{
    // Yes, the HF calendar is under the VHF directory!
    return "https://www.rsgbcc.org/vhf/";
}
//---------------------------------------------------------------------------
QString HFBARTGCalendarYear::getSite()
{
    return "https://bartg.rsgbcc.org/";
}
//---------------------------------------------------------------------------
QString MicroCalendarYear::getSite()
{
    return "https://microwave.rsgbcc.org/";
}
//---------------------------------------------------------------------------
QString VHFOtherCalendarYear::getSite()
{
    return "";
}
QString VHFOtherCalendarYear::getPath()
{
    QString p = getDirectoryLocation(dlConfiguration) + "/" +  calendarNameString[ type ] + ".xml";
    return p;
}
QString VHFOtherCalendarYear::getURL()
{
    // No URL
    return QString();
}
//---------------------------------------------------------------------------
QString HFOtherCalendarYear::getSite()
{
    return "";
}
QString HFOtherCalendarYear::getPath()
{
    QString p = getDirectoryLocation(dlConfiguration) + "/" + calendarNameString[ type ] + ".xml";
    return p;
}
QString HFOtherCalendarYear::getURL()
{
    // No URL
    return QString();
}
//---------------------------------------------------------------------------
QString CTYCalendarYear::getSite()
{
    return "https://www.country-files.com/cty/cty.dat";
}
QString CTYCalendarYear::getPath()
{
    QString p = getDirectoryLocation(dlConfiguration) + "/cty.dat";
    return p;
}
QString CTYCalendarYear::getURL()
{
    return getSite();
}
//---------------------------------------------------------------------------
QString ClubListCalendarYear::getSite()
{
    return "https://www.rsgbcc.org/cgi-bin/vhfenter.pl?afsdownload=y";
}
QString ClubListCalendarYear::getPath()
{
    QString p = getDirectoryLocation(dlConfiguration) + "/clublist.txt";
    return p;
}
QString ClubListCalendarYear::getURL()
{
    return getSite();
}
bool ClubListCalendarYear::downloadProvisionalFile ( bool /*showError*/ )
{
    return false;
}
//---------------------------------------------------------------------------
void CalendarYear::ignoreSslErrors(const QList<QSslError> &errors)
{
// All then error ignore stuff comes from
// https://myprogrammingnotes.com/ssltls-handshake-failed-encrypted-channel-established-sslerrors-signal-emitted.html

// strangely, it only appears to have been needed on a "virgin" Windows 10 VM
    for (const auto &e:errors)
    {
        trace(QString("CalendarYear::ignoreSslErrors %1").arg(e.errorString()));
    }
}
//---------------------------------------------------------------------------
bool CalendarYear::downloadFile ( bool showError, QWidget *parent )
{
    QString calendarURL = getURL();


    QNetworkAccessManager m_NetworkMngr;

    QUrl qurl( calendarURL );
    QNetworkRequest qnr( qurl );

    QSslConfiguration conf = qnr.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    qnr.setSslConfiguration(conf);

    qnr.setRawHeader( "User-Agent" , "Mozilla/4.0 (compatible;Minos2)" );

    QSharedPointer<QNetworkReply> reply = QSharedPointer<QNetworkReply>(m_NetworkMngr.get( qnr ));

    connect( reply.data(), &QNetworkReply::sslErrors, this, &CalendarYear::ignoreSslErrors);

    QEventLoop loop;
    connect( reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);

    loop.exec();

    if ( reply->error() == QNetworkReply::NoError )
    {
        int raw = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (raw == 301)
        {
            QUrl redirect =  reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

            QNetworkRequest qnr1( redirect );
            QSslConfiguration conf = qnr1.sslConfiguration();
            conf.setPeerVerifyMode(QSslSocket::VerifyNone);
            qnr1.setSslConfiguration(conf);
            qnr1.setRawHeader( "User-Agent" , "Mozilla/4.0 (compatible;Minos2)" );

            reply = QSharedPointer<QNetworkReply>(m_NetworkMngr.get( qnr1 ));
            QEventLoop loop;
            connect( reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
            connect( reply.data(), &QNetworkReply::sslErrors, this, &CalendarYear::ignoreSslErrors);
            loop.exec();
            raw = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        }
        QByteArray data = reply->readAll();
        if (raw == 200 && data.size() > 0)
        {
            QFile file( getPath() );
            file.open( QIODevice::WriteOnly );
            file.write( data );
            trace ( "HTTP Get of " + calendarURL + " OK size " + QString::number(data.size()) );
        }
        else
        {
           trace ( "HTTP Get of " + calendarURL + " failed - zero length data returned with attribute " + QString::number(raw));
           if (data.size() > 0)
           {
               trace(data);
           }
           return false;
        }
        return true;
    }
    else
    {
        trace ( QString( "HTTP Get of " ) + calendarURL + " failed: " + reply->errorString() );
        if ( showError )
        {
            mShowMessage ( QString( "HTTP Get of " ) + calendarURL + " failed: " + reply->errorString(), parent );
        }
    }
    return false;
}
