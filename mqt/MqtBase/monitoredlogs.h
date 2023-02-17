#ifndef MONITOREDLOGS_H
#define MONITOREDLOGS_H

#include <QTreeView>

namespace Ui {
class MonitoredLogs;
}

class MonitorTreeModel;
class RemoteLogs;
class MonitoredLog;

class MonitoredLogs : public QTreeView
{
    Q_OBJECT

    Ui::MonitoredLogs *ui;

    MonitorTreeModel *treeModel = nullptr;

    QTimer *monitorTimer = nullptr;
    bool syncstat = false;
    RemoteLogs *remoteLogs = nullptr;
    void syncStations();


public:
    explicit MonitoredLogs(QWidget *parent);
    ~MonitoredLogs();

private slots:
    void onMonitorTree_doubleClicked(const QModelIndex &index);
    void onSyncNeeded();

    void on_monitorTimeout();

signals:
    void logStarted(QSharedPointer<MonitoredLog>);
    void logClosed(QSharedPointer<MonitoredLog>);
};

#endif // MONITOREDLOGS_H
