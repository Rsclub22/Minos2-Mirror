#ifndef MONITORINGFRAME_H
#define MONITORINGFRAME_H

#include <QFrame>
#include "TreeUtils.h"
#include "qmenu.h"

namespace Ui {
class MonitoringFrame;
}
class BaseContestLog;
class MonitorMain;
class QSOMapFrame;
class BaseContact;

class MonitoringFrame : public QFrame
{
    Q_OBJECT
public:
    explicit MonitoringFrame(MonitorMain *parent = nullptr);
    ~MonitoringFrame();

    void initialise( BaseContestLog * contest );
    void showQSOs();
    void setScore();
    void on_monitorTimeout();
    BaseContestLog * getContest()
    {
       return contest;
    }
    QSOGridModel qsoModel;

    bool rescanNeeded = false;
    bool newStanzas = false;
    bool armScan = false;

    void on_AfterLogContact(BaseContestLog *c, QSharedPointer<BaseContact> lct);
private:
    Ui::MonitoringFrame *ui;
    MonitorMain *mparent = nullptr;
    BaseContestLog *contest;
    QSOMapFrame *qsoMapFrame = nullptr;
    QMenu columnsMenu;
    bool inRestoreColumns = false;

    void saveQSOTableColumns();
    void restoreQSOTableColumns();
private slots:
    void onQSOTable_sectionResized(int, int, int);
    void onQSOTable_customContextMenuRequested(const QPoint &pos);
    void onQSOTable_sectionMoved(int, int, int);

    void viewColumn();
    void on_mapButton_clicked();
};

#endif // MONITORINGFRAME_H
