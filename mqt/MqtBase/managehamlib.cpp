#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QDir>
#include "MShowMessageDlg.h"
#include "MTrace.h"
#include "ServerEvent.h"
#include "ConfigFile.h"
#include "fileutils.h"
#include "managehamlib.h"
#include "ui_managehamlib.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

ManageHamlib::ManageHamlib(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ManageHamlib)
{
    ui->setupUi(this);
    checkHamlib();
}

ManageHamlib::~ManageHamlib()
{
    delete ui;
}
#ifdef Q_OS_WIN
QString lastError( DWORD erno )
{
    LPVOID lpMsgBuf;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        erno,
        MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),    // Default language
        ( LPTSTR ) & lpMsgBuf,
        0,
        NULL
        );
    const wchar_t * s;
    s = reinterpret_cast<const wchar_t*>( lpMsgBuf );
    QString qs = QString("%1").arg(s);

    // Free the buffer.
    LocalFree( lpMsgBuf );
    return qs;
}
QString lastError( void )
{
    return lastError( GetLastError() );
}
#endif
bool ManageHamlib::checkHamlib()
{
#ifdef Q_OS_WIN
    HMODULE h = LoadLibraryA("libhamlib-4.dll");
    if (!h)
    {
        QString mess = tr("Failed to load libhamlib-4.dll %1").arg(lastError());
        trace(mess);
        mShowMessage(mess, this);
        return false;
    }

    bool lenOK = false;

    int blen = 1024;
    while (!lenOK)
    {
        wchar_t buff[blen];
        int len = GetModuleFileName(h, buff, blen);
        if (len == blen)
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                blen += 1024;
            }
            else
            {
                lenOK = true;
            }

        }
        else
        {
            lenOK = true;
        }
        if (lenOK)
        {
            hamlibDLLPath = QString::fromWCharArray(buff);
            trace(QString("ManageHamlib %1").arg(hamlibDLLPath));
        }
    }

    ui->hamlibPathLabel->setText(hamlibDLLPath);

    typedef const char * (__stdcall * pRV)();

#if !defined (_MSC_VER)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

    pRV p = reinterpret_cast<pRV>(GetProcAddress(h, "rig_version"));

#if !defined (_MSC_VER)
#pragma GCC diagnostic pop
#endif
    if (p)
    {
        const char * c = p();
        if (c)
        {
            trace(QString("ManageHamlib %1").arg(c));
            ui->verLabel->setText(c);
        }
    }

    FreeLibrary(h);

    QString dataPath = ExtractFileDir(hamlibDLLPath);
    QString hPath = dataPath + "/" + "libhamlib-4.dll";
    QString fs1 = dataPath + "/" + "libhamlib-4_old.dll";
    QString fs2 = dataPath + "/" + "libhamlib-4_new.dll";
    {
        QFile f1 {fs1};
        QFile f2 {fs2};
        QFile h {hPath};

        ui->updateHamlibButton->setEnabled (h.exists());
        ui->revertHamlibButton->setEnabled (f1.exists() && h.exists());
    }

#endif
    return true;
}
void ManageHamlib::on_cancelButton_clicked()
{
    reject();
}

//---------------------------------------------------------------------------
void ManageHamlib::ignoreSslErrors(const QList<QSslError> &errors)
{
    // All then error ignore stuff comes from
    // https://myprogrammingnotes.com/ssltls-handshake-failed-encrypted-channel-established-sslerrors-signal-emitted.html

    // strangely, it only appears to have been needed on a "virgin" Windows 10 VM
    for (const auto &e:errors)
    {
        trace(QString("ManageHamlib::ignoreSslErrors %1").arg(e.errorString()));
    }
}
//---------------------------------------------------------------------------
bool ManageHamlib::downloadFile ( QString url, QString path, bool showError, QWidget *parent )
{

    QNetworkAccessManager m_NetworkMngr;

    QUrl qurl( url );
    QNetworkRequest qnr( qurl );

    QSslConfiguration conf = qnr.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    qnr.setSslConfiguration(conf);

    qnr.setRawHeader( "User-Agent" , "Mozilla/4.0 (compatible;Minos2)" );

    QSharedPointer<QNetworkReply> reply = QSharedPointer<QNetworkReply>(m_NetworkMngr.get( qnr ));

    connect( reply.data(), &QNetworkReply::sslErrors, this, &ManageHamlib::ignoreSslErrors);

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
            connect( reply.data(), &QNetworkReply::sslErrors, this, &ManageHamlib::ignoreSslErrors);
            loop.exec();
            raw = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        }
        QByteArray data = reply->readAll();
        qsizetype ds = data.size();
        if (raw == 200 && ds > 0)
        {
            QFile file( path );
            file.open( QIODevice::WriteOnly );
            file.write( data );
            trace ( "HTTP Get of " + url + " OK size " + QString::number(ds) );
        }
        else
        {
            trace ( "HTTP Get of " + url + " failed - zero length data returned with attribute " + QString::number(raw));
            if (ds > 0)
            {
                trace(data);
            }
            return false;
        }
        return true;
    }
    else
    {
        trace ( QString( "HTTP Get of " ) + url + " failed: " + reply->errorString() );
        if ( showError )
        {
            mShowMessage ( QString( "HTTP Get of " ) + url + " failed: " + reply->errorString(), parent );
        }
    }
    return false;
}

