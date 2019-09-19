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
#include <QHeaderView>
#include <QTextStream>
#include <QDebug>

#include "clustermainwindow.h"
#include "clustercommon.h"
#include "rigutils.h"
#include "cutils.h"
#include "ui_clustermainwindow.h"

#include <QDebug>

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

    connect(&stdinReader, SIGNAL(stdinLine(QString)), this, SLOT(onStdInRead(QString)));
    stdinReader.start();

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    appName = env.value("MQTRPCNAME", "") ;

    trace(QString("AppName = %1").arg(appName));
    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName());
    Q_UNUSED(rpc);

    createCloseEvent();

    disconnectTimer = new QTimer();
    connect(disconnectTimer, SIGNAL(timeout()), this, SLOT(disconnectTimeout()));

    connect(&LogTimer, SIGNAL(timeout()), this, SLOT(LogTimerTimer()));
    LogTimer.start(100);

    spotsList.clear();
    getSpotsTimer = new QTimer();
    connect(getSpotsTimer, SIGNAL(timeout()), this, SLOT(getSpotsFromQueue()));
    getSpotsTimer->start(1000);


    setWindowTitle("Minos Cluster Server");
    status = new QLabel;
    ui->statusBar->addWidget(status);

    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));

    loadVhfAndUpBands(bands);

    modeBandPlan = new checkModeAgainstFreq();
    if (modeBandPlan->loadFile("./Configuration/mode_bandplan.json"))
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

    sendSpotsTimer = new QTimer();
    connect(sendSpotsTimer, SIGNAL(timeout()), this, SLOT(getSpotsFromSendQueue()));
    sendSpotsTimer->start(SEND_SPOTS_DUR);

    client = new QtTelnet(parent);
    dxCluster = new Cluster();

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

    // read enable hf spots flag
    QString fileName = CLUSTER_SETTINGS_FILE;
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("HFSpots");
    enableHFSpots = config.value("enable", false).toBool();
    config.endGroup();


    dxSpotDataModel = new DxSpotDataModel();


    dxSpotView = new QTableView();

    delegate = QSharedPointer<HtmlDelegate>( new HtmlDelegate(1.0, 1.0)) ;

    dxSpotDataModel->delegate = delegate;

    dxSpotProxyModel = new QSortFilterProxyModel();
    dxSpotProxyModel->setSourceModel(dxSpotDataModel);
    dxSpotProxyModel->sort(RXTIME_COL_NUM, Qt::DescendingOrder);

    dxSpotView->setModel(dxSpotProxyModel);
    dxSpotView->setAlternatingRowColors(true);
    dxSpotView->setSelectionMode( QAbstractItemView::NoSelection );
    dxSpotView->setItemDelegate(delegate.data());




    QHeaderView *verticalHeader = dxSpotView->verticalHeader();
    verticalHeader->setVisible(false);
    verticalHeader->setDefaultSectionSize(10);
    verticalHeader->setMinimumSectionSize(10);

    verticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);

    restoreDxSpotViewColumns();
    dxSpotView->horizontalHeader()->setStretchLastSection(true);
    connect( dxSpotView->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( on_sectionResized(int, int , int)));


    dxSpotView->setColumnHidden(DXBRG_COL_NUM, true);
    dxSpotView->setColumnHidden(DXBANDMASK_COL_NUM, true);
    //dxSpotView->setColumnHidden(DXSPOT_MODE_COL_NUM, true);
    dxSpotView->setColumnHidden(DXMODEMASK_COL_NUM, true);
    dxSpotView->setColumnHidden(DXSPOT_TO_MEMORY_FLAG_COL_NUM, true);
    dxSpotView->setColumnHidden(DXSPOT_CALL_WORKED_COL_NUM, true);
    dxSpotView->setColumnHidden(DXLOC_WORKED_COL_NUM, true);
    dxSpotView->setColumnHidden(DXDIST_COL_NUM, true);
    dxSpotView->setColumnHidden(RXTIME_COL_NUM, true);


    rawClusterDataView = new QPlainTextEdit();
    rawClusterDataView->setReadOnly(true);

    ui->clusterViewsTab->addTab(dxSpotView, "DX Spots");
    ui->clusterViewsTab->addTab(rawClusterDataView, "Raw Data");

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
    //connect(client, SIGNAL(message(QString)), this, SLOT(checkStationDetails(QString)));
    //connect(ui->sendLine, SIGNAL(returnPressed()), this, SLOT(sendText()));

    statusTimer = new QTimer(this);
    connect(statusTimer, SIGNAL(timeout()), this, SLOT(handleStatusTimer()));
    statusTimer->start(STATUS_TIMER_DUR);

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


}



