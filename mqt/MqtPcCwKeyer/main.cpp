#include "pccwkeyermainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    pcCwKeyerMainWindow w;
    w.show();
    return a.exec();
}
