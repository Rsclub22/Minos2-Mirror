#include "kstmainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    appStartup(rpcConstants::KSTClientApp);
    KSTMainWindow w;
    w.show();
    return a.exec();
}
