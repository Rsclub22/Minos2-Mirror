#include "RSMainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    appStartup("mqtRigSync");

    RSMainWindow w;
    w.show();
    return a.exec();
}
