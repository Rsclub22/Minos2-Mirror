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
#include <QTableView>
#include <QMessageBox>

#include "regsettings.h"
#include "AppStartup.h"
#include "MShowMessageDlg.h"
#include "clustercommon.h"
#include "fileutils.h"
#include "mults.h"
#include "qrzServerCommon.h"
#include "BandList.h"
#include "delayedaction.h"
#include "LogEvents.h"
#include "MTrace.h"
#include "cutils.h"

#include "clustermainwindow.h"
#include "ui_clustermainwindow.h"

static const char * sendClusterReasonText[] = {QT_TRANSLATE_NOOP("cluster", "Ok"), QT_TRANSLATE_NOOP("cluster", "Failed - comms error"),
                                           QT_TRANSLATE_NOOP("cluster", "Not Logged On"), QT_TRANSLATE_NOOP("cluster", "Freq out of band"),
                                           QT_TRANSLATE_NOOP("cluster", "Callsign or Locator Empty")};
enum sendClusterReason_e {TX_OK, COMMS_ERR, NOT_LOGGED_ON, FREQ_ERR, CALL_LOC_EMPTY};

const char * ClusterMainWindow::DXSPOT_TAB_TITLE = QT_TR_NOOP("DX Spots");
const char * ClusterMainWindow::SENT_SPOT_TAB_TITLE = QT_TR_NOOP("Sent Spots");
const char * ClusterMainWindow::RAW_DATA_TAB_TITLE = QT_TR_NOOP("Raw Data");

const char *ClusterMainWindow::userCmdButtonLabels[4] = {QT_TR_NOOP("&Send"), QT_TR_NOOP("&New"),
                                                QT_TR_NOOP("&Edit"), QT_TR_NOOP("&Clear")};



#ifdef TEST_SPOTS
const char * CLUSTER_SPOT_TEST_FILE = "testspots.txt";
#endif

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
    reconnectFlag(false)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // most of startup can be done immediately, so it all gets built before showing it
    doStartup();

    // but delay the actual connection
    delayedAction(this, [=](){
        connectToCluster();
    });
}
void ClusterMainWindow::connectToCluster()
{
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

    connect(setupCluster, &SetupDialog::sendSpotToTxEnabled, this, &ClusterMainWindow::sendSpotToTxEnabled);

    removeInsertSendSpotTab(setupCluster->getSendToDXClusterEnabled());

    ui->clusterTab->setCurrentWidget(ui->bandFilter);
    ui->startCloseFileTab->setAutoFillBackground(true);

    RegSettings settings;
    ui->clusterTab->setCurrentIndex(settings.getSettings().value("ClusterServer/curTab", 0).toInt());
}

void ClusterMainWindow::doStartup()
{

    connect(stdinReader, &StdInReader::stdinLine, this, &ClusterMainWindow::onStdInRead);

    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName());
    Q_UNUSED(rpc)

    createCloseEvent();

    disconnectTimer = new QTimer();
    connect(disconnectTimer, &QTimer::timeout, this, &ClusterMainWindow::disconnectTimeout);

    connect(&LogTimer, &QTimer::timeout, this, &ClusterMainWindow::LogTimerTimer);
    LogTimer.start(100);


    spotsList.clear();

    setWindowTitle(tr("Minos Cluster Server"));
    status = new QLabel;
    ui->statusBar->addWidget(status);

   // connect(ui->actionAbout, &QAction::triggered, this, &ClusterMainWindow::about);
   // connect(ui->actionUser_Command_Shortcuts, &QAction::triggered, this, &ClusterMainWindow::clusterNodeCommandsShortcutHelp);

    BandList::getBandList().loadAllBands(bands, false);

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

    if (FileExists(CLUSTER_PATH() + CLUSTER_SPOT_TEST_FILE))
    {
        ui->testSpotsPb->setVisible(true);
        connect(ui->testSpotsPb, &QPushButton::clicked, this, &ClusterMainWindow::testSpotPbClicked);
        spotTestTimer = new QTimer();
        connect(spotTestTimer, &QTimer::timeout, this, &ClusterMainWindow::onSpotTestTimerTimeOut);
    }
    else
    {
        ui->testSpotsPb->setVisible(false);

    }

#endif


    RegSettings settings;
    geoStr = QString("clusterServer/geometry");
    QByteArray geometry = settings.getSettings().value(geoStr).toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    setupCluster = new SetupDialog();

    connect(setupCluster, &SetupDialog::personalDataUpdated, this, &ClusterMainWindow::personalDataChanged);

    clusterRpc = new Clusterrpc();
    connect(clusterRpc, &Clusterrpc::sendSpotToDXCluster, this, &ClusterMainWindow::sendSpotToDXCluster);
    connect(clusterRpc, &Clusterrpc::resendSpotToClients, this, &ClusterMainWindow::onResendSpotToClients);
    connect(clusterRpc, &Clusterrpc::reconnectCmdFromLog, this, [=](bool state){onReconnectCommandFromLog(state);});
    connect(clusterRpc, &Clusterrpc::clusterQrzResponse,
            this, [=](QString dxCall, QString dxGrid, QString dxCallState, QString spotterCall, QString spotterGrid, QString spotterState){onclusterQrzResponse(dxCall, dxGrid, dxCallState, spotterCall, spotterGrid, spotterState);});

    handleSpotsInQueues = new QTimer();
    connect(handleSpotsInQueues, &QTimer::timeout, this, &ClusterMainWindow::onHandleSpotsInQueues);
    handleSpotsInQueues->start(SEND_SPOTS_DUR);


    client = new QtTelnet(parent());
    dxClusterCommand = new ClusterCommands();


    if (!FileExists(CLUSTER_SETTINGS_FILE()))
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

    readBandFilterSettings();
    loadBandFilterSettingsToTab();
    ui->saveBandFilterSettingChkBox->setToolTip(tr("Always loads last saved settings"));

    readStartEndScriptSettings();



    connect(setupCluster, &SetupDialog::sendSpotToTxEnabled, this, &ClusterMainWindow::sendSpotToTxEnabled);

    // in comming spot tab

    dxSpotDataModel = new DxSpotDataModel();


    dxSpotView = new QTableView();

    dxSpotViewDelegate = QSharedPointer<HtmlDelegate>( new HtmlDelegate("dxSpotViewDelegate", 1.0, 1.0)) ;

    dxSpotDataModel->delegate = dxSpotViewDelegate;



    dxSpotProxyModel = new DxSpotSortFilterProxyModel(&filterSettings);
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

    verticalHeader->setSectionResizeMode(QHeaderView::Interactive);


    restoreDxSpotViewColumns();
    dxSpotView->horizontalHeader()->setStretchLastSection(true);
    connect( dxSpotView->horizontalHeader(), &QHeaderView::sectionResized,
             this, &ClusterMainWindow::dxSpotView_sectionResized);

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
    dxSpotView->setColumnHidden(DXCLUSTER_SPOT_TYPE, true);



    // sent spot tab

    sentSpotDataModel = new SentSpotDataModel();
    sentSpotView = new QTableView();
    sentSpotViewDelegate = QSharedPointer<HtmlDelegate>( new HtmlDelegate("sentSpotViewDelegate", 1.0, 1.0)) ;
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

    sentSpotVerticalHeader->setSectionResizeMode(QHeaderView::Interactive);

    restoreSentSpotViewColumns();
    sentSpotView->horizontalHeader()->setStretchLastSection(true);
    connect( sentSpotView->horizontalHeader(), &QHeaderView::sectionResized,
             this, &ClusterMainWindow::sentSpotView_sectionResized);

    sentSpotView->setColumnHidden(SENT_SPOT_RXTIME_COL_NUM, true);

    // rawdata tab

    rawClusterDataView = new QPlainTextEdit();
    rawClusterDataView->setReadOnly(true);

    ui->clusterViewsTab->addTab(dxSpotView, tr(DXSPOT_TAB_TITLE));
    //ui->clusterViewsTab->addTab(sentSpotView, tr(SENT_SPOT_TAB_TITLE));
    ui->clusterViewsTab->addTab(rawClusterDataView, tr(RAW_DATA_TAB_TITLE));



    setAllTabsColor(CLUSTER_TAB_NOT_SELECT_COLOR);
    ui->clusterViewsTab->setTabColor(ui->clusterViewsTab->currentIndex(), CLUSTER_TAB_SELECT_COLOR);
    connect(ui->clusterViewsTab, &QLogTabWidget::currentChanged, this, &ClusterMainWindow::onSpotTabChanged);

    //connect(ui->actionSetup, &QAction::triggered, this, &ClusterMainWindow::onLaunchSetup);
    //connect(ui->actionClear_All_Spots, &QAction::triggered, this, &ClusterMainWindow::onClearAllSpots);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(ui->nodeCb, &QComboBox::textActivated, this, &ClusterMainWindow::connectToNode);
