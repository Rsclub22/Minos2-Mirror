#include <QApplication>
#include <QMessageBox>

#ifdef Q_OS_WIN
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QQmlApplicationEngine>
#endif
#endif

#include "RPCCommandConstants.h"
#include "SecondInstall.h"
#include "singleapplication.h"
#include "tlogcontainer.h"

#include "AppStartup.h"

#ifdef _MSC_VER
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
#endif // _MSC_VER

#ifdef Q_OS_WIN
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) || QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
QSharedPointer<QQmlApplicationEngine> appQmlEngine;
#endif
#endif

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    SecondInstall::parseSecondInstall(argc, argv);
    int appError = 1;
    {
        SingleApplication a( SecondInstall::getSingleAppLoggerName(), argc, argv);
        if (a.isRunning())
        {
            if (argc > 1)
            {
                a.sendArgs();
            }
            else
            {
                QMessageBox msgBox;
                msgBox.setText(SecondInstall::getSingleAppLoggerDescription() + " is already running!");
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.addButton("Close", QMessageBox::RejectRole);
                msgBox.exec();
            }
            return appError;
        }
        appStartup(rpcConstants::loggerApp);

#ifdef Q_OS_ANDROID
        QString sdCard = getenv("EXTERNAL_STORAGE")+ QString("/uk.org.g0gjv.minos");

        SetCurrentDir(sdCard);
        QString here = GetCurrentDir();

        if (!DirectoryExists(sdCard + "/Configuration"))
        {
            CreateDir(sdCard + "/Configuration");
            QFile::copy("assets:/Configuration/MinosLogger.ini",sdCard + "/Configuration/MinosLogger.ini");
            //If it's a db file, you need write access:
            QFile::setPermissions(sdCard + "/Configuration/MinosLogger.ini",QFile::ReadOwner|QFile::WriteOwner);
        }
#endif

        //a.setStyle("fusion");

        TLogContainer *w = new TLogContainer();
        w->connect(&a, &SingleApplication::argsReceived, w, &TLogContainer::onArgsReceived);

        setAppFont();

        bool ret = w->show(argc, argv);
        if (ret == true)
        {
            appError = a.exec();
        }
#ifdef Q_OS_WIN
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) || QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    appQmlEngine.clear();
#endif
#endif
        delete w;
    }

    return appError;
}
