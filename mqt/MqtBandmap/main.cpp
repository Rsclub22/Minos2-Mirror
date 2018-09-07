#include "base_pch.h"
#include "AppStartup.h"
#include "bandmapmainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    appStartup(rpcConstants::bandmapApp);

    BandMapMainWindow w;
    w.show();

    return a.exec();
}
