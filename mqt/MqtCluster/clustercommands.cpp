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

#include "clustercommands.h"

ClusterCommands::ClusterCommands()
{

}




QString ClusterCommands::setQthMsg(QString qth)
{
    return QString("set/qth %1\n").arg(qth);
}

QString ClusterCommands::setQraMsg(QString qra)
{
    return QString("set/qra %1\n").arg(qra);
}

QString ClusterCommands::setNameMsg(QString name)
{
    return QString("set/name %1\n").arg(name);
}

QString ClusterCommands::setDxGridMsg()
{
    return QString("set/dxgrid\n");
}

QString ClusterCommands::setPageMsg(QString lineNum)
{
    return QString("set/page %1\n").arg(lineNum);
}

QString ClusterCommands::showDxMsg(QString cmd)
{
    return QString("show/dx %1\n").arg(cmd);
}

QString ClusterCommands::showQRZMsg(QString callsign)
{
    return QString("show/qrz %1\n").arg(callsign);
}

QString ClusterCommands::showStationMsg(QString callsign)
{
    return QString("show/station %1\n").arg(callsign);
}

QString ClusterCommands::showPrefix(QString callsign)
{
    return QString("show/prefix %1\n").arg(callsign);
}

QString ClusterCommands::pingMsg()
{
    return QString("echo ping_cluster\n");
}


QString ClusterCommands::quit()
{
    return QString("bye\n");
}