void ClusterMainWindow::clusterListChanged()
{
    loadNodesSelectBox(setupCluster->getListOfClusterNames());
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
            showStatusMessage(QString("Disconnect Timeout"));
            QString msg = QString("Connect to Node - Disconnect Timeout");
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
    showStatusMessage(QString("Connected to: %1 %2 %3").arg(currentNodeName).arg(currentAddress).arg(currentPort));
    QString msg = QString("Connection Established with host %1 %2:%3").arg(currentNodeName).arg(currentAddress).arg(currentPort);
    trace(QString(msg));
    echoMsg(msg);

}

void ClusterMainWindow::connectionError(QAbstractSocket::SocketError error)
{
    nodeConnected = false;
    showStatusMessage(QString("Connection Error: Error Code %1").arg(QString::number(error)));
    QString msg = QString("Connection failed error %1").arg(error);
    trace(msg);
    echoErrorMsg(msg);
}



void ClusterMainWindow::logIn()
{
    QString msg = QString("Login Start - Send logon message\n");
    trace(msg);
    echoMsg(msg);
    client->login(QString("%1\r\n").arg(currentUserCallsign), currentPassword);
    loginStart = true;
    echoMsg(QString("Logging in with callsign %1").arg(currentUserCallsign));

}

void ClusterMainWindow::loggedOut()
{
    QString msg = QString("Logged Out of node  %1").arg(currentNodeName);
    trace(QString(msg));
    echoErrorMsg(msg);
    nodeConnected = false;
    loginStart = false;
    loginSuccess = false;
    loginStatDetails = false;
    showStatusMessage((QString("Disconnected")));
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
        QString msg = dxCluster->quit();
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
            txText(dxCluster->setNameMsg(currentUserName));
            txText(dxCluster->setQthMsg(currentUserQTH));
            txText(dxCluster->setQraMsg(currentUserLocator));


            //txText("SH/ST\n");      // ask for station details
            //loginStatDetails = true;


            if (setupCluster->getRunStartFileFlag())
            {
                handleStartFile();          // send user commands
            }

        }

    }


}


