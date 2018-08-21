#include "base_pch.h"
#include "AppStartup.h"
#include "KeyerMain.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    appStartup(rpcConstants::keyerApp);

    KeyerMain w;
    w.show();

    return a.exec();
}
