/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Cluster Server
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
///
//
//
/////////////////////////////////////////////////////////////////////////////


#include <QSettings>
#include <QTimer>
#include <QProcessEnvironment>
#include <QHeaderView>
#include <QTextStream>
#include <QDebug>

#include "clustermainwindow.h"
#include "clustercommon.h"
#include "rigutils.h"
#include "cutils.h"
#include "BandList.h"
#include "delayedaction.h"

#include "ui_clustermainwindow.h"
#include "latlong.h"



static const char * sendClusterReasonText[] = {QT_TRANSLATE_NOOP("cluster", "Ok"), QT_TRANSLATE_NOOP("cluster", "Failed - comms error"),
                                           QT_TRANSLATE_NOOP("cluster", "Not Logged On"), QT_TRANSLATE_NOOP("cluster", "Freq out of band"),
                                           QT_TRANSLATE_NOOP("cluster", "Callsign or Locator Empty")};
enum sendClusterReason_e {TX_OK, COMMS_ERR, NOT_LOGGED_ON, FREQ_ERR, CALL_LOC_EMPTY};

const char * ClusterMainWindow::DXSPOT_TAB_TITLE = QT_TR_NOOP("DX Spots");
const char * ClusterMainWindow::SENT_SPOT_TAB_TITLE = QT_TR_NOOP("Sent Spots");
const char * ClusterMainWindow::RAW_DATA_TAB_TITLE = QT_TR_NOOP("Raw Data");

const char *ClusterMainWindow::userCmdButtonLabels[4] = {QT_TR_NOOP("&Send"), QT_TR_NOOP("&New"),
                                                QT_TR_NOOP("&Edit"), QT_TR_NOOP("&Clear")};




#define TXSPOT  // enable to actually send to cluster
//#define TEST_PLEASE_IGNORE // comment out to stop this in tx spot remarks

ClusterMainWindow::ClusterMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ClusterMainWindow),
    loginStart(false),
    loginSuccess(false),
    loginStatDetails(false),
    nodeConnected(false),
    purgeSpotFlag(false),
    reconnectFlag(false),
    enableHFSpots(false)
{
    ui->setupUi(this);

    delayedAction(this, [=](){
        doStartup();
    });
}
void ClusterMainWindow::doStartup()
{

    connect(&stdinReader, SIGNAL(stdinLine(QString)), this, SLOT(onStdInRead(QString)));
    stdinReader.start();

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    appName = env.value("MQTRPCNAME", "") ;

    trace(QString("AppName = %1").arg(appName));
    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName());
    Q_UNUSED(rpc)

    createCloseEvent();

    disconnectTimer = new QTimer();
    connect(disconnectTimer, SIGNAL(timeout()), this, SLOT(disconnectTimeout()));

    connect(&LogTimer, SIGNAL(timeout()), this, SLOT(LogTimerTimer()));
    LogTimer.start(100);

    spotsList.clear();
    //getSpotsTimer = new QTimer();
    //connect(getSpotsTimer, SIGNAL(timeout()), this, SLOT(getSpotsFromDisplayQueue()));
    //getSpotsTimer->start(1000);


    setWindowTitle(tr("Minos Cluster Server"));
    status = new QLabel;
    ui->statusBar->addWidget(status);

    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));

    BandList::getBandList().loadVhfAndUpBands(bands);

    modeBandPlan = new checkModeAgainstFreq();
    if (modeBandPlan->loadBandsFromBandList())
    {
        trace(QString("Mode frequency bandplan loaded OK"));

    }
    else
    {
        trace(QString("Mode frequency bandplan loaded failed to Load"));

    }


#ifdef TEST_SPOTS

    if (FileExists(CLUSTER_PATH + CLUSTER_SPOT_TEST_FILE))
    {
        ui->testSpotsPb->setVisible(true);
        ui->testSpotsLab->setVisible(true);
        connect(ui->testSpotsPb, SIGNAL(clicked()), this, SLOT(testSpotPbClicked()));
        spotTestTimer = new QTimer();
        connect(spotTestTimer, SIGNAL(timeout()), this, SLOT(spotTimerTimeOut()));
    }
    else
    {
        ui->testSpotsPb->setVisible(false);
        ui->testSpotsLab->setVisible(false);
    }

