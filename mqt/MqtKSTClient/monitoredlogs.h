#ifndef MONITOREDLOGS_H
#define MONITOREDLOGS_H

#include <QDialog>

namespace Ui {
class MonitoredLogs;
}

class MonitoredLogs : public QDialog
{
    Q_OBJECT

public:
    explicit MonitoredLogs(QWidget *parent = nullptr);
    ~MonitoredLogs();

private:
    Ui::MonitoredLogs *ui;
};

#endif // MONITOREDLOGS_H
