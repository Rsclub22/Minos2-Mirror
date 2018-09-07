#include "base_pch.h"
#include "AppStartup.h"
#include "ControlMain.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    appStartup(rpcConstants::controlApp);

    ControlMain w;
    w.show();

    return a.exec();
}
