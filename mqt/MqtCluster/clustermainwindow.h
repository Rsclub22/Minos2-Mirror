#ifndef CLUSTERMAINWINDOW_H
#define CLUSTERMAINWINDOW_H

#include <QMainWindow>
#include "qttelnet.h"
#include "cluster.h"
#include "setupdialog.h"

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
    void logIn(const QString callsign, const QString password);
    void sendText();
    void parseDX(QString txt);
    void checkedLoggedIn(QString msg, QString nodeName);
    void onLaunchSetup();
    void connectToNode(const QString &nodeName);
private:
    Ui::ClusterMainWindow *ui;
    QtTelnet* client;
    Cluster* dxCluster;

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

    void txText(QString msg);
    int upackSpot(QString txt);
    void loadNodesSelectBox(QStringList listOfNodes);
    ;
};

#endif // CLUSTERMAINWINDOW_H