#else
    connect(ui->nodeCb, QOverload<const QString &>::of(&QComboBox::activated), this, &ClusterMainWindow::connectToNode);
#endif

    connect(setupCluster, &SetupDialog::clusterListChanged, this, &ClusterMainWindow::clusterListChanged);

    connect(ui->hfLogFilterCheckBox, &QCheckBox::clicked, this, &ClusterMainWindow::onLogFilterCheckBoxClicked);
    connect(ui->vhfMwLogFilterCheckBox, &QCheckBox::clicked, this, &ClusterMainWindow::onLogFilterCheckBoxClicked);

    connect(client, &QtTelnet::socketConnected, this, &ClusterMainWindow::connectionEstab);
    connect(client, &QtTelnet::loginRequired, this, &ClusterMainWindow::logIn);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(client, &QtTelnet::connectionError, this, &ClusterMainWindow::connectionError);
#else
    connect(client, SIGNAL(connectionError(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
#endif

    connect(client, &QtTelnet::loggedOut, this, &ClusterMainWindow::loggedOut);
    connect(client, &QtTelnet::message, this, &ClusterMainWindow::messageRx);
    connect(client, &QtTelnet::message, this, &ClusterMainWindow::parseDX);
    connect(client, &QtTelnet::message, this, &ClusterMainWindow::checkedLoggedIn);
    connect(client, &QtTelnet::message, this, &ClusterMainWindow::cancelPingTimeOut);


    statusTimer = new QTimer(this);
    connect(statusTimer, &QTimer::timeout, this, &ClusterMainWindow::handleStatusTimer);
    statusTimer->start(STATUS_TIMER_DUR);


    pingClusterNodeTimer = new QTimer(this);
    connect(pingClusterNodeTimer, &QTimer::timeout, this, &ClusterMainWindow::handlePingClusterNodeTimeout);
    pingOk = false;

    purgeTimer = new QTimer(this);
    connect (purgeTimer, &QTimer::timeout, this, &ClusterMainWindow::purgeSpots);
    purgeTimer->start(PURGE_TIME);
}

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
    RegSettings settings;
    QByteArray state;

    state = dxSpotView->horizontalHeader()->saveState();
    settings.getSettings().setValue("dxSpotView/state", state);

}


void ClusterMainWindow::sentSpotView_sectionResized(int, int, int)
{
    RegSettings settings;
    QByteArray state;

    state = sentSpotView->horizontalHeader()->saveState();
    settings.getSettings().setValue("sentSpotView/state", state);

}

void ClusterMainWindow::restoreDxSpotViewColumns()
{
    RegSettings settings;
    QByteArray state;

    state = settings.getSettings().value("dxSpotView/state").toByteArray();
    dxSpotView->horizontalHeader()->restoreState(state);
}

void ClusterMainWindow::restoreSentSpotViewColumns()
{
    RegSettings settings;
    QByteArray state;

    state = settings.getSettings().value("sentSpotView/state").toByteArray();
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
            connect( this, &ClusterMainWindow::disconnectTimerfinished, &loop, &QEventLoop::quit);
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
                                              "Do you want to enter\\change your details?").arg(currentUserName, currentUserCallsign, currentUserQTH, currentUserLocator),
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
    showStatusMessage(tr("Connected to: %1 %2 %3").arg(currentNodeName, currentAddress, currentPort), "Connected");
    QString msg = tr("Connection Established with host %1 %2:%3").arg(currentNodeName, currentAddress, currentPort);
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
        if (ui->runEndCmdFileChkBox->isChecked())
        {
            handleEndFile();          // send user commands
        }
        QString msg = dxClusterCommand->quit();
        txText(msg);
        echoCmd(msg);
    }


}

void ClusterMainWindow::onReconnectCommandFromLog(bool state)
{

    trace(QString("reconnect command from log = %1, connection = %2").arg(state ? "True" : "False", nodeConnected ? "True" : "False"));
    if (state && !nodeConnected)
    {
        trace(QString("reconnecting node to cluster node %1").arg(ui->nodeCb->currentText()));
        connectToNode(ui->nodeCb->currentText());
    }
}


void ClusterMainWindow::messageRx(QString msg)
{
    while(msg[msg.length() - 1] == '\r' || msg[msg.length() - 1] == '\n')
    {
        msg = msg.left(msg.length() - 1);
    }
    msg.remove('\x07');
    rawClusterDataView->appendPlainText(msg);   // append paragraph, so has newline
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


            if (ui->runStartCmdFileChkBox->isChecked())
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


    if (ui->runStartCmdFileChkBox->isChecked())
    {
        handleStartFile();          // send user commands
    }



}


void ClusterMainWindow::handleStartFile()
{
    if (ui->vhfScriptRadioButton->isChecked())
    {
        handleCmdFile(CLUSTER_PATH() + CLUSTER_START_FILE);
    }
    else if (ui->hfScriptRadioButton->isChecked())
    {
        handleCmdFile(CLUSTER_PATH() + CLUSTER_START_HF_FILE);
    }
    else
    {
        trace(QString("Start Script file requested, but no radiobutton selection"));
    }

}


void ClusterMainWindow::handleEndFile()
{
    if (ui->vhfScriptRadioButton->isChecked())
    {
        handleCmdFile(CLUSTER_PATH() + CLUSTER_END_FILE);
    }
    else if(ui->hfScriptRadioButton->isChecked())
    {
        handleCmdFile(CLUSTER_PATH() + CLUSTER_END_HF_FILE);
    }
    else
    {
        trace(QString("End Script file requested, but no radiobutton selection"));
    }


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

    QString line;
    if (loginSuccess)
    {
        trace(QString("raw spot = %1").arg(txt));

        do
        {
            QSharedPointer<ClusterSpotData> newSpot = QSharedPointer<ClusterSpotData>(new ClusterSpotData());
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

            }
       } while (!line.isNull());
    }

    trace(QString("ParseDx: Finished"));
}




