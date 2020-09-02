#ifndef CLUSTERCOMMANDS_H
#define CLUSTERCOMMANDS_H

#include <QObject>

class ClusterCommands
{
public:
    ClusterCommands();
    QString setQthMsg(QString qth);
    QString setQraMsg(QString qra);
    QString setNameMsg(QString name);
    QString setDxGridMsg();
    QString setPageMsg(QString lineNum);
    QString showDxMsg(QString cmd);
    QString quit();
    QString showQRZMsg(QString callsign);
    QString pingMsg();
    QString showStationMsg(QString callsign);
    QString showPrefix(QString callsign);
};

#endif // CLUSTERCOMMANDS_H
