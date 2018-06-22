#include "clustermainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ClusterMainWindow w;
    w.show();

    return a.exec();
}