// revisit this to improve login process???

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
        txText(dxCluster->setNameMsg(currentUserName));
    }
    else if (!foundMatch[1])
    {
        txText(dxCluster->setQthMsg(currentUserQTH));
    }
    else if (!foundMatch[2])
    {
        txText(dxCluster->setQraMsg(currentUserLocator));
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
        QString msg = QString("handleCmdFile: Command file found - %1").arg(fileName);
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
        QString msg = QString("handleCmdFile: Command File missing - %1!").arg(fileName);
        trace(msg);
        echoErrorMsg(msg);
        return;
    }

    if (!listCmds.isEmpty())
    {
        QString msg = QString("handleCmdFile: Sending Commands");
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

        msg = QString("handleCmdFile: Finished sending Commands");
        trace(msg);
        echoMsg(msg);

    }
    else
    {
        QString msg = QString("handleCmdFile: Command file empty %1").arg(fileName);
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

    int retCode = -100;
    spotCall = "";
    QString line;
    if (loginSuccess)
    {
        trace(QString("raw spot = %1").arg(txt));

        do
        {
            line = in.readLine();
            if (!line.isEmpty())
            {
                if (line.contains("DX de"))
                {
                   retCode = upackDxSpot(line, spotCall);
                   trace(QString("ParseDx - Unpack DxSpot retcode = %1").arg(retCode));

                }
                else if (checkShowDxMsg(line, spotCall))
                {
                    retCode = upackShowDxSpot(line, spotCall);
                    trace(QString("ParseDx - Unpack ShowDxSpot retcode = %1").arg(retCode));
                }



                if (retCode >= 0)
                {
                    trace(QString("Parse DX de %1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12 %13 %14")
                    .arg(dxCall).arg(dxFreq).arg(dxBandStr).arg(dxBandMask).arg(dxModeStr).arg(dxModeMask)
                    .arg(spotCall).arg(dxLocator).arg(spotLocator).arg(dxPropMode).arg(spotTime).arg(spotDate).arg(spotComment).arg(setupCluster->getTimeToLive()));

                    qint64 rxTime = spotDateTime.toMSecsSinceEpoch()/1000;

                    // is spot older than time to live time
                    int timeToLive = setupCluster->getTimeToLive().toInt() * 60;
                    if (timeToLive == 0 || (timeToLive > 0 && !spotTimedOut(rxTime, timeToLive)))
                    {
                        trace(QString("ParseDx: Spot within timeToLive - Send Spot to Queue"));
                        sendSpotsQueue.append(createSpotToSend(QString("%1:%2:%3:%4:%5:%6:%7:%8:%9:%10:%11:%12:%13:%14").arg(dxCall).arg(dxLocator).arg(dxFreq).arg(dxBandStr).arg(dxBandMask).arg(dxModeStr).arg(dxModeMask).arg(spotCall).arg(spotLocator).arg(spotTime).arg(spotDate).arg(spotComment).arg(dxPropMode).arg(setupCluster->getTimeToLive())));

                        trace(QString("ParseDx: rxTime = %1").arg(rxTime));
                        trace(QString("ParseDx: Add spot for display"));
                        spotsList += (new SpotData(rxTime, spotTime,
                                                      dxFreq, dxBandStr, dxBandMask,
                                                      dxModeStr, dxModeMask,
                                                      dxCall, false, dxLocator,
                                                      false, "",
                                                      "", spotCall,
                                                      spotLocator, dxPropMode, spotComment));

                    }
                    else
                    {
                       trace(QString("ParseDx: Spot older than time to live time = %1 mins").arg(timeToLive/60));
                    }



                }
                else if (retCode == -100)
                {
                    trace(QString("ParseDx - Not a valid spot retcode = %1").arg(retCode));
                }
            }
        } while (!line.isNull());
    }

    trace(QString("ParseDx: Finished"));
}







int ClusterMainWindow::upackShowDxSpot(const QString txt, const QString _spotCall)
{
    spotCall = _spotCall;

    // clear the rest of spot data
    dxCall = "";
    dxFreq = "";
    dxBandStr = "";
    dxBandMask = "";
    dxModeStr = "";
    dxModeMask = "";

    spotComment = "";
    spotTime = "";
    spotDate = "";
    spotDateTime = QDateTime::currentDateTimeUtc();
    dxLocator = "";
    spotLocator = "";
    dxPropMode = "";

    dxMsg = txt.split(QRegExp("\\s+"), QString::SkipEmptyParts);

    if (dxMsg.count() > 4)
    {
        //dxFreq = convertKhzToMhz(dxMsg[0]);
        QString f = dxMsg[0] + "00";
        f.remove('.');
        dxFreq = convertFreqStrDisp(f);
        getBand(bands, dxFreq, dxBandStr, dxBandMask);
        if (dxBandStr.isEmpty() && !enableHFSpots)
        {
            // discard spot as it is HF
            trace(QString("Unpack Show DX Spot: Discard Spot HF = %1").arg(dxFreq));
            return -3;
        }

        getMode(modeBandPlan, dxFreq, dxBandStr, dxModeStr, dxModeMask);

        dxCall = dxMsg[1];
        spotDate = dxMsg[2];
        spotTime = dxMsg[3].remove('Z');
        spotDateTime = getSpotDateTime(spotDate, spotTime);
        if (!spotDateTime.isValid())
        {
           return -1;
        }
        QString sptCall = spotCall;
        sptCall.prepend('<').append('>');
        // reassemble comment
        for (int i = 4; i < dxMsg.indexOf(sptCall); i++)
        {
            if (dxMsg[i] != "")
            {
                spotComment += dxMsg[i] + " ";
            }
        }

        findLocInComment(spotLocator, dxLocator, spotComment);
        dxPropMode = getPropMode(spotComment);
        return 0;
    }

    return -1;

}


