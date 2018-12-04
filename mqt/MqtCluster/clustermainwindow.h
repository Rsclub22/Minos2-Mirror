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
#include "cluster.h"
#include "setupdialog.h"
#include "clusterrpc.h"
#include "dxspotdatamodel.h"
#include "clusterClientServer.h"
#include "userclustercommanddialog.h"
#include "rotpresetbutton.h"
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

class ClusterMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ClusterMainWindow(QWidget *parent = nullptr);
    ~ClusterMainWindow();



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

    void on_sectionResized(int, int, int);
    void LogTimerTimer();

    void onStdInRead(QString cmd);
    void loggedOut();

    void showUserCmdButtonMenu(int buttonNumber);
    void userCmdButtonRead(int buttonNumber);
    void userCmdButtonEdit(int buttonNumber);
    void userCmdButtonClear(int buttonNumber);
    void userCmdButtonWrite(int buttonNumber);

     void onClearAllSpots();
     void getSpotsFromQueue();

    void onSpotTabChanged(int index);
    void disconnectTimeout();

signals:

    void disconnectTimerfinished();
private:
    Ui::ClusterMainWindow *ui;
    StdInReader stdinReader;
    class QTimer LogTimer;
    QTimer *disconnectTimer;
    HtmlDelegate *delegate = nullptr;

    QString appName;
    QLabel* status;

    QVector<BandDetail*> bands;

    const QStringList userCmdButtonLabels = {"&Send", "&New", "&Edit", "&Clear"};
    QList<RotPresetButton *> userCmdButton;
    QList<QShortcut *> shortCutKeyList;
    QList<QShortcut *> shiftShortCutKeyList;
    QStringList startCommands;
    QStringList userCommands;

    QtTelnet* client;
    Clusterrpc* clusterRpc;
    Cluster* dxCluster;

    DxSpotDataModel* dxSpotDataModel;
    QSortFilterProxyModel* dxSpotProxyModel;        // use base as we are not doing custom filtering
    QTableView* dxSpotView;
    QPlainTextEdit* rawClusterDataView;

    SetupDialog *setupCluster;

    QVector<SpotData*> spotsList;
    QTimer* getSpotsTimer;

    QString currentNodeName;
    QString currentAddress;
    QString currentPort;
    QString currentPassword;

    QString currentUserCallsign;
    QString currentUserName;
    QString currentUserQTH;
    QString currentUserLocator;



    QStringList dxMsg;
    QString dxCall;
    QString dxFreq;
    QString dxBandStr;
    QString dxBandMask;
    QString dxModeStr;
    QString dxModeMask;
    QString spotCall;
    QString spotComment;
    QString spotTime;
    QString dxLocator;
    QString spotLocator;

    bool loginStart;
    bool loginSuccess;
    bool nodeConnected;
    bool purgeSpotFlag;

    bool reconnectFlag;



    QString geoStr;         // geometry registry location


    void txText(QString msg);
    int upackSpot(QString txt);
    void loadNodesSelectBox(QStringList listOfNodes);

    void restoreDxSpotViewColumns();

    void closeEvent(QCloseEvent *event);
    void disconnectNode();
    void connectToHost(QString hostName);
    void getBand(QString freq, QString &band, QString &bandMask);

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
    QString extractLocator(const QString &text, const QRegExp fullLocExp, const QRegExp partLocExp);
    void handleStartFile();
    void showStatusMessage(const QString &message);
    void startDisconnectTimer(int time);

    void echoCmdRawTextWindow(QString cmd);
    void echoCmd(QString cmd);
    void echoErrorMsg(QString err);
    void echoMsg(QString msg);

#ifdef TEST_SPOTS
    QTimer* spotTestTimer;
    QStringList testSpotList;
    int spotNum = 0;




private slots:
    void testSpotPbClicked();
    void spotTimerTimeOut();
#endif


    void personalDataChanged(QString callsign, QString name, QString locator, QString qth);
};

#endif // CLUSTERMAINWINDOW_H
