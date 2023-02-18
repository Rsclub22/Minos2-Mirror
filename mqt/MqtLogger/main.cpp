#include <QApplication>
#include <QMessageBox>

#ifdef INC_MAP
#include <QQmlApplicationEngine>
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

#ifdef INC_MAP
QSharedPointer<QQmlApplicationEngine> appQmlEngine;
#endif

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
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
#ifdef INC_MAP
    appQmlEngine.clear();
#endif
        delete w;
    }

    return appError;
}
