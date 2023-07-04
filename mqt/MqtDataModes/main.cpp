#include <QApplication>
#include "AppStartup.h"
#include "DMMinosParameters.h"
#include "RPCCommandConstants.h"
#include "SecondInstall.h"

#include "dmmainwindow.h"

int main(int argc, char *argv[])
{
    DMMinosParameters mp;

    SecondInstall::parseSecondInstall(argc, argv);
    QApplication a(argc, argv);
    appStartup(rpcConstants::datamodesApp);
    DMMainWindow w;
    w.show();

    return a.exec();
}
