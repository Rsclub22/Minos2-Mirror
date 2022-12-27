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

#ifndef NDEBUG
#ifdef Q_OS_WIN
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) || QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        QSharedPointer<QQmlApplicationEngine> appQmlEngine;
#endif
#endif
#endif

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    SecondInstall::parseSecondInstall(argc, argv);
    QApplication a(argc, argv);

    appStartup(rpcConstants::monitorApp);

    MonitorMain w;
    w.show();

    auto r = a.exec();
#ifndef NDEBUG
#ifdef Q_OS_WIN
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) || QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    appQmlEngine.clear();
#endif
#endif
#endif
    return r;

}
