#include <QApplication>
#include "AppStartup.h"
#include "RPCCommandConstants.h"
#include "SecondInstall.h"
#include "ControlMain.h"

int main(int argc, char *argv[])
{
    SecondInstall::parseSecondInstall(argc, argv);
    QApplication a(argc, argv);

    appStartup(rpcConstants::controlApp);

    ControlMain w;
    w.show();

    return a.exec();
}
