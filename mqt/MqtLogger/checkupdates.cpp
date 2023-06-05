#include <QDesktopServices>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>

#include "SecondInstall.h"
#include "MShowMessageDlg.h"
#include "MTrace.h"
#include "regsettings.h"

#include "checkupdates.h"
#include "ui_checkupdates.h"

CheckUpdates::CheckUpdates(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CheckUpdates)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("CheckUpdates/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    QString Version = QString(STRINGVERSION)  + " " + PRERELEASETYPE + " " + SecondInstall::getSecondInstallText() ;

    QString sfJson = downloadFile();

    QString fileName;

    QJsonParseError err;
    QJsonDocument json = QJsonDocument::fromJson(sfJson.toUtf8(), &err);
    if (!err.error)
    {
        if (json.isObject())
        {
            QJsonObject sconf = json.object();
            QJsonObject prel = sconf.value("platform_releases").toObject();
            QJsonObject rel = prel.value("windows").toObject();
            fileName = getString(rel, "filename", QString());

            QStringList ver1 = fileName.split(".");
            QStringList ver2 = ver1[0].split("_");
            ver2.removeAt(0);
            if (ver2.count() == 3)
            {
                ver2.append("0");
            }
            fileName = ver2.join(".");

        }
    }
    ui->currentVersionLabel->setText(tr("This Minos version is %1").arg(Version));
    ui->latestVersionLabel->setText(tr("Latest release Minos version is %1").arg(fileName));
}
QString CheckUpdates::getString(QJsonObject o, QString key, QString def)
{
    QJsonValue pe = o.value(key);
    if (pe.isString())
    {
        return pe.toString();
    }
    return def;
}

QString CheckUpdates::downloadFile ( )
{
    QNetworkAccessManager m_NetworkMngr;
    QString sfUrl("https://sourceforge.net/projects/minos/best_release.json");

    QUrl qurl( sfUrl );
    QNetworkRequest qnr( qurl );

    qnr.setRawHeader( "User-Agent" , "Windows NT (compatible;Minos2)" );

    QSharedPointer<QNetworkReply> reply = QSharedPointer<QNetworkReply>(m_NetworkMngr.get( qnr ));

    QEventLoop loop;
    QObject::connect( reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if ( reply->error() == QNetworkReply::NoError )
    {
        int raw = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (raw == 301)
        {
            QUrl redirect =  reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

            QNetworkRequest qnr1( redirect );
            qnr1.setRawHeader( "User-Agent" , "Windows NT (compatible;Minos2)" );

            reply = QSharedPointer<QNetworkReply>(m_NetworkMngr.get( qnr1 ));
            QEventLoop loop;
            QObject::connect( reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
            loop.exec();
            raw = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        }
        QByteArray data = reply->readAll();
        if (raw == 200 && data.size() > 0)
        {
            trace ( "HTTP Get of " + sfUrl + " OK size " + QString::number(data.size()) );
            return data;
        }
        else
        {
           trace ( "HTTP Get of " + sfUrl + " failed - zero length data returned with attribute " + QString::number(raw));
           if (data.size() > 0)
           {
               trace(data);
           }
           return QString();
        }
        return data;
    }
    else
    {
        trace ( QString( "HTTP Get of " ) + sfUrl + " failed: " + reply->errorString() );
        mShowMessage ( QString( "HTTP Get of " ) + sfUrl + " failed: " + reply->errorString(), this );
    }
    return QString();
}

CheckUpdates::~CheckUpdates()
{
    delete ui;
}
void CheckUpdates::doCloseEvent()
{
    RegSettings settings;
    settings.getSettings().setValue("CheckUpdates/geometry", saveGeometry());
}

void CheckUpdates::on_closeButton_clicked()
{
    doCloseEvent();
    accept();
}


void CheckUpdates::on_sourceforgeButton_clicked()
{
    QDesktopServices::openUrl(QUrl("https://minos.sourceforge.net/download.html"));
}

