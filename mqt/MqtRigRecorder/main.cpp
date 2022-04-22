#include <QApplication>

#include "AppStartup.h"
#include "rcmainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    appStartup("RigRecorder");

    MainWindow w;
    w.show();

    return a.exec();
}
