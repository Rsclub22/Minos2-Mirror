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
    Q_UNUSED(rpc);

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


    initUserCommandButtons();
    readUserCommandStrings();
    userCommandAllButtonUpdate();
    // spotTimeToLive
    setupCluster->readGeneralSettings();

    //QString spotLoc;
    //QString dxLoc;
    //QString comment = QString("IO91SN tr ");
    //findLocInComment(spotLoc, dxLoc, comment);


    dxSpotDataModel = new DxSpotDataModel();


    dxSpotView = new QTableView();
    dxSpotView->setModel(dxSpotDataModel);
    dxSpotView->setAlternatingRowColors(true);
    dxSpotView->setSelectionMode( QAbstractItemView::NoSelection );
    //dxSpotView->setStyleSheet("QHeaderView::section { font: bold; height: 14px }");
    dxSpotView->setColumnHidden(DXSPOT_CALL_WORKED_COL_NUM, true);
    dxSpotView->setColumnHidden(DXLOC_WORKED_COL_NUM, true);
    dxSpotView->setColumnHidden(DXBANDMASK_COL_NUM, true);
    dxSpotView->setColumnHidden(MODEMASK_COL_NUM, true);
    dxSpotView->setColumnWidth(TIME_COL_NUM, TIME_COL_WIDTH);
    dxSpotView->setColumnWidth(FREQ_COL_NUM, FREQ_COL_WIDTH);
    dxSpotView->setColumnWidth(DXSPOT_CALL_COL_NUM, DXSPOT_CALL_COL_WIDTH);
    dxSpotView->setColumnWidth(DXLOC_COL_NUM, DXLOC_COL_WIDTH);
    dxSpotView->setColumnWidth(SPOT_CALL_COL_NUM, SPOT_CALL_COL_WIDTH);
    dxSpotView->setColumnWidth(SPOTLOC_COL_NUM, SPOTLOC_COL_WIDTH);
    dxSpotView->setColumnWidth(COMMENT_COL_NUM, COMMENT_COL_WIDTH);


    QHeaderView *verticalHeader = dxSpotView->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(18);

    connect( dxSpotView->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( on_sectionResized(int, int , int)));




    restoreDxSpotViewColumns();

    rawClusterDataView = new QPlainTextEdit();

    ui->clusterViewsTab->addTab(dxSpotView, "DX Spots");
    ui->clusterViewsTab->addTab(rawClusterDataView, "Raw Data");

    setAllTabsColor(CLUSTER_TAB_NOT_SELECT_COLOR);
    ui->clusterViewsTab->setTabColor(ui->clusterViewsTab->currentIndex(), CLUSTER_TAB_SELECT_COLOR);
    connect(ui->clusterViewsTab, SIGNAL(currentChanged(int)), this, SLOT(onSpotTabChanged(int)));

    connect(ui->actionSetup, SIGNAL(triggered()), this, SLOT(onLaunchSetup()));

    connect(ui->nodeCb, SIGNAL(activated(QString)), this, SLOT(connectToNode(QString)));


    connect(client, SIGNAL(socketConnected()), this, SLOT(connectionEstab()));
    connect(client, SIGNAL(loginRequired()), this, SLOT(logIn()));
    connect(client, SIGNAL(connectionError(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
    connect(client, SIGNAL(loggedOut()), this, SLOT(loggedOut()));
    connect(client, SIGNAL(message(QString)), this, SLOT(messageRx(QString)));
    connect(client, SIGNAL(message(QString)), this, SLOT(parseDX(QString)));
    connect(client, SIGNAL(message(QString)), this, SLOT(checkedLoggedIn(QString)));
    //connect(ui->sendLine, SIGNAL(returnPressed()), this, SLOT(sendText()));

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
            setupCluster->saveCurrentNodeName(currentNodeName);
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

            trace(QString("Parse DX de %1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12")
            .arg(dxCall).arg(dxFreq).arg(dxBandStr).arg(dxBandMask).arg(dxModeStr).arg(dxModeMask).arg(spotCall).arg(dxLocator).arg(spotLocator).arg(spotTime).arg(spotComment).arg(setupCluster->getTimeToLive()));
            // Display
            QString displayFreq = alignFreqRight(dxFreq);
            clusterRpc->sendDXSpot(QString("%1:%2:%3:%4:%5:%6:%7:%8:%9:%10:%11:%12").arg(dxCall).arg(dxLocator).arg(dxFreq).arg(dxBandStr).arg(dxBandMask).arg(dxModeStr).arg(dxModeMask).arg(spotCall).arg(spotLocator).arg(spotTime).arg(spotComment).arg(setupCluster->getTimeToLive()));
            dxSpotDataModel->rowData = new SpotData(spotTime, displayFreq, dxBandMask, dxModeMask, dxCall, dxLocator, spotCall, spotLocator, spotComment);
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
    spotLocator = "";

    txt.remove('\x07');
    if (!txt.contains("DX de"))
    {
        return -2;
    }

    dxMsg = txt.split(QRegExp("\\s+"));
    spotCall = dxMsg[2].remove(':');
    dxFreq = convertKhzToMhz(dxMsg[3]);
    getBand(dxFreq, dxBandStr, dxBandMask);
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

    // look for locator
    if (dxMsg[timePos + 1] == "")
    {
        spotLocator = "";
    }
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


void ClusterMainWindow::findLocInComment(QString &spotLoc, QString &dxLoc, const QString &comment)
{
    QStringList loc;
    trace(QString("Extract locators - comment = %1").arg(comment));
    const QRegExp fullLocExp = FULL_LOC_EXP;
    const QRegExp partLocExp = PART_LOC_EXP;

    for (int i = 0; i < locatorSeperators.count(); i++)
    {
        trace(QString("Exract locators - looks for seperator %1").arg(locatorSeperators[i]));
        if (comment.contains(locatorSeperators[i], Qt::CaseInsensitive))
        {
            trace(QString("Exract locators - found seperator %1").arg(locatorSeperators[i]));
            loc = comment.split(locatorSeperators[i], QString::KeepEmptyParts, Qt::CaseInsensitive);
            spotLoc = extractLocator(loc[0], fullLocExp, partLocExp);
            dxLoc = extractLocator(loc[1], fullLocExp, partLocExp);
            trace(QString("Extracted dxLoc = %1 spotLoc= %2").arg(dxLoc).arg(spotLoc));
            return;
        }


    }


    if (comment.contains(fullLocExp))      // look for a single locator, which we assume is DX locator
    {
        trace(QString("Look for dxLoc only"));
        dxLoc = extractLocator(comment, fullLocExp, partLocExp);
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
    if (connected)
    {
       client->sendData(msg);
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

        userCmdButton.append(new RotPresetButton(ui_userCommandButtons[i], i, shortCutKeyList[i], shiftShortCutKeyList[i]));

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
                    txText(userCommands[buttonNumber]);
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

