#include <QApplication>
#include "AppStartup.h"
#include "RPCCommandConstants.h"
#include "SecondInstall.h"
#include "MonitorMain.h"

int main(int argc, char *argv[])
{
    SecondInstall::parseSecondInstall(argc, argv);
    QApplication a(argc, argv);

    appStartup(rpcConstants::monitorApp);

    MonitorMain w;
    w.show();

    return a.exec();
}
