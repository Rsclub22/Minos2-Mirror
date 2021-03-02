#ifndef CLUSTERMAINWINDOW_H
#define CLUSTERMAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QPlainTextEdit>
#include <QVector>
#include <QSortFilterProxyModel>
#include "BandList.h"
#include "base_pch.h"
#include "qttelnet.h"
#include "clustercommands.h"
#include "setupdialog.h"
#include "clusterrpc.h"
#include "dxspotdatamodel.h"
#include "sentSpotdatamodel.h"
//#include "clusterClientServer.h"
#include "userclustercommanddialog.h"
#include "checkmodeagainstfreq.h"
#include "presetbutton.h"
#include "clustercommon.h"
#include "htmldelegate.h"

namespace Ui {
class ClusterMainWindow;
}

#define TEST_SPOTS



#ifdef TEST_SPOTS

const QString CLUSTER_SPOT_TEST_FILE = "testspots.txt";

#endif




const QStringList userVHFUHFCommandShortCutKeys = { "Ctrl+1", "Ctrl+2",
                                                    "Ctrl+3", "Ctrl+4",
                                                    "Ctrl+5", "Ctrl+6",
                                                    "Ctrl+7", "Ctrl+8",
                                                    "Ctrl+9", "Ctrl+0"};

const QStringList userHFCommandShortCutKeys = { "Ctrl+Shift+1", "Ctrl+Shift+2",
                                                "Ctrl+Shift+3", "Ctrl+Shift+4",
                                                "Ctrl+Shift+5", "Ctrl+Shift+6",
                                                "Ctrl+Shift+7", "Ctrl+Shift+8",
                                                "Ctrl+Shift+9", "Ctrl+Shift+0"};





const QStringList userVHFUHFCommandMenuShortCutKeys = {"Ctrl+Alt+1", "Ctrl+Alt+2",
                                                        "Ctrl+Alt+3", "Ctrl+Alt+4",
                                                        "Ctrl+Alt+5", "Ctrl+Alt+6",
                                                        "Ctrl+Alt+7", "Ctrl+Alt+8",
                                                        "Ctrl+Alt+9", "Ctrl+Alt+0"};


const QStringList userHFCommandMenuShortCutKeys = { "Ctrl+Shift+Alt+1", "Ctrl+Shift+Alt+2",
                                                    "Ctrl+Shift+Alt+3", "Ctrl+Shift+Alt+4",
                                                    "Ctrl+Shift+Alt+5", "Ctrl+Shift+Alt+6",
                                                    "Ctrl+Shift+Alt+7", "Ctrl+Shift+Alt+8",
                                                    "Ctrl+Shift+Alt+9", "Ctrl+Shift+Alt+0"};



const int TIME_TO_LIVE_TABNUM = 0;
const int PERSONAL_TABNUM = 1;
const int NODELIST_TABNUM = 2;
const int SEND_SPOTS_DUR = 1000;
const int STATUS_TIMER_DUR = 1000;
const int ASKQRA_QUEUE_TIMER_PERIOD = 5000;
const int ASKQRA_TIMEOUT = 10000;


//const int SPOT_IS_HF = -3;
//const int SPOT_DATE_TIME_IS_INVALID = -1;
//const int TOO_FEW_PARTS = -1;
//const int SPOTTIME_EMPTY = -2;

const int HF_TABNUM = 0;
const int VHFUHF_TABNUM = 1;

class ClusterAddress
{

public:
    ClusterAddress(QString _name, QString _add, QString _port, QString _password)
    {
        name = _name;
        address = _add;
        port = _port;
        password = _password;
    }

    QString name;
    QString address;
    QString port;
    QString password;


};


class ResendSpotCommand
{
public:
    QString getCmd(){return cmd;}
    void setCmd(QString cmd_){cmd = cmd_;}
    QString getBandmask(){return bandmask;}
    void setBandmask(QString bandmask_){bandmask = bandmask_;}
    QString getuuid(){return uuid;}
    void setUuid(QString uuid_){uuid = uuid_;}
    int getFrameId(){return frameId;}
    void setFrameId(int frameId_){frameId = frameId_;}

private:

