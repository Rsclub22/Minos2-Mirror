#ifndef KSTMONITOREDLOGS_H
#define KSTMONITOREDLOGS_H

#include <QDialog>
#include "MonitoredLog.h"

namespace Ui {
class KSTMonitoredLogs;
}

class KSTMonitoredLogs : public QDialog
{
    Q_OBJECT

public:
    explicit KSTMonitoredLogs(QWidget *parent = nullptr);
    ~KSTMonitoredLogs();

    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

private slots:
    void on_hideButton_clicked();

signals:
    void logStarted(QSharedPointer<MonitoredLog>);
    void logClosed(QSharedPointer<MonitoredLog>);

private:
    Ui::KSTMonitoredLogs *ui;
};

#endif // KSTMONITOREDLOGS_H