bool ClusterMainWindow::checkShowDxMsg(const QString txt, QString &spotCall)
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
                spotCall = extractStr[i];
                return true;

            }
        }
    }

    return false;

}






QString ClusterMainWindow::createSpotToSend(QString spot)
{
    return DXSPOT + spot;
}

QString ClusterMainWindow::createStatusToSend(QString status)
{
    return CLUSTER_STATUS + status;
}

void ClusterMainWindow::getSpotsFromSendQueue()
{
    if (!sendSpotsQueue.isEmpty())
    {
        // get spots from queue and send to client
        while (sendSpotsQueue.count() > 0)
        {
            trace(QString("Sending spot from send queue, queue length = %1, spot = %2").arg(sendSpotsQueue.count()).arg(sendSpotsQueue[0]));
            clusterRpc->sendDXSpot(sendSpotsQueue[0]);
            sendSpotsQueue.removeFirst();
        }
    }

}





// this is the queue of spots for display
void ClusterMainWindow::getSpotsFromQueue()
{
    if (!spotsList.isEmpty())
    {
        trace(QString("GetSpotsFromQueue: spots available = %1").arg(spotsList.count()));
        // get spots from queue
        int slsize= spotsList.count();
        for (int i = slsize -1 ; i > -1; i--)
        {

            if (purgeSpotFlag)
            {
                trace(QString("GetSpotsFromQueue: PurgeFlag On"));
                return;
            }

            dxSpotDataModel->rowData = spotsList[i];
            spotsList.remove(i);
            //dxSpotDataModel->insertRows(0, 1);
            dxSpotDataModel->insertRows(dxSpotDataModel->rowCount(), 1);
            trace(QString("GetSpotsFromQueue: finished loop"));


        }

        trace(QString("GetSpotsFromQueue: finished"));
    }
}


