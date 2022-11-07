#include <QApplication>
#include "AppStartup.h"
#include "SecondInstall.h"
#include "RSMainWindow.h"

int main(int argc, char *argv[])
{
    SecondInstall::parseSecondInstall(argc, argv);

    QApplication a(argc, argv);

    appStartup("mqtRigSync");

    RSMainWindow w;
    w.show();
    return a.exec();
}