void ClusterMainWindow::processNewSpot(QSharedPointer<ClusterSpotData> newSpot)
{

    QString msg = QString("ProcessNewSpot: DX de %1 %2 %3 %4 %5").arg(newSpot->getDxCallStr(), newSpot->getFreq().traceStr(),
                                                                 newSpot->getBand(),newSpot->getBandType(), newSpot->getMode())
                   + QString(" %6 %7 %8 %9 %10 %11 %12 %13").arg(newSpot->getSpotterCallStr(),
                                                                 newSpot->getDxLocator(), newSpot->getSpotterLocator(),
                                                                 newSpot->getDxPropMode(), newSpot->getSpotTime(),
                                                                 newSpot->getSpotDate(), newSpot->getSpotComment(),
                                                                 setupCluster->getTimeToLive());
    trace(msg);


    // is spot older than time to live time
    int timeToLive = setupCluster->getTimeToLive().toInt() * 60;
    if (timeToLive == 0 || (timeToLive > 0 && !spotTimedOut(newSpot->getRxTime(), timeToLive)))
    {
        trace(QString("ProcessNewSpot: Spot within timeToLive"));

        if (newSpot->getDxLocator().isEmpty())
        {
            trace(QString("processNewSpot: dxLocator empty for DXcall %1").arg(newSpot->getDxCall().getFullCall()));
            if (getUseQrzForQraFlag())
            {
                trace(QString("processNewSpot: ask Qrz for qra locator"));
                askQrzForQraLocator(newSpot);
                return;
            }
            else
            {
                // get locator based upon prefix
                newSpot->setDxLocator(getQraFromCallsignPrefix(newSpot->getDxCall()));
                trace(QString("Process DX Spot: sent locator empty, get from prefix = %1").arg(newSpot->getDxLocator()));
                newSpot->setDxLocatorIsFromNode(true);
            }
        }
        else if (newSpot->getDxLocator() == ASKQRZ_FAILEDQRA)
        {
            // failed to get QRA from QRZ, use prefix
            newSpot->setDxLocator(getQraFromCallsignPrefix(newSpot->getDxCall()));
            trace(QString("Process DX Spot: failed to get qra from qrz, get from prefix = %1").arg(newSpot->getDxLocator()));
            newSpot->setDxLocatorIsFromNode(true);
        }




        if (currentUserCallsign != newSpot->getSpotterCallStr())
        {
            // send spot to clients if spotter isn't this station
            trace(QString("ProcessNewSpot: Spotter not this station, pass to clients, callsign %1").arg(newSpot->getDxCallStr()));
            trace(QString("processNewSpot:Check if HF Spots or VHF/MW Spots are filtered to logger"));
            if ((ui->hfLogFilterCheckBox->isChecked() && newSpot->getBandType() == HF_BANDTYPE))
            {
                trace(QString("processNewSpot: HF Spots checked for pass to client - pass HF Spot"));
                sendSpotsToClientQueue.append(createSpotToSend(assembleSpotMsgToSendToClients(newSpot, setupCluster->getTimeToLive())));

            }
            else if(ui->vhfMwLogFilterCheckBox->isChecked() && (newSpot->getBandType() == VHF_BANDTYPE || newSpot->getBandType() == MW_BANDTYPE))
            {
                trace(QString("processNewSpot: VHF/MW Spots checked for pass to client - pass VHF/MW Spot"));
                sendSpotsToClientQueue.append(createSpotToSend(assembleSpotMsgToSendToClients(newSpot, setupCluster->getTimeToLive())));

            }
            else
            {
                trace(QString("processNewSpot: Neither HF or VHF/MW spots checked to pass to log - nothing sent"));
            }

        }
        else
        {
            trace(QString("ProcessNewSpot: Spotter is this station, only display on server"));
        }


        trace(QString("ProcessNewSpot: Add spot for display callsign = %1, rxTime = %2").arg(newSpot->getDxCallStr()).arg(newSpot->getRxTime()));
        spotsList.append(newSpot);

    }
    else
    {
        trace(QString("ProcessNewSpot: Spot %1, older than time to live time = %2 mins").arg(newSpot->getDxCallStr()).arg(timeToLive/60));
    }


}


void ClusterMainWindow::askQrzForQraLocator(QSharedPointer<ClusterSpotData> newSpot)
{
    QString dxCall = newSpot->getDxCall().getFullCall();
    QString spotterCall = newSpot->getSpotterCall().getFullCall();

    trace(QString("askQrzForQraLocator: dxCall = %1, spotterCall = %2").arg(dxCall, spotterCall));
    clusterRpc->askQrzServerForQra(dxCall, spotterCall);

    askQrzQueue.insert((dxCall + ":" + spotterCall), newSpot);


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

    trace(QString("getQraFromCallsignPrefix: callsign = %1, prefix = %2").arg(cs.getFullCall(), prefix));

    QSharedPointer<CountrySynonym> syn = MultLists::getMultLists()->searchCountrySynonym ( prefix );
    if (!syn)
    {
        return "";
    }
    return syn->getCentral().getLoc();
}

int ClusterMainWindow::upackShowDxSpot(const QString txt, QSharedPointer<ClusterSpotData> newSpot)
{
    static QRegularExpression rews("\\s+");

    trace(QString("UnpackShowDXSpot - %1").arg(txt));

    newSpot->setClusterSpotType(clusterSpotType::SHOW_DXSPOT_TYPE);

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    dxMsg = txt.split(rews, Qt::SkipEmptyParts);
#else
    dxMsg = txt.split(rews, QString::SkipEmptyParts);
#endif

    if (dxMsg.count() > 4)
    {
        QString f = dxMsg[0] + "00";
        f.remove('.');
        newSpot->setFreq(f);
        QString dxBandStr;
        QString dxBandType;
        if (!getBand(bands, newSpot->getFreq().str(), dxBandStr, dxBandType))
        {
            trace(QString("Spot is not in contest band list discard - Call = %1, Freq. = %2").arg(newSpot->getDxCall().getFullCall(), newSpot->getFreq().traceStr()));
            return DISCARD_SPOT_NOT_CONTEST_BAND * -1;
        }

        newSpot->setBand(dxBandStr);
        newSpot->setBandType(dxBandType);


        QString dxModeStr = getMode(modeBandPlan, newSpot->getFreq().str(), dxBandStr);
        newSpot->setMode(dxModeStr);

        newSpot->setDxCall(dxMsg[1]);

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
        findLocInComment(spotLocator, dxLocator, spotComment);
        newSpot->setSpotterLocator(spotLocator);
        newSpot->setDxLocator(dxLocator);

        newSpot->setDxPropMode(getPropMode(spotComment));

        // look for mode in comments, if found overide freq mode
        int commentModeNum;
        QString commentMode;
        if (lookforModeInComment(spotComment, commentModeNum, commentMode))
        {
            newSpot->setMode(commentMode);

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



void ClusterMainWindow::onResendSpotToClients(int frameId, QString loggerUuid, QString cmd, QString bandMask)
{
    ResendSpotCommand spotCmd;
    spotCmd.setCmd(cmd);
    spotCmd.setBandmask(bandMask);
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

                // check band to set logBandFilter
                resendAllSpotsToClients(resendSpotsToClientQueue[i]);

            }
        }

        resendSpotsToClientQueue.clear();
    }
}
QString ClusterMainWindow::getBandType(QString band)
{

    for (const auto &b: QASCONST(bands))
    {
        if (b->uk == band)
        {
            return b->getType();
            break;
        }
    }

    return NO_BANDTYPE;     // shouldn't get here if band is correct
}

