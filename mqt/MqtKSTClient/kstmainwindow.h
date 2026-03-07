#ifndef KSTMAINWINDOW_H
#define KSTMAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QRadioButton>
#include <QCheckBox>
#include <QTimer>
#include "CommandReader.h"
#include "kstcallgridmodel.h"
#include "kstmessagegridmodel.h"
#include "kstplanesmodel.h"
#include "cutils.h"
#include "airscoutlink.h"
#include "kstscreenoptions.h"
#include "qmenu.h"


QT_BEGIN_NAMESPACE
namespace Ui { class KSTMainWindow; }
QT_END_NAMESPACE

class KSTMonitoredLogs;
class KSTMainFrame;

extern QStringList services;

//class RemoteLogs;
class MonitoredLog;
class QPushButton;


class KSTMainWindow : public QMainWindow
{
    Q_OBJECT
private:
    KSTScreenOptions kstScreenOptions;
    QTimer CloseTimer;
    QTimer userCallTimer;

    bool inTestMsg = false;

    QTimer KSTTestTimer;
    QSharedPointer<QFile> KSTexpFile;
    QFile KSTImportFile;
    QTextStream KSTImportStream;

    KstPlanesModel kstPlanesModel;
    KstPlanesGridSortFilterModel kstPlanesFilterModel;

    QTcpSocket* kstclient;

    QString KSTserverName;
    QString KSTserverPort;

    bool kstconnected = false;

    QString msgbuf;

    int messageSequence = 0;
    int activeChat = 1;
    int maxDistance = 99999;

    bool ASActive = false;
    ASBand ASActiveBand = asb144M;
    QString ASServerName = "AS";
    QString ASMyName = "Minos";
    int ASMinDistance = 300;
    int ASMaxDistance = 1000;
    int ASPort = 9872;
    int ASTimeout = 10;

    QString curScreenLayout = "default";

    void closeEvent(QCloseEvent *event) override;

    void playMeepSound();
    void analyseKstMessage(QString atj);
    void reconnect();
    void connectToHost();
    void setNameFromCall(const Callsign &call);
    bool doConfiguration(bool showForm);

public:
    KSTMainWindow(QWidget *parent = nullptr);
    ~KSTMainWindow() override;
    QStringList routerList();
    bool started = false;
    QMenu kstPopup;
    QAction *layoutAction = nullptr;
    QAction *logsAction = nullptr;
    QAction *configureAction = nullptr;
    QAction *clearMessagesAction = nullptr;
    QAction *connectAction = nullptr;
    QAction *closeAction = nullptr;
    QAction *awayAction = nullptr;

    QSharedPointer<QVector <QSharedPointer<KstMessageLine> > > messageVector;
    QSharedPointer<QVector<QSharedPointer<KstUser> > > callVector;
    bool callVectorChanged = false;

    QMap<KstUser /*key*/, QSharedPointer<KstUser> > callMap;

    KstMessageGridModel kstMessageModel;
    KstMessageGridSortFilterModel kstMessageFilterModel;

    KstMeepGridSortFilterModel kstMeepFilterModel;

    KstCallGridModel kstCallModel;
    KstCallGridSortFilterModel kstCallFilterModel;

    QSharedPointer<AirScoutLink> asl;
    UpperCaseValidator ucValidator;

    KSTMainFrame *mainFrame = nullptr;
    QString iniName;
    QString TNServerName;
    QString TNServerPort;

    Callsign myCallsign;
    QString password;
    QString firstName;
    QString recName;

    QString myLoc;
    QString recLoc;
    bool autoConnect = false;

    bool meepNotifyLogger = false;
    bool meepPlaySound = false;
    QString meepSoundFile;
    int meepVolume = 50;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

    QVector<int> kstChatSelection;
    QVector<int> kstLoggedIn;

    void setMaxDistance(int);
     int getMaxDistance() const;

    int getASActive() const;

    QString getASServerName() const;

    QString getASMyName() const;

    int getASMinDistance() const;

    int calcDistance(const Callsign &c);

    int getASMaxDistance() const;

    Callsign getMyCallsign() const;

    QString getMyLoc() const;

    QSharedPointer<QVector<QSharedPointer<KstUser> > > getCallVector() const;

    void setActiveChat(int c);
    int getActiveChat() const;

    void showPlanes(QSharedPointer<KstUser> user);

    QSharedPointer<KstUser> getUser(const KstUser &test);
    int getASPort() const;

    int getASTimeout() const;

    void createScreenComponents();
    void applyScreenLayout();
    QString getCurScreenLayout() const;
    void setCurScreenLayout(const QString &value);

    void selectLayout(QString layout);

    void do_connectButton_clicked();
    void do_configureButton_clicked();
    void do_awayButton_clicked();
    void do_layoutButton_clicked();
    void sendKST(QString msg);
    void userCallTimerTimer();
    void doLoginChanges();
    void do_KSTTestButton_clicked();
    void do_logsButton_clicked();
    void do_closeButton_clicked();
    void do_clearLogsButton_clicked();
    void checkAwayButton();

private slots:
    void CloseTimerTimer();

    void connected();
    void disconnected();
    void connectionError(QAbstractSocket::SocketError error);
    void onReadyRead();

    void onNewLog(QSharedPointer<MonitoredLog> ml);
    void onLogChanged(QSharedPointer<MonitoredLog> ml);
    void onNewStanzas();
    void onLogStarted(QSharedPointer<MonitoredLog>);
    void onLogClosed(QSharedPointer<MonitoredLog>);

    void on_FontChanged();

    void testTimeout();

    void onScreenConfigApply(QString curConfigName);
    void onSetDefaultName(QString def);
    void onSetProtectedName(QString prot);

private:
    Ui::KSTMainWindow *ui;
    QSharedPointer<CommandReader> commandReader = QSharedPointer<CommandReader>(new CommandReader(this));

    KSTMonitoredLogs* ml = nullptr;

    QString chatSelection;

    void getSettings(QSettings &settings);

    void clearConnection();
    void testAutoStart();
    void addMessage(QSharedPointer<KstMessageLine> kst);
    void checkUserMessages(QSharedPointer<KstUser> user);

    QAction *newAction(int n, QMenu *m, void (KSTMainWindow::*slotparam)(),QAction::MenuRole mr = QAction::TextHeuristicRole);
    QAction *newAction(const char *text, QMenu *m, void (KSTMainWindow::*slotparam)() ,QAction::MenuRole mr = QAction::TextHeuristicRole);
    QMenu *newMenu(QMenu *m, const char *text);
    QAction *newCheckableAction(const char *text, QMenu *m, void (KSTMainWindow::*slotparam)(bool) );
    QAction *newCheckableAction(const QString text, QMenu *m, void (KSTMainWindow::*slotparam)(bool) );
};

extern KSTMainWindow *mainWindow;
#endif // KSTMAINWINDOW_H
