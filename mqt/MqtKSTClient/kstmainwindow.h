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
#include "kstpage.h"
#include "kstplanesmodel.h"
#include "cutils.h"
#include "airscoutlink.h"
#include "kstscreenoptions.h"
#include "minossplitter.h"


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

    KstMessageGridModel kstMessageModel;
    KstMessageGridSortFilterModel kstMessageFilterModel;

    KstMeepGridSortFilterModel kstMeepFilterModel;

    QSharedPointer<QVector <QSharedPointer<KstMessageLine> > > messageVector;

    KstCallGridModel kstCallModel;
    KstCallGridSortFilterModel kstCallFilterModel;

    QSharedPointer<QVector<QSharedPointer<KstUser> > > callVector;
    bool callVectorChanged = false;
    
    QMap<KstUser /*key*/, QSharedPointer<KstUser> > callMap;

    bool inTestMsg = false;

    QTimer KSTTestTimer;
    QSharedPointer<QFile> KSTexpFile;
    QFile KSTImportFile;
    QTextStream KSTImportStream;
//    bool replayEnabled = false;


    KstPlanesModel kstPlanesModel;
    KstPlanesGridSortFilterModel kstPlanesFilterModel;

    QSharedPointer<HtmlDelegate> meepDelegate;
    QSharedPointer<HtmlDelegate> messageDelegate;
    QSharedPointer<HtmlDelegate> CSDelegate;
    QSharedPointer<HtmlDelegate> PlanesDelegate;

    QTcpSocket* kstclient;

    QString KSTserverName;
    QString KSTserverPort;

    Callsign myCallsign;
    QString password;
    QString firstName;
    QString recName;
    QVector<int> kstChatSelection;
    QVector<int> kstLoggedIn;
    int activeChat = 1;

    QString myLoc;
    QString recLoc;
    bool autoConnect = false;

    bool meepNotifyLogger = false;
    bool meepPlaySound = false;
    QString meepSoundFile;
    int meepVolume = 50;


    int maxDistance = 99999;

    bool ASActive = false;
    ASBand ASActiveBand = asb144M;
    QString ASServerName = "AS";
    QString ASMyName = "Minos";
    int ASMinDistance = 300;
    int ASMaxDistance = 1000;
    int ASPort = 9872;
    int ASTimeout = 10;

    bool kstconnected = false;
    bool started = false;

    QString msgbuf;
    //QStringList filelines;
    //int curline = 0;

    int messageChatFilter = 0;
    int CSChatFilter = 0;

    int messageSequence = 0;

    QSharedPointer<AirScoutLink> asl;
    QSharedPointer<KstUser> planeActive;

    UpperCaseValidator ucValidator;

    bool mouseInMessages = false;

//    RemoteLogs *remoteLogs = nullptr;

    void buildScreenLayout(int slotNo);
    void clearScreenLayout(bool clearAllTabs);
    void buildScreen(SCScreen &s, int t, int &auxInstance);

    void closeEvent(QCloseEvent *event) override;

    void sendKST(QString msg);
    void playMeepSound();
    void analyseKstMessage(QString atj);
    void reconnect();
    void connectToHost();
    virtual bool eventFilter(QObject *obj, QEvent *event) override;
    void setNameFromCall(const Callsign &call);
    void doLoginChanges();
    void setActive(int chat);
    bool doConfiguration(bool showForm);
    void setDefaultButton(QPushButton *d);

public:
    KSTMainWindow(QWidget *parent = nullptr);
    ~KSTMainWindow() override;

    KSTMainFrame *mainFrame = nullptr;
    QString iniName;
    QString TNServerName;
    QString TNServerPort;


    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

    int getMaxDistance() const;

    bool getASActive() const;

    ASBand getASActiveBand() const;

    QString getASServerName() const;

    QString getASMyName() const;

    int getASMinDistance() const;

    int calcDistance(const Callsign &c);

    int getASMaxDistance() const;

    Callsign getMyCallsign() const;

    QString getMyLoc() const;

    QSharedPointer<QVector<QSharedPointer<KstUser> > > getCallVector() const;

    int getActiveChat() const;

    void showPlanes(QSharedPointer<KstUser> user);

    QSharedPointer<KstUser> getUser(const KstUser &test);
    int getASPort() const;

    int getASTimeout() const;

    void createScreenComponents();


    void buildRow(KSTPage *cp, SCRow &scrow, int &auxInstance, MinosSplitter *splitterParent);
    void applyScreenLayout();
    QString getCurScreenLayout() const;
    void setCurScreenLayout(const QString &value);

    void selectLayout(QString layout);