int ClusterMainWindow::upackDxSpot(QString txt, QString &spotCall)
{


    int timePos = 0;

    // clear spot data
    dxCall = "";
    dxFreq = "";
    dxBandStr = "";
    dxBandMask = "";
    dxModeStr = "";
    dxModeMask = "";
    //spotCall = "";
    spotComment = "";
    spotTime = "";
    spotDate = "";
    spotDateTime = QDateTime::currentDateTimeUtc();
    dxLocator = "";
    spotLocator = "";
    dxPropMode = "";

    txt.remove('\x07');
    //if (!txt.contains("DX de"))
    //{
    //    return -2;
    //}

    dxMsg = txt.split(QRegExp("\\s+"));

    if (dxMsg.count() > 5)
    {
        spotCall = dxMsg[2].remove(':');
        QString f = dxMsg[3] + "00";
        f.remove('.');
        //dxFreq = convertKhzToMhz(dxMsg[3]);
        dxFreq = convertFreqStrDisp(f);
        getBand(bands, dxFreq, dxBandStr, dxBandMask);
        if (dxBandStr.isEmpty() && !enableHFSpots)
        {
            // discard spot as it is HF
            trace(QString("Unpack DX Spot: Discard Spot HF = %1").arg(dxFreq));
            return -3;
        }

        getMode(modeBandPlan, dxFreq, dxBandStr, dxModeStr, dxModeMask);

        dxCall = dxMsg[4];
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

        // get current date
        QDate d = QDate::currentDate();
        spotDate = d.toString("dd-MMM-yyyy");
        spotDateTime = getSpotDateTime(spotDate, spotTime);
        if (!spotDateTime.isValid())
        {
           return -1;
        }

        // look for locator
        if (timePos + 1 >= dxMsg.count())  // make sure not out of range
        {
            // no spotlocator sent
            spotLocator = "";
        }
        //else if (dxMsg[timePos + 1] == "")
        //{
        //    spotLocator = "";
        //}
        else
        {
            spotLocator = dxMsg[timePos + 1];
        }
        // reassemble the comment
        for (int i = 5; i < timePos; i++)
        {
            if (dxMsg[i] != "")
            {
                spotComment += dxMsg[i] + " ";
            }
        }

        // remove seperator char from comment
        spotComment.remove(SPOT_DATA_SEPERATOR);
        findLocInComment(spotLocator, dxLocator, spotComment);
        dxPropMode = getPropMode(spotComment);


        return 0;
    }

    return -1;

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


QString ClusterMainWindow::extractLocator(const QString &text, const QRegExp fullLocExp, const QRegExp partLocExp)
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


// ************* Send text *************************************************

void ClusterMainWindow::sendText()
{
    //client->sendData(ui->sendLine->text()+'\n');
    //ui->sendLine->clear();
}


void ClusterMainWindow::txText(QString msg)
{
    if (loginSuccess)
    {
       client->sendData(msg);
       echoCmd(msg);

    }
    else
    {
        QString err = "Sending command - Not logged in  - " + msg;
        echoErrorMsg(err);
    }

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

    disconnectNode();

    trace("Minos Cluster Server Closing");
    QWidget::closeEvent(event);
}


void ClusterMainWindow::onStdInRead(QString cmd)
{
    trace("Command read from stdin: " + cmd);
    if (cmd.indexOf("ShowServers", 0, Qt::CaseInsensitive) >= 0)
        setShowServers(true);
    if (cmd.indexOf("HideServers", 0, Qt::CaseInsensitive) >= 0)
        setShowServers(false);
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




    for (int i = 0; i < ui_userCommandButtons.count(); i++)
    {

        userCmdButton.append(new RotPresetButton(ui_userCommandButtons[i], i, shortCutKeyList[i], shiftShortCutKeyList[i], userCmdButtonLabels));

        connect(userCmdButton[i], &RotPresetButton::presetShortCutRecall, [this, i]() {userCmdButtonRead(i);});
        connect(userCmdButton[i], &RotPresetButton::presetShiftShortCutRecall, [this, i]() {showUserCmdButtonMenu(i);});
        connect(userCmdButton[i], &RotPresetButton::presetReadAction, [this, i]() {userCmdButtonRead(i);});
        connect(userCmdButton[i], &RotPresetButton::presetEditAction, [this, i]() {userCmdButtonEdit(i);});
        connect(userCmdButton[i], &RotPresetButton::presetWriteAction, [this, i]() {userCmdButtonWrite(i);});
        connect(userCmdButton[i], &RotPresetButton::presetClearAction, [this, i]() {userCmdButtonClear(i);});


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
                    d[1].append('\n');
                    trace(QString("UserCmdButton Read - Send Command to cluster = %1").arg(d[1]));
                    txText(d[1]);
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

void ClusterMainWindow::showStatusMessage(const QString &message)
{
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
          clusterRpc->publishState(status->text());
    }

    // send status message if it has changed
    else if (!status->text().isEmpty()  && clusterRpc->getServerListCount() > 0)
    {
        if (oldStatusMsg != status->text())
        {
            oldStatusMsg = status->text();

            // send status to clients
            trace(QString("handleStatusTimer: Send Status to Cluster Clients - %1").arg(status->text()));
            //sendSpotsQueue.append(createStatusToSend(status->text()));
            clusterRpc->publishState(status->text());
        }
    }

}




void ClusterMainWindow::about()
{
    QMessageBox::about(this, "Minos Cluster Server", "Minos Rotator\nCopyright D Balharrie G8FKH/M0DGB 2016 - 2019");
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
