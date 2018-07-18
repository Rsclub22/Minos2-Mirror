#include "MainWindow.h"
#include <QApplication>
#include "AppStartup.h"
static QtMessageHandler oldHandler = nullptr;
void myMessageOutput(QtMsgType type,
                     const QMessageLogContext &context,
                     const QString &msg)
{
    oldHandler(type, context, msg);
}

int main(int argc, char *argv[])
{
    oldHandler = qInstallMessageHandler(myMessageOutput);
    QApplication a(argc, argv);

    appStartup("Qs1rSync");

    MainWindow w;
    w.show();

    return a.exec();
}
