#include "MainWindow.h"
#include <QApplication>
#include "AppStartup.h"
void myMessageOutput(QtMsgType /*type*/,
                     const QMessageLogContext &/*context*/,
                     const QString &/*msg*/)
{
    int a;
    Q_UNUSED(a);
    a = 0;
}
int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageOutput);
    QApplication a(argc, argv);

    appStartup("Qs1rSync");

    MainWindow w;
    w.show();

    return a.exec();
}
