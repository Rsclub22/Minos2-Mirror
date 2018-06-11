#include "base_pch.h"
#include "singleapplication.h"
#include "tlogcontainer.h"

#include "fileutils.h"
#include "AppStartup.h"

#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _MSC_VER

void myMessageOutput(QtMsgType /*type*/,
                     const QMessageLogContext &/*context*/,
                     const QString &/*msg*/)
{
    int a;
    Q_UNUSED(a);
    a = 0;
}

#if defined(_MSC_VER)

// Code to display the memory leak report
// We use a custom report hook to filter out Qt's own memory leaks
// Credit to Andreas Schmidts - http://www.schmidt-web-berlin.de/winfig/blog/?p=154

_CRT_REPORT_HOOK prevHook;

int customReportHook(int /* reportType */, char* message, int* /* returnValue */)
{
    // This function is called several times for each memory leak.
    // Each time a part of the error message is supplied.
    // This holds number of subsequent detail messages after
    // a leak was reported
    const int numFollowupDebugMsgParts = 2;
    static bool ignoreMessage = false;
    static int debugMsgPartsCount = 0;

    // check if the memory leak reporting starts
    if ((strncmp(message, "Detected memory leaks!\n", 10) == 0)
        || ignoreMessage)
    {
        // check if the memory leak reporting ends
        if (strncmp(message, "Object dump complete.\n", 10) == 0)
        {
            _CrtSetReportHook(prevHook);
            ignoreMessage = false;
        }
        else
            ignoreMessage = true;

        // something from our own code?
        if (strstr(message, ".cpp") == NULL)
        {
            if (debugMsgPartsCount++ < numFollowupDebugMsgParts)
                // give it back to _CrtDbgReport() to be printed to the console
                return false;
            else
                return true;  // ignore it
        }
        else
        {
            debugMsgPartsCount = 0;
            // give it back to _CrtDbgReport() to be printed to the console
            return false;
        }
    }
    else
        // give it back to _CrtDbgReport() to be printed to the console
        return true;
}

#endif

int main(int argc, char *argv[])
{
#if defined(_MSC_VER)
//    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);
//    prevHook = _CrtSetReportHook(customReportHook);
//     _CrtSetBreakAlloc(1312120); // Use this line to break at the nth memory allocation
#endif
qInstallMessageHandler(myMessageOutput);
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

        appStartup("MinosQtLogger");

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
        /*
        QSharedMemory mem( "MinosQtLoggerSingleApplication" );
        if ( !mem.create( 1 ) )
        {
            QMessageBox::critical( 0, "Minos Qt Logger Instance detected!", "Application is already running!\nApplication terminating...", "Ok" );
            exit( 0 );
        }
        */

        //a.setStyle("fusion");

        TLogContainer w;
        w.connect(&a, SIGNAL(argsReceived(QString)), &w, SLOT(onArgsReceived(QString)));

        bool ret = w.show(argc, argv);
        if (ret == true)
        {
            appError = a.exec();
        }
    }

#if defined(_MSC_VER)
    // Once the app has finished running and has been deleted,
    // we run this command to view the memory leaks:
//    _CrtDumpMemoryLeaks();
#endif
    return appError;
}
