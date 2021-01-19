/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Cluster Server
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2020
//
//
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
    connect(ui->actionUser_Command_Shortcuts, SIGNAL(triggered()), this, SLOT(clusterNodeCommandsShortcutHelp()));

    //BandList::getBandList().loadVhfAndUpBands(bands);
    BandList::getBandList().loadAllBands(bands);

    filterSettings.initFilterSettings(bands);
    initFilterCheckBoxs();

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
        connect(spotTestTimer, SIGNAL(timeout()), this, SLOT(onSpotTestTimerTimeOut()));
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


    handleSpotsInQueues = new QTimer();
    connect(handleSpotsInQueues, SIGNAL(timeout()), this, SLOT(onHandleSpotsInQueues()));
    handleSpotsInQueues->start(SEND_SPOTS_DUR);


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



    dxSpotProxyModel = new DxSpotSortFilterProxyModel(filterSettings);
    dxSpotProxyModel->setSourceModel(dxSpotDataModel);
    dxSpotProxyModel->sort(RXTIME_COL_NUM, Qt::DescendingOrder);

    dxSpotView->setModel(dxSpotProxyModel);
    dxSpotView->setAlternatingRowColors(true);
    dxSpotView->setSelectionMode( QAbstractItemView::NoSelection );
    dxSpotView->setItemDelegate(dxSpotViewDelegate.data());

    QHeaderView *verticalHeader = dxSpotView->verticalHeader();
    verticalHeader->setVisible(false);
    verticalHeader->setDefaultSectionSize(10);
    verticalHeader->setMinimumSectionSize(10);

    //verticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);
    verticalHeader->setSectionResizeMode(QHeaderView::Interactive);


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
    dxSpotView->setColumnHidden(DATE_TIME_COL_NUM, true);



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

    //sentSpotVerticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);
    sentSpotVerticalHeader->setSectionResizeMode(QHeaderView::Interactive);

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

    readBandFilterSettings();
    loadBandFilterSettingsToTab();


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

    purgeTimer = new QTimer(this);
    connect (purgeTimer, SIGNAL(timeout()), this, SLOT(purgeSpots()));
    purgeTimer->start(PURGE_TIME);



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

    connect(ui->pushButton, SIGNAL(pressed()), this, SLOT(onpbpressed()));

    //initFilterCheckBoxs();

    setHF(false);

}


void ClusterMainWindow::onpbpressed()
{
    static bool state = false;
    if (!state)
    {
        state = true;
        setHF(state);
    }
    else
    {
        state = false;
        setHF(state);
    }
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
    QSharedPointer<ClusterSpotData> newSpot = QSharedPointer<ClusterSpotData>(new ClusterSpotData());

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
                        trace(QString("ParseDx - waiting for qrzInfo for askcallsign = %1, spot callsign = %2").arg(askQraData.getAskCallsign()).arg(spotWaitingForQraFromNode->getDxCallStr()));
                    }
                    else
                    {
                        // got all the data
                        trace(QString("ParseDx - got all the data from qrz for callsign = %1, spot callsign = %2").arg(qrzInfo.getCall()).arg(spotWaitingForQraFromNode->getDxCallStr()));

                        newSpot = spotWaitingForQraFromNode;

                        if (qrzInfo.getError())
                        {
                            trace(QString("ParseDx - qrzInfo error no data found for callsign = %1, lookup using prefix").arg(qrzInfo.getCall()));
                            // asking qrz via node didn't give qra, use prefix
                            // let's lookup using prefix
                            newSpot->setDxLocator(getQraFromCallsignPrefix(newSpot->getDxCall()));
                            newSpot->setDxLocatorIsFromNode(true);
                        }
                        else if (qrzInfo.getCall() == askQraData.getAskCallsign())
                        {

                            trace(QString("ParseDx - qrz info matches waiting callsign = %1").arg(askQraData.getAskCallsign()));


                            trace(QString("ParseDx - qrzInfo no error, add locator to spot = %1").arg(qrzInfo.getGrid()));

                            newSpot->setDxLocator(qrzInfo.getGrid());
                            newSpot->setDxLocatorIsFromNode(true);

                         }
                         else
                         {
                              trace(QString("ParseDx - QrzInfo call = %1, does not match waiting call %2").arg(qrzInfo.getCall()).arg(askQraData.getAskCallsign()));
                         }

                        //spotListNoQra.remove(qrzInfo.getCall());
                        spotWaitingForQraFromNode.clear();
                        qrzInfo.clear();
                        askQraData.clear();
                        //getQrzInfo = false;
                        retCode = SPOT_OK;

                        processNewSpot(newSpot);

                    }

                }

            }
       } while (!line.isNull());
    }

    trace(QString("ParseDx: Finished"));
}




