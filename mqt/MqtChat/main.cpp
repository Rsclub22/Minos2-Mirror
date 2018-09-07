#include "base_pch.h"
#include "chatmain.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    appStartup(rpcConstants::chatApp);

    TMinosChatForm w;
    w.show();

    return a.exec();
}
