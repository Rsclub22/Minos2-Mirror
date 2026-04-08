#ifndef KSTMAINWINDOW_H
#define KSTMAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QRadioButton>
#include <QCheckBox>
#include <QTimer>
#include <QFile>
#include <QMenu>

#include "CommandReader.h"
#include "cutils.h"
#include "callsign.h"

#include "airscoutlink.h"
#include "kstscreenoptions.h"


QT_BEGIN_NAMESPACE
namespace Ui { class KSTMainWindow; }
QT_END_NAMESPACE

class KSTMonitoredLogs;
class KSTPageFrame;

class KSTASActiveFrame ;
class KSTButtonsFrame;
class KSTCallsFrame;
class KSTLoginFrame;
class KSTMsgFrame;
class KSTPlanesFrame;
class KSTSendMeepFrame;
class KSTTomeFrame ;

extern QStringList services;

//class RemoteLogs;
class MonitoredLog;
class QPushButton;

class KstUser
{
public:
    int chat;
    Callsign call;
    QString loc;
    QString name;
    QString prefix;
    QString country;
    QString dxcc;
    bool away = false;
    bool recent = false;
    int distance = -1;
    int bearing = -1;
    int messageCount = 0;

    QString lastCalcTime;
    QString fromCall;
    QString fromLoc;
    QString toCall;
    QString toLoc;
    QVector<Aircraft> planes;
    bool planeResponseSeen = false;

    KstUser()
    {}
    KstUser(const Callsign &c, int achat):call(c),chat(achat)
    {}

    bool operator< ( const KstUser& rhs ) const;
    bool operator== ( const KstUser& rhs ) const;

    qHashRet qHash() const;
};
extern bool KstUserCompare (QSharedPointer<KstUser> i, QSharedPointer<KstUser> j);

class KstMessageLine
{
public:
    bool markedRead = false;
    int sequence = -1;
    int chat = -1;
    QDateTime dtg;
    QString fullLine;
    Callsign call;
    int distance = -1;
    QString name;
    Callsign otherCall;
    int otherDistance = -1;
    QString message;

    KstMessageLine(){}
    ~KstMessageLine(){}
};
extern bool compMessages ( QSharedPointer<KstMessageLine> q1, const QSharedPointer<KstMessageLine> q2 );

class KSTMainWindow : public QMainWindow
{
    Q_OBJECT
private:
    static bool inApplyScreenLayout;

    KSTScreenOptions kstScreenOptions;
    QTimer CloseTimer;
    QTimer userCallTimer;
    QTimer KSTTestTimer;

    bool inTestMsg = false;

    QSharedPointer<QFile> KSTexpFile;
    QFile KSTImportFile;
    QTextStream KSTImportStream;

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
//    void setNameFromCall(const Callsign &call);
    bool doConfiguration(bool showForm);

public:
    KSTMainWindow(QWidget *parent = nullptr);
    ~KSTMainWindow() override;
    QStringList routerList();

    QVector<KSTPageFrame *> pages;

    KSTASActiveFrame *kstASActiveFrame = nullptr;
    KSTButtonsFrame *kstButtonsFrame = nullptr;
    KSTCallsFrame *kstCallsFrame = nullptr;
    KSTLoginFrame *kstLoginFrame = nullptr;
    KSTMsgFrame *kstMsgFrame = nullptr;
    KSTPlanesFrame *kstPlanesFrame = nullptr;
    KSTSendMeepFrame *kstSendMeepFrame = nullptr;
    KSTTomeFrame *kstTomeFrame = nullptr;

    bool splitIcons = false;
    bool started = false;
    bool inStartup = true;
    bool inClosedown = false;

    QMenu kstPopup;
    QAction *layoutAction = nullptr;
    QAction *logsAction = nullptr;
    QAction *configureAction = nullptr;
    QAction *clearMessagesAction = nullptr;
    QAction *connectAction = nullptr;
    QAction *closeAction = nullptr;
    QAction *awayAction = nullptr;
    QAction *testAction = nullptr;

    QAction *kstASActiveAction = nullptr;
    QAction *kstButtonsAction = nullptr;
    QAction *kstCallsAction = nullptr;
    QAction *kstLoginAction = nullptr;
    QAction *kstMsgAction = nullptr;
    QAction *kstPlanesAction = nullptr;
    QAction *kstSendMeepAction = nullptr;
    QAction *kstTomeAction = nullptr;

#ifdef Q_OS_WIN
    QAction *splitIconsAction = nullptr;
#endif

    QSharedPointer<QVector <QSharedPointer<KstMessageLine> > > messageVector;
    QSharedPointer<QVector<QSharedPointer<KstUser> > > callVector;
    bool callVectorChanged = false;

    QMap<KstUser /*key*/, QSharedPointer<KstUser> > callMap;

    QSharedPointer<AirScoutLink> asl;
    UpperCaseValidator ucValidator;

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
    void buildRow(KSTPageFrame *cp, SCRow &scrow, MinosSplitter *splitterParent);

    QString getCurScreenLayout() const;
    void setCurScreenLayout(const QString &value);

    void selectLayout(QString layout);

    void userCallTimerTimer();
    void sendKST(QString msg);
    void doLoginChanges();

    void do_connectButton_clicked();
    void do_configureButton_clicked();
    void do_awayButton_clicked();
    void do_layoutButton_clicked();
    void do_KSTTestButton_clicked();
    void do_logsButton_clicked();
    void do_closeButton_clicked();
    void do_clearLogsButton_clicked();
    void do_splitIcons(bool);
    void do_ASActive(bool);
    void checkAwayButton();

    void clearScreenLayout();
    void buildScreenLayout();
    void buildScreen(SCScreen &s, int t);
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

    void do_dialog_clicked();
private:
    Ui::KSTMainWindow *ui;
    QSharedPointer<CommandReader> commandReader = QSharedPointer<CommandReader>(new CommandReader(this));

    KSTMonitoredLogs* monitoredLogs = nullptr;

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
