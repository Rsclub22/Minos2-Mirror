#ifndef CLUSTERMAINWINDOW_H
#define CLUSTERMAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QPlainTextEdit>
#include "mqtUtils_pch.h"
#include "qttelnet.h"
#include "cluster.h"
#include "setupdialog.h"
#include "dxspotdatamodel.h"

namespace Ui {
class ClusterMainWindow;
}


const QString CLUSTER_PATH = "./Configuration/Cluster/";
const QString CLUSTER_SITES = "ClusterSites.ini";


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

private:
    Ui::ClusterMainWindow *ui;
    StdInReader stdinReader;
    class QTimer LogTimer;

    QtTelnet* client;
    Cluster* dxCluster;

    DxSpotDataModel* dxSpotDataModel;
    QTableView* dxSpotView;
    QPlainTextEdit* rawClusterDataView;

    SetupDialog *setupCluster;


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
    QString spotCall;
    QString spotComment;
    QString spotTime;
    QString dxLocator;

    bool loginStart;
    bool loginSuccess;
    bool nodeConnected;

    QString geoStr;         // geometry registry location


    void txText(QString msg);
    int upackSpot(QString txt);
    void loadNodesSelectBox(QStringList listOfNodes);

    void restoreDxSpotViewColumns();

    void closeEvent(QCloseEvent *event);
};

#endif // CLUSTERMAINWINDOW_H