    QString cmd;
    QString bandmask;
    QString uuid;
    int frameId;
};



class ClusterStationInfo
{
public:
    ClusterStationInfo()
    {
        clear();
    }

    void setUser(QString user_){user = user_;}
    QString getUser(){return user;}

    void setFound(bool found_){found = found_;}
    bool getFound(){return found;}


    void setGotAllData(bool gotAllData_){gotAllData = gotAllData_;}
    bool getGotAllData(){return gotAllData;}

    void setName(QString name_){name = name_;}
    QString getName(){return name;}

    void setLastConnect(QString lastConnect_){lastConnect = lastConnect_;}
    QString getLastConnect(){return lastConnect;}

    void setQth(QString qth_){qth = qth_;}
    QString getQth(){return qth;}

    void setLocation(QString location_){location = location_;}
    QString getLocation(){return location;}

    void setHeading(QString heading_){heading = heading_;}
    QString getHeading(){return heading;}


    void setHomeNode(QString homeNode_){homeNode = homeNode_;}
    QString getHomeNode(){return homeNode;}

    void clear()
    {
        user.clear();
        found = false;
        gotAllData = false;
        name.clear();
        lastConnect.clear();
        qth.clear();
        location.clear();
        heading.clear();
        homeNode.clear();
    }

private:

    QString user;
    bool found;
    bool gotAllData;
    QString name;
    QString lastConnect;
    QString qth;
    QString location;
    QString heading;
    QString homeNode;

};





class DxSpotSortFilterProxyModel : public QSortFilterProxyModel
{
public:

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool matchBand(int sourceRow) const;

    bool traceDebugFlag;

    DxSpotSortFilterProxyModel(ClusterClientFilterSettings *filterSettings_);


    ClusterClientFilterSettings *filterSettings;

};






class ClusterMainWindow : public QMainWindow
{
    Q_OBJECT
    static const  char * DXSPOT_TAB_TITLE;
    static const char * SENT_SPOT_TAB_TITLE;
    static const char * RAW_DATA_TAB_TITLE;

public:
    explicit ClusterMainWindow(QWidget *parent = nullptr);
    void doStartup();
    ~ClusterMainWindow();

    static const char *userCmdButtonLabels[4];

private slots:
    void connectionEstab();
    void connectionError(QAbstractSocket::SocketError);
    void messageRx(QString msg);
    void sendText();
    void parseDX(QString txt);
    void checkedLoggedIn(QString msg);
    void onLaunchSetup();
    void connectToNode(const QString &nodeName);
    void logIn();
    void checkStationDetails(QString msg);
    void dxSpotView_sectionResized(int, int, int);
    void sentSpotView_sectionResized(int, int, int);
    void LogTimerTimer();

    void onStdInRead(QString cmd);
    void loggedOut();


     void onClearAllSpots();
     void getSpotsFromDisplayQueue();

    void onSpotTabChanged(int index);
    void disconnectTimeout();
    void sendSpotToDXCluster(Frequency freq, QString call, QString loc);
    void sendSpotToTxEnabled(bool state);

signals:

    void disconnectTimerfinished();
private:
    Ui::ClusterMainWindow *ui;
    StdInReader stdinReader;


    class QTimer LogTimer;
    QTimer *disconnectTimer;
    QSharedPointer<HtmlDelegate> dxSpotViewDelegate;
    QSharedPointer<HtmlDelegate> sentSpotViewDelegate;



    QString appName;
    QLabel* status;
    QString rawStatus;

    QVector<QSharedPointer<BandInfo> > bands;
    checkModeAgainstFreq* modeBandPlan;

    QList<PresetButton *> userVHFUHFCmdButton;
    QList<PresetButton *> userHFCmdButton;

