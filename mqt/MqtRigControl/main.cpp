/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2017 - 2021
//
//
//
/////////////////////////////////////////////////////////////////////////////
#include <QApplication>
#include "SecondInstall.h"
#include "AppStartup.h"
#include "rigcontrolmainwindow.h"

int main(int argc, char *argv[])
{
    SecondInstall::parseSecondInstall(argc, argv);

    QApplication a(argc, argv);
    appStartup(rpcConstants::rigControlApp);


    RigControlMainWindow w;
    w.show();

    return a.exec();
}
