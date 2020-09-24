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

const QString USER_COMMAND1 = "Ctrl+1";
const QString USER_COMMAND2 = "Ctrl+2";
const QString USER_COMMAND3 = "Ctrl+3";
const QString USER_COMMAND4 = "Ctrl+4";
const QString USER_COMMAND5 = "Ctrl+5";
const QString USER_COMMAND6 = "Ctrl+6";
const QString USER_COMMAND7 = "Ctrl+7";
const QString USER_COMMAND8 = "Ctrl+8";
const QString USER_COMMAND9 = "Ctrl+9";
const QString USER_COMMAND10 = "Ctrl+0";

const QStringList userCommandShortCutKeys = {
                                            USER_COMMAND1, USER_COMMAND2,
                                            USER_COMMAND3, USER_COMMAND4,
                                            USER_COMMAND5, USER_COMMAND6,
                                            USER_COMMAND7, USER_COMMAND8,
                                            USER_COMMAND9, USER_COMMAND10
                                        };


const QString USER_COMMAND_MENU1 = "Ctrl+Alt+1";
const QString USER_COMMAND_MENU2 = "Ctrl+Alt+2";
const QString USER_COMMAND_MENU3 = "Ctrl+Alt+3";
const QString USER_COMMAND_MENU4 = "Ctrl+Alt+4";
const QString USER_COMMAND_MENU5 = "Ctrl+Alt+5";
const QString USER_COMMAND_MENU6 = "Ctrl+Alt+6";
const QString USER_COMMAND_MENU7 = "Ctrl+Alt+7";
const QString USER_COMMAND_MENU8 = "Ctrl+Alt+8";
const QString USER_COMMAND_MENU9 = "Ctrl+Alt+9";
const QString USER_COMMAND_MENU10 = "Ctrl+Alt+0";

const QStringList userCommandMenuShortCutKeys = {
                                                        USER_COMMAND_MENU1, USER_COMMAND_MENU2,
                                                        USER_COMMAND_MENU3, USER_COMMAND_MENU4,
                                                        USER_COMMAND_MENU5, USER_COMMAND_MENU6,
                                                        USER_COMMAND_MENU7, USER_COMMAND_MENU8,
                                                        USER_COMMAND_MENU9, USER_COMMAND_MENU10
                                             };


const int TIME_TO_LIVE_TABNUM = 0;
const int PERSONAL_TABNUM = 1;
const int NODELIST_TABNUM = 2;
const int SEND_SPOTS_DUR = 1000;
const int STATUS_TIMER_DUR = 1000;
const int ASKQRZ_QUEUE_TIMER_PERIOD = 5000;
const int ASKQRZ_TIMEOUT = 10000;


//const int SPOT_IS_HF = -3;
//const int SPOT_DATE_TIME_IS_INVALID = -1;
//const int TOO_FEW_PARTS = -1;
//const int SPOTTIME_EMPTY = -2;


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
    int getBandmask(){return bandmask;}
    void setBandmak(int bandmask_){bandmask = bandmask_;}
    QString getuuid(){return uuid;}
    void setUuid(QString uuid_){uuid = uuid_;}
    int getFrameId(){return frameId;}
    void setFrameId(int frameId_){frameId = frameId_;}

private:

    QString cmd;
    int bandmask;
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


class ClusterQRZDetails
{
public:
    ClusterQRZDetails()
    {
        clear();
    }

    void setCall(const QString call_){call = call_.trimmed();}
    QString getCall()const {return call;}

    void setAdif(const QString adif_){adif = adif_.trimmed();}
    QString getAdif()const {return adif;}

    void setError(const bool error_){error = error_;}
    bool getError()const {return error;}

    void setFound(const bool found_){found = found_;}
    bool getFound()const {return found;}


    void setGotAllData(const bool gotAllData_){gotAllData = gotAllData_;}
    bool getGotAllData()const {return gotAllData;}

    void setFname(const QString fname_){fname = fname_.trimmed();}
    QString getFname()const{return fname;}

    void setName(const QString name_){name = name_.trimmed();}
    QString getName()const {return name;}

    void setAddr2(const QString addr2_){addr2 = addr2_.trimmed();}
    QString getAddr2()const {return addr2;}

    void setCountry(const QString country_){country = country_.trimmed();}
    QString getCountry(){return country;}

    void setLat(const QString lat_){lat = lat_.trimmed();}
    QString getLat()const {return lat;}

    void setLon(const QString lon_){lon = lon_.trimmed();}
    QString getLon()const {return lon;}

    void setGrid(const QString grid_){grid = grid_.trimmed();}
    QString getGrid()const{return grid;}


    void setModdate(const QString moddate_){moddate = moddate_.trimmed();}
    QString getHomeNode()const {return moddate;}

