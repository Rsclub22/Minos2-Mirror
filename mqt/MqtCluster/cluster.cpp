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



