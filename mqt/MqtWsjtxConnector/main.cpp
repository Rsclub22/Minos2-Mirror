#include "WConMain.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    appStartup(rpcConstants::wsjtxConnectorApp);

    WConMain w;
    w.show();

    return a.exec();
}
