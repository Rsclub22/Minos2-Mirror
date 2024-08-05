#include "winkeyermainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WinkeyerMainWindow w;
    w.show();
    return a.exec();
}
