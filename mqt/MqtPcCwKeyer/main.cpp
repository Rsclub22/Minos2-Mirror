#include "pccwkeyermainwindow.h"
#include "RPCCommandConstants.h"
#include "SecondInstall.h"
#include "AppStartup.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    SecondInstall::parseSecondInstall(argc, argv);

    QApplication a(argc, argv);
    appStartup(rpcConstants::pcCwKeyerApp);
    PcCwKeyerMainWindow w;
    w.show();
    return a.exec();
}
