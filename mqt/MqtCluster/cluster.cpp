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

#include "cluster.h"

Cluster::Cluster()
{

}




QString Cluster::setQthMsg(QString qth)
{
    return QString("set/qth %1\n").arg(qth);
}

QString Cluster::setQraMsg(QString qra)
{
    return QString("set/qra %1\n").arg(qra);
}

QString Cluster::setNameMsg(QString name)
{
    return QString("set/name %1\n").arg(name);
}

QString Cluster::setDxGridMsg()
{
    return QString("set/dxgrid\n");
}

QString Cluster::setPageMsg(QString lineNum)
{
    return QString("set/page %1\n").arg(lineNum);
}

QString Cluster::showDxMsg(QString cmd)
{
    return QString("show/dx %1\n").arg(cmd);
}

QString Cluster::showQRZMsg(QString callsign)
{
    return QString("show/qrz %1\n").arg(callsign);
}

QString Cluster::showStationMsg(QString callsign)
{
    return QString("show/station %1\n").arg(callsign);
}

QString Cluster::pingMsg()
{
    return QString("echo ping_cluster\n");
}


QString Cluster::quit()
{
    return QString("bye\n");
}