void ClusterMainWindow::resendAllSpotsToClients(ResendSpotCommand cmd)
{

    if (dxSpotDataModel->rowCount() > 0)
    {
        for (int row = 0; row < dxSpotDataModel->rowCount(); row ++)
        {
            QString bandMask = dxSpotDataModel->data(dxSpotDataModel->index(row, DXBANDSTR_COL_NUM), DataStoredRole).toString();
            QString bandType = getBandType(bandMask);
            if (bandType == NO_BANDTYPE)
            {
                trace(QString("resendAllSpotsToClients: error no bandType found for band %1").arg(bandMask));
            }

            if (cmd.getBandmask() == bandMask)
            {
                if ((ui->hfLogFilterCheckBox->isChecked() && bandType == HF_BANDTYPE)
                        || (ui->vhfMwLogFilterCheckBox->isChecked() && (bandType == VHF_BANDTYPE || bandType == MW_BANDTYPE)))
                {

                    QString spot = createResendSpotToSend(assembleSpotMsgToSendToClients(dxSpotDataModel->getSpotData(row), setupCluster->getTimeToLive()));
                    trace(QString("resending this spot - %1 to uuid = %2").arg(spot, cmd.getuuid()));
                    clusterRpc->sendDXSpot(spot, cmd.getuuid(), cmd.getFrameId());   // send spot and loggeruuid
                }
            }
            else
            {
                trace(QString("processNewSpot: Neither HF or VHF/MW spots checked to pass to log - nothing sent"));
            }

        }
    }

}




QString ClusterMainWindow::assembleSpotMsgToSendToClients(const QSharedPointer<ClusterSpotData> spotData, const QString timeToLive)
{
    QString spotMsg = QString("%1:%2:%3:%4:%5:%6:%7:%8")
                       .arg(spotData->getClusterSpotType(), // %1
                       spotData->getDxCallStr(),           // %2
                       spotData->getDxLocator(),            // %3
                       spotData->getDxLocatorIsFromNode() ? "locFromNode-true" : "locFromNode-false", // %4
                       spotData->getFreq().str(),           // %5
                       spotData->getBand(),                 // %6
                       spotData->getBandType(),             // %7
                       spotData->getMode())                 // %8

            + QString(":%9:%10:%11:%12:%13:%14")
                       .arg(spotData->getSpotterCallStr(),      // %9
                       spotData->getSpotterLocator(),       // %10
                       spotData->getSpotDateTime().toString("yyyyMMMddHHmmss"),  // %11
                       spotData->getSpotComment(),         // %12
                       spotData->getDxPropMode(),         // %13
                       timeToLive);                      // %14

    return spotMsg;

}


void ClusterMainWindow::onclusterQrzResponse(QString dxCall, QString dxGrid, QString dxCallState, QString spotterCall, QString spotterGrid, QString spotterState)
{

    trace(QString("onclusterQrzResponse: dxCall = %1, dxGrid = %2, dxCallState = %3, spotterCall = %4, spotterGrid = %5, spotterState = %6")
          .arg(dxCall, dxGrid, dxCallState, spotterCall, spotterGrid, spotterState));

    QString callsignKey = dxCall + ":" + spotterCall;

    if (askQrzQueue.contains(callsignKey))
    {
        trace(QString("onclusterQrzResponse: askQrzQueue contains key = %1").arg(callsignKey));

        QSharedPointer<ClusterSpotData> newSpot = askQrzQueue.value(callsignKey);
        askQrzQueue.remove(callsignKey);


        if (!dxGrid.isEmpty() && dxCallState == QRA_LOOKUP_OK)
        {
            trace(QString("Qrz Server Response for callsign = %1, qra = %2").arg(dxCall, dxGrid));
            newSpot->setDxLocator(dxGrid);
            newSpot->setDxLocatorIsFromNode(true);
            processNewSpot(newSpot);
        }
        else
        {
            // flag no Qra found for callsign from Qrz
            newSpot->setDxLocator(ASKQRZ_FAILEDQRA);  // flag failure
            trace(QString("Qrz Server Response for callsign = %1, error = %2").arg(dxCallState));
            processNewSpot(newSpot);
        }
    }
    else
    {
        trace(QString("onclusterQrzResponse: askQrzQueue does not contain key = %1").arg(callsignKey));
    }


}

void ClusterMainWindow::cancelPingTimeOut(QString msg)
{
    if (msg.contains("ping_cluster"))
    {
        pingOk = true;
        trace(QString("response to ping received ok"));
    }
    // we've had some kind of message from the cluster - so delay the next ping
    pingClusterNodeTimer->start(getPingTimeoutValue());
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
        trace(QString("logging out - node connected %1").arg(nodeConnected ? "True" : "False"));
        loggedOut();
    }

}


int ClusterMainWindow::getPingTimeoutValue()
{
    QSettings settings(CLUSTER_SETTINGS_FILE(), QSettings::IniFormat);
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
        while (!spotsList.empty())
        {
            if (purgeSpotFlag)
            {
                trace(QString("GetSpotsFromDisplayQueue: PurgeFlag On"));
                return;
            }

            dxSpotDataModel->rowData = spotsList.last();
            spotsList.removeLast();

            dxSpotDataModel->insertRows(dxSpotDataModel->rowCount(), 1);
            trace(QString("GetSpotsFromDisplayQueue: finished loop"));


        }


        trace(QString("GetSpotsFromDisplayQueue: finished"));
    }
}


