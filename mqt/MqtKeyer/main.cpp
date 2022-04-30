#include <QApplication>
#include "AppStartup.h"
#include "SecondInstall.h"
#include "KeyerMain.h"

int main(int argc, char *argv[])
{
    SecondInstall::parseSecondInstall(argc, argv);

    QApplication a(argc, argv);

    appStartup(rpcConstants::keyerApp);

    KeyerMain w;
    w.show();

    return a.exec();
}
