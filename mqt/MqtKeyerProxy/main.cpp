#include <QApplication>
#include "AppStartup.h"
#include "RPCCommandConstants.h"
#include "SecondInstall.h"
#include "kpmainwindow.h"

int main(int argc, char *argv[])
{
    SecondInstall::parseSecondInstall(argc, argv);
    QApplication a(argc, argv);
    appStartup(rpcConstants::keyerProxyApp);
    KPMainWindow w;
    w.show();
    return a.exec();
}
