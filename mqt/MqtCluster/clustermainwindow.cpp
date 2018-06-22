#include <QSettings>
#include <QDebug>
#include "clustermainwindow.h"
#include "ui_clustermainwindow.h"

#include <QDebug>

ClusterMainWindow::ClusterMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ClusterMainWindow),
    loginStart(false),
    loginSuccess(false)
{
    ui->setupUi(this);

    setupCluster = new SetupDialog();

    client = new QtTelnet(parent);
    dxCluster = new Cluster();


    // get list of clusters
    getClusterAddresses();
    loadNodesSelectBox();



    // QRegExp logPrompt("(login:)");
   // client->setLoginPattern(logPrompt);
    client->login("m0dgb", "");


    connect(ui->actionSetup, SIGNAL(triggered()), this, SLOT(onLaunchSetup()));

    connect(client, SIGNAL(socketConnected()), this, SLOT(connectionEstab()));
    connect(client, SIGNAL(loginRequired()), this, SLOT(logIn()));
    connect(client, SIGNAL(connectionError(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
    connect(client, SIGNAL(message(QString)), this, SLOT(messageRx(QString)));
    connect(client, SIGNAL(message(QString)), this, SLOT(parseDX(QString)));
    connect(client, SIGNAL(message(QString)), this, SLOT(checkedLoggedIn(QString)));
//    connect(ui->sendLine, SIGNAL(returnPressed()), this, SLOT(sendText()));
    client->connectToHost("gb7mbc.spoo.org", 8000);

//    txText("set/echo enable");
//    txText(dxCluster->setNameMsg("david"));
//    txText(dxCluster->setQthMsg("Uxbridge"));
//   txText(dxCluster->setQraMsg("IO91SN"));




}

ClusterMainWindow::~ClusterMainWindow()
{
    delete ui;
}

void ClusterMainWindow::onLaunchSetup()
{


    setupCluster->exec();
}


void ClusterMainWindow::connectionEstab()
{
    qDebug() << "connection established";


}

void ClusterMainWindow::connectionError(QAbstractSocket::SocketError error)
{
    qDebug() << "connection failed ";
}

void ClusterMainWindow::messageRx(QString msg)
{
    //qDebug() << msg;
    ui->textWindow->appendPlainText(msg.remove('\x07'));
}

void ClusterMainWindow::logIn()
{
    qDebug() << "send logon message\n";
    client->login("m0dgb\r\n", "");
    loginStart = true;

}


void ClusterMainWindow::checkedLoggedIn(QString msg)
{
    QString endOfMsg = "GB7MBC>\r\n";

    if (loginStart && !loginSuccess)
    {
        if (msg.contains(endOfMsg))
        {
            loginSuccess = true;
            txText("set/echo enable\n");
            txText(dxCluster->setNameMsg("David"));
            txText(dxCluster->setQthMsg("Uxbridge"));
            txText(dxCluster->setQraMsg("IO91SN"));
        }

    }


}


void ClusterMainWindow::sendText()
{
    client->sendData(ui->sendLine->text()+'\n');
    ui->sendLine->clear();
}


void ClusterMainWindow::txText(QString msg)
{
    client->sendData(msg);
}


void ClusterMainWindow::parseDX(QString txt)
{
    int retCode = upackSpot(txt);
    if (retCode == 0)
    {
        qDebug() << QString("Dx de %1 %2 %3 %4 %5 %6").arg(dxCall).arg(dxFreq).arg(spotCall).arg(dxLocator).arg(spotTime).arg(spotComment);
    }
    else
    {
        qDebug() << QString("Error unpacking spot");
    }


}


int ClusterMainWindow::upackSpot(QString txt)
{
    int timePos = 0;

    // clear spot data
    dxCall = "";
    dxFreq = "";
    spotCall = "";
    spotComment = "";
    spotTime = "";
    dxLocator = "";

    txt.remove('\x07');
    if (txt.contains("DX de"))
    {
        dxMsg = txt.split(QRegExp("\\s+"));
        dxCall = dxMsg[2].remove(':');
        dxFreq = dxMsg[3];
        spotCall = dxMsg[4];
        // find time
        for (int i = 4; i < dxMsg.count(); i++)
        {
            QRegularExpression re("\\d\\d\\d\\dZ");
            QRegularExpressionMatch match = re.match(dxMsg[i]);
            if (match.hasMatch())
            {
                spotTime = dxMsg[i].remove('Z');
                timePos = i;
                break;
            }
        }

        if (spotTime == "")
        {
            //error
            return -1;
        }

        // look for locator
        if (dxMsg[timePos + 1] == "")
        {
            dxLocator = "";
        }
        else
        {
            dxLocator = dxMsg[timePos + 1];
        }
        // reassemble the comment
        for (int i = 5; i < timePos; i++)
        {
            if (dxMsg[i] != "")
            {
                spotComment += dxMsg[i] + " ";
            }
        }




    }
    else
    {
        return -1;
    }

    return 0;
}


void ClusterMainWindow::loadNodesSelectBox()
{
    ui->nodeCb->addItem("");
    for (int i = 0; i < numClusterSites; i++)
    {
        ui->nodeCb->addItem(availClusters[i]->name);
    }
}


bool ClusterMainWindow::getClusterAddresses()
{

    QString fileName;
    fileName = CLUSTER_PATH + CLUSTER_SITES ;
    QSettings  config(fileName, QSettings::IniFormat);

    clusterSiteNames = config.childGroups();

    numClusterSites = clusterSiteNames.count();

    if (numClusterSites != 0)
    {
        availClusters.clear();
        for (int i = 0; i < numClusterSites; i++)
        {
            config.beginGroup(clusterSiteNames[i]);
            availClusters.append(new ClusterAddress(clusterSiteNames[i],
                                                    config.value("address", "").toString(),
                                                    config.value("port", "").toString(),
                                                    config.value("password", "").toString()));
            config.endGroup();
        }
        return true;
    }

    return false;


}