    QList<QShortcut *> vhfUhfCommandShortCutKeyList;
    QList<QShortcut *> vhfUhfMenuShortCutKeyList;



    QStringList vhfUhfUserCommands;
    QStringList hfUserCommands;

    QList<QShortcut *> hfCommandShortCutKeyList;
    QList<QShortcut *> hfMenuShortCutKeyList;


    QMap<QString, ClusterClientBandFilterDialogDetails> bandCheckBoxes;

    ClusterClientFilterSettings filterSettings;

    QMap<QString, QSharedPointer<ClusterSpotData> > askQrzQueue;

    QtTelnet* client;
    Clusterrpc* clusterRpc;
    ClusterCommands* dxClusterCommand;

    DxSpotDataModel* dxSpotDataModel;
    QSortFilterProxyModel* dxSpotProxyModel;        // use base as we are not doing custom filtering
    QTableView* dxSpotView;
    QPlainTextEdit* rawClusterDataView;

    SentSpotDataModel* sentSpotDataModel;
    QSortFilterProxyModel* sentSpotProxyModel;
    QTableView* sentSpotView;


    SetupDialog *setupCluster;

    QVector<QSharedPointer<ClusterSpotData> > spotsList;
    //QTimer* getSpotsTimer;

    QTimer* handleSpotsInQueues;

    QStringList sendSpotsToClientQueue;


    QTimer* pingClusterNodeTimer;
    bool pingOk;

    QTimer* purgeTimer;


    QList<ResendSpotCommand> resendSpotsToClientQueue;

    QString currentNodeName;
    QString currentAddress;
    QString currentPort;
    QString currentPassword;

    QString currentUserCallsign;
    QString currentUserName;
    QString currentUserQTH;
    QString currentUserLocator;

    QString sentCallsign;
    QString sentFreq;
    QString sentLoc;
    QString sentComment;

    QStringList dxMsg;


    ClusterSpotData curSpot;


    bool loginStart;
    bool loginSuccess;
    bool loginStatDetails;
    bool nodeConnected;
    bool purgeSpotFlag;

    bool reconnectFlag;

    bool hfFlag;

    QTimer *statusTimer;

    QString geoStr;         // geometry registry location




    int txText(QString msg);
    int upackDxSpot(QString txt, QSharedPointer<ClusterSpotData> newSpot);
    void loadNodesSelectBox(QStringList listOfNodes);

    void restoreDxSpotViewColumns();
    void restoreSentSpotViewColumns();
    void closeEvent(QCloseEvent *event);
    void disconnectNode();
    void connectToHost(QString hostName);


    void getStartCommands();
    void getUserCommands();


    void initUserCommandButtons();
    void userCommandButtonUpdate(QString tabSelected, int buttonNumber, ClusterUserCommandData &buttonData);
    void userCommandAllButtonUpdate();
    void saveRotPresetButton(ClusterUserCommandData &buttonData);
    void saveUserCommandString(QString tabSelected, int buttonNumber, ClusterUserCommandData &buttonData);
    void readUserCommandStrings();
    void findLocInComment(QString &spotLoc, QString &dxLoc, const QString &comment);
    void setAllTabsColor(QColor c);
    QString extractLocator(const QString &text, const QRegularExpression fullLocExp, const QRegularExpression partLocExp);

    void showStatusMessage(const QString &message, const QString &raw);
    void startDisconnectTimer(int time);

    void echoCmdRawTextWindow(QString cmd);
    void echoCmd(QString cmd);
    void echoErrorMsg(QString err);
    void echoMsg(QString msg);
    QString createSpotToSend(QString spot);
    QString createStatusToSend(QString status);
    int upackShowDxSpot(const QString txt, QSharedPointer<ClusterSpotData> newSpot);
    bool checkShowDxMsg(const QString txt, QSharedPointer<ClusterSpotData> newSpot);

    void handleStartFile();
    void handleEndFile();
    void handleCmdFile(QString fileName);

