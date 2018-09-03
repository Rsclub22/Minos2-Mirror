#include "MainWindow.h"
#include <QApplication>
#include "AppStartup.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    appStartup("Qs1rSync");

    MainWindow w;
    w.show();

    return a.exec();
}
