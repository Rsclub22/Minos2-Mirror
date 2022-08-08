#ifndef MONITORMAIN_H
#define MONITORMAIN_H

#include <QAction>
#include <QMenu>
#include "ScreenContact.h"
#include "MonitoredLog.h"
#include "MonitoringFrame.h"
#include "MonitorTreeModel.h"
#include "cutils.h"


class MonitoringFrame;
class MonitoredLog;

namespace Ui {
class MonitorMain;
}

class MonitoredStation
{
   public:
      QVector< QSharedPointer<MonitoredLog> > slotList;

      MonitoredStation()
      {}
      ~MonitoredStation()
      {}
};

class MonitorMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit MonitorMain(QWidget *parent = nullptr);
    ~MonitorMain() override;

    QMap<Provider, MonitoredStation *> stationList;
    ScreenContact screenContact;

    void closeTab(MonitoringFrame *tab);

    int getContestSlotCount();
    BaseContestLog *getContestSlot(int);
    BaseContestLog *getCurrentContest();

private slots:
    void on_notify(AnalysePubSubNotify an, const QString from );
    void on_routerCall( bool err, QSharedPointer<MinosRPCObj>, const QString from );
    void on_provider(Provider provider, QString cat);

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

private:
    Ui::MonitorMain *ui;
    UpperCaseValidator ucValidator;

    QAction *newAction(const QString &text, QMenu *m, void (MonitorMain::*slotparam)() );
    QMenu TabPopup;
    QAction *closeMonitoredLog;


    int splitterHandleWidth;

    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

    StdInReader *stdinReader = new StdInReader(this);
    QString localRouterName;

    MonitorTreeModel *treeModel;

    QTimer *monitorTimer;

    bool syncstat = false;
    void syncStations();
    void addSlot(QSharedPointer<MonitoredLog> ct );
    MonitoringFrame *findCurrentLogFrame();
    MonitoringFrame *findContestPage( BaseContestLog *ct );
    void searchChanged();

    bool inReadPersistedLogs = false;
    void readPersistedLogs();
    void writePersistedLogs();
};

extern MonitorMain *monitorMain;

#endif // MONITORMAIN_H
