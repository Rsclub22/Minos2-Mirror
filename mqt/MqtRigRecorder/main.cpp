#include <QApplication>
#include "SecondInstall.h"
#include "AppStartup.h"
#include "rcmainwindow.h"

int main(int argc, char *argv[])
{
    SecondInstall::parseSecondInstall(argc, argv);

    QApplication a(argc, argv);

    appStartup("RigRecorder");

    MainWindow w;
    w.show();

    return a.exec();
}
