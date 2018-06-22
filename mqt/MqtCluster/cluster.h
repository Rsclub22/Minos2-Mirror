#ifndef CLUSTER_H
#define CLUSTER_H

#include <QObject>

class Cluster
{
public:
    Cluster();
    QString setQthMsg(QString qth);
    QString setQraMsg(QString qra);
    QString setNameMsg(QString name);
    QString setDxGridMsg();
    QString setPageMsg(QString lineNum);
    QString showDxMsg(QString cmd);
};

#endif // CLUSTER_H