void ClusterMainWindow::processNewSpot(const QSharedPointer<ClusterSpotData> newSpot)
{

    trace(QString("ProcessNewSpot: DX de %1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12 %13 %14 %15")
                        .arg(newSpot->getDxCallStr()).arg(newSpot->getFreq().traceStr()).arg(newSpot->getBand()).arg(newSpot->getBandMask()).arg(newSpot->getBandType()).arg(newSpot->getMode()).arg(newSpot->getModeMask())
                        .arg(newSpot->getSpotterCallStr()).arg(newSpot->getDxLocator()).arg(newSpot->getSpotterLocator()).arg(newSpot->getDxPropMode()).arg(newSpot->getSpotTime()).arg(newSpot->getSpotDate()).arg(newSpot->getSpotComment()).arg(setupCluster->getTimeToLive()));

    // is spot older than time to live time
    int timeToLive = setupCluster->getTimeToLive().toInt() * 60;
    if (timeToLive == 0 || (timeToLive > 0 && !spotTimedOut(newSpot->getRxTime(), timeToLive)))
    {
        trace(QString("ProcessNewSpot: Spot within timeToLive"));
        // does the spot have a dxLocator
        if (newSpot->getDxLocator().isEmpty() && (newSpot->getBandType() == "VHF" || newSpot->getBandType() == "MWAVE"))
        {
            // queue to ask Qrz for locator, only for VHF/UHF spots
            trace(QString("ProcessNewSpot: No DxCall locator, queue to ask qrz call = %1").arg(newSpot->getDxCallStr()));
            spotListNoQra.append(newSpot);
        }
        else
        {
            if (currentUserCallsign != newSpot->getSpotterCallStr())
            {
                // send spot to clients if spotter isn't this station
                trace(QString("ProcessNewSpot: Spotter not this station, pass to clients, callsign %1").arg(newSpot->getDxCallStr()));
                sendSpotsToClientQueue.append(createSpotToSend(assembleSpotMsgToSendToClients(newSpot, setupCluster->getTimeToLive())));

            }
            else
            {
                trace(QString("ProcessNewSpot: Spotter is this station, only display on server"));
            }
/*
            // is spot already in the display list?
            for (int i = 0; i < dxSpotDataModel->rowCount(); i++)
            {
                if (*dxSpotDataModel->getSpotData(i) == newSpot)
                {
                    trace(QString("Spot Call = %1, already in display, skip").arg(newSpot.getDxCall().realCall));
                    return;
                }
            }

*/

            trace(QString("ProcessNewSpot: Add spot for display callsign = %1, rxTime = %2").arg(newSpot->getDxCallStr()).arg(newSpot->getRxTime()));
            spotsList.append(QSharedPointer<ClusterSpotData>( new ClusterSpotData(newSpot)));

        }
    }
    else
    {
        trace(QString("ProcessNewSpot: Spot %1, older than time to live time = %2 mins").arg(newSpot->getDxCallStr()).arg(timeToLive/60));
    }


}


void ClusterMainWindow::handAskQraTimer()
{


    if (!spotListNoQra.isEmpty() && !askQraData.getAskQrz())
    {

           spotWaitingForQraFromNode = spotListNoQra.first();
           spotListNoQra.removeFirst();
           trace(QString("Get QRA for callsign %1").arg(spotWaitingForQraFromNode->getDxCallStr()));

           // use call to get QRA from node QRZ command
           askQraData.setAskCallsign(spotWaitingForQraFromNode->getDxCall().realCall);
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
        if (qrzInfo.getCall().isEmpty())
        {
            // some sites return the qrz.com, but not the data
            trace(QString("getQrzReply: end message, but no data for callsign = %1").arg(spotWaitingForQraFromNode->getDxCallStr()));
            qrzInfo.setError(true);
        }
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
    if (!syn)
    {
        return "";
    }
    return syn->getCentral().getLoc();
}





int ClusterMainWindow::upackShowDxSpot(const QString txt, QSharedPointer<ClusterSpotData> newSpot)
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
        newSpot->setFreq(f);
        QString dxBandStr;
        QString dxBandMask;
        getBand(bands, newSpot->getFreq().str(), dxBandStr, dxBandMask);
        newSpot->setBand(dxBandStr);
        newSpot->setBandMask(dxBandMask);

        newSpot->setBandType(BandList::getBandList().findType(newSpot->getBand()));


        if (newSpot->getBand() == "HF" && !enableHFSpots)
        {
            // discard spot as it is HF
            trace(QString("Unpack Show DX Spot: Discard Spot HF = %1").arg(newSpot->getFreq().traceStr()));
            return DISCARD_HF_SPOT * -1;
        }

        QString dxModeStr;
        QString dxModeMask;
        getMode(modeBandPlan, newSpot->getFreq().str(), dxBandStr, dxModeStr, dxModeMask);
        newSpot->setMode(dxModeStr);
        newSpot->setModeMask(dxModeMask);

        newSpot->setDxCall(dxMsg[1]);
        //newSpot.setSpotDate(dxMsg[2]);
        //newSpot.setSpotTime(dxMsg[3].remove('Z'));
        //newSpot.setSpotDateTime(getSpotDateTime(newSpot.getSpotDate(), newSpot.getSpotTime()));

        // get date/time
        QDate d;

        QStringList dl = dxMsg[2].split('-');

        if (dl.count() == 3)
        {
            d = QDate(dl[2].toInt(), dl[1].toInt(), dl[0].toInt());
        }

        QDateTime dt = getSpotDateTime(dxMsg[2], dxMsg[3].remove('Z'));

        if (! dt.isValid())
        {
           return SPOT_DATETIME_INVALID * -1;
        }

        newSpot->setSpotDateTime(dt);

        QString sptCall = newSpot->getSpotterCallStr();
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

        newSpot->setSpotComment(spotComment);

        qint64 rxTime = newSpot->getSpotDateTime().toMSecsSinceEpoch()/1000;
        newSpot->setRxTime(rxTime);


        QString spotLocator;
        QString dxLocator;
        findLocInComment(spotLocator, dxLocator, spotComment, dxBandMask.toInt());
        newSpot->setSpotterLocator(spotLocator);
        newSpot->setDxLocator(dxLocator);


        if (enableHFSpots && newSpot->getBandType() == "HF" && newSpot->getDxLocator().isEmpty())
        {
            // get locator based up prefix
            newSpot->setDxLocator(getQraFromCallsignPrefix(newSpot->getDxCall()));
            newSpot->setDxLocatorIsFromNode(true);
        }


        newSpot->setDxPropMode(getPropMode(spotComment));

        // look for mode in comments, if found overide freq mode
        int commentModeNum;
        QString commentMode;
        if (lookforModeInComment(spotComment, commentModeNum, commentMode))
        {
            newSpot->setMode(commentMode);
            newSpot->setModeMask(QString::number(commentModeNum));
        }

        return SPOT_OK;
    }

    return SPOT_TOO_MANY_SECTIONS * -1;

}


