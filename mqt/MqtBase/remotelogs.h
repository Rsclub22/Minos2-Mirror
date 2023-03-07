#ifndef REMOTELOGS_H
#define REMOTELOGS_H

#include <QObject>
#include <QMap>

#include "AnalysePubSubNotify.h"
#include "MinosRPC.h"
#include "callsign.h"
#include "monitoredstation.h"

class RemoteLogs : public QObject
{
    Q_OBJECT
public:
    QMap<Provider, MonitoredStation *> stationList;

    RemoteLogs();

    static RemoteLogs *getRemoteLogs()
    {
        static RemoteLogs rl;
        return &rl;
    }
    void closeLog(MonitoredLog *);
    void closeAll();

    bool hasWorked(const Callsign &, QString band, QString mode);
    Callsign myCall();
private:
    QString localRouterName;

signals:
    void syncNeeded();
    void newMonitoredLog(MonitoredLog *);
    void currentLogChanged(MonitoredLog *);

private slots:
    void on_notify(AnalysePubSubNotify an, const QString from );
    void on_provider(Provider provider, QString cat);

};

#endif // REMOTELOGS_H
