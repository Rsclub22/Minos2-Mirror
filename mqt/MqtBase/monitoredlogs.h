#ifndef MONITOREDLOGS_H
#define MONITOREDLOGS_H

#include <QFrame>

namespace Ui {
class MonitoredLogs;
}

class MonitorTreeModel;
class RemoteLogs;
class MonitoredLog;
class QTreeView;
class QGroupBox;

class MonitoredLogs : public QFrame
{
    Q_OBJECT

    Ui::MonitoredLogs *ui;

    MonitorTreeModel *treeModel = nullptr;

    QTimer *monitorTimer = nullptr;
    bool syncstat = false;
    void syncStations();

    QTreeView *logTree = nullptr;
    QGroupBox *autoStationsBox;

public:
    explicit MonitoredLogs(QWidget *parent);
    ~MonitoredLogs();

    QTreeView *getLogTree() const
    {
        return logTree;
    }

    void setSettingsFile(const QString &fn);

private slots:
    void onMonitorTree_doubleClicked(const QModelIndex &index);
    void onSyncNeeded();

    void on_monitorTimeout();

    void autoSyncChanged(int);
signals:
    void logStarted(QSharedPointer<MonitoredLog>);
    void logClosed(QSharedPointer<MonitoredLog>);
};

#endif // MONITOREDLOGS_H
