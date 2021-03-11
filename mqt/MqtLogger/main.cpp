#include "base_pch.h"
#include "singleapplication.h"
#include "tlogcontainer.h"

#include "fileutils.h"
#include "AppStartup.h"

#ifdef _MSC_VER
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
#endif // _MSC_VER


int main(int argc, char *argv[])
{
    int appError = 1;
    {
        SingleApplication a( QString("MinosLogger"), argc, argv);
        if (a.isRunning())
        {
            if (argc > 1)
            {
                a.sendArgs();
            }
            else
            {
                QMessageBox msgBox;
                msgBox.setText("Minos Logger is already running!");
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
        delete w;
    }

    return appError;
}
