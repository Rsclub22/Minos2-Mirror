#include "mqtUtils_pch.h"
#include <QPalette>
#include <QApplication>
#include <QFileDialog>
#include <QProcessEnvironment>

static QString appStartupName;
QString getAppStartupName()
{
    return appStartupName;
}
static QtMessageHandler oldHandler = nullptr;
void myMessageOutput(QtMsgType type,
                     const QMessageLogContext &context,
                     const QString &msg)
{
    // catch (and trace) application output before a crash
    oldHandler(type, context, msg);

    QString mtype;
    switch (type)
    {
    case QtDebugMsg:
        mtype = "Debug";
        break;
#if QT_VERSION > QT_VERSION_CHECK(5, 4, 0)
    case QtInfoMsg:
        mtype = "Info";
        break;
#endif
    case QtWarningMsg:
        mtype = "Warning";
        break;
    case QtCriticalMsg:
        mtype = "Critical";
        break;
    case QtFatalMsg:
        mtype = "Fatal";
        break;
    }
    QString res = QString("AppMessageHandler (%1, %2 %3:%4): %5").arg(mtype).arg(context.file).arg(context.line).arg(context.function).arg(msg);
    trace(res);
}

void appStartup(const QString &pappName)
{
    oldHandler = qInstallMessageHandler(myMessageOutput);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    appStartupName = env.value("MQTRPCNAME", "") ;

    if (appStartupName.isEmpty())
    {
        appStartupName = pappName;
    }

    QApplication::setOrganizationName( "Minos2Qt" );
    QApplication::setOrganizationDomain( "g0gjv.org.uk" );
    QApplication::QCoreApplication::setApplicationName( appStartupName );

    QSettings settings;
    QVariant qfont = settings.value( "font" );
    if ( qfont != QVariant() )
    {
        QApplication::setFont( qfont.value<QFont>() );
    }

    QApplication *qa = dynamic_cast<QApplication *>(QApplication::instance());
    qa->setStyleSheet(QString("[readOnly=\"true\"] { background-color: %0 }").arg(qa->palette().color(QPalette::Window).name(QColor::HexRgb)));

    if (!DirectoryExists("./Configuration"))
    {
#ifdef Q_OS_ANDROID
        bool createOK = CreateDir("./Configuration");
        if (!mShowOKCancelMessage(0, createOK?"./Config created":"create ./Config failed; Cancel for abort"))
        {
            exit(0);
        }
#else
        // try for executable directory
        QString fpath = QCoreApplication::applicationDirPath();

        if (DirectoryExists(fpath + "/../Configuration"))
        {
            QDir::setCurrent(fpath + "/..");
        }
        int confTries = 0;
        while (!DirectoryExists("./Configuration") )
        {
            if (++confTries > 2)
            {
                exit(-1);
            }
            QString destDir = QFileDialog::getExistingDirectory(
                        nullptr,
                        "Set Minos Working Directory",
                        fpath,
                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
                        );
            if ( !destDir.isEmpty() )
            {

                if (destDir.toUpper().indexOf("/CONFIGURATION") == destDir.size() - QString("/Configuration").size())
                {
                    destDir = destDir.left(destDir.size() - QString("/Configuration").size());
                }
                QDir::setCurrent(destDir);
            }
        }
#endif
    }

    enableTrace( "./TraceLog", appStartupName + "_" );
}