int ClusterMainWindow::upackDxSpot(QString txt, QSharedPointer<ClusterSpotData> newSpot)
{
    static QRegularExpression tre("\\d\\d\\d\\dZ");
    static QRegularExpression dre("\\s+");

    trace(QString("UnpackDXSpot - %1").arg(txt));

    newSpot->setClusterSpotType(clusterSpotType::DXSPOT_TYPE);

    int timePos = 0;

    txt.remove('\x07');

    dxMsg = txt.split(dre);

    if (dxMsg.count() > 5)
    {
        newSpot->setSpotterCall(dxMsg[2].remove(':'));
        QString f = dxMsg[3] + "00";
        f.remove('.');

        //dxFreq = convertKhzToMhz(dxMsg[3]);
        newSpot->setFreq(f);


        QString dxBandStr;
        QString dxBandType;
        if (!getBand(bands, newSpot->getFreq().str(), dxBandStr, dxBandType))
        {
            trace(QString("Spot is not in contest band list discard - Call = %1, Freq. = %2").arg(newSpot->getDxCall().getFullCall(), newSpot->getFreq().traceStr()));
            return DISCARD_SPOT_NOT_CONTEST_BAND * -1;
        }

        newSpot->setBand(dxBandStr);
        newSpot->setBandType(dxBandType);


        QString dxModeStr = getMode(modeBandPlan, newSpot->getFreq().str(), newSpot->getBand());
        newSpot->setMode(dxModeStr);

        newSpot->setDxCall(dxMsg[4]);

        // find time

        QString time;
        for (int i = 4; i < dxMsg.count(); i++)
        {
            QRegularExpressionMatch match = tre.match(dxMsg[i]);
            if (match.hasMatch())
            {
                //newSpot.setSpotTime(dxMsg[i].remove('Z'));
                time = dxMsg[i].remove('Z');
                timePos = i;
                break;
            }
        }


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
        findLocInComment(spotLocator, dxLocator, spotComment);
        newSpot->setSpotterLocator(spotLocator);
        newSpot->setDxLocator(dxLocator);

        newSpot->setDxPropMode(getPropMode(spotComment));

        // look for mode in comments, if found overide freq mode
        int commentModeNum;
        QString commentMode;
        if (lookforModeInComment(spotComment, commentModeNum, commentMode))
        {
            newSpot->setMode(commentMode);

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

    QRegularExpression full_loc_exp;
    QRegularExpression part_loc_exp;

    full_loc_exp = FULL_LOC_EXP_HF;
    part_loc_exp = PART_LOC_EXP_HF;


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

    trace(QString("Extracted dxLoc = %1 spotLoc= %2").arg(dxLoc, spotLoc));

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


void ClusterMainWindow::setSendSpotsToLogWarning(QString txt)
{

    ui->sendSpotsToLogWarningLabel->setText(HtmlFontColour(Qt::red) + txt);
}

void ClusterMainWindow::onLogFilterCheckBoxClicked()
{
    if (!ui->hfLogFilterCheckBox->isChecked() && !ui->vhfMwLogFilterCheckBox->isChecked())
    {
        setSendSpotsToLogWarning(tr("No spots will be sent to the log - please check one of the boxes!"));
    }
    else
    {
        setSendSpotsToLogWarning("");
    }
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
    QString spotMsg = assembleSpotForDXCluster(freq, call, loc);
    if (setupCluster->getSendToDXClusterEnabled() && loginSuccess && !freq.isClear() && !call.isEmpty())
    {
        trace(QString("SendSpotToDXCluster: sending spot, call %1, freq %2, locator %3").arg(call, freq.traceStr(), loc));
        if (BandList::getBandList().checkValidBand(freq))
        {

#ifdef TXSPOT
            int error = txText(spotMsg);
            if (error < 0)
            {
                trace(QString("SendSpotToDXCluster: sending spot %1 failed to send").arg(spotMsg));
                bool spotStatus = false;
                addSentSpotToDisplayQueue(spotStatus, tr(sendClusterReasonText[COMMS_ERR]));
            }
            else
            {
                trace(QString("SendSpotToDXCluster: sending spot %1 sent Ok").arg(spotMsg));
                bool spotStatus = true;
                addSentSpotToDisplayQueue(spotStatus, tr(sendClusterReasonText[TX_OK]));
            }
#endif


        }
        else
        {
            trace(QString("SendSpotToDXCluster: spot freq is out of band %1, spot callsign %2").arg(freq.traceStr(), call));
            addSentSpotToDisplayQueue(false, tr(sendClusterReasonText[FREQ_ERR]));
        }
    }
    else
    {
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
    sentComment = QString("%1< >%2").arg(setupCluster->getUserLocator(), loc);


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


    QString spotmsg = QString("DX %1 %2").arg(call, QString::number(f));
    if (!loc.isEmpty())
    {
        spotmsg = QString("%1 %2").arg(spotmsg, sentComment);
    }

    return spotmsg + QChar('\n');
}

void ClusterMainWindow::closeEvent(QCloseEvent *event)
{


    if (nodeConnected)
    {
        if (ui->runEndCmdFileChkBox->isChecked())
        {
            handleEndFile();          // send user commands
        }

    }



    LogTimer.stop();

    if (ui->saveBandFilterSettingChkBox->isChecked())
    {
        saveBandFilterSettings();
    }

    if (ui->saveStartSciptCheckBox->isChecked())
    {
        saveStartEndScriptSettings();
    }


    // and tidy up all loose ends

    RegSettings settings;
    settings.getSettings().setValue(geoStr, saveGeometry());


    settings.getSettings().setValue("ClusterServer/curTab", ui->clusterTab->currentIndex());

    disconnectNode();

    trace("Minos Cluster Server Closing");
    QWidget::closeEvent(event);
}


void ClusterMainWindow::onStdInRead(QString cmd)
{
    if (cmd.indexOf("Shutdown", 0, Qt::CaseInsensitive) >= 0)
    {
        close();
    }
}
void ClusterMainWindow::moveEvent(QMoveEvent * event)
{
    RegSettings settings;
    settings.getSettings().setValue(geoStr, saveGeometry());
    QWidget::moveEvent(event);
}
void ClusterMainWindow::resizeEvent(QResizeEvent * event)
{
    RegSettings settings;
    settings.getSettings().setValue(geoStr, saveGeometry());
    QWidget::resizeEvent(event);
}
void ClusterMainWindow::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        RegSettings settings;
        settings.getSettings().setValue(geoStr, saveGeometry());
    }
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

        connect(userVHFUHFCmdButton[i], &PresetButton::presetShortCutRecall, this, [this, i]() {userVhfUhfCmdButtonRead(i);});
        connect(userVHFUHFCmdButton[i], &PresetButton::presetShiftShortCutRecall, this, [this, i]() {showVhfUhfUserCmdButtonMenu(i);});
        connect(userVHFUHFCmdButton[i], &PresetButton::presetReadAction, this, [this, i]() {userVhfUhfCmdButtonRead(i);});
        connect(userVHFUHFCmdButton[i], &PresetButton::presetEditAction, this, [this, i]() {userVhfUhfCmdButtonEdit(i);});
        connect(userVHFUHFCmdButton[i], &PresetButton::presetWriteAction, this, [this, i]() {userVhfUhfCmdButtonWrite(i);});
        connect(userVHFUHFCmdButton[i], &PresetButton::presetClearAction, this, [this, i]() {userVhfUhfCmdButtonClear(i);});



    }


    for (int i = 0; i < ui_userHFCommandButtons.count(); i++)
    {

        userHFCmdButton.append(new PresetButton(ui_userHFCommandButtons[i], i, hfCommandShortCutKeyList[i], hfMenuShortCutKeyList[i], buttonLabels));

        connect(userHFCmdButton[i], &PresetButton::presetShortCutRecall, this, [this, i]() {userHfCmdButtonRead(i);});
        connect(userHFCmdButton[i], &PresetButton::presetShiftShortCutRecall, this, [this, i]() {showHfUserCmdButtonMenu(i);});
        connect(userHFCmdButton[i], &PresetButton::presetReadAction, this, [this, i]() {userHfCmdButtonRead(i);});
        connect(userHFCmdButton[i], &PresetButton::presetEditAction, this, [this, i]() {userHfCmdButtonEdit(i);});
        connect(userHFCmdButton[i], &PresetButton::presetWriteAction, this, [this, i]() {userHfCmdButtonWrite(i);});
        connect(userHFCmdButton[i], &PresetButton::presetClearAction, this, [this, i]() {userHfCmdButtonClear(i);});


    }


}

void ClusterMainWindow::showVhfUhfUserCmdButtonMenu(int buttonNumber)
{
    if (ui->clusterTab->currentIndex() == 1)
    {
       userVHFUHFCmdButton[buttonNumber]->showButtonMenu();
    }
}

void ClusterMainWindow::showHfUserCmdButtonMenu(int buttonNumber)
{
    if (ui->clusterTab->currentIndex() == 0)
    {
        userHFCmdButton[buttonNumber]->showButtonMenu();
    }
}

void ClusterMainWindow::userVhfUhfCmdButtonRead(int buttonNumber)
{
    if (ui->clusterTab->currentIndex() == 1)
    {
        userCmdButtonRead(vhfUhfUserCommands, VHF_UHF_USERCOMMAND_TABNAME, buttonNumber);
    }
}

void ClusterMainWindow::userHfCmdButtonRead(int buttonNumber)
{
    if (ui->clusterTab->currentIndex() == 0)
    {
        userCmdButtonRead(hfUserCommands, HF_USERCOMMAND_TABNAME, buttonNumber);
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
                        trace(QString("UserCmdButton %1 Read - Send Command to cluster = %2").arg(tabSelected, d[1]));

                        if (ui->runEndCmdFileChkBox->isChecked())
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

   userCmdButtonEdit(vhfUhfUserCommands, VHF_UHF_USERCOMMAND_TABNAME, buttonNumber);

}


void ClusterMainWindow::userHfCmdButtonEdit(int buttonNumber)
{
    if (ui->clusterTab->currentIndex() == HF_TABNUM)
    {
        userCmdButtonEdit(hfUserCommands, HF_USERCOMMAND_TABNAME, buttonNumber);
    }
}


void ClusterMainWindow::userCmdButtonEdit(QStringList userCommands, QString tabSelected, int buttonNumber)
{


    trace(QString("UserCmdButton %1 Edit Selected = %2").arg(tabSelected, QString::number(buttonNumber + 1)));
    if (!userCommands[buttonNumber].isEmpty() && buttonNumber < userVHFUHFCmdButton.count())
    {

        if (userCommands[buttonNumber].contains(':'))
        {
            QStringList d = userCommands[buttonNumber].split(':');
            if (d.count() == 2)
            {
                ClusterUserCommandData editData(d[0], d[1]);
                ClusterUserCommandData curData(d[0], d[1]);

                trace(QString("UserCmdButton %1 - Edit Data - name = %2, cmdString = %3").arg(tabSelected, d[0], d[1]));
                userClusterCommandDialog cmdStringDialog(this, tabSelected, buttonNumber, &editData, &curData, QString("Edit"));


                if (cmdStringDialog.exec() == QDialog::Accepted)
                {
                    if (editData.name != curData.name || editData.cmdString != curData.cmdString)
                    {
                        trace(QString("UserCmdButton - Saving Edited Data - name = %1, cmdString = %2").arg(editData.name, editData.cmdString));
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

    userCmdButtonClear(vhfUhfUserCommands, VHF_UHF_USERCOMMAND_TABNAME, buttonNumber);

}


void ClusterMainWindow::userHfCmdButtonClear(int buttonNumber)
{
    if (ui->clusterTab->currentIndex() == HF_TABNUM)
    {
        userCmdButtonClear(hfUserCommands, HF_USERCOMMAND_TABNAME, buttonNumber);
    }
}

void ClusterMainWindow::userCmdButtonClear(QStringList userCommands, QString tabSelected, int buttonNumber)
{
    trace(QString("UserCommand Clear Selected = %1").arg(QString::number(buttonNumber +1)));

    if (!userCommands[buttonNumber].isEmpty() || (!userVHFUHFCmdButton.isEmpty()  && buttonNumber < userVHFUHFCmdButton.count()))
    {
//        static StandardButton question(QWidget *parent, const QString &title,
//             const QString &text,
//             StandardButtons buttons = StandardButtons(Yes | No),
//             StandardButton defaultButton = NoButton);

        QMessageBox::StandardButton status = QMessageBox::question( this,
                                tr("Cluster %1 User Command Clear").arg(tabSelected),
                                tr("Do you really want to clear cluster %1 user command number:%2?")
                                .arg(tabSelected).arg(buttonNumber + 1),

                                QMessageBox::StandardButtons(QMessageBox::Yes|QMessageBox::StandardButton::No|QMessageBox::StandardButton::Escape),

                                QMessageBox::StandardButton::NoButton);

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
    userCmdButtonWrite(VHF_UHF_USERCOMMAND_TABNAME, buttonNumber);

}

void ClusterMainWindow::userHfCmdButtonWrite(int buttonNumber)
{
    if (ui->clusterTab->currentIndex() == HF_TABNUM)
    {
        userCmdButtonWrite(HF_BANDTYPE, buttonNumber);
    }
}



void ClusterMainWindow::userCmdButtonWrite(QString tabSelected, int buttonNumber)
{
    trace(QString("UserCommand %1 New Selected = %2").arg(tabSelected, QString::number(buttonNumber +1)));
    if (!userVHFUHFCmdButton.isEmpty()  && buttonNumber < userVHFUHFCmdButton.count())
    {

        ClusterUserCommandData editData("", "");
        ClusterUserCommandData curData("", "");
        userClusterCommandDialog cmdStringDialog(this, tabSelected, buttonNumber, &editData, &curData, QString("New"));


        if (cmdStringDialog.exec() == QDialog::Accepted)
        {
            if (editData.name != curData.name || editData.cmdString != curData.cmdString)
            {
                trace(QString("%1 UserCommand New Selected - Saving new data name = %2, cmdString = %3").arg(tabSelected, editData.name, editData.cmdString));
                saveUserCommandString(tabSelected, buttonNumber, editData);
                userCommandButtonUpdate(tabSelected, buttonNumber, editData);
            }

        }

    }
}



void ClusterMainWindow::userCommandButtonUpdate(QString tabSelected, int buttonNumber, ClusterUserCommandData& buttonData)
{
    if (tabSelected == VHF_UHF_USERCOMMAND_TABNAME)
    {
        userVHFUHFCmdButton[buttonNumber]->setText(QString("%1: %2").arg(QString::number(buttonNumber + 1), buttonData.name) );
        // update store
        vhfUhfUserCommands[buttonNumber] = buttonData.name + ":" + buttonData.cmdString;
    }
    else if (tabSelected == HF_USERCOMMAND_TABNAME)
    {
        userHFCmdButton[buttonNumber]->setText(QString("%1: %2").arg(QString::number(buttonNumber + 1), buttonData.name) );
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
               userCommandButtonUpdate(VHF_UHF_USERCOMMAND_TABNAME, i, buttonData);
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
               userCommandButtonUpdate(HF_USERCOMMAND_TABNAME, i, buttonData);
            }

        }
    }
}



void ClusterMainWindow::readUserCommandStrings()
{
    QSettings config(CLUSTER_PATH() + CLUSTER_COMMANDS, QSettings::IniFormat);

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
    QSettings config(CLUSTER_PATH() + CLUSTER_COMMANDS, QSettings::IniFormat);

    config.beginGroup("UserCommandStrings");
    QStringList keys = config.allKeys();
    if (keys.count() > 0)
    {
        for (int i = 0; i < keys.count(); i++)
        {
            vhfUhfUserCommands.append(config.value(QString("command%1").arg(QString::number(i+1)), "").toString());
        }
    }

    config.endGroup();


    for (int i = 0; i < userVHFUHFCmdButton.count(); i++)
    {
        ClusterUserCommandData buttonData;
        if (i < vhfUhfUserCommands.count())
        {
            QStringList cl = vhfUhfUserCommands[i].split(':');

            if (cl.count() == 2)
            {
                buttonData.name = cl[0];
                buttonData.cmdString = cl[1];
            }

            saveUserCommandString(VHF_UHF_USERCOMMAND_TABNAME, i, buttonData);
        }
    }

    config.remove("UserCommandStrings");



}


void ClusterMainWindow:: saveUserCommandString(QString tabSelected, int buttonNumber, ClusterUserCommandData& buttonData)
{

    QSettings config(CLUSTER_PATH() + CLUSTER_COMMANDS, QSettings::IniFormat);
    if (tabSelected == VHF_UHF_USERCOMMAND_TABNAME)
    {
       config.beginGroup("VHF_UHF_UserCommandStrings");
    }
    else if (tabSelected == HF_USERCOMMAND_TABNAME)
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
    QGridLayout *hfLayout = new QGridLayout();
    ui->HFFrame->setLayout(hfLayout);

    QGridLayout *vhfLayout = new QGridLayout();
    ui->VHFFrame->setLayout(vhfLayout);

    QGridLayout *mwLayout = new QGridLayout();
    ui->MWFrame->setLayout(mwLayout);

    // bands only contains displayable bands (<10GHz) and is sorted
    int hfRow = 0;
    int hfCol = 0;
    int vhfRow = 0;
    int vhfCol = 0;
    int mwRow = 0;
    int mwCol = 0;

    for (auto const &b: QASCONST(bands))
    {
        QCheckBox *cb = new QCheckBox();
        connect(cb, &QCheckBox::clicked, this, [=]() {onbandCheckBoxStateChanged();});

        cb->setText(b->uk);
        ClusterClientBandFilterDialogDetails ccfd;
        ccfd.bandChkBox = cb;
        ccfd.bandType = b->getType();
        bandCheckBoxes.insert(b->uk, ccfd);

        if (b->getType() == HF_BANDTYPE)
        {
            hfLayout->addWidget(cb, hfRow, hfCol);
            hfCol++;
            if (hfCol %4 == 0)
            {
                hfCol = 0;
                hfRow++;
            }
        }
        else if (b->getType() == VHF_BANDTYPE)
        {
            vhfLayout->addWidget(cb, vhfRow, vhfCol);
            vhfCol++;
            if (vhfCol %4 == 0)
            {
                vhfCol = 0;
                vhfRow++;
            }
        }
        else if (b->getType() == MW_BANDTYPE)
        {
            mwLayout->addWidget(cb, mwRow, mwCol);
            mwCol++;
            if (mwCol %4 == 0)
            {
                mwCol = 0;
                mwRow++;
            }
        }
        else
        {
            mShowMessage(b->uk, this);
        }
    }

    connect(ui->hfSelectBandPb, &QPushButton::pressed, this, [=]() {onHfSelectBandPbPressed();});
    connect(ui->vhfSelectBandPb, &QPushButton::pressed, this, [=]() {onVhfSelectBandPbPressed();});
    connect(ui->uhfSelectBandPb, &QPushButton::pressed, this, [=]() {onUhfSelectBandPbPressed();});



}

void ClusterMainWindow::loadBandFilterSettingsToTab()
{
    for (const auto &b: QASCONST(bands))
    {
        QString band = b->uk;

        bandCheckBoxes.value(band).bandChkBox->setChecked(filterSettings.getBandFilter(band));
    }
}


void ClusterMainWindow::saveBandFilterSettings()
{
    QSettings config(CLUSTER_SETTINGS_FILE(), QSettings::IniFormat);
    config.beginGroup("DXSPOT_Display_BandFilter");

    for (auto const &b: QASCONST(bands))
    {
        QString band = b->name();
        QString iniBandName = b->normalisedName();
        config.setValue(QString("bandFilter_%1").arg(iniBandName), filterSettings.getBandFilter(band));
    }

    config.setValue(QString("saveBandFilterSettingsOnClose"), ui->saveBandFilterSettingChkBox->isChecked() );

    config.endGroup();

    config.beginGroup("Spots_To_Log_Filter");

    config.setValue(QString("logFilterHF"), ui->hfLogFilterCheckBox->isChecked());
    config.setValue(QString("logFilterVHFMW"),ui->vhfMwLogFilterCheckBox->isChecked());

    config.endGroup();


}

void ClusterMainWindow::readBandFilterSettings()
{
    QSettings config(CLUSTER_SETTINGS_FILE(), QSettings::IniFormat);
    config.beginGroup("DXSPOT_Display_BandFilter");
    for (auto const &b: QASCONST(bands))
    {
        QString band = b->uk;
        QString iniBandName = b->normalisedName();
        filterSettings.setBandFilter(band, config.value(QString("bandFilter_%1").arg(iniBandName), true).toBool());

    }

    ui->saveBandFilterSettingChkBox->setChecked(config.value("saveBandFilterSettingsOnClose", true).toBool());

    config.endGroup();

    config.beginGroup("Spots_To_Log_Filter");

    ui->hfLogFilterCheckBox->setChecked(config.value("logFilterHF", true).toBool());
    ui->vhfMwLogFilterCheckBox->setChecked(config.value("logFilterVHFMW", true).toBool());

    config.endGroup();

    onLogFilterCheckBoxClicked();
}




void ClusterMainWindow::saveStartEndScriptSettings()
{

    QSettings config(CLUSTER_SETTINGS_FILE(), QSettings::IniFormat);
    config.beginGroup("StartEndScript");

    if (ui->hfScriptRadioButton->isEnabled())
    {
        config.setValue("hfScriptFileEnabled", true);
    }
    else
    {
        config.setValue("vhfScriptFileEnabled", false);
    }


    config.setValue("enableStartCommandFile", ui->runStartCmdFileChkBox->isChecked());
    config.setValue("enableEndCommandFile", ui->runEndCmdFileChkBox->isChecked());
    config.setValue("saveStartScriptSettingsOnClose", ui->saveStartSciptCheckBox->isChecked());
    config.endGroup();
}


void ClusterMainWindow::readStartEndScriptSettings()
{
    QSettings config(CLUSTER_SETTINGS_FILE(), QSettings::IniFormat);
    config.beginGroup("StartEndScript");

    ui->hfScriptRadioButton->setChecked(config.value("hfScriptFileEnabled", false).toBool());
    ui->vhfScriptRadioButton->setChecked(config.value("vhfScriptFileEnabled", true).toBool());


    ui->runStartCmdFileChkBox->setChecked(config.value("enableStartCommandFile", false).toBool());
    ui->runEndCmdFileChkBox->setChecked(config.value("enableEndCommandFile", false).toBool());
    ui->saveStartSciptCheckBox->setChecked(config.value("saveStartScriptSettingsOnClose", false).toBool());
    config.endGroup();
}
void ClusterMainWindow::setHfFilterControlsVisible()
{

    for (const auto &b: QASCONST(bands))
    {
        if (b->getType() == HF_BANDTYPE)
        {
            QString band = b->uk;
            bandCheckBoxes.value(band).bandChkBox->setVisible(true);
        }

    }

    ui->hfSelectBandPb->setVisible(true);

}


void ClusterMainWindow::saveEnableStartEndScriptFileFlags()
{
    QSettings config(CLUSTER_SETTINGS_FILE(), QSettings::IniFormat);

    config.beginGroup("CommandFile");

   // config.setValue("enableStartCommandFile", enableStartCmdFiles);
    config.endGroup();




    config.beginGroup("CommandFile");
    //config.setValue("enableEndCommandFile", enableEndCmdFiles);
    config.endGroup();


}


void ClusterMainWindow::saveBandFilterOnSaveFlag()
{
    QSettings config(CLUSTER_SETTINGS_FILE(), QSettings::IniFormat);
/*
    config.beginGroup("General");
    if (band)
    config.setValue("bandFilterSaveOnClose", bandFilterOnSaveFlag);
    config.endGroup();
 */
}

void ClusterMainWindow::onbandCheckBoxStateChanged( )
{
    bool changed = false;
    for (const auto &b: QASCONST(bands))
    {
        QString band = b->uk;

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


    if (areAnyBandsChecked(HF_BANDTYPE))
    {

        setAllHFBandsFilter(false);
    }
    else
    {

        setAllHFBandsFilter(true);
    }
}


void ClusterMainWindow::setAllHFBandsFilter(bool state)
{
    for (const auto &b: QASCONST(bands))
    {
        if (b->getType() == HF_BANDTYPE)
        {
            setBandsCheckBoxAndFilterFlag(b->uk, state);
        }
    }

     updateDisplay();
}








void ClusterMainWindow::setBandsCheckBoxAndFilterFlag(const QString band, const bool state)
{
    filterSettings.setBandFilter(band, state);
    bandCheckBoxes.value(band).bandChkBox->setChecked(state);
}


bool ClusterMainWindow::areAnyBandsChecked(QString bandType)
{
    for (const auto &b: QASCONST(bands))
    {
        if (b->getType() == bandType)
        {
            if (bandCheckBoxes.value(b->uk).bandChkBox->isChecked())
            {
                return true;
            }
        }
    }

    return false;
}

void ClusterMainWindow::onVhfSelectBandPbPressed()
{


    if (areAnyBandsChecked(VHF_BANDTYPE))
    {

        setAllVHFBandsFilter(false);
    }
    else
    {

        setAllVHFBandsFilter(true);
    }
}

void ClusterMainWindow::setAllVHFBandsFilter(bool state)
{
    for (const auto &b: QASCONST(bands))
    {
        if (b->getType() == VHF_BANDTYPE)
        {
            setBandsCheckBoxAndFilterFlag(b->uk, state);
        }
    }

     updateDisplay();

}

void ClusterMainWindow::onUhfSelectBandPbPressed()
{


    if (areAnyBandsChecked(MW_BANDTYPE))
    {

        setAllUHFBandsFilter(false);
    }
    else
    {

        setAllUHFBandsFilter(true);
    }
}

void ClusterMainWindow::setAllUHFBandsFilter(bool state)
{
    for (const auto &b: QASCONST(bands))
    {
        if (b->getType() == MW_BANDTYPE)
        {
            setBandsCheckBoxAndFilterFlag(b->uk, state);
        }
    }

    updateDisplay();
}


void ClusterMainWindow::updateDisplay()
{
    if (dxSpotProxyModel)
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        dxSpotProxyModel->setFilterRegularExpression("");
#else
        dxSpotProxyModel->setFilterRegExp("");
#endif
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


bool ClusterMainWindow::getUseQrzForQraFlag()
{
    QSettings config(CLUSTER_SETTINGS_FILE(), QSettings::IniFormat);
    config.beginGroup("UseQRZServer");
    bool useQrzFlag =  config.value("enableGetQraFromQrz", false).toBool();
    config.endGroup();

    return useQrzFlag;
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

DxSpotSortFilterProxyModel::DxSpotSortFilterProxyModel(ClusterClientFilterSettings *filterSettings_)
{
    filterSettings = filterSettings_;
}

bool DxSpotSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
    bool match_band = matchBand(sourceRow);

    if (traceDebugFlag)
    {
        trace(QString("filterAcceptsRow: callsign = %1, matchBand = %2")
            .arg(sourceModel()->data(sourceModel()->index(sourceRow, DXSPOT_CALL_COL_NUM), DataStoredRole).toString(),
            match_band ? "True" : "False"));
   }
    return match_band;
}

bool DxSpotSortFilterProxyModel::matchBand(int sourceRow) const
{

    QString band = sourceModel()->data(sourceModel()->index(sourceRow, DXBANDSTR_COL_NUM), DataStoredRole).toString();
    if (traceDebugFlag)
    {
        trace(QString("matchBand: band = %1").arg(band));
    }

    return filterSettings->getBandFilter(band);

}





/****************************** Test Routine *********************************/

#ifdef TEST_SPOTS


void ClusterMainWindow::testSpotPbClicked()
{
    testSpotList.clear();

    // get list of test spots from file
    if (FileExists(CLUSTER_PATH() + CLUSTER_SPOT_TEST_FILE))
    {
        QFile inputFile(CLUSTER_PATH() + CLUSTER_SPOT_TEST_FILE);
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

    if (toInt(setupCluster->getTimeToLive()) > 0 /*&& !holdUpdateFlag && (ct && ct == TContestApp::getContestApp()->getCurrentContest())*/)      // don't purge spots if == 0 and holdupdateflag is on
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
    QStringList splitSpot;
    QTime time;
    QString timeStr;
    const QRegularExpression TIME = QRegularExpression("\\d\\d\\d\\dZ");

    if (spotNum >= testSpotList.count())
    {
        spotTestTimer->stop();
        return;
    }
    if (!testSpotList.isEmpty())
    {
        spot = testSpotList[spotNum].remove('\n');
        splitSpot = spot.split(TIME);

        if (splitSpot.count() == 2)
        {
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
            timeStr = QString("   %1%2Z").arg(hourStr, minStr);
            spot = splitSpot[0].append(timeStr).append(splitSpot[1]).append('\n');
        }

        parseDX(spot);
        spotNum++;

    }

}


#endif

void ClusterMainWindow::on_setupButton_clicked()
{
    onLaunchSetup();
}


void ClusterMainWindow::on_clearSpots_clicked()
{
    onClearAllSpots();
}


void ClusterMainWindow::on_shortcuts_clicked()
{
    clusterNodeCommandsShortcutHelp();
}