    void showUserCmdButtonMenu(int buttonNumber);
    void userCmdButtonRead(QStringList userCommands, QString tabSelected, int buttonNumber);
    void userCmdButtonEdit(QStringList userCommands, QString tabSelected, int buttonNumber);
    void userCmdButtonClear(QStringList userCommands, QString tabSelected, int buttonNumber);
    void userCmdButtonWrite(QString tabSelected, int buttonNumber);




#ifdef TEST_SPOTS
    QTimer* spotTestTimer;
    QStringList testSpotList;
    int spotNum = 0;

#endif


    QString getPropMode(const QString comment);
    QString assembleSpotForDXCluster(Frequency freq, QString call, QString loc);

    void removeInsertSendSpotTab(bool state);
    void addSentSpotToDisplayQueue(bool spotStatus, QString reason);
    bool lookforModeInComment(const QString &spotComment, int &commnetModeNum, QString &commentMode);


    void handleResendSpotToClientsCmds();

    void getSpotsFromSendToClientQueue();
    void resendAllSpotsToClients(ResendSpotCommand cmd);

    QString createResendSpotToSend(QString spot);



    void processNewSpot(const QSharedPointer<ClusterSpotData> newSpot);
    int getQrzReply(QString &line);



    int getPingTimeoutValue();
    void sendPingMessage();



    QString getQraFromCallsignPrefix(Callsign prefix);
    QString assembleSpotMsgToSendToClients(const QSharedPointer<ClusterSpotData> spotData, const QString timeToLive);
    void getLocatorFromPrefix(ClusterSpotData &newSpot);

    void updateToNewVhfUhfGroupKey();

    void initFilterCheckBoxs();
    void setHfFilterControlsVisible(bool visible);

    void setAllUHFBandsFilter(bool state);
    void setAllVHFBandsFilter(bool state);
    void setAllHFBandsFilter(bool state);
    void setHF(bool hfFlag);
    void updateDisplay();


    void saveBandFilterSettings();
    void readBandFilterSettings();
    void loadBandFilterSettingsToTab();

    void setBandsCheckBoxAndFilterFlag(const QString band, const bool state);
    bool getUseQrzForQraFlag();
    void askQrzForQraLocator(QSharedPointer<ClusterSpotData> newSpot);
private slots:
    void personalDataChanged(QString callsign, QString name, QString locator, QString qth);

    void clusterListChanged();
    void about();
    void clusterNodeCommandsShortcutHelp();
    void handleStatusTimer();
    void onResendSpotToClients(int frameId,  QString loggerUuid, QString cmd, QString bandmask );


#ifdef TEST_SPOTS
    void testSpotPbClicked();
    void onSpotTestTimerTimeOut();
#endif








    void onHandleSpotsInQueues();



    void cancelPingTimeOut(QString msg);
    void handlePingClusterNodeTimeout();


    void purgeSpots();
    void userVhfUhfCmdButtonEdit(int buttonNumber);
    void userHfCmdButtonEdit(int buttonNumber);
    void userVhfUhfCmdButtonClear(int buttonNumber);
    void userHfCmdButtonClear(int buttonNumber);
    void userVhfUhfCmdButtonWrite(int buttonNumber);
    void userHfCmdButtonWrite(int buttonNumber);
    void showVhfUhfUserCmdButtonMenu(int buttonNumber);
    void showHfUserCmdButtonMenu(int buttonNumber);
    void userVhfUhfCmdButtonRead(int buttonNumber);
    void userHfCmdButtonRead(int buttonNumber);
    void onbandCheckBoxStateChanged(int i, bool state);
    void onHfSelectBandPbPressed();
    void onVhfSelectBandPbPressed();
    void onUhfSelectBandPbPressed();
    void onpbpressed(); // for test .. remove
    void onReconnectCommandFromLog(bool state);
};

#endif // CLUSTERMAINWINDOW_H
