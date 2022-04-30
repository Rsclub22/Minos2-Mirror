#include <QApplication>
#include "SecondInstall.h"
#include "mqtktMainWindow.h"

int main(int argc, char *argv[])
{
    SecondInstall::parseSecondInstall(argc, argv);

    QApplication a(argc, argv);
    mqtktMainWindow w;
    w.show();

    return a.exec();
}
