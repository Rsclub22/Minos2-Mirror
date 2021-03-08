#include "RSMainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RSMainWindow w;
    w.show();
    return a.exec();
}
