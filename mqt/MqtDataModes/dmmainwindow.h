#ifndef DMMAINWINDOW_H
#define DMMAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "RigCache.h"
#include "StdInReader.h"
#include "XMPPRPCObj.h"
#include "AnalysePubSubNotify.h"

#ifdef Q_OS_WIN
#include "MMVARIFrame.h"
#include "MMTTYFrame.h"
#include "grittyframe.h"
#endif
#include "FLDigiFrame.h"
#include "testframe.h"

QT_BEGIN_NAMESPACE
namespace Ui { class DMMainWindow; }
QT_END_NAMESPACE

class RtAudio;
class QFileSystemWatcher;
class QPushButton;
class MonitoredLog;
class MonitoredLogs;

class DMMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    DMMainWindow(QWidget *parent = nullptr);
    virtual ~DMMainWindow() override;

    static const QString mmvari;
    static const QString mmtty;
    static const QString twotone;
    static const QString gritty;
    static const QString fldigi;
    static const QString test;

    QString baseTitle;

    QString router;

    unsigned int inChannels = 0;
    unsigned int outChannels = 0;

    QStringList inputDevices;
    QStringList outputDevices;

    QMap<QString, int> inDeviceIds;
    QMap<QString, int> outDeviceIds;

    void startPreviousEngine();

private:
    Ui::DMMainWindow *ui;
    StdInReader *stdinReader = new StdInReader(this);
    QString me;
    QFileSystemWatcher *qfsw = nullptr;

    MonitoredLogs *logsTreeView = nullptr;

    QMap<QAction *, const char *> actionList;
    QMap<QMenu *, const char *> menuList;
    QAction *clearAction;

    QVector<QPushButton *> fButtons;

    QString appName;
    RigCache rigCache;
    PubSubName mainRig;

    QTimer *watchDog = nullptr;

#ifdef Q_OS_WIN
    QAction *actionMMVARI;
    QAction *actionMMTTY;
    QAction *action2Tone;
    QAction *actionGritty;
#endif
    QAction *actionFLDigi;
    QAction *actionTest;

    QAction *actionConfigure_Engines;

    QMenu *configMenu;
    QMenu *engineMenu;

    QTimer LogTimer;
    QString geoStr;         // geometry registry location

#ifdef Q_OS_WIN
    MMVARIFrame *mmvariFrame = nullptr;
    MMTTYFrame *mmttyFrame = nullptr;
    GrittyFrame *grittyFrame = nullptr;
#endif
    FLDigiFrame *fldigiFrame = nullptr;
    TestFrame *testFrame = nullptr;

    void closeAllEngines(bool clearCurrent);

    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

    void doCloseEvent();
    QMenu *newMenu(QMenu *m, const char *text);
    QAction *newAction(const char *text, QMenu *m, void (DMMainWindow::*slotparam)());
    QAction *newCheckableAction(const char *text, QMenu *m, void (DMMainWindow::*slotparam)(bool));
    QAction *newCheckableAction(const QString text, QMenu *m, void (DMMainWindow::*slotparam)(bool));
    void checkEnginesAvailable();
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

    bool doKeyPressEvent(QKeyEvent *event);
    QStringList populateRig();
//    void configureRig(const QString s);
//    QString getRig();
    void doSendButton_clicked(QString d, int c);
    void testAutoStart();
    void onNewStanzas();
protected:
    virtual void showEvent(QShowEvent *) override;

signals:
    void rigModeFreq(QString, Frequency);
    void sendCharacters(QString, int);

public slots:
    void onTxChanged(bool);

private slots:
    void LogTimerTimer();

    void onStdInRead(QString cmd);
    void onNewBackLine(QString);


#ifdef Q_OS_WIN

    void onActionMMVARI_triggered(bool checked);

    void onActionMMTTY_triggered(bool checked);

    void onAction2Tone_triggered(bool checked);

    void onActionGritty_triggered(bool checked);
#endif

    void onActionFLDigi_triggered(bool checked);

    void onActionTest_triggered(bool checked);

    void onActionExit_triggered();

    void onActionConfigure_Engines_triggered();

    void on_sendButton_clicked();

    void onNewCharacter();

    void wordSelected(QString, int carrier);

    void onMenuClear();
    void on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from);
    void on_sendercb_stateChanged(int arg1);
    void on_notify(AnalysePubSubNotify an, const QString from);
    void iniFileChanged();

    void fButtonClicked();

    void fKey(int key);

    void on_stopButton_clicked();
    void on_mainRigComboBox_activated(const QString &psn);
    void on_backDataButton_clicked();
    void onNewLog(MonitoredLog *ml);

    void onWatchdogTimer();
};
extern DMMainWindow *mainWindow;

#endif // DMMAINWINDOW_H