bool ClusterMainWindow::checkShowDxMsg(const QString txt, QSharedPointer<ClusterSpotData> newSpot)
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
            Callsign callsign;
            callsign.setFullCall(extractStr[i]);
            if (callsign.getValRes() == CS_OK)
            {
                newSpot->setSpotterCall(extractStr[i]);
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
                QString spot = createResendSpotToSend(assembleSpotMsgToSendToClients(dxSpotDataModel->getSpotData(row), setupCluster->getTimeToLive()));
                trace(QString("resending this spot - %1 to uuid = %2").arg(spot).arg(cmd.getuuid()));
                clusterRpc->sendDXSpot(spot, cmd.getuuid(), cmd.getFrameId());   // send spot and loggeruuid
            }

        }
    }

}




QString ClusterMainWindow::assembleSpotMsgToSendToClients(const QSharedPointer<ClusterSpotData> spotData, const QString timeToLive)
{
    QString spotMsg = QString("%1:%2:%3:%4:%5:%6:%7:%8:%9:%10:%11:%12:%13:%14:%15")
                       .arg(spotData->getDxCallStr())   // %1
                       .arg(spotData->getDxLocator())   // %2
                       .arg(spotData->getDxLocatorIsFromNode() ? "locFromNode-true" : "locFromNode-false")  // %3
                       .arg(spotData->getFreq().str())  // %4
                       .arg(spotData->getBand())        // %5
                       .arg(spotData->getBandMask())    // %6
                       .arg(spotData->getBandType())    // %7
                       .arg(spotData->getMode())        // %8
                       .arg(spotData->getModeMask())    // %9
                       .arg(spotData->getSpotterCallStr())  //%10
                       .arg(spotData->getSpotterLocator())  // %11
                       .arg(spotData->getSpotDateTime().toString("yyyyMMMddHHmmss"))  // %12
                       .arg(spotData->getSpotComment())  // %13
                       .arg(spotData->getDxPropMode())   // %14
                       .arg(timeToLive);        // %15

    return spotMsg;

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


void ClusterMainWindow::onHandleSpotsInQueues()
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


int ClusterMainWindow::upackDxSpot(QString txt, QSharedPointer<ClusterSpotData> newSpot)
{

    trace(QString("UnpackDXSpot - %1").arg(txt));
    int timePos = 0;

    txt.remove('\x07');

    dxMsg = txt.split(QRegularExpression("\\s+"));

    if (dxMsg.count() > 5)
    {
        newSpot->setSpotterCall(dxMsg[2].remove(':'));
        QString f = dxMsg[3] + "00";
        f.remove('.');

        //dxFreq = convertKhzToMhz(dxMsg[3]);
        newSpot->setFreq(f);


        QString dxBandStr;
        QString dxBandMask;
        getBand(bands, newSpot->getFreq().str(), dxBandStr, dxBandMask);
        newSpot->setBand(dxBandStr);
        newSpot->setBandMask(dxBandMask);

        newSpot->setBandType(BandList::getBandList().findType(newSpot->getBand()));


        if (newSpot->getBand() == "HF" && !enableHFSpots)
        {
            // discard spot as it is HF
            trace(QString("Unpack Show DX Spot: Discard Spot HF = %1").arg(newSpot->getFreq().traceStr()));
            return DISCARD_HF_SPOT * -1;
        }


        QString dxModeStr;
        QString dxModeMask;
        getMode(modeBandPlan, newSpot->getFreq().str(), newSpot->getBand(), dxModeStr, dxModeMask);
        newSpot->setMode(dxModeStr);
        newSpot->setModeMask(dxModeMask);

        newSpot->setDxCall(dxMsg[4]);

        // find time

        QString time;
        for (int i = 4; i < dxMsg.count(); i++)
        {
            QRegularExpression re("\\d\\d\\d\\dZ");
            QRegularExpressionMatch match = re.match(dxMsg[i]);
            if (match.hasMatch())
            {
                //newSpot.setSpotTime(dxMsg[i].remove('Z'));
                time = dxMsg[i].remove('Z');
                timePos = i;
                break;
            }
        }

        //if (newSpot.getSpotTime() == "")
        if (time == "")
        {
            //error
            return NO_SPOT_TIME * -1;
        }

        QDate d = QDate::currentDate();
        QString spotDate = d.toString("dd-MMM-yyyy");

        QDateTime dt = getSpotDateTime(spotDate, time);

        if (!dt.isValid())
        {
           return SPOT_DATETIME_INVALID * -1;
        }

        newSpot->setSpotDateTime(dt);

        qint64 rxTime = dt.toMSecsSinceEpoch()/1000;
        newSpot->setRxTime(rxTime);



        // look for locator
        if (timePos + 1 >= dxMsg.count())  // make sure not out of range
        {
            // no spotlocator sent
            newSpot->setSpotterLocator("");
        }
        else
        {
            newSpot->setSpotterLocator(dxMsg[timePos + 1]);
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

        newSpot->setSpotComment(spotComment);

        // remove seperator char from comment
        spotComment.remove(SPOT_DATA_SEPERATOR);
        QString spotLocator;
        QString dxLocator;
        findLocInComment(spotLocator, dxLocator, spotComment, dxBandMask.toInt());
        newSpot->setSpotterLocator(spotLocator);
        newSpot->setDxLocator(dxLocator);


        if (enableHFSpots && newSpot->getBandType() == "HF" && newSpot->getDxLocator().isEmpty())
        {
            // get locator based up prefix
            newSpot->setDxLocator(getQraFromCallsignPrefix(newSpot->getDxCall()));
            newSpot->setDxLocatorIsFromNode(true);
        }

        newSpot->setDxPropMode(getPropMode(spotComment));

        // look for mode in comments, if found overide freq mode
        int commentModeNum;
        QString commentMode;
        if (lookforModeInComment(spotComment, commentModeNum, commentMode))
        {
            newSpot->setMode(commentMode);
            newSpot->setModeMask(QString::number(commentModeNum));
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

void ClusterMainWindow::findLocInComment(QString &spotLoc, QString &dxLoc, const QString &comment, int bandmask)
{
    QStringList loc;
    trace(QString("Extract locators - comment = %1").arg(comment));
    // this should hopefully cope with different scenarios, independent of seperation chars
    // it is dependent on the sender correctly ordering the spotLoc and dxLoc, the spotLoc should be first

    QRegularExpression full_loc_exp;
    QRegularExpression part_loc_exp;

    if (!enableHFSpots || (enableHFSpots && bandmask > END_HF))
    {
        full_loc_exp = FULL_LOC_EXP;
        part_loc_exp = PART_LOC_EXP;

    }
    else
    {
        full_loc_exp = FULL_LOC_EXP_HF;
        part_loc_exp = PART_LOC_EXP_HF;
    }


    int fullLocExpCount = comment.count(full_loc_exp);
    int partLocExpCount = comment.count(part_loc_exp);

    if (fullLocExpCount == 2)
    {
        int firstIndex = comment.indexOf(full_loc_exp);
        int secondIndex = comment.indexOf(full_loc_exp, firstIndex + 4);
        // extract locators
        spotLoc = comment.mid(firstIndex, 6).toUpper();
        dxLoc = comment.mid(secondIndex, 6).toUpper();
    }
    else if (partLocExpCount == 2)
    {
        int firstIndex = comment.indexOf(part_loc_exp);
        int secondIndex = comment.indexOf(part_loc_exp, firstIndex + 4);
        // extract locators
        spotLoc = comment.mid(firstIndex, 4).toUpper();
        dxLoc = comment.mid(secondIndex, 4).toUpper();
    }
    else if (fullLocExpCount == 1 && partLocExpCount == 1)
    {
        int firstIndex = comment.indexOf(full_loc_exp);
        int secondIndex = comment.indexOf(part_loc_exp);
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
        int firstIndex = comment.indexOf(full_loc_exp);
        dxLoc = comment.mid(firstIndex, 6).toUpper();
    }
    else if (partLocExpCount == 1 && fullLocExpCount == 0)
    {
        int firstIndex = comment.indexOf(part_loc_exp);
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

    if (setupCluster->getBandFilterOnSaveFlag())
    {
        saveBandFilterSettings();
    }


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

/*
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
*/


/**************************** User Command Buttons **************************/


void ClusterMainWindow::initUserCommandButtons()
{


    QList<QToolButton*> ui_userVHFUHFCommandButtons;
    ui_userVHFUHFCommandButtons << ui->vhfUhfSendButton0 << ui->vhfUhfSendButton1 << ui->vhfUhfSendButton2 << ui->vhfUhfSendButton3 << ui->vhfUhfSendButton4
                     << ui->vhfUhfSendButton5 << ui->vhfUhfSendButton6 << ui->vhfUhfSendButton7 << ui->vhfUhfSendButton8 << ui->vhfUhfSendButton9;

    QList<QToolButton*> ui_userHFCommandButtons;
    ui_userHFCommandButtons << ui->hfSendButton0 << ui->hfSendButton1 << ui->hfSendButton2 << ui->hfSendButton3 << ui->hfSendButton4 << ui->hfSendButton5
                                << ui->hfSendButton6 << ui->hfSendButton7 << ui->hfSendButton8 << ui->hfSendButton9;

    for (int i = 0; i < userVHFUHFCommandShortCutKeys.count(); i++)
    {
        vhfUhfCommandShortCutKeyList.append(new QShortcut(QKeySequence(userVHFUHFCommandShortCutKeys[i]), this));
    }

    for (int i = 0; i < userVHFUHFCommandMenuShortCutKeys.count(); i++)
    {
        vhfUhfMenuShortCutKeyList.append(new QShortcut(QKeySequence(userVHFUHFCommandMenuShortCutKeys[i]), this));
    }

    for (int i = 0; i < userHFCommandShortCutKeys.count(); i++)
    {
        hfCommandShortCutKeyList.append(new QShortcut(QKeySequence(userHFCommandShortCutKeys[i]), this));
    }

    for (int i = 0; i < userHFCommandMenuShortCutKeys.count(); i++)
    {
        hfMenuShortCutKeyList.append(new QShortcut(QKeySequence(userHFCommandMenuShortCutKeys[i]), this));
    }


    QStringList buttonLabels;
    for (unsigned int i = 0; i < sizeof(userCmdButtonLabels)/sizeof(const char *); i++)
    {
        buttonLabels.append(tr(userCmdButtonLabels[i]));
    }

    for (int i = 0; i < ui_userVHFUHFCommandButtons.count(); i++)
    {

        userVHFUHFCmdButton.append(new PresetButton(ui_userVHFUHFCommandButtons[i], i, vhfUhfCommandShortCutKeyList[i], vhfUhfMenuShortCutKeyList[i], buttonLabels));

        connect(userVHFUHFCmdButton[i], &PresetButton::presetShortCutRecall, [this, i]() {userVhfUhfCmdButtonRead(i);});
        connect(userVHFUHFCmdButton[i], &PresetButton::presetShiftShortCutRecall, [this, i]() {showVhfUhfUserCmdButtonMenu(i);});
        connect(userVHFUHFCmdButton[i], &PresetButton::presetReadAction, [this, i]() {userVhfUhfCmdButtonRead(i);});
        connect(userVHFUHFCmdButton[i], &PresetButton::presetEditAction, [this, i]() {userVhfUhfCmdButtonEdit(i);});
        connect(userVHFUHFCmdButton[i], &PresetButton::presetWriteAction, [this, i]() {userVhfUhfCmdButtonWrite(i);});
        connect(userVHFUHFCmdButton[i], &PresetButton::presetClearAction, [this, i]() {userVhfUhfCmdButtonClear(i);});



    }


    for (int i = 0; i < ui_userHFCommandButtons.count(); i++)
    {

        userHFCmdButton.append(new PresetButton(ui_userHFCommandButtons[i], i, hfCommandShortCutKeyList[i], hfMenuShortCutKeyList[i], buttonLabels));

        connect(userHFCmdButton[i], &PresetButton::presetShortCutRecall, [this, i]() {userHfCmdButtonRead(i);});
        connect(userHFCmdButton[i], &PresetButton::presetShiftShortCutRecall, [this, i]() {showHfUserCmdButtonMenu(i);});
        connect(userHFCmdButton[i], &PresetButton::presetReadAction, [this, i]() {userHfCmdButtonRead(i);});
        connect(userHFCmdButton[i], &PresetButton::presetEditAction, [this, i]() {userHfCmdButtonEdit(i);});
        connect(userHFCmdButton[i], &PresetButton::presetWriteAction, [this, i]() {userHfCmdButtonWrite(i);});
        connect(userHFCmdButton[i], &PresetButton::presetClearAction, [this, i]() {userHfCmdButtonClear(i);});


    }


}

void ClusterMainWindow::showVhfUhfUserCmdButtonMenu(int buttonNumber)
{
    if ((enableHFSpots && ui->clusterTab->currentIndex() == 1) || (!enableHFSpots && ui->clusterTab->currentIndex() == 0))
    {
       userVHFUHFCmdButton[buttonNumber]->showButtonMenu();
    }
}

void ClusterMainWindow::showHfUserCmdButtonMenu(int buttonNumber)
{
    if (enableHFSpots && ui->clusterTab->currentIndex() == 0)
    {
        userHFCmdButton[buttonNumber]->showButtonMenu();
    }
}

void ClusterMainWindow::userVhfUhfCmdButtonRead(int buttonNumber)
{
    if ((enableHFSpots && ui->clusterTab->currentIndex() == 1) || (!enableHFSpots && ui->clusterTab->currentIndex() == 0))
    {
        userCmdButtonRead(vhfUhfUserCommands, "VHF/UHF", buttonNumber);
    }
}

void ClusterMainWindow::userHfCmdButtonRead(int buttonNumber)
{
    if (enableHFSpots && ui->clusterTab->currentIndex() == 0)
    {
        userCmdButtonRead(hfUserCommands, "HF", buttonNumber);
    }
}


void ClusterMainWindow::userCmdButtonRead(QStringList userCommands, QString tabSelected, int buttonNumber)
{

    trace(QString("UserCmdButton %1 Button Read = %2").arg(tabSelected).arg(buttonNumber + 1));
    if (!userCommands[buttonNumber].isEmpty() && buttonNumber < userVHFUHFCmdButton.count())
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
                        trace(QString("UserCmdButton %1 Read - Send Command to cluster = %2").arg(tabSelected).arg(d[1]));

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

void ClusterMainWindow::userVhfUhfCmdButtonEdit(int buttonNumber)
{
    if ((enableHFSpots && ui->clusterTab->currentIndex() == 1) || (!enableHFSpots && ui->clusterTab->currentIndex() == 0))
    {
        userCmdButtonEdit(vhfUhfUserCommands, "VHF/UHF", buttonNumber);
    }
}


void ClusterMainWindow::userHfCmdButtonEdit(int buttonNumber)
{
    if (enableHFSpots && ui->clusterTab->currentIndex() == HF_TABNUM)
    {
        userCmdButtonEdit(hfUserCommands, "HF", buttonNumber);
    }
}


void ClusterMainWindow::userCmdButtonEdit(QStringList userCommands, QString tabSelected, int buttonNumber)
{


    trace(QString("UserCmdButton %1 Edit Selected = %2").arg(tabSelected).arg(QString::number(buttonNumber + 1)));
    if (!userCommands[buttonNumber].isEmpty() && buttonNumber < userVHFUHFCmdButton.count())
    {

        if (userCommands[buttonNumber].contains(':'))
        {
            QStringList d = userCommands[buttonNumber].split(':');
            if (d.count() == 2)
            {
                ClusterUserCommandData editData(d[0], d[1]);
                ClusterUserCommandData curData(d[0], d[1]);

                trace(QString("UserCmdButton %1 - Edit Data - name = %2, cmdString = %3").arg(tabSelected).arg(d[0]).arg(d[1]));
                userClusterCommandDialog cmdStringDialog(this, tabSelected, buttonNumber, &editData, &curData, QString("Edit"));


                if (cmdStringDialog.exec() == QDialog::Accepted)
                {
                    if (editData.name != curData.name || editData.cmdString != curData.cmdString)
                    {
                        trace(QString("UserCmdButton - Saving Edited Data - name = %1, cmdString = %2").arg(editData.name).arg(editData.cmdString));
                        saveUserCommandString(tabSelected, buttonNumber, editData);
                        userCommandButtonUpdate(tabSelected, buttonNumber, editData);
                    }

                }
            }
        }
    }
}


void ClusterMainWindow::userVhfUhfCmdButtonClear(int buttonNumber)
{
    if ((enableHFSpots && ui->clusterTab->currentIndex() == 1) || (!enableHFSpots && ui->clusterTab->currentIndex() == 0))
    {
        userCmdButtonClear(vhfUhfUserCommands, "VHF/UHF", buttonNumber);
    }
}


void ClusterMainWindow::userHfCmdButtonClear(int buttonNumber)
{
    if (enableHFSpots && ui->clusterTab->currentIndex() == HF_TABNUM)
    {
        userCmdButtonClear(hfUserCommands, "HF", buttonNumber);
    }
}

void ClusterMainWindow::userCmdButtonClear(QStringList userCommands, QString tabSelected, int buttonNumber)
{
    trace(QString("UserCommand Clear Selected = %1").arg(QString::number(buttonNumber +1)));

    if (!userCommands[buttonNumber].isEmpty() || (!userVHFUHFCmdButton.isEmpty()  && buttonNumber < userVHFUHFCmdButton.count()))
    {
        int status = QMessageBox::question( this,
                                tr("Cluster %1 User Command Clear").arg(tabSelected),
                                tr("Do you really want to clear cluster %1 user command number:%2?")
                                .arg(tabSelected).arg(buttonNumber + 1),
                                QMessageBox::Yes|QMessageBox::Default,
                                QMessageBox::No|QMessageBox::Escape,
                                QMessageBox::NoButton);

        if (status == QMessageBox::Yes)
        {
             trace(QString("UserCommand Clear - Clearing Button = %1").arg(QString::number(buttonNumber +1)));
            ClusterUserCommandData pData("", "");
            saveUserCommandString(tabSelected, buttonNumber, pData);
            userCommandButtonUpdate(tabSelected, buttonNumber, pData);
        }


    }


}


void ClusterMainWindow::userVhfUhfCmdButtonWrite(int buttonNumber)
{
    if ((enableHFSpots && ui->clusterTab->currentIndex() == 1) || (!enableHFSpots && ui->clusterTab->currentIndex() == 0))
    {
        userCmdButtonWrite("VHF/UHF", buttonNumber);
    }
}

void ClusterMainWindow::userHfCmdButtonWrite(int buttonNumber)
{
    if (enableHFSpots && ui->clusterTab->currentIndex() == HF_TABNUM)
    {
        userCmdButtonWrite("HF", buttonNumber);
    }
}



void ClusterMainWindow::userCmdButtonWrite(QString tabSelected, int buttonNumber)
{
    trace(QString("UserCommand %1 New Selected = %2").arg(tabSelected).arg(QString::number(buttonNumber +1)));
    if (!userVHFUHFCmdButton.isEmpty()  && buttonNumber < userVHFUHFCmdButton.count())
    {

        ClusterUserCommandData editData("", "");
        ClusterUserCommandData curData("", "");
        userClusterCommandDialog cmdStringDialog(this, tabSelected, buttonNumber, &editData, &curData, QString("New"));


        if (cmdStringDialog.exec() == QDialog::Accepted)
        {
            if (editData.name != curData.name || editData.cmdString != curData.cmdString)
            {
                trace(QString("%1 UserCommand New Selected - Saving new data name = %2, cmdString = %3").arg(tabSelected).arg(editData.name).arg(editData.cmdString));
                saveUserCommandString(tabSelected, buttonNumber, editData);
                userCommandButtonUpdate(tabSelected, buttonNumber, editData);
            }

        }

    }
}



void ClusterMainWindow::userCommandButtonUpdate(QString tabSelected, int buttonNumber, ClusterUserCommandData& buttonData)
{
    if (tabSelected == "VHF/UHF")
    {
        userVHFUHFCmdButton[buttonNumber]->setText(QString("%1: %2").arg(QString::number(buttonNumber + 1)).arg(buttonData.name) );
        // update store
        vhfUhfUserCommands[buttonNumber] = buttonData.name + ":" + buttonData.cmdString;
    }
    else if (tabSelected == "HF")
    {
        userHFCmdButton[buttonNumber]->setText(QString("%1: %2").arg(QString::number(buttonNumber + 1)).arg(buttonData.name) );
        // update store
        hfUserCommands[buttonNumber] = buttonData.name + ":" + buttonData.cmdString;
    }


}


void ClusterMainWindow::userCommandAllButtonUpdate()
{
    ClusterUserCommandData buttonData;
    QStringList cmdData;

    if (vhfUhfUserCommands.count() > 0)
    {
        for (int i = 0; i < vhfUhfUserCommands.count(); i++)
        {
            cmdData = vhfUhfUserCommands[i].split(':');
            if (cmdData.count() == 2)
            {
               buttonData.name = cmdData[0];
               buttonData.cmdString = cmdData[1];
               userCommandButtonUpdate("VHF/UHF", i, buttonData);
            }

        }
    }

    if (hfUserCommands.count() > 0)
    {
        for (int i = 0; i < hfUserCommands.count(); i++)
        {
            cmdData = hfUserCommands[i].split(':');
            if (cmdData.count() == 2)
            {
               buttonData.name = cmdData[0];
               buttonData.cmdString = cmdData[1];
               userCommandButtonUpdate("HF", i, buttonData);
            }

        }
    }
}



void ClusterMainWindow::readUserCommandStrings()
{
    QSettings config(CLUSTER_PATH + CLUSTER_COMMANDS, QSettings::IniFormat);

    QStringList keys = config.childGroups();
    if (keys.contains("UserCommandStrings"))
    {
        updateToNewVhfUhfGroupKey();        // legacy before HF support
    }
    else
    {
        config.beginGroup("VHF_UHF_UserCommandStrings");
    }

    if (userVHFUHFCmdButton.count() > 0)
    {
        for (int i = 0; i < userVHFUHFCmdButton.count(); i++)
        {
            vhfUhfUserCommands.append(config.value(QString("command%1").arg(QString::number(i+1)), "").toString());
        }
    }
    config.endGroup();

    config.beginGroup("HF_UserCommandStrings");
    if (userHFCmdButton.count() > 0)
    {
        for (int i = 0; i < userHFCmdButton.count(); i++)
        {
            hfUserCommands.append(config.value(QString("command%1").arg(QString::number(i+1)), "").toString());
        }
    }
    config.endGroup();
}

// to copy and remove legacy entry

void ClusterMainWindow::updateToNewVhfUhfGroupKey()
{
    QSettings config(CLUSTER_PATH + CLUSTER_COMMANDS, QSettings::IniFormat);
    config.beginGroup("UserCommandStrings");
    if (userVHFUHFCmdButton.count() > 0)
    {
        for (int i = 0; i < userVHFUHFCmdButton.count(); i++)
        {
            vhfUhfUserCommands.append(config.value(QString("command%1").arg(QString::number(i+1)), "").toString());
        }
    }

    config.endGroup();


    for (int i = 0; i < userVHFUHFCmdButton.count(); i++)
    {
        ClusterUserCommandData buttonData;
        QStringList cl = vhfUhfUserCommands[i].split(':');

        if (cl.count() == 2)
        {
            buttonData.name = cl[0];
            buttonData.cmdString = cl[0];
        }

        saveUserCommandString("VHFUHF", i, buttonData);

    }

    config.remove("UserCommandStrings");



}


void ClusterMainWindow:: saveUserCommandString(QString tabSelected, int buttonNumber, ClusterUserCommandData& buttonData)
{

    QString cmd = buttonData.name + ":" + buttonData.cmdString;
    QSettings config(CLUSTER_PATH + CLUSTER_COMMANDS, QSettings::IniFormat);
    if (tabSelected == "VHF/UHF")
    {
       config.beginGroup("VHF_UHF_UserCommandStrings");
    }
    else if (tabSelected == "HF")
    {
       config.beginGroup("HF_UserCommandStrings");
    }
    else
    {
        return;
    }

    config.setValue(QString("command%1").arg(QString::number(buttonNumber + 1)), QString(buttonData.name + ":" + buttonData.cmdString));
    config.endGroup();

}


void ClusterMainWindow::initFilterCheckBoxs()
{


    QList<QCheckBox*> bandChkBoxList;

    bandChkBoxList << ui->_1_8MHzCheckBox << ui->_3_5MHzCheckBox  << ui->_7MHzCheckBox
                   << ui->_14MHzCheckBox << ui->_21MHzCheckBox << ui->_28MHzCheckBox
                   << ui->_50MHzCheckBox << ui->_70MHzCheckBox << ui->_144MHzCheckBox << ui->_432MHzCheckBox
                   << ui->_1296MHzCheckBox << ui->_2300MHzCheckBox << ui->_3_4GHzCheckBox << ui->_5_6GHzCheckBox << ui->_10GHzCheckBox;

    ClusterClientBandFilterDialogDetails ccfd;
    for (int i = 0; i <bands.count(); i++)
    {
        ccfd.bandChkBox = bandChkBoxList[i];
        ccfd.bandType = bands[i].data()->getType();
        bandCheckBoxes.insert(bands[i].data()->uk, ccfd);
    }


    for (int i = 0; i < bandChkBoxList.count(); i++)
    {
        connect(bandChkBoxList[i], &QCheckBox::stateChanged, this, [=](int state) {onbandCheckBoxStateChanged(i, state);});

    }



    connect(ui->hfSelectBandPb, &QPushButton::pressed, this, [=]() {onHfSelectBandPbPressed();});
    connect(ui->vhfSelectBandPb, &QPushButton::pressed, this, [=]() {onVhfSelectBandPbPressed();});
    connect(ui->uhfSelectBandPb, &QPushButton::pressed, this, [=]() {onUhfSelectBandPbPressed();});



}

void ClusterMainWindow::loadBandFilterSettingsToTab()
{
    for (auto const &b:bands)
    {
        QString band = b.data()->uk;
        bandCheckBoxes.value(band).bandChkBox->setChecked(filterSettings.getBandFilter(band));
    }
}


void ClusterMainWindow::saveBandFilterSettings()
{
    QSettings config(CLUSTER_COMMANDS, QSettings::IniFormat);
    config.beginGroup("BandFilter");

    for (auto const &b:bands)
    {
        QString band = b.data()->uk;
        config.setValue(QString("bandFilter_+%1").arg(band), filterSettings.getBandFilter(band));
    }

    config.endGroup();


}

void ClusterMainWindow::readBandFilterSettings()
{
    QSettings config(CLUSTER_COMMANDS, QSettings::IniFormat);
    config.beginGroup("BandFilter");
    for (auto const &b:bands)
    {
        QString band = b.data()->uk;
        filterSettings.setBandFilter(band, config.value(QString("bandFilter_+%1").arg(band), true).toBool());

    }

    config.endGroup();
}


void ClusterMainWindow::setHF(bool hfFlag)
{
    QString hfTabName = "HF User Commands";

    if (hfFlag)
    {
       // set hf Tab "visible"
       if (ui->clusterTab->tabText(0) != hfTabName)
       {
           QWidget *hfTab = ui->clusterTab->findChild<QWidget *>(hfTabName);
           if (hfTab)
           {
               ui->clusterTab->insertTab(0, hfTab, hfTabName);
           }
       }
       // ensure hf Settings are correct
       readBandFilterSettings();
       loadBandFilterSettingsToTab();
   }
    else
    {
        // set hf tab "invisible"
        QString n = ui->clusterTab->tabText(0);
        if (ui->clusterTab->tabText(0) == hfTabName)
        {
            ui->clusterTab->removeTab(0);
        }

        // clear the HF Bandfilters
        for (auto const &b:bands)
        {
            if (b->getType() == HF_BANDTYPE)
            {
                QString band = b.data()->uk;
                bandCheckBoxes.value(band).bandChkBox->setChecked(false);
                filterSettings.setBandFilter(band, false);
            }


        }
    }

    setHfFilterControlsVisible(hfFlag);

}


void ClusterMainWindow::setHfFilterControlsVisible(bool visible)
{

    for(auto const &b: bands)
    {
        if (b->getType() == HF_BANDTYPE)
        {
            QString band = b.data()->uk;
            bandCheckBoxes.value(band).bandChkBox->setVisible(visible);
        }

    }

    ui->hfSelectBandPb->setVisible(visible);

}

void ClusterMainWindow::onbandCheckBoxStateChanged(int i, int state)
{
    Q_UNUSED(i)
    Q_UNUSED(state)
    bool changed = false;
    for (auto const &b: bands)
    {
        QString band = b.data()->uk;

        if (bandCheckBoxes.value(band).bandChkBox->isChecked() != filterSettings.getBandFilter(band))
        {
            filterSettings.setBandFilter(band, bandCheckBoxes.value(band).bandChkBox->isChecked());
            changed = true;
        }

        if (changed)
        {
           updateDisplay();
        }


    }

}


void ClusterMainWindow::onHfSelectBandPbPressed()
{
    static bool selectButtonState = false;

    if (!selectButtonState)
    {
        selectButtonState = true;
        setAllHFBandsFilter(selectButtonState);
    }
    else
    {
        selectButtonState = false;
        setAllHFBandsFilter(selectButtonState);
    }
}


void ClusterMainWindow::setAllHFBandsFilter(bool state)
{
    for (auto const &b:bands)
    {
        if (b->getType() == HF_BANDTYPE)
        {
            setBandsCheckBoxAndFilterFlag(b.data()->uk, state);
        }
    }

     updateDisplay();
}


void ClusterMainWindow::setBandsCheckBoxAndFilterFlag(const QString band, const bool state)
{
    filterSettings.setBandFilter(band, state);
    bandCheckBoxes.value(band).bandChkBox->setChecked(state);
}

void ClusterMainWindow::onVhfSelectBandPbPressed()
{
    static bool selectButtonState = false;

    if (!selectButtonState)
    {
        selectButtonState = true;
        setAllVHFBandsFilter(selectButtonState);
    }
    else
    {
        selectButtonState = false;
        setAllVHFBandsFilter(selectButtonState);
    }
}

void ClusterMainWindow::setAllVHFBandsFilter(bool state)
{
    for (auto const &b:bands)
    {
        if (b->getType() == VHF_BANDTYPE)
        {
            setBandsCheckBoxAndFilterFlag(b.data()->uk, state);
        }
    }

     updateDisplay();

}

void ClusterMainWindow::onUhfSelectBandPbPressed()
{
    static bool selectButtonState = false;

    if (!selectButtonState)
    {
        selectButtonState = true;
        setAllUHFBandsFilter(selectButtonState);
    }
    else
    {
        selectButtonState = false;
        setAllUHFBandsFilter(selectButtonState);
    }
}

void ClusterMainWindow::setAllUHFBandsFilter(bool state)
{
    for (auto const &b:bands)
    {
        if (b->getType() == MW_BANDTYPE)
        {
            setBandsCheckBoxAndFilterFlag(b.data()->uk, state);
        }
    }

    updateDisplay();
}


void ClusterMainWindow::updateDisplay()
{
    if (dxSpotProxyModel)
    {
        dxSpotProxyModel->setFilterRegExp("");
    }

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
    QMessageBox::about(this, tr("Minos Cluster Server"), tr("Minos Cluster\nCopyright D Balharrie G8FKH/M0DGB 2016 - 2020"));
}


void ClusterMainWindow::clusterNodeCommandsShortcutHelp()
{
    QMessageBox::information(this, tr("Cluster Node User Command Shortcut Keys"),
                             tr("VHF/UHF User Commands\n\nSend Cmd\nCtrl+1, Ctrl+2, Ctrl+3, Ctrl+4, Ctrl+5\n"
                                "Ctrl+6, Ctrl+7, Ctrl+8, Ctrl+9, Ctrl+0\n\n"
                                "Menu Recall\n"
                                "Ctrl+Shift+1, Ctrl+Shift+2, Ctrl+Shift+3, Ctrl+Shift+4, Ctrl+Shift+5\n"
                                "Ctrl+Shift+6, Ctrl+Shift+7, Ctrl+Shift+8, Ctrl+Shift+9, Ctrl+Shift+0\n"
                                "Then letter:\n"
                                "S - Send cmd\n"
                                "N - New cmd\n"
                                "E - Edit cmd\n"
                                "C - Clear cmd\n"));
}


bool DxSpotSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
    bool match_band = matchBand(sourceRow);
    //bool match_distance = matchDistance(sourceRow);
    //bool match_mode = matchMode(sourceRow);
    bool matchFlag = match_band /*&& match_distance && match_mode*/;
    if (traceDebugFlag)
    {
        trace(QString("filter - callsign = %1, matchBand = %2, matchFlag = %3")
            .arg(sourceModel()->data(sourceModel()->index(sourceRow, DXSPOT_CALL_COL_NUM), DataStoredRole).toString())
            .arg(match_band ? "True" : "False")
            //.arg(match_distance ? "True" : "False")
            //.arg(match_mode ? "True" : "False")

            .arg(matchFlag ? "True" : "False"));

    }
    return matchFlag;
}

bool DxSpotSortFilterProxyModel::matchBand(int sourceRow) const
{

    QString band = sourceModel()->data(sourceModel()->index(sourceRow, DXBANDSTR_COL_NUM), DataStoredRole).toString();

    return filterSettings.getBandFilter(band);

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


void ClusterMainWindow::purgeSpots()
{

    if (setupCluster->getTimeToLive() > 0 /*&& !holdUpdateFlag && (ct && ct == TContestApp::getContestApp()->getCurrentContest())*/)      // don't purge spots if == 0 and holdupdateflag is on
    {
        if (dxSpotDataModel->rowCount() > 0)
        {
           purgeSpotFlag = true;
           int idx = dxSpotDataModel->rowCount() - 1;
           while (idx >= 0 && dxSpotDataModel->rowCount() > 0)
           {
               if (spotTimedOut(dxSpotDataModel->data(dxSpotDataModel->index(idx, RXTIME_COL_NUM), DataStoredRole).toLongLong(), setupCluster->getTimeToLive().toLongLong() * 60))
               {
                   dxSpotDataModel->removeRows(idx, 1, QModelIndex());
                   trace(QString("purged spot = %1").arg(dxSpotDataModel->data(dxSpotDataModel->index(idx, DXSPOT_CALL_COL_NUM), DataStoredRole).toString()));
               }
               idx--;
           }
           purgeSpotFlag = false;
        }
    }

}

void ClusterMainWindow::onSpotTestTimerTimeOut()
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
