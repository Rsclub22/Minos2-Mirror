#ifndef MONITOREDLOGS_H
#define MONITOREDLOGS_H

#include "remotelogs.h"
#include <QDialog>

namespace Ui {
class MonitoredLogs;
}

class MonitorTreeModel;

class MonitoredLogs : public QDialog
{
    Q_OBJECT

public:
    explicit MonitoredLogs(QWidget *parent);
    ~MonitoredLogs();

private slots:
    void on_monitorTree_doubleClicked(const QModelIndex &index);
    void onSyncNeeded();
    void onNewLog(MonitoredLog *ml);
    void onNewStanzas(MonitoredLog *l);
    void onNewLastContact(MonitoredLog *l);
    void onContactChanged(MonitoredLog *l);

    void on_monitorTimeout();
    void on_OKButton_clicked();

private:
    Ui::MonitoredLogs *ui;
    MonitorTreeModel *treeModel = nullptr;

    QTimer *monitorTimer = nullptr;
    bool syncstat = false;
    RemoteLogs *remoteLogs = nullptr;
    void syncStations();
};

#endif // MONITOREDLOGS_H
