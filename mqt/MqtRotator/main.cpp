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

#include <QApplication>
#include "RPCCommandConstants.h"
#include "SecondInstall.h"
#include "AppStartup.h"
#include "rotatormainwindow.h"

int main(int argc, char *argv[])
{
    SecondInstall::parseSecondInstall(argc, argv);

    QApplication a(argc, argv);

    appStartup(rpcConstants::rotatorApp);

    RotatorMainWindow w;
    w.show();

    return a.exec();
}
