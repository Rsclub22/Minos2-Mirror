#ifndef MONITORMAIN_H
#define MONITORMAIN_H

#include <QMainWindow>
#include <QAction>
#include <QMenu>

#include "ScreenContact.h"
#include "remotelogs.h"
#include "MonitoredLog.h"
#include "MonitoringFrame.h"
#include "MonitorTreeModel.h"
#include "StdInReader.h"
#include "cutils.h"


class MonitoringFrame;
class MonitoredLog;
class ContestLog;

namespace Ui {
class MonitorMain;
}

class MonitoredStation;

class MonitorMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit MonitorMain(QWidget *parent = nullptr);
    ~MonitorMain() override;

    ScreenContact screenContact;

    void closeTab(MonitoringFrame *tab);

    int getContestSlotCount();
    BaseContestLog *getContestSlot(int);
    BaseContestLog *getCurrentContest();

private slots:
    void on_monitorTimeout();

    void on_monitorTree_doubleClicked(const QModelIndex &index);
    void on_monitorSplitter_splitterMoved(int /*pos*/, int /*index*/);
    void on_closeMonitoredLog();
    void on_contestPageControl_customContextMenuRequested(const QPoint &pos);
    void CancelClick();

    void on_contestPageControl_tabCloseRequested(int index);

    void on_callsignEdit_textChanged(const QString &arg1);

    void on_locEdit_textChanged(const QString &arg1);

    void on_exchangeEdit_textChanged(const QString &arg1);

    void on_contestPageControl_currentChanged(int index);

    void on_mainSplitter_splitterMoved(int pos, int index);

    void on_searchSplitter_splitterMoved(int pos, int index);

    void on_monitorTree_clicked(const QModelIndex &index);

    void onSyncNeeded();
    void onNewLog(MonitoredLog *m);

    void onNewStanzas(MonitoredLog *m);
    void onNewLastContact(MonitoredLog *m);
    void onContactChanged(MonitoredLog *m);
private:
    Ui::MonitorMain *ui;
    UpperCaseValidator ucValidator;

    QMenu TabPopup;
    QAction *closeMonitoredLog;
    int splitterHandleWidth;

    StdInReader *stdinReader = new StdInReader(this);

    RemoteLogs *remoteLogs = nullptr;

    MonitorTreeModel *treeModel;

    QTimer *monitorTimer;

    bool syncstat = false;

    QAction *newAction(const QString &text, QMenu *m, void (MonitorMain::*slotparam)() );
    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

    void syncStations();
    void addSlot(QSharedPointer<MonitoredLog> ct );
    MonitoringFrame *findCurrentLogFrame();
    MonitoringFrame *findContestPage( BaseContestLog *ct );
    void searchChanged();

    void testAutoStart();
};

extern MonitorMain *monitorMain;

#endif // MONITORMAIN_H
