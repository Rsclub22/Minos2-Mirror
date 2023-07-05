#include "MonitorParameters.h"
#include <QApplication>
#ifdef INC_MAP
#include <QQmlApplicationEngine>
#endif

#include "AppStartup.h"
#include "RPCCommandConstants.h"
#include "SecondInstall.h"
#include "MonitorMain.h"

#ifdef INC_MAP
    QSharedPointer<QQmlApplicationEngine> appQmlEngine;
#endif

int main(int argc, char *argv[])
{
    MonitorParameters mp;

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    SecondInstall::parseSecondInstall(argc, argv);
    QApplication a(argc, argv);

    appStartup(rpcConstants::monitorApp);

    MonitorMain w;
    w.show();

    auto r = a.exec();
#ifdef INC_MAP
    appQmlEngine.clear();
#endif
    return r;

}
