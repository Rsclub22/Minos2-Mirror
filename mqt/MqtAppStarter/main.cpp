#include <QApplication>
#include "AppStartup.h"
#include "SecondInstall.h"
#include "AsMainWindow.h"

int main(int argc, char *argv[])
{
    SecondInstall::parseSecondInstall(argc, argv);
    QApplication a(argc, argv);

    appStartup("MinosQtAppStarter");

    MainWindow w;

    setAppFont();

    w.show();

    return a.exec();
}
