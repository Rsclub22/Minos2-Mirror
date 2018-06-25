#include <QSettings>
#include <QDebug>
#include "clustermainwindow.h"
#include "ui_clustermainwindow.h"

#include <QDebug>

ClusterMainWindow::ClusterMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ClusterMainWindow),
    loginStart(false),
    loginSuccess(false),
    nodeConnected(false)
{
    ui->setupUi(this);

    setupCluster = new SetupDialog();

    client = new QtTelnet(parent);
    dxCluster = new Cluster();

    dxSpotView = new QTableView();
    rawClusterDataView = new QPlainTextEdit();

    ui->clusterViewsTab->addTab(dxSpotView, "DX Spots");
    ui->clusterViewsTab->addTab(rawClusterDataView, "Raw Data");

    connect(ui->actionSetup, SIGNAL(triggered()), this, SLOT(onLaunchSetup()));

    connect(ui->nodeCb, SIGNAL(activated(const QString &nodeName)), this, SLOT(connectToNode(const QString &nodeName)));

    connect(client, SIGNAL(socketConnected()), this, SLOT(connectionEstab()));
    connect(client, SIGNAL(loginRequired()), this, SLOT(logIn()));
    connect(client, SIGNAL(connectionError(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
    connect(client, SIGNAL(message(QString)), this, SLOT(messageRx(QString)));
    connect(client, SIGNAL(message(QString)), this, SLOT(parseDX(QString)));
    connect(client, SIGNAL(message(QString)), this, SLOT(checkedLoggedIn(QString)));
    connect(ui->sendLine, SIGNAL(returnPressed()), this, SLOT(sendText()));

    // get list of clusters
    loadNodesSelectBox(setupCluster->getListOfClusterNames());

    // get user data
    currentUserCallsign  = setupCluster->getUserCallsign();
    currentUserName = setupCluster->getUserName();
    currentUserLocator = setupCluster->getUserLocator();
    currentUserQTH = setupCluster->getUserQth();

    // get current node from file
    currentNodeName = setupCluster->getCurrentNodeName();
    if (setupCluster->doesClusterNameExist(currentNodeName))
    {
        ui->nodeCb->setCurrentText(currentNodeName);
        // get current node data
        QStringList nd = setupCluster->getClusterInfo(currentNodeName);
        currentNodeName = nd[0];
        currentAddress = nd[1];
        currentPort = nd[2];
        currentPassword = nd[3];


        client->connectToHost(currentAddress, currentPort.toUShort());

    }
}


ClusterMainWindow::~ClusterMainWindow()
{
    delete ui;
}

void ClusterMainWindow::onLaunchSetup()
{


    setupCluster->exec();
}


void ClusterMainWindow::connectToNode(const QString &nodeName)
{
    QString selNodeName = nodeName;

    if (nodeName.isEmpty() && nodeConnected)
    {
        //disconnectNode();
       // currentNode = "";
        currentAddress = "";
        currentPort = "";
        currentPassword = "";
        //saveCurrentNode(currentNode);
    }
    else
    {
        //if (currentNode == nodeName)
       // {
            // reconnect
      //      disconnectNode();
     //       connectNode(currentCallsign, currentPassword, currentNode, currentAddress, currentPort);
       // }
    }



}



void ClusterMainWindow::connectionEstab()
{
    qDebug() << "connection established";


}

void ClusterMainWindow::connectionError(QAbstractSocket::SocketError error)
{
    qDebug() << "connection failed ";
}



void ClusterMainWindow::logIn()
{
    qDebug() << "send logon message\n";
    client->login(QString("%1\r\n").arg(currentUserCallsign), currentPassword);
    loginStart = true;

}


// ********** handle rx messages *********** //


void ClusterMainWindow::messageRx(QString msg)
{
    //qDebug() << msg;
    rawClusterDataView->appendPlainText(msg.remove('\x07'));
}

void ClusterMainWindow::checkedLoggedIn(QString msg)
{
    QString endOfMsg = QString(">\r\n");

    if (loginStart && !loginSuccess)
    {
        if (msg.contains(endOfMsg))
        {
            loginSuccess = true;
            txText("set/echo enable\n");
            txText(dxCluster->setNameMsg(currentUserName));
            txText(dxCluster->setQthMsg(currentUserQTH));
            txText(dxCluster->setQraMsg(currentUserLocator));
        }

    }


}




void ClusterMainWindow::parseDX(QString txt)
{
    if (loginSuccess)
    {
        int retCode = upackSpot(txt);

        switch(retCode)
        {
            case 0:
            qDebug() << QString("DX de %1 %2 %3 %4 %5 %6").arg(dxCall).arg(dxFreq).arg(spotCall).arg(dxLocator).arg(spotTime).arg(spotComment);
            break;
            case -1:
            qDebug() << QString("Error unpacking spot");
            break;


        }

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
    if (!txt.contains("DX de"))
    {
        return -2;
    }

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


    return 0;
}




// ************* Send text *************************************************

void ClusterMainWindow::sendText()
{
    client->sendData(ui->sendLine->text()+'\n');
    ui->sendLine->clear();
}


void ClusterMainWindow::txText(QString msg)
{
    client->sendData(msg);
}








void ClusterMainWindow::loadNodesSelectBox(QStringList listOfNodes)
{
    ui->nodeCb->addItem("");
    ui->nodeCb->addItems(listOfNodes);
}