private slots:
    void CloseTimerTimer();

    void userCallTimerTimer();

    void on_closeButton_clicked();

    void on_connectButton_clicked();

    void on_messageFilter_textChanged(const QString &arg1);

    void on_CSFilter_textChanged(const QString &arg1);

    void on_kstSplitter_splitterMoved(int pos, int index);

    void on_sectionResized(int, int, int);

    void connected();
    void disconnected();
    void connectionError(QAbstractSocket::SocketError error);
    void onReadyRead();

    void on_configureButton_clicked();

    void on_genmsgButton_clicked();

    void on_meepButton_clicked();

    void on_msgSplitter_splitterMoved(int pos, int index);

    void on_messageTable_clicked(const QModelIndex &index);

    void on_meepTable_clicked(const QModelIndex &index);

    void on_clearButton_clicked();

    void on_sortIndicatorChanged(int, Qt::SortOrder);

    void on_callEdit_textChanged(const QString &arg1);

    void on_clearMessageButton_clicked();

    void on_awayButton_clicked();

    void logincb_stateChanged(int arg1);
    void activerb_clicked();
    void on_CSChatFilter_currentIndexChanged(int index);
    void on_messageChatFilter_currentIndexChanged(int index);

    void on_clearMessageFilter_clicked();

    void on_clearUserFilter_clicked();

    void on_sectionMoved(int, int, int);

    void acChanged(QSharedPointer<KstUser>);

    void on_asBandCombo_currentIndexChanged(int index);

    void on_ASActivecb_stateChanged(int arg1);

    void on_showInAS_clicked();

    void on_showMPath_clicked();

    void on_callSplitter_splitterMoved(int pos, int index);

    void on_maxDistanceEdit_editingFinished();

    void on_showReadcb_stateChanged(int arg1);

    void on_stringRb_clicked();

    void on_countryRb_clicked();

    void onCSTableSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void on_clearSelectedMessage_clicked();

    void on_includeMeCb_stateChanged(int arg1);

    void on_toMeFilter_textChanged(const QString &arg1);

    void on_clearMeepFiltersButton_clicked();

    void on_logsButton_clicked();

    void onNewLog(QSharedPointer<MonitoredLog> ml);
    void onLogChanged(QSharedPointer<MonitoredLog> ml);
    void onNewStanzas();
    void onLogStarted(QSharedPointer<MonitoredLog>);
    void onLogClosed(QSharedPointer<MonitoredLog>);
    void on_loggerXferButton_clicked();

    void on_msgEdit_textChanged(const QString &arg1);

    void on_FontChanged();

    void on_awayCallscb_stateChanged(int);

    void on_inactiveCallscb_stateChanged(int);
    void on_KSTTestButton_clicked();

    void testTimeout();
    void on_meepTable_doubleClicked(const QModelIndex &index);

    void on_messageTable_doubleClicked(const QModelIndex &index);

    void on_layoutButton_clicked();

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
    void checkActive();
    void resetVectors(QCheckBox *cb, QRadioButton *rb, int c, QStringList &s, QVector<int> &v, QVector<int> &a);
    void checkAwayButton();
    void setMeepFilters();
    void scrollMeepToBotton();
    void scrollMesToBottom();
    void testAutoStart();
    void addMessage(QSharedPointer<KstMessageLine> kst);
    void checkUserMessages(QSharedPointer<KstUser> user);
    QStringList routerList();
};

extern KSTMainWindow *mainWindow;
#endif // KSTMAINWINDOW_H
