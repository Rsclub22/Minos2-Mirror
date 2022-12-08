#ifndef REMOTELOGS_H
#define REMOTELOGS_H

#include <QObject>
#include <QMap>

#include "AnalysePubSubNotify.h"
#include "MinosRPC.h"
#include "monitoredstation.h"

class RemoteLogs : public QObject
{
    Q_OBJECT
public:
    RemoteLogs();

    QMap<Provider, MonitoredStation *> stationList;

private:
    QString localRouterName;

signals:
    void syncNeeded();
    void newMonitoredLog(MonitoredLog *);

private slots:
    void on_notify(AnalysePubSubNotify an, const QString from );
    void on_provider(Provider provider, QString cat);

};

#endif // REMOTELOGS_H
