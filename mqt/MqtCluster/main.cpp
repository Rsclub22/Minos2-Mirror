/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Cluster Server
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
///
//
//
/////////////////////////////////////////////////////////////////////////////

#include <QApplication>
#include "AppStartup.h"
#include "SecondInstall.h"
#include "clustermainwindow.h"

int main(int argc, char *argv[])
{
    SecondInstall::parseSecondInstall(argc, argv);
    QApplication a(argc, argv);
    appStartup(rpcConstants::clusterApp);
    ClusterMainWindow w;
    w.show();

    return a.exec();
}