    void clear()
    {
        call.clear();
        adif.clear();
        error = false;
        found = false;
        gotAllData = false;
        fname.clear();
        name.clear();
        addr2.clear();
        country.clear();
        lat.clear();
        lon.clear();
        grid.clear();
        moddate.clear();
    }

private:

    QString call;
    QString adif;
    bool error;
    bool found;
    bool gotAllData;
    QString fname;
    QString name;
    QString addr2;
    QString country;
    QString lat;
    QString lon;
    QString grid;
    QString moddate;


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

    void showUserCmdButtonMenu(int buttonNumber);
    void userCmdButtonRead(int buttonNumber);
    void userCmdButtonEdit(int buttonNumber);
    void userCmdButtonClear(int buttonNumber);
    void userCmdButtonWrite(int buttonNumber);

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

    QTimer *startUpTimer;

    QString appName;
    QLabel* status;
    QString rawStatus;

    QVector<QSharedPointer<BandInfo> > bands;
    checkModeAgainstFreq* modeBandPlan;

    QList<PresetButton *> userCmdButton;
    QList<QShortcut *> shortCutKeyList;
    QList<QShortcut *> shiftShortCutKeyList;
    QStringList startCommands;
    QStringList userCommands;

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

    QVector<SpotData*> spotsList;
    //QTimer* getSpotsTimer;


    QStringList sendSpotsToClientQueue;
    QTimer* sendSpotsToClientTimer;

    QTimer* pingClusterNodeTimer;
    bool pingOk;


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


    SpotData curSpot;

    ClusterQRZDetails qrzInfo;
    bool getQrzInfo = false;
    bool qrzQueryAvail = false;
    bool testQrzInfo = false;

    bool getPrefixInfo = false;
    bool prefixQueryAvail = false;
    QString prefixQra;



    QString waitingForCallFromQrz;
    QMap<QString, SpotData> spotListNoQra;
    QTimer *askQrzTimer;
    QTimer *askQrzTimeout;


/*
    QString dxCall;
    Frequency dxFreq;
    QString dxBandStr;
    QString dxBandMask;
    QString dxModeStr;
    QString dxModeMask;
    QString spotCall;
    QString spotComment;
    QString spotTime;
    QString spotDate;
    QDateTime spotDateTime;
    QString dxLocator;
    QString spotLocator;
    QString dxPropMode;

*/
    bool loginStart;
    bool loginSuccess;
    bool loginStatDetails;
    bool nodeConnected;
    bool purgeSpotFlag;

    bool reconnectFlag;

    bool enableHFSpots;

    QTimer *statusTimer;

    QString geoStr;         // geometry registry location




    int txText(QString msg);
    int upackDxSpot(QString txt, SpotData &newSpot);
    void loadNodesSelectBox(QStringList listOfNodes);

    void restoreDxSpotViewColumns();
    void restoreSentSpotViewColumns();
    void closeEvent(QCloseEvent *event);
    void disconnectNode();
    void connectToHost(QString hostName);


    void getStartCommands();
    void getUserCommands();


    void initUserCommandButtons();
    void userCommandButtonUpdate(int buttonNumber, ClusterUserCommandData &buttonData);
    void userCommandAllButtonUpdate();
    void saveRotPresetButton(ClusterUserCommandData &buttonData);
    void saveUserCommandString(int buttonNumber, ClusterUserCommandData &buttonData);
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
    int upackShowDxSpot(const QString txt, SpotData &newSpot);
    bool checkShowDxMsg(const QString txt, SpotData &newSpot);

    void handleStartFile();
    void handleEndFile();
    void handleCmdFile(QString fileName);



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


    QString getSpotFromDisplayDb(int row);

    void handleResendSpotToClientsCmds();

    void getSpotsFromSendToClientQueue();
    void resendAllSpotsToClients(ResendSpotCommand cmd);

    QString createResendSpotToSend(QString spot);



    void processNewSpot(SpotData &newSpot);
    int getQrzReply(QString &line);
    int getPrefixReply(QString &line, QString &callsign);
    QString txgeoloc(double *n, double *e, int f, char t);
    int geotoloc(double lat, double longi, QString &gridref);



    int getPingTimeoutValue();
    void sendPingMessage();

private slots:
    void personalDataChanged(QString callsign, QString name, QString locator, QString qth);

    void clusterListChanged();
    void about();
    void handleStatusTimer();
    void onResendSpotToClients(int frameId,  QString loggerUuid, QString cmd, int bandmask );


#ifdef TEST_SPOTS
    void testSpotPbClicked();
    void spotTimerTimeOut();
#endif








    void getSpotsToSendToClientQueues();

    void handAskQrzTimer();

    void handleAskQrzTimeout();

    void cancelPingTimeOut(QString msg);
    void handlePingClusterNodeTimeout();


};

#endif // CLUSTERMAINWINDOW_H
