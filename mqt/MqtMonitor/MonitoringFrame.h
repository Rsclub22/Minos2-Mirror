#ifndef MONITORINGFRAME_H
#define MONITORINGFRAME_H

#include "base_pch.h"

namespace Ui {
class MonitoringFrame;
}
class BaseContestLog;
class MonitorMain;

class MonitoringFrame : public QFrame
{
    Q_OBJECT

    MonitorMain *mparent;

public:
    explicit MonitoringFrame(MonitorMain *parent = nullptr);
    ~MonitoringFrame();

    void initialise( BaseContestLog * contest );
    void showQSOs();
    void setScore();
    BaseContestLog * getContest()
    {
       return contest;
    }

    void update();

private:
    Ui::MonitoringFrame *ui;
    BaseContestLog *contest;
    QMenu columnsMenu;
    bool inRestoreColumns = false;

    QSOGridModel qsoModel;

    void saveQSOTableColumns();
    void restoreQSOTableColumns();
private slots:
    void onQSOTable_sectionResized(int, int, int);
    void onQSOTable_customContextMenuRequested(const QPoint &pos);
    void onQSOTable_sectionMoved(int, int, int);

    void viewColumn();
};

#endif // MONITORINGFRAME_H