#endif


    QSettings settings;
    geoStr = QString("clusterServer/geometry");
    QByteArray geometry = settings.value(geoStr).toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    setupCluster = new SetupDialog();

    connect(setupCluster, SIGNAL(personalDataUpdated(QString, QString, QString, QString)), SLOT(personalDataChanged(QString, QString, QString, QString)));

    clusterRpc = new Clusterrpc();
    connect(clusterRpc, SIGNAL(sendSpotToDXCluster(Frequency, QString, QString)), this, SLOT(sendSpotToDXCluster(Frequency, QString, QString)));
    connect(clusterRpc, SIGNAL(resendSpotToClients(int, QString, QString, int)), this, SLOT(onResendSpotToClients(int, QString, QString, int)));


    sendSpotsToClientTimer = new QTimer();
    connect(sendSpotsToClientTimer, SIGNAL(timeout()), this, SLOT(getSpotsToSendToClientQueues()));
    sendSpotsToClientTimer->start(SEND_SPOTS_DUR);


    client = new QtTelnet(parent());
    dxClusterCommand = new ClusterCommands();


    if (!FileExists(CLUSTER_SETTINGS_FILE))
    {
        // missing cluster settings file, create default
        setupCluster->createDefaultGeneralSettingsFile();
    }


    initUserCommandButtons();
    readUserCommandStrings();
    userCommandAllButtonUpdate();
    // spotTimeToLive
    setupCluster->readGeneralSettings();
    setupCluster->loadGeneralToSetupTab();
    setupCluster->readPersonal();
    setupCluster->loadPersonalToSetupTab();

    connect(setupCluster, SIGNAL(sendSpotToTxEnabled(bool)), this, SLOT(sendSpotToTxEnabled(bool)));

    // read enable hf spots flag
    QString fileName = CLUSTER_SETTINGS_FILE;
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("HFSpots");
    enableHFSpots = config.value("enable", false).toBool();
    config.endGroup();

    // in comming spot tab

    dxSpotDataModel = new DxSpotDataModel();


    dxSpotView = new QTableView();

    dxSpotViewDelegate = QSharedPointer<HtmlDelegate>( new HtmlDelegate(1.0, 1.0)) ;

    dxSpotDataModel->delegate = dxSpotViewDelegate;

    dxSpotProxyModel = new QSortFilterProxyModel();
    dxSpotProxyModel->setSourceModel(dxSpotDataModel);
    //dxSpotProxyModel->sort(RXTIME_COL_NUM, Qt::DescendingOrder);
    dxSpotProxyModel->sort(TIME_COL_NUM, Qt::DescendingOrder);

    dxSpotView->setModel(dxSpotProxyModel);
    dxSpotView->setAlternatingRowColors(true);
    dxSpotView->setSelectionMode( QAbstractItemView::NoSelection );
    dxSpotView->setItemDelegate(dxSpotViewDelegate.data());




    QHeaderView *verticalHeader = dxSpotView->verticalHeader();
    verticalHeader->setVisible(false);
    verticalHeader->setDefaultSectionSize(10);
    verticalHeader->setMinimumSectionSize(10);

    verticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);

    restoreDxSpotViewColumns();
    dxSpotView->horizontalHeader()->setStretchLastSection(true);
    connect( dxSpotView->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( dxSpotView_sectionResized(int, int , int)));

    // set these columns visible
    dxSpotView->setColumnHidden(TIME_COL_NUM, false);
    dxSpotView->setColumnHidden(FREQ_COL_NUM, false);
    dxSpotView->setColumnHidden(DXSPOT_CALL_COL_NUM, false);
    dxSpotView->setColumnHidden(DXSPOT_MODE_COL_NUM, false);
    dxSpotView->setColumnHidden(DXLOC_COL_NUM, false);
    dxSpotView->setColumnHidden(SPOTTER_CALL_COL_NUM, false);
    dxSpotView->setColumnHidden(COMMENT_COL_NUM, false);

    // hide these columns
    dxSpotView->setColumnHidden(DXBRG_COL_NUM, true);
    dxSpotView->setColumnHidden(DXBANDMASK_COL_NUM, true);
    dxSpotView->setColumnHidden(DXMODEMASK_COL_NUM, true);
    dxSpotView->setColumnHidden(DXSPOT_TO_MEMORY_FLAG_COL_NUM, true);
    dxSpotView->setColumnHidden(DXSPOT_CALL_WORKED_COL_NUM, true);
    dxSpotView->setColumnHidden(DXLOC_WORKED_COL_NUM, true);
    dxSpotView->setColumnHidden(DXDIST_COL_NUM, true);
    dxSpotView->setColumnHidden(RXTIME_COL_NUM, true);
    dxSpotView->setColumnHidden(DXSPOT_PROP_MODE_COL_NUM, true);
    dxSpotView->setColumnHidden(DXBANDSTR_COL_NUM, true);
    dxSpotView->setColumnHidden(DATE_COL_NUM, true);
    dxSpotView->setColumnHidden(DXLOC_FROM_NODE_FLAG_COL_NUM, true);



    // sent spot tab

    sentSpotDataModel = new SentSpotDataModel();
    sentSpotView = new QTableView();
    sentSpotViewDelegate = QSharedPointer<HtmlDelegate>( new HtmlDelegate(1.0, 1.0)) ;
    sentSpotDataModel->delegate = sentSpotViewDelegate;

    sentSpotProxyModel = new QSortFilterProxyModel();
    sentSpotProxyModel->setSourceModel(sentSpotDataModel);
    sentSpotProxyModel->sort(SENT_SPOT_RXTIME_COL_NUM, Qt::DescendingOrder);

    sentSpotView->setModel(sentSpotProxyModel);
    sentSpotView->setAlternatingRowColors(true);
    sentSpotView->setSelectionMode( QAbstractItemView::NoSelection );
    sentSpotView->setItemDelegate(dxSpotViewDelegate.data());



    QHeaderView *sentSpotVerticalHeader = sentSpotView->verticalHeader();
    sentSpotVerticalHeader->setVisible(false);
    sentSpotVerticalHeader->setDefaultSectionSize(10);
    sentSpotVerticalHeader->setMinimumSectionSize(10);

    sentSpotVerticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);

    restoreSentSpotViewColumns();
    sentSpotView->horizontalHeader()->setStretchLastSection(true);
    connect( sentSpotView->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( sentSpotView_sectionResized(int, int , int)));

    sentSpotView->setColumnHidden(SENT_SPOT_RXTIME_COL_NUM, true);

    // rawdata tab

    rawClusterDataView = new QPlainTextEdit();
    rawClusterDataView->setReadOnly(true);

    ui->clusterViewsTab->addTab(dxSpotView, tr(DXSPOT_TAB_TITLE));
    //ui->clusterViewsTab->addTab(sentSpotView, tr(SENT_SPOT_TAB_TITLE));
    ui->clusterViewsTab->addTab(rawClusterDataView, tr(RAW_DATA_TAB_TITLE));



    setAllTabsColor(CLUSTER_TAB_NOT_SELECT_COLOR);
    ui->clusterViewsTab->setTabColor(ui->clusterViewsTab->currentIndex(), CLUSTER_TAB_SELECT_COLOR);
    connect(ui->clusterViewsTab, SIGNAL(currentChanged(int)), this, SLOT(onSpotTabChanged(int)));

    connect(ui->actionSetup, SIGNAL(triggered()), this, SLOT(onLaunchSetup()));
    connect(ui->actionClear_All_Spots, SIGNAL(triggered()), this, SLOT(onClearAllSpots()));

    connect(ui->nodeCb, SIGNAL(activated(QString)), this, SLOT(connectToNode(QString)));

    connect(setupCluster, SIGNAL(clusterListChanged()), this, SLOT(clusterListChanged()));

    connect(client, SIGNAL(socketConnected()), this, SLOT(connectionEstab()));
    //connect(client, SIGNAL(connected(bool)), this, SLOT(connected(bool)));
    connect(client, SIGNAL(loginRequired()), this, SLOT(logIn()));
    connect(client, SIGNAL(connectionError(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
    connect(client, SIGNAL(loggedOut()), this, SLOT(loggedOut()));
    connect(client, SIGNAL(message(QString)), this, SLOT(messageRx(QString)));
    connect(client, SIGNAL(message(QString)), this, SLOT(parseDX(QString)));
    connect(client, SIGNAL(message(QString)), this, SLOT(checkedLoggedIn(QString)));
    connect(client, SIGNAL(message(QString)), this, SLOT(cancelPingTimeOut(QString)));
    //connect(client, SIGNAL(message(QString)), this, SLOT(checkStationDetails(QString)));
    //connect(ui->sendLine, SIGNAL(returnPressed()), this, SLOT(sendText()));

    statusTimer = new QTimer(this);
    connect(statusTimer, SIGNAL(timeout()), this, SLOT(handleStatusTimer()));
    statusTimer->start(STATUS_TIMER_DUR);

    askQraData.clear();

    askQraTimer = new QTimer(this);
    connect(askQraTimer, SIGNAL(timeout()), this, SLOT(handAskQraTimer()));
    askQraTimer->start(ASKQRA_QUEUE_TIMER_PERIOD);

    askQraTimeout = new QTimer(this);
    connect(askQraTimeout, SIGNAL(timeout()), this, SLOT(handleAskQraTimeout()));



    //testQrzTimeout = new QTimer(this);
    //connect(testQrzTimeout, SIGNAL(timeout()), this, SLOT(handleTestQrzTimeout()));

    pingClusterNodeTimer = new QTimer(this);
    connect(pingClusterNodeTimer, SIGNAL(timeout()), this, SLOT(handlePingClusterNodeTimeout()));
    pingOk = false;



    // get list of clusters
    loadNodesSelectBox(setupCluster->getListOfClusterNames());

    // get user data
    currentUserCallsign  = setupCluster->getUserCallsign();
    currentUserName = setupCluster->getUserName();
    currentUserLocator = setupCluster->getUserLocator();
    currentUserQTH = setupCluster->getUserQth();




    // get current node from file and then connect to host
    currentNodeName = setupCluster->getCurrentNodeName();

    connectToHost(currentNodeName);

    connect(setupCluster, SIGNAL(sendSpotToTxEnabled(bool)), this, SLOT(sendSpotToTxEnabled(bool)));

    removeInsertSendSpotTab(setupCluster->getSendToDXClusterEnabled());



}

/*
void ClusterMainWindow::startSendSpotsTimer()
{
    sendSpotsTimer->start(SEND_SPOTS_DUR);

}
*/

void ClusterMainWindow::clusterListChanged()
{
    loadNodesSelectBox(setupCluster->getListOfClusterNames());
}


void ClusterMainWindow::sendSpotToTxEnabled(bool state)
{
    QString stateMsg;
    if (state)
    {
        stateMsg = SPOT_TX_ON;
        removeInsertSendSpotTab(true);

    }
    else
    {
        stateMsg = SPOT_TX_OFF;
        removeInsertSendSpotTab(false);
    }

    clusterRpc->publishTXEnable(stateMsg);
}

void ClusterMainWindow::removeInsertSendSpotTab(bool state)
{
    if (ui->clusterViewsTab->count() >= 1)
    {
        if (state)
        {
            // insert the tab
            if (ui->clusterViewsTab->tabText(1) == tr(SENT_SPOT_TAB_TITLE))
            {
                // tab must exist
                return;
            }
            else
            {
                ui->clusterViewsTab->insertTab(1, sentSpotView, tr(SENT_SPOT_TAB_TITLE));
            }
        }
        else if (ui->clusterViewsTab->tabText(1) != tr(SENT_SPOT_TAB_TITLE))
        {
            // missing don't remove
            return;
        }
        else
        {
            ui->clusterViewsTab->removeTab(1);
        }



    }
}


ClusterMainWindow::~ClusterMainWindow()
{
    delete ui;
}


void ClusterMainWindow::personalDataChanged(QString callsign, QString name, QString locator, QString qth)
{
    currentUserCallsign = callsign;
    currentUserName = name;
    currentUserLocator = locator;
    currentUserQTH = qth;
}



void ClusterMainWindow::onLaunchSetup()
{


    setupCluster->exec();
}


void ClusterMainWindow::onClearAllSpots()
{
    if (dxSpotDataModel->rowCount() > 0)
    {
        int ret = QMessageBox::warning(this, tr("Cluster"),
                                       tr("Please confirm you want to delete all the spots?"),
                                       QMessageBox::Yes | QMessageBox::No);

        if (ret == QMessageBox::Yes)
        {
            purgeSpotFlag = true;
            dxSpotDataModel->removeRows(0, dxSpotDataModel->rowCount(), QModelIndex());
            purgeSpotFlag = false;
        }
    }
}


void ClusterMainWindow::setAllTabsColor(QColor c)
{
    for (int i = 0; i < ui->clusterViewsTab->count(); i++)
    {
        ui->clusterViewsTab->setTabColor(i, c);
    }
}


void ClusterMainWindow::onSpotTabChanged(int index)
{
    if (index == -1)
    {
        return;
    }
    else
    {
        setAllTabsColor(CLUSTER_TAB_NOT_SELECT_COLOR);
        ui->clusterViewsTab->setTabColor(index, CLUSTER_TAB_SELECT_COLOR);
    }
}




void ClusterMainWindow::dxSpotView_sectionResized(int, int, int)
{
    QSettings settings;
    QByteArray state;

    state = dxSpotView->horizontalHeader()->saveState();
    settings.setValue("dxSpotView/state", state);

}


void ClusterMainWindow::sentSpotView_sectionResized(int, int, int)
{
    QSettings settings;
    QByteArray state;

    state = sentSpotView->horizontalHeader()->saveState();
    settings.setValue("sentSpotView/state", state);

}

void ClusterMainWindow::restoreDxSpotViewColumns()
{
    QSettings settings;
    QByteArray state;

    state = settings.value("dxSpotView/state").toByteArray();
    dxSpotView->horizontalHeader()->restoreState(state);
}

void ClusterMainWindow::restoreSentSpotViewColumns()
{
    QSettings settings;
    QByteArray state;

    state = settings.value("sentSpotView/state").toByteArray();
    sentSpotView->horizontalHeader()->restoreState(state);
}

void ClusterMainWindow::connectToNode(const QString &nodeName)
{
    //QString selNodeName = nodeName;

    if ((nodeName.isEmpty() && nodeConnected))
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
        if (nodeConnected)
        {
            if (currentNodeName != nodeName)
            {
                currentNodeName = nodeName;
                setupCluster->saveCurrentNodeName(currentNodeName);
            }

            //reconnect
            disconnectNode();
            // wait for disconnection, set flag to reconnect in loggedout slot
            reconnectFlag = true;
            startDisconnectTimer(15000);
            QEventLoop loop;
            QObject::connect( this, SIGNAL( disconnectTimerfinished() ), &loop, SLOT( quit() ) );
            loop.exec();

            // error if got here
            showStatusMessage(tr("Disconnect Timeout"), "Disconnect Timeout");
            QString msg = tr("Connect to Node - Disconnect Timeout");
            trace(msg);
            echoErrorMsg(msg);
        }
        else
        {
            currentNodeName = nodeName;
            connectToHost(nodeName);
            setupCluster->saveCurrentNodeName(currentNodeName);
        }

    }



}


void ClusterMainWindow::connectToHost(QString hostName)
{

    //currentNodeName = nodeName;

    if (setupCluster->doesClusterNameExist(hostName))
    {
        ui->nodeCb->setCurrentText(hostName);
        // get current node data
        QStringList nd = setupCluster->getClusterInfo(hostName);
        currentNodeName = nd[0];
        currentAddress = nd[1];
        currentPort = nd[2];
        currentPassword = nd[3];

        if (currentUserName.isEmpty() || currentUserCallsign.isEmpty()
                || currentUserQTH.isEmpty() || currentUserLocator.isEmpty())
        {
            int ret = QMessageBox::warning(this, tr("Connect to Cluster Node"),
                                           tr("Personal Data missing.\n"
                                              "User Name: %1\n"
                                              "User Callsign: %2\n"
                                              "User QTH: %3\n"
                                              "User Locator: %4\n"
                                              "Do you want to enter\\change your details?").arg(currentUserName).arg(currentUserCallsign).arg(currentUserQTH).arg(currentUserLocator),
                                           QMessageBox::Yes | QMessageBox::Cancel);
            switch (ret) {
              case QMessageBox::Yes:
                  setupCluster->setTabNum(PERSONAL_TABNUM);
                  onLaunchSetup();
                  break;
              case QMessageBox::Discard:
                  // Discard was clicked
                  break;
              default:
                  // should never be reached
                  break;
            }
        }
        else
        {
            client->connectToHost(currentAddress, currentPort.toUShort());
        }


    }

}




void ClusterMainWindow::connectionEstab()
{
    nodeConnected = true;
    showStatusMessage(tr("Connected to: %1 %2 %3").arg(currentNodeName).arg(currentAddress).arg(currentPort), "Connected");
    QString msg = tr("Connection Established with host %1 %2:%3").arg(currentNodeName).arg(currentAddress).arg(currentPort);
    trace(msg);
    echoMsg(msg);

}

void ClusterMainWindow::connectionError(QAbstractSocket::SocketError error)
{
    nodeConnected = false;
    showStatusMessage(tr("Connection Error: Error Code %1").arg(QString::number(error)), "Connection Error");
    QString msg = tr("Connection failed error %1").arg(error);
    trace(msg);
    echoErrorMsg(msg);
}



void ClusterMainWindow::logIn()
{
    QString msg = tr("Login Start - Send logon message\n");
    trace(msg);
    echoMsg(msg);
    client->login(QString("%1\r\n").arg(currentUserCallsign), currentPassword  + "\r\n");
    loginStart = true;
    echoMsg(tr("Logging in with callsign %1").arg(currentUserCallsign));

}

void ClusterMainWindow::loggedOut()
{
    QString msg = tr("Logged Out of node  %1").arg(currentNodeName);
    trace(QString(msg));
    echoErrorMsg(msg);
    nodeConnected = false;
    loginStart = false;
    loginSuccess = false;
    loginStatDetails = false;
    showStatusMessage((tr("Disconnected")), "Disconnected");
    if (reconnectFlag)
    {

        trace(QString("Logged Out - Reconnect to %1 ").arg(currentNodeName));
        reconnectFlag = false;
        disconnectTimer->stop();
        connectToHost(currentNodeName);
    }
    else
    {
        currentNodeName = "";
        currentAddress = "";
        currentPort = "";

    }
}

void ClusterMainWindow::disconnectNode()
{
    trace(QString("Disconnect Node %1").arg(currentNodeName));
    //client->logout();
    if (nodeConnected)
    {
        if (setupCluster->getRunEndFileFlag())
        {
            handleEndFile();          // send user commands
        }
        QString msg = dxClusterCommand->quit();
        txText(msg);
        echoCmd(msg);
    }


}




void ClusterMainWindow::messageRx(QString msg)
{
    //qDebug() << msg;
    rawClusterDataView->appendPlainText(msg.remove('\x07'));
}


void ClusterMainWindow::echoCmdRawTextWindow(QString cmd)
{
    rawClusterDataView->appendPlainText(cmd.remove('\n'));
}

void ClusterMainWindow::echoCmd(QString cmd)
{
    echoCmdRawTextWindow(QString("<%1>").arg(cmd));
}

void ClusterMainWindow::echoMsg(QString msg)
{
    echoCmdRawTextWindow(QString("[%1]").arg(msg));
}

void ClusterMainWindow::echoErrorMsg(QString err)
{
    echoCmdRawTextWindow(QString("[Error: %1]").arg(err));
}

void ClusterMainWindow::checkedLoggedIn(QString msg)
{
    QString endOfMsg = QString(">\r\n");

    if (loginStart && !loginSuccess)  // loginSuccess not used at the moment!
    {
        if (msg.contains(endOfMsg))
        {
            loginSuccess = true;
            txText("set/echo enable\n");
            txText(dxClusterCommand->setNameMsg(currentUserName));
            txText(dxClusterCommand->setQthMsg(currentUserQTH));
            txText(dxClusterCommand->setQraMsg(currentUserLocator));


            //txText("SH/ST\n");      // ask for station details
            //loginStatDetails = true;


            if (setupCluster->getRunStartFileFlag())
            {
                handleStartFile();          // send user commands
            }

            sendPingMessage();

        }

    }


}




void ClusterMainWindow::checkStationDetails(QString msg)
{
    QStringList details = {"Name", "QTH", "Location"};
    QStringList ourDetails = {currentUserName, currentUserQTH, currentUserLocator};
    bool foundMatch[] = {false, false, false};
    QString buf;
    QTextStream in;
    in.setString(&buf, QIODevice::ReadOnly);
    buf = msg;
    QString line;
    QStringList data;
    if (loginStatDetails)
    {

        do
        {
            line = in.readLine();
            if (!line.isEmpty())
            {
                data.append(line);
            }
        }while (!line.isNull());

        if (data.count() >= details.count())
        {
            for (int i = 0; i < details.count(); i++)
            {
                for (int x = 0; x < data.count(); x++)
                {
                    if (data[x].contains(details[i]) && data[x].contains(ourDetails[i]))
                    {
                        foundMatch[i] = true;
                    }

                }
            }
        }
    }


    txText("set/echo enable\n");

    if (!foundMatch[0])
    {
        txText(dxClusterCommand->setNameMsg(currentUserName));
    }
    else if (!foundMatch[1])
    {
        txText(dxClusterCommand->setQthMsg(currentUserQTH));
    }
    else if (!foundMatch[2])
    {
        txText(dxClusterCommand->setQraMsg(currentUserLocator));
    }


    if (setupCluster->getRunStartFileFlag())
    {
        handleStartFile();          // send user commands
    }



}


void ClusterMainWindow::handleStartFile()
{
    handleCmdFile(CLUSTER_PATH + CLUSTER_START_FILE);
}


void ClusterMainWindow::handleEndFile()
{
    handleCmdFile(CLUSTER_PATH + CLUSTER_END_FILE);
}

void ClusterMainWindow::handleCmdFile(QString fileName)
{
    QStringList listCmds;
    //QString fileName = CLUSTER_PATH + CLUSTER_START_FILE;
    if (FileExists(fileName))
    {
        QString msg = tr("handleCmdFile: Command file found - %1").arg(fileName);
        trace(msg);
        echoMsg(msg);
        QFile inputFile(fileName);
        if (inputFile.open(QIODevice::ReadOnly))
        {
           QTextStream in(&inputFile);
           while (!in.atEnd())
           {
              QString line = in.readLine().append('\n');
              listCmds.append(line);
           }
           inputFile.close();
        }
    }
    else
    {
        QString msg = tr("handleCmdFile: Command File missing - %1!").arg(fileName);
        trace(msg);
        echoErrorMsg(msg);
        return;
    }

    if (!listCmds.isEmpty())
    {
        QString msg = tr("handleCmdFile: Sending Commands");
        trace(msg);
        echoMsg(msg);
        for (int i = 0; i < listCmds.count(); ++i)
        {
            QString cmd = listCmds[i];
            if (cmd != "")
            {
                if (cmd[0] != CLUSTER_START_COMMENT_DELIMTER)
                {
                    txText(cmd);
                }
            }
        }

        msg = tr("handleCmdFile: Finished sending Commands");
        trace(msg);
        echoMsg(msg);

    }
    else
    {
        QString msg = tr("handleCmdFile: Command file empty %1").arg(fileName);
        trace(msg);
        echoErrorMsg(msg);
    }
}


void ClusterMainWindow::parseDX(const QString txt)
{
    QString buf;
    QTextStream in;
    in.setString(&buf, QIODevice::ReadOnly);
    buf = txt;

    int retCode = SPOT_OK;
    SpotData newSpot;
    newSpot.clear();

    QString line;
    if (loginSuccess)
    {
        trace(QString("raw spot = %1").arg(txt));

        do
        {
            line = in.readLine();
            trace(QString("ParseDx - readLine = %1").arg(line));
            if (!line.isEmpty())
            {
                if (line.contains("DX de"))
                {
                   retCode = upackDxSpot(line, newSpot);
                   trace(QString("ParseDx - Unpack DxSpot error = %1").arg(clusterErrorMsg[retCode * -1]));
                   if (retCode == SPOT_OK)
                   {
                       processNewSpot(newSpot);
                   }

                }
                else if (checkShowDxMsg(line, newSpot))
                {
                    retCode = upackShowDxSpot(line, newSpot);
                    trace(QString("ParseDx - Unpack ShowDxSpot error = %1").arg(clusterErrorMsg[retCode * -1]));
                    if (retCode == SPOT_OK)
                    {
                        processNewSpot(newSpot);
                    }
                }
                // look for qrz info

                else if (askQraData.getAskQrz() && line.contains("qrz"))
                {

                    retCode = getQrzReply(line);        // not using retCode here...
                    if (!qrzInfo.getGotAllData())
                    {
                        // still waiting
                        trace(QString("ParseDx - waiting for qrzInfo for askcallsign = %1, spot callsign = %2").arg(askQraData.getAskCallsign()).arg(spotWaitingForQraFromNode.getDxCall()));
                    }
                    else
                    {
                        // got all the data
                        trace(QString("ParseDx - got all the data from qrz for callsign = %1, spot callsign = %2").arg(qrzInfo.getCall()).arg(spotWaitingForQraFromNode.getDxCall()));

                        if (qrzInfo.getCall() == askQraData.getAskCallsign())
                        {

                            trace(QString("ParseDx - qrz info matches waiting callsign = %1").arg(askQraData.getAskCallsign()));


                            newSpot = spotWaitingForQraFromNode;

                            if (!qrzInfo.getError())
                            {
                                trace(QString("ParseDx - qrzInfo no error, add locator to spot = %1").arg(qrzInfo.getGrid()));
                                newSpot.setDxLocator(qrzInfo.getGrid());
                                newSpot.setDxLocatorIsFromNode(true);
                            }
                            else
                            {
                                trace(QString("ParseDx - qrzInfo error no data found for callsign = %1, lookup using prefix").arg(qrzInfo.getCall()));
                                // asking qrz via node didn't give qra, use prefix
                                // let's lookup using prefix
                                QString loc = getQraFromCallsignPrefix(newSpot.getDx_Call());
                                trace(QString("ParseDx - get QRA from Prefix, add locator to spot = %1").arg(loc));
                                newSpot.setDxLocator(loc);
                                newSpot.setDxLocatorIsFromNode(true);
                            }

                            //spotListNoQra.remove(qrzInfo.getCall());
                            spotWaitingForQraFromNode.clear();
                            qrzInfo.clear();
                            askQraData.clear();
                            //getQrzInfo = false;
                            retCode = SPOT_OK;

                            if (!newSpot.getDxLocator().isEmpty())
                            {
                                processNewSpot(newSpot);
                            }


                        }
                        else
                        {
                              trace(QString("ParseDx - QrzInfo call = %1, does not match waiting call %2").arg(qrzInfo.getCall()).arg(askQraData.getAskCallsign()));
                        }
                    }

                }

            }
       } while (!line.isNull());
    }

    trace(QString("ParseDx: Finished"));
}




void ClusterMainWindow::processNewSpot(SpotData &newSpot)
{
    trace(QString("ProcessNewSpot: DX de %1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12 %13 %14")
                        .arg(newSpot.getDxCall()).arg(newSpot.getDxFreq()).arg(newSpot.getDxBandStr()).arg(newSpot.getDxBandMask()).arg(newSpot.getDxModeStr()).arg(newSpot.getDxModeMaskStr())
                        .arg(newSpot.getSpotterCall()).arg(newSpot.getDxLocator()).arg(newSpot.getSpotterLocator()).arg(newSpot.getDxPropMode()).arg(newSpot.getSpotTime()).arg(newSpot.getSpotDate()).arg(newSpot.getSpotComment()).arg(setupCluster->getTimeToLive()));

    qint64 rxTime = newSpot.getSpotDateTime().toMSecsSinceEpoch()/1000;

    // is spot older than time to live time
    int timeToLive = setupCluster->getTimeToLive().toInt() * 60;
    if (timeToLive == 0 || (timeToLive > 0 && !spotTimedOut(rxTime, timeToLive)))
    {
        trace(QString("ProcessNewSpot: Spot within timeToLive"));
        // does the spot have a dxLocator
        if (newSpot.getDxLocator().isEmpty())
        {
            // queue to ask Qrz for locator
            trace(QString("ProcessNewSpot: No DxCall locator, queue to ask qrz call = %1").arg(newSpot.getDxCall()));
            spotListNoQra.append(newSpot);
        }
        else
        {
            if (currentUserCallsign != newSpot.getSpotterCall())
            {
                // send spot to clients if spotter isn't this station
                trace(QString("ProcessNewSpot: Spotter not this station, pass to clients, callsign %1").arg(newSpot.getDxCall()));
                sendSpotsToClientQueue.append(createSpotToSend(QString("%1:%2:%3:%4:%5:%6:%7:%8:%9:%10:%11:%12:%13:%14:%15")
                                                               .arg(newSpot.getDxCall())
                                                               .arg(newSpot.getDxLocator())
                                                               .arg(newSpot.getDxLocatorIsFromNode() ? "locFromNode-true" : "locFromNode-false")
                                                               .arg(newSpot.getDxFreq())
                                                               .arg(newSpot.getDxBandStr())
                                                               .arg(newSpot.getDxBandMask())
                                                               .arg(newSpot.getDxModeStr())
                                                               .arg(newSpot.getDxModeMaskStr())
                                                               .arg(newSpot.getSpotterCall())
                                                               .arg(newSpot.getSpotterLocator())
                                                               .arg(newSpot.getSpotTime())
                                                               .arg(newSpot.getSpotDate())
                                                               .arg(newSpot.getSpotComment())
                                                               .arg(newSpot.getDxPropMode())
                                                               .arg(setupCluster->getTimeToLive())));
            }
            else
            {
                trace(QString("ProcessNewSpot: Spotter is this station, only display on server"));
            }

            trace(QString("ProcessNewSpot: Add spot for display callsign = %1, rxTime = %2").arg(newSpot.getDxCall()).arg(rxTime));
            spotsList.append(new SpotData(newSpot));

        }
    }
    else
    {
        trace(QString("ProcessNewSpot: Spot %1, older than time to live time = %2 mins").arg(newSpot.getDxCall()).arg(timeToLive/60));
    }


}


void ClusterMainWindow::handAskQraTimer()
{


    if (!spotListNoQra.isEmpty() && !askQraData.getAskQrz())
    {

           spotWaitingForQraFromNode = spotListNoQra.first();
           spotListNoQra.removeFirst();
           trace(QString("Get QRA for callsign %1").arg(spotWaitingForQraFromNode.getDxCall()));

           // use call to get QRA from node QRZ command
           askQraData.setAskCallsign(spotWaitingForQraFromNode.getDx_Call().realCall);
           // flag we are asking QRZ
           trace(QString("Get QRA from node with QRA command, callsign = %1").arg(askQraData.getAskCallsign()));
           askQraData.setAskQrz(true);
           askQraTimeout->start(ASKQRA_TIMEOUT);
           txText(dxClusterCommand->showQRZMsg(askQraData.getAskCallsign()));


     }

}


void ClusterMainWindow::handleAskQraTimeout()
{

    askQraTimeout->stop();
    trace(QString("handleAsKQraTimeout: timeout expired for callsign = %1").arg(askQraData.getAskCallsign()));


}




int ClusterMainWindow::getQrzReply(QString &line)
{
    if (line.contains("Error"))
    {
        // callsign not found
        QStringList sl = line.split(':');
        if (sl.count() == 3)
        {
            qrzInfo.setCall(sl[2]);
            qrzInfo.setError(true);
            qrzInfo.setGotAllData(true);
        }
        return SPOT_OK;
    }

    else if (line.contains("call") && line.contains(':'))
    {
        QStringList sl = line.split(':');
        if (sl.count() == 2)
        {
            qrzInfo.setCall(sl[1]);
            qrzInfo.setFound(true);

        }
    }
    else if (line.contains("ADIF") && line.contains(':'))
    {
        QStringList sl = line.split(':');
        if (sl.count() == 2)
        {
            qrzInfo.setAdif(sl[1]);

        }
    }
    else if (line.contains("fname") && line.contains(':'))
    {
        QStringList sl = line.split(':');
        if (sl.count() == 2)
        {
            qrzInfo.setFname(sl[1]);

        }
    }
    else if (line.contains("name") && line.contains(':'))
    {
        QStringList sl = line.split(':');
        if (sl.count() == 2)
        {
            qrzInfo.setName(sl[1]);

        }
    }
    else if (line.contains("addr2") && line.contains(':'))
    {
        QStringList sl = line.split(':');
        if (sl.count() == 2)
        {
            qrzInfo.setAddr2(sl[1]);

        }
    }
    else if (line.contains("country") && line.contains(':'))
    {
        QStringList sl = line.split(':');
        if (sl.count() == 2)
        {
            qrzInfo.setCountry(sl[1]);

        }
    }
    else if (line.contains("lat") && line.contains(':'))
    {
        QStringList sl = line.split(':');
        if (sl.count() == 2)
        {
            qrzInfo.setLat(sl[1]);

        }
    }
    else if (line.contains("lon") && line.contains(':'))
    {
        QStringList sl = line.split(':');
        if (sl.count() == 2)
        {
            qrzInfo.setLon(sl[1]);

        }
    }
    else if (line.contains("grid") && line.contains(':'))
    {
        QStringList sl = line.split(':');
        if (sl.count() == 2)
        {
            qrzInfo.setGrid(sl[1].toUpper());

        }
    }
    else if (line.contains("moddate") && line.contains(':'))
    {
        QStringList sl = line.split(':');
        if (sl.count() == 4)
        {
            QString modDate = sl[1] + ":" + sl[2] + ":" + sl[3];
            qrzInfo.setModdate(modDate);
        }
    }
    else if (line.contains("www.qrz.com"))
    {
        qrzInfo.setGotAllData(true);
        return SPOT_OK;
    }

    return ASKQRZ_FAILED_QRA;
}


QString ClusterMainWindow::getQraFromCallsignPrefix(Callsign cs)
{
    QString prefix;

    if (cs.locCtryPrefix != cs.dupPrefix)
    {
        prefix = cs.locCtryPrefix;
    }
    else
    {
        prefix = cs.dupPrefix;
    }

    QSharedPointer<CountrySynonym> syn = MultLists::getMultLists()->searchCountrySynonym ( prefix );
    return syn.data()->country.data()->central.loc.getValue();
}





int ClusterMainWindow::upackShowDxSpot(const QString txt, SpotData &newSpot)
{

    trace(QString("UnpackShowDXSpot - %1").arg(txt));


#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    dxMsg = txt.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
#else
    dxMsg = txt.split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
#endif

    if (dxMsg.count() > 4)
    {
        QString f = dxMsg[0] + "00";
        f.remove('.');
        newSpot.setDxFreq(Frequency(f).convertFreqStrDisp());
        QString dxBandStr;
        QString dxBandMask;
        getBand(bands, newSpot.getDxFreq(), dxBandStr, dxBandMask);
        newSpot.setDxBandStr(dxBandStr);
        newSpot.setDxBandMask(dxBandMask);

        if (dxBandStr.isEmpty() && !enableHFSpots)
        {
            // discard spot as it is HF
            trace(QString("Unpack Show DX Spot: Discard Spot HF = %1").arg(newSpot.getDxFreq()));
            return DISCARD_HF_SPOT * -1;
        }

        QString dxModeStr;
        QString dxModeMask;
        getMode(modeBandPlan, newSpot.getDxFreq(), dxBandStr, dxModeStr, dxModeMask);
        newSpot.setDxModeStr(dxModeStr);
        newSpot.setDxModeMaskStr(dxModeMask);

        newSpot.populateDxCall(dxMsg[1]); // populate QString dxCall and Callsign dx_Call
        newSpot.setSpotDate(dxMsg[2]);
        newSpot.setSpotTime(dxMsg[3].remove('Z'));
        newSpot.setSpotDateTime(getSpotDateTime(newSpot.getSpotDate(), newSpot.getSpotTime()));
        if (! newSpot.getSpotDateTime().isValid())
        {
           return SPOT_DATETIME_INVALID * -1;
        }
        QString sptCall = newSpot.getSpotterCall();
        sptCall.prepend('<').append('>');
        // reassemble comment
        QString spotComment;
        for (int i = 4; i < dxMsg.indexOf(sptCall); i++)
        {
            if (dxMsg[i] != "")
            {
                spotComment += dxMsg[i] + " ";
            }
        }

        newSpot.setSpotComment(spotComment);

        QString spotLocator;
        QString dxLocator;
        findLocInComment(spotLocator, dxLocator, spotComment);
        newSpot.setSpotterLocator(spotLocator);
        newSpot.setDxLocator(dxLocator);

        newSpot.setDxPropMode(getPropMode(spotComment));

        // look for mode in comments, if found overide freq mode
        int commentModeNum;
        QString commentMode;
        if (lookforModeInComment(spotComment, commentModeNum, commentMode))
        {
            newSpot.setDxModeStr(commentMode);
            newSpot.setDxModeMaskStr(QString::number(commentModeNum));
        }

        return SPOT_OK;
    }

    return SPOT_TOO_MANY_SECTIONS * -1;

}


bool ClusterMainWindow::checkShowDxMsg(const QString txt, SpotData &newSpot)
{

    QChar sep1 = '<';
    QChar sep2 = '>';

    int countSep1 = 0;
    int countSep2 = 0;

    countSep1 = txt.count(sep1);
    countSep2 = txt.count(sep2);
    QList<int> SepIdx1;
    QList<int> SepIdx2;
    QStringList extractStr;
    //bool foundCall = false;


    if (((countSep1 - countSep2) == 0) && countSep1 >= 1 && countSep2 >= 1)
    {
        // we have a matching pair of seperators, get their index positions
        int pos = 0;
        for (int i = 0; i < countSep1; i++)
        {
            pos = txt.indexOf(sep1, pos);
            SepIdx1 += pos + 1;
            pos++;
        }

        pos = 0;
        for (int i = 0; i < countSep2; i++)
        {
            pos = txt.indexOf(sep2, pos);
            SepIdx2 += pos;
            pos++;
        }
    }

    for (int i = 0; i < SepIdx1.count(); i++)
    {
        QString str;
        if (SepIdx2[i] - SepIdx1[i] > 0)
        {
            extractStr += txt.mid(SepIdx1[i], SepIdx2[i] - SepIdx1[i]);
        }

    }

    if (extractStr.count() > 0)
    {
        for (int i = 0; i < extractStr.count(); i++)
        {
            Callsign callsign(extractStr[i]);
            if (callsign.validate() == CS_OK)
            {
                newSpot.setSpotterCall(extractStr[i]);
                return true;

            }
        }
    }

    return false;

}



void ClusterMainWindow::onResendSpotToClients(int frameId, QString loggerUuid, QString cmd, int bandMask)
{
    ResendSpotCommand spotCmd;
    spotCmd.setCmd(cmd);
    spotCmd.setBandmak(bandMask);
    spotCmd.setUuid(loggerUuid);
    spotCmd.setFrameId(frameId);
    resendSpotsToClientQueue.append(spotCmd);

}


void ClusterMainWindow::handleResendSpotToClientsCmds()
{

    if (!resendSpotsToClientQueue.isEmpty())
    {

        for (int i = 0; i < resendSpotsToClientQueue.count(); i++)
        {
            if (resendSpotsToClientQueue[i].getCmd().contains(RESEND_ALL_SPOTS))
            {

                resendAllSpotsToClients(resendSpotsToClientQueue[i]);

            }
        }

        resendSpotsToClientQueue.clear();
    }
}

void ClusterMainWindow::resendAllSpotsToClients(ResendSpotCommand cmd)
{

    if (dxSpotDataModel->rowCount() > 0)
    {
        for (int row = 0; row < dxSpotDataModel->rowCount(); row ++)
        {
            if (cmd.getBandmask() | dxSpotDataModel->data(dxSpotDataModel->index(row, DXBANDMASK_COL_NUM), DataStoredRole).toString().toInt())
            {
                QString spot = createResendSpotToSend(getSpotFromDisplayDb(row));
                trace(QString("resending this spot - %1 to uuid = %2").arg(spot).arg(cmd.getuuid()));
                clusterRpc->sendDXSpot(spot, cmd.getuuid(), cmd.getFrameId());   // send spot and loggeruuid
            }

        }
    }

}


QString ClusterMainWindow::getSpotFromDisplayDb(int row)
{

    QString dxCall = dxSpotDataModel->data(dxSpotDataModel->index(row, DXSPOT_CALL_COL_NUM), DataStoredRole).toString();
    QString dxLocator = dxSpotDataModel->data(dxSpotDataModel->index(row, DXLOC_COL_NUM), DataStoredRole).toString();
    bool dxLocFromNodeFlag = dxSpotDataModel->data(dxSpotDataModel->index(row, DXLOC_FROM_NODE_FLAG_COL_NUM), DataStoredRole).toBool();
    Frequency dxFreq = qvariant_cast<Frequency>(dxSpotDataModel->data(dxSpotDataModel->index(row, FREQ_COL_NUM), DataStoredRole));
    QString dxBandStr = dxSpotDataModel->data(dxSpotDataModel->index(row, DXBANDSTR_COL_NUM), DataStoredRole).toString();
    QString dxBandMask = dxSpotDataModel->data(dxSpotDataModel->index(row, DXBANDMASK_COL_NUM), DataStoredRole).toString();
    QString dxModeStr = dxSpotDataModel->data(dxSpotDataModel->index(row, DXSPOT_MODE_COL_NUM), DataStoredRole).toString();
    QString dxModeMask = dxSpotDataModel->data(dxSpotDataModel->index(row, DXMODEMASK_COL_NUM), DataStoredRole).toString();
    QString spotCall = dxSpotDataModel->data(dxSpotDataModel->index(row, SPOTTER_CALL_COL_NUM), DataStoredRole).toString();
    QString spotLocator = dxSpotDataModel->data(dxSpotDataModel->index(row, SPOTTER_LOC_COL_NUM), DataStoredRole).toString();
    QString spotTime = dxSpotDataModel->data(dxSpotDataModel->index(row, TIME_COL_NUM), DataStoredRole).toString();
    //qint64 rxTimeMsecs = dxSpotDataModel->data(dxSpotDataModel->index(row, RXTIME_COL_NUM), DataStoredRole).toLongLong();
    //QDateTime spotDateTime  = QDateTime::fromMSecsSinceEpoch(rxTimeMsecs);
    QString spotDate = dxSpotDataModel->data(dxSpotDataModel->index(row, DATE_COL_NUM), DataStoredRole).toString();
    QString spotComment = dxSpotDataModel->data(dxSpotDataModel->index(row, COMMENT_COL_NUM), DataStoredRole).toString();
    QString dxPropMode = dxSpotDataModel->data(dxSpotDataModel->index(row, DXSPOT_PROP_MODE_COL_NUM), DataStoredRole).toString();

    return QString("%1:%2:%3:%4:%5:%6:%7:%8:%9:%10:%11:%12:%13:%14:%15")
            .arg(dxCall)
            .arg(dxLocator)
            .arg(dxLocFromNodeFlag ? "locFromNode-true" : "locFromNode-false")
            .arg(dxFreq.str())
            .arg(dxBandStr)
            .arg(dxBandMask)
            .arg(dxModeStr)
            .arg(dxModeMask)
            .arg(spotCall)
            .arg(spotLocator)
            .arg(spotTime)
            .arg(spotDate)
            .arg(spotComment)
            .arg(dxPropMode)
            .arg(setupCluster->getTimeToLive());

}


void ClusterMainWindow::cancelPingTimeOut(QString msg)
{
    if (msg.contains("ping_cluster"))
    {
        pingOk = true;
        trace(QString("response to ping received ok"));
    }


}

void ClusterMainWindow::sendPingMessage()
{
    pingOk = false;
    txText(dxClusterCommand->pingMsg());
    pingClusterNodeTimer->start(getPingTimeoutValue());
    trace(QString("sent ping message to cluster node"));
}

void ClusterMainWindow::handlePingClusterNodeTimeout()
{
    if(pingOk)
    {
        // send a ping to test connection
        trace(QString("ping received ok, ping timeout, send another ping"));
        sendPingMessage();
    }
    else
    {
        trace(QString("ping response was not received ok - connection lost?"));
    }

}


int ClusterMainWindow::getPingTimeoutValue()
{
    QString filename = "./Configuration/Cluster/ClusterSettings.ini";
    QSettings settings(filename, QSettings::IniFormat);
    settings.beginGroup("PingTimeout");
    int timeout = settings.value("PingTimeout", 60000).toInt();
    settings.endGroup();
    return timeout;
}

QString ClusterMainWindow::createSpotToSend(QString spot)
{
    return DXSPOT + spot;
}

QString ClusterMainWindow::createStatusToSend(QString status)
{
    return CLUSTER_STATUS + status;
}

QString ClusterMainWindow::createResendSpotToSend(QString spot)
{
    return RESENTSPOT + spot;
}


void ClusterMainWindow::getSpotsToSendToClientQueues()
{
    getSpotsFromDisplayQueue();

    getSpotsFromSendToClientQueue();

    handleResendSpotToClientsCmds();


}



void ClusterMainWindow::getSpotsFromSendToClientQueue()
{

        if (!sendSpotsToClientQueue.isEmpty())
        {

            // get spots from queue and send to client
            trace(QString("getSpotsFromSendQueue: spots available = %1").arg(sendSpotsToClientQueue.count()));
            while (sendSpotsToClientQueue.count() > 0)
            {
                trace(QString("Sending spot from send queue, queue length = %1, spot = %2").arg(sendSpotsToClientQueue.count()).arg(sendSpotsToClientQueue[0]));
                clusterRpc->sendDXSpot(sendSpotsToClientQueue[0], "", resendFrameId::ALL_CLIENTS);      // uuid = space all logs
                sendSpotsToClientQueue.removeFirst();
            }
        }


}





// this is the queue of spots for display
void ClusterMainWindow::getSpotsFromDisplayQueue()
{
    if (!spotsList.isEmpty())
    {
        trace(QString("GetSpotsFromDisplayQueue: spots available = %1").arg(spotsList.count()));
        // get spots from queue
        int slsize= spotsList.count();
        for (int i = slsize -1 ; i > -1; i--)
        {

            if (purgeSpotFlag)
            {
                trace(QString("GetSpotsFromDisplayQueue: PurgeFlag On"));
                return;
            }

            dxSpotDataModel->rowData = spotsList[i];
            spotsList.remove(i);
            //dxSpotDataModel->insertRows(0, 1);
            dxSpotDataModel->insertRows(dxSpotDataModel->rowCount(), 1);
            trace(QString("GetSpotsFromDisplayQueue: finished loop"));


        }

        trace(QString("GetSpotsFromDisplayQueue: finished"));
    }
}


int ClusterMainWindow::upackDxSpot(QString txt, SpotData &newSpot)
{

    trace(QString("UnpackDXSpot - %1").arg(txt));
    int timePos = 0;

    txt.remove('\x07');

    dxMsg = txt.split(QRegularExpression("\\s+"));

    if (dxMsg.count() > 5)
    {
        newSpot.setSpotterCall(dxMsg[2].remove(':'));
        QString f = dxMsg[3] + "00";
        f.remove('.');

        //dxFreq = convertKhzToMhz(dxMsg[3]);
        newSpot.setDxFreq(Frequency(f).convertFreqStrDisp());


        QString dxBandStr;
        QString dxBandMask;
        getBand(bands, newSpot.getDxFreq(), dxBandStr, dxBandMask);
        newSpot.setDxBandStr(dxBandStr);
        newSpot.setDxBandMask(dxBandMask);

        if (dxBandStr.isEmpty() && !enableHFSpots)
        {
            // discard spot as it is HF
            trace(QString("Unpack DX Spot: Discard Spot HF = %1").arg(newSpot.getDxFreq()));

            return DISCARD_HF_SPOT * -1;
        }

        QString dxModeStr;
        QString dxModeMask;
        getMode(modeBandPlan, newSpot.getDxFreq(), newSpot.getDxBandStr(), dxModeStr, dxModeMask);
        newSpot.setDxModeStr(dxModeStr);
        newSpot.setDxModeMaskStr(dxModeMask);

        newSpot.populateDxCall(dxMsg[4]); // populate QString dxCall and Callsign dx_Call
        // find time
        for (int i = 4; i < dxMsg.count(); i++)
        {
            QRegularExpression re("\\d\\d\\d\\dZ");
            QRegularExpressionMatch match = re.match(dxMsg[i]);
            if (match.hasMatch())
            {
                newSpot.setSpotTime(dxMsg[i].remove('Z'));
                timePos = i;
                break;
            }
        }

        if (newSpot.getSpotTime() == "")
        {
            //error
            return NO_SPOT_TIME * -1;
        }

        // get current date
        QDate d = QDate::currentDate();
        newSpot.setSpotDate(d.toString("dd-MMM-yyyy"));
        newSpot.setSpotDateTime( getSpotDateTime(newSpot.getSpotDate(), newSpot.getSpotTime()));
        if (!newSpot.getSpotDateTime().isValid())
        {
           return SPOT_DATETIME_INVALID * -1;
        }

        // look for locator
        if (timePos + 1 >= dxMsg.count())  // make sure not out of range
        {
            // no spotlocator sent
            newSpot.setSpotterLocator("");
        }
        else
        {
            newSpot.setSpotterLocator(dxMsg[timePos + 1]);
        }
        // reassemble the comment
        QString spotComment;
        for (int i = 5; i < timePos; i++)
        {
            if (dxMsg[i] != "")
            {
                spotComment += dxMsg[i] + " ";
            }
        }

        newSpot.setSpotComment(spotComment);

        // remove seperator char from comment
        spotComment.remove(SPOT_DATA_SEPERATOR);
        QString spotLocator;
        QString dxLocator;
        findLocInComment(spotLocator, dxLocator, spotComment);
        newSpot.setSpotterLocator(spotLocator);
        newSpot.setDxLocator(dxLocator);

        newSpot.setDxPropMode(getPropMode(spotComment));

        // look for mode in comments, if found overide freq mode
        int commentModeNum;
        QString commentMode;
        if (lookforModeInComment(spotComment, commentModeNum, commentMode))
        {
            newSpot.setDxModeStr(commentMode);
            newSpot.setDxModeMaskStr(QString::number(commentModeNum));
        }

        return SPOT_OK;
    }

    return SPOT_TOO_MANY_SECTIONS * -1;

}




QString ClusterMainWindow::getPropMode(const QString comment)
{
    for (int i = 0; i < clusterPropModes.count(); i++)
    {
        if (comment.contains(clusterPropModes[i], Qt::CaseInsensitive))
        {
            return clusterPropModes[i];
        }
    }

    return "";

}

void ClusterMainWindow::findLocInComment(QString &spotLoc, QString &dxLoc, const QString &comment)
{
    QStringList loc;
    trace(QString("Extract locators - comment = %1").arg(comment));
    // this should hopefully cope with different scenarios, independent of seperation chars
    // it is dependent on the sender correctly ordering the spotLoc and dxLoc, the spotLoc should be first

    int fullLocExpCount = comment.count(FULL_LOC_EXP);
    int partLocExpCount = comment.count(PART_LOC_EXP);

    if (fullLocExpCount == 2)
    {
        int firstIndex = comment.indexOf(FULL_LOC_EXP);
        int secondIndex = comment.indexOf(FULL_LOC_EXP, firstIndex + 4);
        // extract locators
        spotLoc = comment.mid(firstIndex, 6).toUpper();
        dxLoc = comment.mid(secondIndex, 6).toUpper();
    }
    else if (partLocExpCount == 2)
    {
        int firstIndex = comment.indexOf(PART_LOC_EXP);
        int secondIndex = comment.indexOf(PART_LOC_EXP, firstIndex + 4);
        // extract locators
        spotLoc = comment.mid(firstIndex, 4).toUpper();
        dxLoc = comment.mid(secondIndex, 4).toUpper();
    }
    else if (fullLocExpCount == 1 && partLocExpCount == 1)
    {
        int firstIndex = comment.indexOf(FULL_LOC_EXP);
        int secondIndex = comment.indexOf(PART_LOC_EXP);
        if (firstIndex < secondIndex)
        {
            spotLoc = comment.mid(firstIndex, 6).toUpper();
            dxLoc = comment.mid(secondIndex, 4).toUpper();
        }
        else
        {
            spotLoc = comment.mid(secondIndex, 4).toUpper();
            dxLoc = comment.mid(firstIndex, 6).toUpper();
        }
    }
    else if (fullLocExpCount == 1 && partLocExpCount == 0)
    {
        int firstIndex = comment.indexOf(FULL_LOC_EXP);
        dxLoc = comment.mid(firstIndex, 6).toUpper();
    }
    else if (partLocExpCount == 1 && fullLocExpCount == 0)
    {
        int firstIndex = comment.indexOf(PART_LOC_EXP);
        dxLoc = comment.mid(firstIndex, 4).toUpper();
    }

    trace(QString("Extracted dxLoc = %1 spotLoc= %2").arg(dxLoc).arg(spotLoc));

}


QString ClusterMainWindow::extractLocator(const QString &text, const QRegularExpression fullLocExp, const QRegularExpression partLocExp)
{
    if (text.contains(fullLocExp))
    {
        return text.mid(text.indexOf(fullLocExp, 0), 6).toUpper();

    }
    else if (text.contains(partLocExp))
    {
       return text.mid(text.indexOf(partLocExp, 0), 4).toUpper();
    }

    return "";
}


bool ClusterMainWindow::lookforModeInComment(const QString &spotComment, int &commentModeNum, QString &commentMode)
{
    for (int i = 0; i < clusterModes.count(); i++)
    {
        if (spotComment.contains(clusterModes[i], Qt::CaseInsensitive))
        {
            commentModeNum = i;
            commentMode = clusterModes[i];
            return true;
        }
    }

    return false; // nothing found
}

// ************* Send text *************************************************

void ClusterMainWindow::sendText()
{
    //client->sendData(ui->sendLine->text()+'\n');
    //ui->sendLine->clear();
}


int ClusterMainWindow::txText(QString msg)
{
    trace(QString("txText: Sending text - %1 - to dxCluster").arg(msg));
    int error = 0;
    if (loginSuccess)
    {
       error = client->sendData(msg);

       echoCmd(msg);
       return error;

    }
    else
    {
        QString err = tr("Sending command - Not logged in  - %1").arg(msg);
        echoErrorMsg(err);
    }

    return -1; // error

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


void ClusterMainWindow::sendSpotToDXCluster(Frequency freq, QString call, QString loc)
{


    bool spotStatus = false;
    QString spotMsg = assembleSpotForDXCluster(freq, call, loc);
    if (setupCluster->getSendToDXClusterEnabled() && loginSuccess && !freq.isClear() && !call.isEmpty())
    {
        trace(QString("SendSpotToDXCluster: sending spot, call %1, freq %2, locator %3").arg(call).arg(freq.traceStr()).arg(loc));
        if (BandList::getBandList().checkValidBand(freq))
        {

#ifdef TXSPOT
            int error = txText(spotMsg);
            if (error < 0)
            {
                trace(QString("SendSpotToDXCluster: sending spot %1 failed to send").arg(spotMsg));
                spotStatus = false;
                addSentSpotToDisplayQueue(spotStatus, tr(sendClusterReasonText[COMMS_ERR]));
            }
            else
            {
                trace(QString("SendSpotToDXCluster: sending spot %1 sent Ok").arg(spotMsg));
                spotStatus = true;
                addSentSpotToDisplayQueue(spotStatus, tr(sendClusterReasonText[TX_OK]));
            }
#endif


        }
        else
        {
            trace(QString("SendSpotToDXCluster: spot freq is out of band %1, spot callsign %2").arg(freq.traceStr()).arg(call));
            spotStatus = false;
            addSentSpotToDisplayQueue(false, tr(sendClusterReasonText[FREQ_ERR]));
        }
    }
    else
    {
        spotStatus = false;
        if (!loginSuccess)
        {
           addSentSpotToDisplayQueue(false, tr(sendClusterReasonText[NOT_LOGGED_ON]));
        }
        else if (!freq.isClear() || !call.isEmpty())
        {
           addSentSpotToDisplayQueue(false, tr(sendClusterReasonText[CALL_LOC_EMPTY]));
        }
    }
}


void ClusterMainWindow::addSentSpotToDisplayQueue(bool spotStatus, QString reason)
{
    QDateTime sentSpotDateTime = QDateTime::currentDateTimeUtc();
    qint64 rxTime = sentSpotDateTime.toMSecsSinceEpoch()/1000;
    QString sentSpotTime = sentSpotDateTime.toString("HH:mm");
    SentSpotData* sentSpotData = new SentSpotData(rxTime, sentSpotTime,
                                                   sentFreq, sentCallsign,
                                                  sentLoc, sentComment, spotStatus, reason);

    sentSpotDataModel->rowData = sentSpotData;
    sentSpotDataModel->insertRows(sentSpotDataModel->rowCount(), 1);

}



QString ClusterMainWindow::assembleSpotForDXCluster(Frequency freq, QString call, QString loc)
{
#ifdef TEST_PLEASE_IGNORE
    bool testMsg = true;

#endif
    sentComment = QString("%1< >%2").arg(setupCluster->getUserLocator()).arg(loc);


#ifdef TEST_PLEASE_IGNORE
    if (testMsg)
    {
        sentComment = sentComment + " Test, Pls ignore";
    }
#endif

    qint64 f = freq;
    f = f / 1000;

    sentCallsign = call;
    sentFreq = QString::number(f);
    sentLoc = loc;


    QString spotmsg = QString("DX %1 %2").arg(call).arg(QString::number(f));
    if (!loc.isEmpty())
    {
        spotmsg = QString("%1 %2").arg(spotmsg).arg(sentComment);
    }

    return spotmsg + QChar('\n');
}

void ClusterMainWindow::closeEvent(QCloseEvent *event)
{


    if (nodeConnected)
    {
        if (setupCluster->getRunEndFileFlag())
        {
            handleEndFile();          // send user commands
        }

    }



    LogTimer.stop();


    // and tidy up all loose ends

    QSettings settings;
    settings.setValue(geoStr, saveGeometry());

    disconnectNode();

    trace("Minos Cluster Server Closing");
    QWidget::closeEvent(event);
}


void ClusterMainWindow::onStdInRead(QString cmd)
{
    bool doClose = false;
    if (cmd.indexOf("Shutdown", 0, Qt::CaseInsensitive) >= 0)
    {
//        closeApp = true;
        doClose = true;
    }
    executeStdIn(cmd);
    if (doClose)
        close();

}


void ClusterMainWindow::getStartCommands()
{
    startCommands.clear();

    QString fileName = CLUSTER_PATH + CLUSTER_COMMANDS;
    QSettings settings(fileName, QSettings::IniFormat);

    settings.beginGroup("StartCommandStrings");
    QStringList commandkeys = settings.allKeys();
    settings.endGroup();

    for (int i = 0; i < commandkeys.count(); i++)
    {
        startCommands.append(settings.value(commandkeys[i]).toString());
    }
}



/**************************** User Command Buttons **************************/


void ClusterMainWindow::initUserCommandButtons()
{

    ui->userCmdButFrame->setVisible(true);
    QList<QToolButton*> ui_userCommandButtons;
    ui_userCommandButtons << ui->sendButton0 << ui->sendButton1 << ui->sendButton2 << ui->sendButton3 << ui->sendButton4
                     << ui->sendButton5 << ui->sendButton6 << ui->sendButton7 << ui->sendButton8 << ui->sendButton9;


    for (int i = 0; i < userCommandShortCutKeys.count(); i++)
    {
        shortCutKeyList.append(new QShortcut(QKeySequence(userCommandShortCutKeys[i]), this));
    }

    for (int i = 0; i < userCommandMenuShortCutKeys.count(); i++)
    {
        shiftShortCutKeyList.append(new QShortcut(QKeySequence(userCommandMenuShortCutKeys[i]), this));
    }



    QStringList buttonLabels;
    for (unsigned int i = 0; i < sizeof(userCmdButtonLabels)/sizeof(const char *); i++)
    {
        buttonLabels.append(tr(userCmdButtonLabels[i]));
    }
    for (int i = 0; i < ui_userCommandButtons.count(); i++)
    {

        userCmdButton.append(new PresetButton(ui_userCommandButtons[i], i, shortCutKeyList[i], shiftShortCutKeyList[i], buttonLabels));

        connect(userCmdButton[i], &PresetButton::presetShortCutRecall, [this, i]() {userCmdButtonRead(i);});
        connect(userCmdButton[i], &PresetButton::presetShiftShortCutRecall, [this, i]() {showUserCmdButtonMenu(i);});
        connect(userCmdButton[i], &PresetButton::presetReadAction, [this, i]() {userCmdButtonRead(i);});
        connect(userCmdButton[i], &PresetButton::presetEditAction, [this, i]() {userCmdButtonEdit(i);});
        connect(userCmdButton[i], &PresetButton::presetWriteAction, [this, i]() {userCmdButtonWrite(i);});
        connect(userCmdButton[i], &PresetButton::presetClearAction, [this, i]() {userCmdButtonClear(i);});


    }

}

void ClusterMainWindow::showUserCmdButtonMenu(int buttonNumber)
{
    userCmdButton[buttonNumber]->showButtonMenu();
}


void ClusterMainWindow::userCmdButtonRead(int buttonNumber)
{
    trace(QString("UserCmdButton Button Read = %1").arg(buttonNumber + 1));
    if (!userCommands[buttonNumber].isEmpty() && buttonNumber < userCmdButton.count())
    {
        if (userCommands[buttonNumber].contains(':'))
        {
            QStringList d = userCommands[buttonNumber].split(':');
            if (d.count() == 2)
            {
                if (!d[1].isEmpty())
                {
                    if (nodeConnected)
                    {
                        d[1].append('\n');
                        trace(QString("UserCmdButton Read - Send Command to cluster = %1").arg(d[1]));

                        if (setupCluster->getRunEndFileFlag())
                        {
                            if (d[1].contains("bye", Qt::CaseInsensitive))
                            {
                                handleEndFile();
                            }
                        }

                        txText(d[1]);


                    }
                }
            }

        }
    }

}

void ClusterMainWindow::userCmdButtonEdit(int buttonNumber)
{
    trace(QString("UserCmdButton Edit Selected = %1").arg(QString::number(buttonNumber + 1)));
    if (!userCommands[buttonNumber].isEmpty() && buttonNumber < userCmdButton.count())
    {

        if (userCommands[buttonNumber].contains(':'))
        {
            QStringList d = userCommands[buttonNumber].split(':');
            if (d.count() == 2)
            {
                ClusterUserCommandData editData(d[0], d[1]);
                ClusterUserCommandData curData(d[0], d[1]);

                trace(QString("UserCmdButton - Edit Data - name = %1, cmdString = %2").arg(d[0]).arg(d[1]));
                userClusterCommandDialog cmdStringDialog(this, buttonNumber, &editData, &curData, QString("Edit"));


                if (cmdStringDialog.exec() == QDialog::Accepted)
                {
                    if (editData.name != curData.name || editData.cmdString != curData.cmdString)
                    {
                        trace(QString("UserCmdButton - Saving Edited Data - name = %1, cmdString = %2").arg(editData.name).arg(editData.cmdString));
                        saveUserCommandString(buttonNumber, editData);
                        userCommandButtonUpdate(buttonNumber, editData);
                    }

                }
            }
        }
    }
}

void ClusterMainWindow::userCmdButtonClear(int buttonNumber)
{
    trace(QString("UserCommand Clear Selected = %1").arg(QString::number(buttonNumber +1)));

    if (!userCommands[buttonNumber].isEmpty() || (!userCmdButton.isEmpty()  && buttonNumber < userCmdButton.count()))
    {
        int status = QMessageBox::question( this,
                                tr("Cluster User Command Clear"),
                                tr("Do you really want to clear cluster user command number:%1?")
                                .arg(buttonNumber + 1),
                                QMessageBox::Yes|QMessageBox::Default,
                                QMessageBox::No|QMessageBox::Escape,
                                QMessageBox::NoButton);

        if (status == QMessageBox::Yes)
        {
             trace(QString("UserCommand Clear - Clearing Button = %1").arg(QString::number(buttonNumber +1)));
            ClusterUserCommandData pData("", "");
            saveUserCommandString(buttonNumber, pData);
            userCommandButtonUpdate(buttonNumber, pData);
        }


    }


}



void ClusterMainWindow::userCmdButtonWrite(int buttonNumber)
{
    trace(QString("UserCommand New Selected = %1").arg(QString::number(buttonNumber +1)));
    if (!userCmdButton.isEmpty()  && buttonNumber < userCmdButton.count())
    {

        ClusterUserCommandData editData("", "");
        ClusterUserCommandData curData("", "");
        userClusterCommandDialog cmdStringDialog(this, buttonNumber, &editData, &curData, QString("New"));


        if (cmdStringDialog.exec() == QDialog::Accepted)
        {
            if (editData.name != curData.name || editData.cmdString != curData.cmdString)
            {
                trace(QString("UserCommand New Selected - Saving new data name = %1, cmdString = %2").arg(editData.name).arg(editData.cmdString));
                saveUserCommandString(buttonNumber, editData);
                userCommandButtonUpdate(buttonNumber, editData);
            }

        }

    }
}



void ClusterMainWindow::userCommandButtonUpdate(int buttonNumber, ClusterUserCommandData& buttonData)
{
    userCmdButton[buttonNumber]->setText(QString("%1: %2").arg(QString::number(buttonNumber + 1)).arg(buttonData.name) );
    // update store
    userCommands[buttonNumber] = buttonData.name + ":" + buttonData.cmdString;
    //QString tTipStr = "Bearing = " + editData.bearing;
    //presetButton[buttonNumber]->presetButton->setToolTip(tTipStr);
}


void ClusterMainWindow::userCommandAllButtonUpdate()
{
    ClusterUserCommandData buttonData;
    QStringList cmdData;
    if (userCommands.count() > 0)
    {
        for (int i = 0; i < userCommands.count(); i++)
        {
            cmdData = userCommands[i].split(':');
            if (cmdData.count() == 2)
            {
               buttonData.name = cmdData[0];
               buttonData.cmdString = cmdData[1];
               userCommandButtonUpdate(i, buttonData);
            }

        }
    }
}



void ClusterMainWindow::readUserCommandStrings()
{
    QSettings config(CLUSTER_PATH + CLUSTER_COMMANDS, QSettings::IniFormat);
    config.beginGroup("UserCommandStrings");
    if (userCmdButton.count() > 0)
    {
        for (int i = 0; i < userCmdButton.count(); i++)
        {
            userCommands.append(config.value(QString("command%1").arg(QString::number(i+1)), "").toString());
        }
    }
    config.endGroup();
}


void ClusterMainWindow:: saveUserCommandString(int buttonNumber, ClusterUserCommandData& buttonData)
{

    QString cmd = buttonData.name + ":" + buttonData.cmdString;
    QSettings config(CLUSTER_PATH + CLUSTER_COMMANDS, QSettings::IniFormat);
    config.beginGroup("UserCommandStrings");
    config.setValue(QString("command%1").arg(QString::number(buttonNumber + 1)), QString(buttonData.name + ":" + buttonData.cmdString));
    config.endGroup();

}

void ClusterMainWindow::showStatusMessage(const QString &message, const QString &raw)
{
    rawStatus = raw;
    status->setText(message);
    trace(QString("showStatusMessage: %1").arg(message));

}

void ClusterMainWindow::startDisconnectTimer(int time)
{
    disconnectTimer->start(time);
}


void ClusterMainWindow::disconnectTimeout()
{

    trace(QString("Cluster Server - Disconnect Timeout"));
    //msgComplete = false;
    //retCode = -52;
    emit disconnectTimerfinished();
}


void ClusterMainWindow::handleStatusTimer()
{
    static QString oldStatusMsg;
    static int oldServerListCount = 0;


    if (oldServerListCount != clusterRpc->getServerListCount())
    {
        oldServerListCount = clusterRpc->getServerListCount();
        // send status to clients
        trace(QString("handleStatusTimer: Cluster Client Count Changed old = %1, new = %2 - Send Status to Cluster Clients - %3").arg(oldServerListCount).arg(clusterRpc->getServerListCount()).arg(status->text()));
    //    sendSpotsQueue.append(createStatusToSend(status->text()));
          clusterRpc->publishState(rawStatus, status->text());
          sendSpotToTxEnabled(setupCluster->getSendToDXClusterEnabled()); // wait for cluster client to open before sending this to qsologframe
    }

    // send status message if it has changed
    else if (!status->text().isEmpty() /* && clusterRpc->getServerListCount() > 0 */)
    {
        if (oldStatusMsg != status->text())
        {
            oldStatusMsg = status->text();

            // send status to clients
            trace(QString("handleStatusTimer: Send Status to Cluster Clients - %1").arg(status->text()));
            //sendSpotsQueue.append(createStatusToSend(rawStatus));
            clusterRpc->publishState(rawStatus, status->text());
        }
    }

}




void ClusterMainWindow::about()
{
    QMessageBox::about(this, tr("Minos Cluster Server"), tr("Minos Cluster\nCopyright D Balharrie G8FKH/M0DGB 2016 - 2019"));
}



/****************************** Test Routine *********************************/

#ifdef TEST_SPOTS


void ClusterMainWindow::testSpotPbClicked()
{
    testSpotList.clear();

    // get list of test spots from file
    if (FileExists(CLUSTER_PATH + CLUSTER_SPOT_TEST_FILE))
    {
        QFile inputFile(CLUSTER_PATH + CLUSTER_SPOT_TEST_FILE);
        if (inputFile.open(QIODevice::ReadOnly))
        {
           QTextStream in(&inputFile);
           while (!in.atEnd())
           {
              QString line = in.readLine().append('\n');
              testSpotList.append(line);
           }
           inputFile.close();
        }
    }
    spotNum = 0;
    loginSuccess = true;  // spoof login to allow parse of spots
    spotTestTimer->start(1000);

}

void ClusterMainWindow::spotTimerTimeOut()
{
    QString spot;
    QTime time;
    QString timeStr;
    if (spotNum >= testSpotList.count())
    {
        spotTestTimer->stop();
        return;
    }
    if (!testSpotList.isEmpty())
    {
        spot = testSpotList[spotNum].remove('\n');
        time = QDateTime::currentDateTimeUtc().time();
        QString hourStr;
        QString minStr;
        if (time.hour() < 10)
        {
            hourStr = QString("0%1").arg(time.hour());
        }
        else
        {
            hourStr = QString("%1").arg(time.hour());
        }
        if (time.minute() < 10)
        {
            minStr = QString("0%1").arg(time.minute());
        }
        else
        {
            minStr = QString("%1").arg(time.minute());
        }
        timeStr = QString("   %1%2Z").arg(hourStr).arg(minStr);
        spot = spot.append(timeStr).append('\n');
        parseDX(spot);
        spotNum++;

    }

}


#endif
