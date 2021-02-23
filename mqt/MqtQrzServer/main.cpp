#include "qrzservermainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    appStartup(rpcConstants::qrzServerApp);
    QrzServerMainWindow w;
    w.show();
    return a.exec();
}