bool isBuild64Bit() {
    static bool b = QSysInfo::buildCpuArchitecture().contains(QLatin1String("64"));
    return b;
}
void ManageHamlib::on_updateHamlibButton_clicked()
{
    QString dataPath = ExtractFileDir(hamlibDLLPath);
    QString hPath = dataPath + "/" + "libhamlib-4.dll";
    QString fs1 = dataPath + "/" + "libhamlib-4_old.dll";
    QString fs2 = dataPath + "/" + "libhamlib-4_new.dll";
    {
        QFile f1 {fs1};
        QFile f2 {fs2};
        if (f1.exists()) f1.remove();
        if (f2.exists()) f2.remove();
    }

    ui->updateHamlibButton->setEnabled (false);
    ui->revertHamlibButton->setEnabled (false);

    QString url;
    if (isBuild64Bit())
    {
        url = "https://n0nb.users.sourceforge.net/dll64/libhamlib-4.dll";
    }
    else
    {
        url = "https://n0nb.users.sourceforge.net/dll32/libhamlib-4.dll";
    }
    bool routerRunning = checkRouterReady();

    if (downloadFile(url, fs2, true, this))
    {
        bool renOK = QFile::rename(hPath, fs1);
        if (!renOK)
        {
            mShowMessage(tr("Failed to rename %1 to %2").arg(hPath, fs1), this);
        }
        else
        {
            renOK = QFile::rename(fs2, hPath);
            if (!renOK)
            {
                mShowMessage(tr("Failed to rename %1 to %2").arg(hPath, fs1), this);
            }
            else
            {
                mShowMessage(tr ("Hamlib Update successful \n\nNew Hamlib will be used after restart"), this);
                if (routerRunning && checkHamlib())
                {
                    MinosConfig::getMinosConfig() ->bounce();
                }
            }
        }
    }
    ui->revertHamlibButton->setEnabled (true);
    ui->updateHamlibButton->setEnabled (true);



}
void ManageHamlib::on_revertHamlibButton_clicked()
{
    QString dataPath = ExtractFileDir(hamlibDLLPath);
    QString hPath = dataPath + "/" + "libhamlib-4.dll";
    QString fs1 = dataPath + "/" + "libhamlib-4_old.dll";
    QString fs2 = dataPath + "/" + "libhamlib-4_new.dll";
    QFile f1 {fs1};
    QFile f2 {fs2};
    if (f1.exists())
    {
        bool routerRunning = checkRouterReady();
        if (f2.exists()) f2.remove();

        ui->updateHamlibButton->setEnabled (false);
        ui->revertHamlibButton->setEnabled (false);
        bool renOK = QFile::rename(hPath, fs2);
        if (!renOK)
        {
            mShowMessage(tr("Failed to rename %1 to %2").arg(hPath, fs2), this);
        }
        else
        {
            QFile::copy(fs1, hPath);
            mShowMessage( tr ("Hamlib successfully reverted \n\nReverted Hamlib will be used after restart"), this);
            if (f1.exists()) f1.remove();

            if (routerRunning && checkHamlib())
            {
                MinosConfig::getMinosConfig() ->bounce();
            }

        }
    }
    else
    {
        mShowMessage( tr ("No Hamlib update found that could be reverted"), this);
    }
    ui->revertHamlibButton->setEnabled (true);
    ui->updateHamlibButton->setEnabled (true);
}

