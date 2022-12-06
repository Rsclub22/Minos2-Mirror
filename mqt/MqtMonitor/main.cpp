#include <QApplication>
#ifdef Q_OS_WIN
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QQmlApplicationEngine>
#endif
#endif

#include "AppStartup.h"
#include "RPCCommandConstants.h"
#include "SecondInstall.h"
#include "MonitorMain.h"


#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QSharedPointer<QQmlApplicationEngine> appQmlEngine;
#endif

int main(int argc, char *argv[])
{
    SecondInstall::parseSecondInstall(argc, argv);
    QApplication a(argc, argv);
#ifdef Q_OS_WIN
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // We must have a QCoreApplication before we do this
    // And we do this now so that the QML debugger can connect
    appQmlEngine = QSharedPointer<QQmlApplicationEngine>(new QQmlApplicationEngine());
#endif
#endif
    appStartup(rpcConstants::monitorApp);

    MonitorMain w;
    w.show();

    auto r = a.exec();

    return r;

}
