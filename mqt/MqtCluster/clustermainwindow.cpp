/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Cluster Server
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
///
//
//
/////////////////////////////////////////////////////////////////////////////


#include <QSettings>
#include <QTimer>
#include <QProcessEnvironment>
#include <QDebug>

#include "clustermainwindow.h"
#include "clustercommon.h"
#include "rigutils.h"
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

    connect(&stdinReader, SIGNAL(stdinLine(QString)), this, SLOT(onStdInRead(QString)));
    stdinReader.start();

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    appName = env.value("MQTRPCNAME", "") ;

    trace(QString("AppName = %1").arg(appName));
    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName());

    createCloseEvent();

    connect(&LogTimer, SIGNAL(timeout()), this, SLOT(LogTimerTimer()));
    LogTimer.start(100);

    setWindowTitle("Minos Cluster Server");
    status = new QLabel;
    ui->statusBar->addWidget(status);

    loadVhfAndUpBands(bands);

    QSettings settings;
    geoStr = QString("clusterServer/geometry");
    QByteArray geometry = settings.value(geoStr).toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    setupCluster = new SetupDialog();
    clusterRpc = new Clusterrpc();

    clusterRpc->setStandAlone();

    client = new QtTelnet(parent);
    dxCluster = new Cluster();

    dxSpotDataModel = new DxSpotDataModel();
    dxSpotView = new QTableView();
    dxSpotView->setModel(dxSpotDataModel);
    dxSpotView->setSelectionMode( QAbstractItemView::NoSelection );
    //dxSpotView->setStyleSheet("QHeaderView::section { font: bold; height: 14px }");

    QHeaderView *verticalHeader = dxSpotView->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(18);


    connect( dxSpotView->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( on_sectionResized(int, int , int)));

    dxSpotView->setColumnWidth(TIME_COL_NUM, TIME_COL_WIDTH);
    dxSpotView->setColumnWidth(FREQ_COL_NUM, FREQ_COL_WIDTH);
    dxSpotView->setColumnWidth(DXSPOT_CALL_COL_NUM, DXSPOT_CALL_COL_WIDTH);
    dxSpotView->setColumnWidth(LOC_COL_NUM, LOC_COL_WIDTH);
    dxSpotView->setColumnWidth(SPOT_CALL_COL_NUM, SPOT_CALL_COL_WIDTH);
    dxSpotView->setColumnWidth(COMMENT_COL_NUM, COMMENT_COL_WIDTH);

    restoreDxSpotViewColumns();

    rawClusterDataView = new QPlainTextEdit();

    ui->clusterViewsTab->addTab(dxSpotView, "DX Spots");
    ui->clusterViewsTab->addTab(rawClusterDataView, "Raw Data");

    connect(ui->actionSetup, SIGNAL(triggered()), this, SLOT(onLaunchSetup()));

    connect(ui->nodeCb, SIGNAL(activated(QString)), this, SLOT(connectToNode(QString)));

    connect(client, SIGNAL(socketConnected()), this, SLOT(connectionEstab()));
    connect(client, SIGNAL(loginRequired()), this, SLOT(logIn()));
    connect(client, SIGNAL(connectionError(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
    connect(client, SIGNAL(loggedOut()), this, SLOT(loggedOut()));
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

    // get current node from file and then connect to host
    connectToSelectedHost(setupCluster->getCurrentNodeName());


}


ClusterMainWindow::~ClusterMainWindow()
{
    delete ui;
}






void ClusterMainWindow::onLaunchSetup()
{


    setupCluster->exec();
}


void ClusterMainWindow::on_sectionResized(int, int, int)
{
    QSettings settings;
    QByteArray state;

    state = dxSpotView->horizontalHeader()->saveState();
    settings.setValue("dxSpotView/state", state);

}

void ClusterMainWindow::restoreDxSpotViewColumns()
{
    QSettings settings;
    QByteArray state;

    state = settings.value("dxSpotView/state").toByteArray();
    dxSpotView->horizontalHeader()->restoreState(state);
}

void ClusterMainWindow::connectToNode(const QString &nodeName)
{
    QString selNodeName = nodeName;

    if ((nodeName.isEmpty() && nodeConnected) || nodeName == "")
    {
        disconnectNode();
        currentNodeName = "";
        currentAddress = "";
        currentPort = "";
        currentPassword = "";
        setupCluster->saveCurrentNodeName(currentNodeName);
    }
    else
    {
        if (currentNodeName == nodeName)
        {
            //reconnect
            disconnectNode();
            client->connectToHost(currentAddress, currentPort.toUShort());
        }
        else
        {
            connectToSelectedHost(nodeName);
        }

    }



}


void ClusterMainWindow::connectToSelectedHost(QString nodeName)
{

    currentNodeName = nodeName;

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









void ClusterMainWindow::connectionEstab()
{
    nodeConnected = true;
    trace(QString("Connection Established with host %1 %2:%3").arg(currentNodeName).arg(currentAddress).arg(currentPort));
}

void ClusterMainWindow::connectionError(QAbstractSocket::SocketError error)
{
    trace(QString("Connection failed error %1").arg(error));
}



void ClusterMainWindow::logIn()
{
    trace(QString("Login Start - Send logon message\n"));
    client->login(QString("%1\r\n").arg(currentUserCallsign), currentPassword);
    loginStart = true;

}

void ClusterMainWindow::loggedOut()
{
    trace(QString("Logged Out of node  %1").arg(currentNodeName));
    currentNodeName = "";
    currentAddress = "";
    currentPort = "";
    nodeConnected = false;

}

void ClusterMainWindow::disconnectNode()
{
    trace(QString("Disconnect Node %1").arg(currentNodeName));
    //client->logout();
    txText(dxCluster->quit());

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

        if (retCode >= 0)
        {

            trace(QString("Parse DX de %1 %2 %3 %4 %5 %6 %7 %8 %9 %10").arg(dxCall).arg(dxFreq).arg(dxBandStr).arg(dxBandMask).arg(dxModeStr).arg(dxModeMask).arg(spotCall).arg(dxLocator).arg(spotTime).arg(spotComment));
            // Display
            QString displayFreq = alignFreqRight(dxFreq);
            clusterRpc->sendDXSpot(QString("%1:%2:%3:%4:%5:%6:%7:%8:%9:%10").arg(dxCall).arg(dxFreq).arg(dxBandStr).arg(dxBandMask).arg(dxModeStr).arg(dxModeMask).arg(spotCall).arg(dxLocator).arg(spotTime).arg(spotComment));
            dxSpotDataModel->rowData = QStringList {spotTime, displayFreq, dxCall, dxLocator, spotCall, spotComment };
            //dxSpotDataModel->insertRows(dxSpotDataModel->rowCount(), 1);
            dxSpotDataModel->insertRows(0, 1);

        }
        else if (retCode < 0)
        {
            trace(QString("ParseDx: Error unpacking spot"));
        }

    }
}




int ClusterMainWindow::upackSpot(QString txt)
{
    int timePos = 0;

    // clear spot data
    dxCall = "";
    dxFreq = "";
    dxBandStr = "";
    dxBandMask = "";
    dxModeStr = "";
    dxModeMask = "";
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
    dxFreq = convertKhzToMhz(dxMsg[3]);
    getBand(dxFreq, dxBandStr, dxBandMask);
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



void ClusterMainWindow::getBand(QString freq, QString &band, QString &bandMask)
{
    double f = freq.append("000").remove('.').toDouble();

    for (int i = 0; i < bands.count(); i++)
    {
        if (f <= bands[i]->fHigh && f >= bands[i]->fLow)
        {
            band = bands[i]->name;
            bandMask = QString::number(allBandMasks[i]);
            break;
        }
    }
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


void ClusterMainWindow::LogTimerTimer()
{
    bool show = getShowServers();
    if ( !isVisible() && show )
    {
        setVisible(true);
    }
    if ( isVisible() && !show )
    {
        setVisible(false);
    }

    static bool closed = false;
    if ( !closed )
    {
        if ( checkCloseEvent() )
        {
            closed = true;
            close();
        }
    }
}



void ClusterMainWindow::closeEvent(QCloseEvent *event)
{

    LogTimer.stop();


    // and tidy up all loose ends

    QSettings settings;
    settings.setValue(geoStr, saveGeometry());
    trace("Minos Cluster Server Closing");
    QWidget::closeEvent(event);
}


void ClusterMainWindow::onStdInRead(QString cmd)
{
    trace("Command read from stdin: " + cmd);
    if (cmd.indexOf("ShowServers", Qt::CaseInsensitive) >= 0)
        setShowServers(true);
    if (cmd.indexOf("HideServers", Qt::CaseInsensitive) >= 0)
        setShowServers(false);
}



