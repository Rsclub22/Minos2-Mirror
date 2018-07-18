/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
// Hamlib Library
//
/////////////////////////////////////////////////////////////////////////////

#include "rotatormainwindow.h"
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QString>
#include <QStringList>

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

    appStartup("MinosQtRotator");

    RotatorMainWindow w;
    w.show();

    return a.exec();
}
