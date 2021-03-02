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


#include "setupdialog.h"
#include "ui_setupdialog.h"
#include "cutils.h"
#include "clustercommon.h"
#include "CallsignLineEdit.h"


#include <QSettings>


SetupDialog::SetupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetupDialog),
    listDataChanged(false),
    timeToLiveChanged(false),
    runStartCmdFilesChanged(false),
    enableStartCmdFiles(false),
    runEndCmdFilesChanged(false),
    enableEndCmdFiles(false),
    sendSpotToDXCluster(false),
    sendSpotsToDXClusterChanged(false),
    personalDataChanged(false),
    bandFilterOnSaveFlag(false),
    bandFilterOnSaveChanged(false),
    useQrzForQraFlag(false),
    useQrzForQraChanged(false)
{
    ui->setupUi(this);

    QSettings settings;
    QByteArray geometry = settings.value("ClusterSetup/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // General Tab
    connect(ui->timeToLive, &QLineEdit::editingFinished, [=](){timeToliveEditFinished();});
    connect(ui->runStartCmdFileChkBox, &QCheckBox::stateChanged, [=](int state){runStartCmdFileChkBoxChanged(state);});
    connect(ui->runEndCmdFileChkBox, &QCheckBox::stateChanged, [=](int state){runEndCmdFileChkBoxChanged(state);});
    connect(ui->sendSpotsToDXClusterChkBox, &QCheckBox::stateChanged, [=](int state){sendSpotsToDXClusterChkBoxChanged(state);});
    connect(ui->saveBandFilterSettingChkBox, &QCheckBox::stateChanged, [=](int state){onSaveBandFilterChkBoxClicked(state);});
    connect(ui->useQrzCheckBox, &QCheckBox::stateChanged, [=](int state){onQrzCheckBoxChkBoxClicked(state);});
    readGeneralSettings();
    loadGeneralToSetupTab();

    // Personal Tab
    ui->callsignEdit->setValidator(&ucValidator);
    ui->locatorEdit->setValidator(&ucValidator);

    connect(ui->callsignEdit, SIGNAL(callsignFinished(const QString&)), this, SLOT(callsignFinished(const QString&)));
    connect(ui->nameEdit, SIGNAL(editingFinished()), this, SLOT(nameEditFinshed()));
    connect(ui->locatorEdit, SIGNAL(locatorFinished(const QString&)), this, SLOT(locatorFinished(const QString&)));
    connect(ui->qthEdit, SIGNAL(editingFinished()), this, SLOT(qthEditFinished()));


    //readPersonal();
    //loadPersonalToSetupTab();



    // Cluster Node List Tab
    loadClusterListToModel();       // this also creates the model
    connect(ui->clustersListAddPbutton, SIGNAL(clicked()), this, SLOT(addClusterNode()));
    connect(ui->clustersListDelPbutton, SIGNAL(clicked()), this, SLOT(deleteClusterNode()));

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(saveButtonPushed()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(cancelButtonPushed()));

    connect(clusterListModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(clusterListDataChanged(QStandardItem*)));

    ui->displayModeCheckBox->setVisible(false);
    ui->displayPropCheckBox->setVisible(false);

}





SetupDialog::~SetupDialog()
{
    delete ui;
}



void SetupDialog::closeEvent (QCloseEvent *event)
{

    cancelButtonPushed();
    doCloseEvent();
    QWidget::closeEvent(event);
}


void SetupDialog::doCloseEvent()
{

    QSettings settings;
    settings.setValue("ClusterSetup/geometry", saveGeometry());
}



void SetupDialog::saveButtonPushed()
{

    clusterListSave();
    savePersonal();
    saveGeneralSettings();
    doCloseEvent();

}


void SetupDialog::cancelButtonPushed()
{


    // load back the data to model
    clusterListModel->clear();      // also clears rows and columns

    QString fileName = CLUSTER_NODE_LIST_FILE;
    QSettings settings(fileName, QSettings::IniFormat);
    QStringList availNodeNames = settings.childGroups();
    clusterListModel->setRowCount(availNodeNames.count());      // restore model row and col
    clusterListModel->setColumnCount(ClusterListNumCols);
    clusterListModel->setHeaderData(NameColNum, Qt::Horizontal, QObject::tr("Name"));
    clusterListModel->setHeaderData(AddressColNum, Qt::Horizontal, QObject::tr("Address"));
    clusterListModel->setHeaderData(PortColNum, Qt::Horizontal, QObject::tr("Port"));
    clusterListModel->setHeaderData(PasswdColNum, Qt::Horizontal, QObject::tr("Password"));
    loadSettingsToModel(availNodeNames, settings);

    readPersonal();
    loadPersonalToSetupTab();

    readGeneralSettings();
    loadGeneralToSetupTab();
    doCloseEvent();



}


void SetupDialog::timeToliveEditFinished()
{
    QString ttl = ui->timeToLive->text().trimmed();
    if (ttl != timeToLive)
    {
        bool ok = false;
        int ittl = ttl.toInt(&ok);
        if (ok)
        {
            if (ittl < MIN_TTL || ittl > MAX_TTL)
            {
                QMessageBox msgBox;
                msgBox.setText(tr("%1 minutes has been entered\nThe time to live must be between %2 minutes and %3 minutes").arg(ui->timeToLive->text()).arg(MIN_TTL).arg(MAX_TTL));
                msgBox.exec();
                ui->timeToLive->setText(timeToLive);  // restore setting
                return;
            }
            timeToLiveChanged = true;
        }
    }
}

void SetupDialog::saveGeneralSettings()
{
    if (timeToLiveChanged || runStartCmdFilesChanged
        || runEndCmdFilesChanged || sendSpotsToDXClusterChanged
        || bandFilterOnSaveChanged || useQrzForQraChanged)
    {
        timeToLive = ui->timeToLive->text().trimmed();
        QString fileName = CLUSTER_SETTINGS_FILE;

        QSettings config(fileName, QSettings::IniFormat);

        if (timeToLiveChanged)
        {
            config.beginGroup("TimeToLive");
            config.setValue("timeToLive", timeToLive);
            config.endGroup();
            timeToLiveChanged = false;
        }

        if (runStartCmdFilesChanged)
        {
            config.beginGroup("CommandFile");
            config.setValue("enableStartCommandFile", enableStartCmdFiles);
            config.endGroup();
            runStartCmdFilesChanged = false;
        }

        if (runEndCmdFilesChanged)
        {
            config.beginGroup("CommandFile");
            config.setValue("enableEndCommandFile", enableEndCmdFiles);
            config.endGroup();
            runEndCmdFilesChanged = false;
        }

        if (sendSpotsToDXClusterChanged)
        {
            config.beginGroup("EnableSendSpotsToDXCluster");
            config.setValue("enableSendToDXCluster", sendSpotToDXCluster);
            config.endGroup();
            emit sendSpotToTxEnabled(sendSpotToDXCluster);
            sendSpotsToDXClusterChanged = false;
        }

        if (bandFilterOnSaveChanged)
        {
            config.beginGroup("General");
            config.setValue("bandFilterSaveOnClose", bandFilterOnSaveFlag);
            config.endGroup();
        }
        if (useQrzForQraChanged)
        {
            config.beginGroup("UseQRZServer");
            config.setValue("enableGetQraFromQrz", useQrzForQraFlag);
            config.endGroup();
        }

    }
}


void SetupDialog::createDefaultGeneralSettingsFile()
{

    QString fileName = CLUSTER_SETTINGS_FILE;
    QSettings config(fileName, QSettings::IniFormat);

    config.beginGroup("Personal");
    config.setValue("Callsign", "");
    config.setValue("Name", "");
    config.setValue("Locator", "");
    config.setValue("Qth", "");
    config.endGroup();

    config.beginGroup("TimeToLive");
    config.setValue("timeToLive", 30);
    config.endGroup();

    config.beginGroup("HFSpots");
    config.setValue("enable", false);
    config.endGroup();

    config.beginGroup("CommandFile");
    config.setValue("enableCommandFile", false);
    config.setValue("enableStartCommandFile", false);
    config.setValue("enableEndCommandFile", false);
    config.endGroup();
    config.beginGroup("EnableSendSpotsToDXCluster");
    config.setValue("enableSendToDXCluster", false);
    config.endGroup();
    config.beginGroup("UseQRZServer");
    config.setValue("enableGetQraFromQrz", false);
    config.endGroup();

}

void SetupDialog::readGeneralSettings()
{
    QString fileName = CLUSTER_SETTINGS_FILE;
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("TimeToLive");
    timeToLive = config.value("timeToLive", "").toString();
    if (timeToLive == "0")      // 0 as no purge removed.
    {
        timeToLive = "10";
    }
    config.endGroup();
    config.beginGroup("CommandFile");
    enableStartCmdFiles = config.value("enableStartCommandFile", false).toBool();
    enableEndCmdFiles = config.value("enableEndCommandFile", false).toBool();
    config.endGroup();
    config.beginGroup("EnableSendSpotsToDXCluster");
    sendSpotToDXCluster = config.value("enableSendToDXCluster", false).toBool();
    config.endGroup();
    config.beginGroup("General");
    bandFilterOnSaveFlag = config.value("bandFilterSaveOnClose", true).toBool();
    config.endGroup();
    config.beginGroup("UseQRZServer");
    useQrzForQraFlag =  config.value("enableGetQraFromQrz", false).toBool();
    config.endGroup();

}




void SetupDialog::loadGeneralToSetupTab()
{

    ui->timeToLive->setText(timeToLive);
    ui->runStartCmdFileChkBox->setChecked(enableStartCmdFiles);
    ui->runEndCmdFileChkBox->setChecked(enableEndCmdFiles);
    ui->sendSpotsToDXClusterChkBox->setChecked(sendSpotToDXCluster);
    ui->saveBandFilterSettingChkBox->setChecked(bandFilterOnSaveFlag);
    ui->useQrzCheckBox->setChecked(useQrzForQraFlag);

}




QString SetupDialog::getTimeToLive()
{
    return timeToLive;
}



void SetupDialog::onSaveBandFilterChkBoxClicked(int state)
{
    if (state == Qt::Checked)
    {
        if (!bandFilterOnSaveFlag)
        {
           bandFilterOnSaveFlag = true;
           bandFilterOnSaveChanged = true;
        }
    }
    else if (state == Qt::Unchecked)
    {
        if (bandFilterOnSaveFlag)
        {
            bandFilterOnSaveFlag = false;
            bandFilterOnSaveChanged = true;
        }
    }
}

void SetupDialog::onQrzCheckBoxChkBoxClicked(int state)
{
    if (state == Qt::Checked)
    {
       if (!useQrzForQraFlag)
       {
           useQrzForQraFlag = true;
           useQrzForQraChanged = true;
       }
    }
    else if (state == Qt::Unchecked)
    {
        useQrzForQraFlag = false;
        useQrzForQraChanged = true;
    }

}


void SetupDialog::runStartCmdFileChkBoxChanged(int state)
{

    if (state ==  Qt::Checked)
    {
        if (!enableStartCmdFiles)
        {
            enableStartCmdFiles = true;
            runStartCmdFilesChanged = true;
        }
    }
    else if (state == Qt::Unchecked)
    {
        if (enableStartCmdFiles)
        {
            enableStartCmdFiles = false;
            runStartCmdFilesChanged = true;
        }
    }
}


void SetupDialog::runEndCmdFileChkBoxChanged(int state)
{

    if (state ==  Qt::Checked)
    {
        if (!enableEndCmdFiles)
        {
            enableEndCmdFiles = true;
            runEndCmdFilesChanged = true;
        }

    }
    else if (state == Qt::Unchecked)
    {
        if (enableEndCmdFiles)
        {
           enableEndCmdFiles = false;
           runEndCmdFilesChanged = true;
        }


    }
}

void SetupDialog::sendSpotsToDXClusterChkBoxChanged(int state)
{

    if (state == Qt::Checked)
    {
        if (!sendSpotToDXCluster)
        {
            sendSpotToDXCluster = true;
            sendSpotsToDXClusterChanged = true;
        }

    }
    else if (state == Qt::Unchecked)
    {
        if (sendSpotToDXCluster)
        {
            sendSpotToDXCluster = false;
            sendSpotsToDXClusterChanged = true;
        }

    }


}

void SetupDialog::callsignFinished(const QString& /*cs*/)
{

    if (ui->callsignEdit->getCallsign() != callsign)
    {
        personalDataChanged = true;

    }

}

bool SetupDialog::getSendToDXClusterEnabled()
{
    return sendSpotToDXCluster;
}



void SetupDialog::nameEditFinshed()
{

    if (ui->nameEdit->text().trimmed() != name)
    {
        personalDataChanged = true;
    }

}

void SetupDialog::locatorFinished(const QString& /*locator*/)
{

    if (ui->locatorEdit->getLocator() != locator) // data changed
    {

        personalDataChanged = true;

    }


}


void SetupDialog::qthEditFinished()
{
    if (ui->qthEdit->text().trimmed() != qth)
    {
        personalDataChanged = true;
    }
}



void SetupDialog::savePersonal()
{
    if (personalDataChanged)
    {

        if (callsign != ui->callsignEdit->getCallsign())
        {
            if (ui->callsignEdit->isValid())
            {
                callsign = ui->callsignEdit->getCallsign();
            }
            else
            {
                ui->callsignEdit->setCallsign(callsign);
            }

        }


        if (name != ui->nameEdit->text().trimmed())
        {
            name = ui->nameEdit->text().trimmed();
        }

        if (locator != ui->locatorEdit->getLocator() )
        {
            if (ui->locatorEdit->isValid())
            {
              locator = ui->locatorEdit->getLocator();
            }
            else
            {
                ui->locatorEdit->setLocator(locator);
            }

        }


        if (qth != ui->qthEdit->text().trimmed())
        {
            qth = ui->qthEdit->text().trimmed();
        }



        QString fileName = CLUSTER_SETTINGS_FILE;

        QSettings config(fileName, QSettings::IniFormat);

        config.beginGroup("Personal");
        config.setValue("Callsign", callsign);
        config.setValue("Name", name);
        config.setValue("Locator", locator);
        config.setValue("Qth", qth);

        config.endGroup();
        emit personalDataUpdated(callsign, name, locator, qth);
    }
}


void SetupDialog::readPersonal()
{


    QString fileName = CLUSTER_SETTINGS_FILE;


    QSettings config(fileName, QSettings::IniFormat);

    config.beginGroup("Personal");

    callsign = config.value("Callsign", "").toString();
    name = config.value("Name", "").toString();
    locator = config.value("Locator", "").toString();
    qth = config.value("Qth", "").toString();

    config.endGroup();

}


void SetupDialog::saveCurrentNodeName(QString nodeName)
{
    QString fileName = CLUSTER_SETTINGS_FILE;

    QSettings config(fileName, QSettings::IniFormat);

    config.beginGroup("CurrentNodeName");
    config.setValue("currentNodeName", nodeName);
    config.endGroup();
}


QString SetupDialog::getCurrentNodeName()
{
    QString fileName = CLUSTER_SETTINGS_FILE;


    QSettings config(fileName, QSettings::IniFormat);

    config.beginGroup("CurrentNodeName");
    QString currentNodeName = config.value("currentNodeName", "").toString();
    config.endGroup();

    return  currentNodeName;
}



void SetupDialog::loadPersonalToSetupTab()
{

    ui->callsignEdit->setText(callsign);
    ui->nameEdit->setText(name);
    ui->locatorEdit->setText(locator);
    ui->qthEdit->setText(qth);
}




void SetupDialog::loadClusterListToModel()
{
    // ****************** need check for no ini file and empty ini file
    // get number of cluster nodes from settings ini file

    QString fileName = CLUSTER_NODE_LIST_FILE;
    QSettings settings(fileName, QSettings::IniFormat);

    QStringList availNodeNames = settings.childGroups();
    int numClusterNodes = availNodeNames.count();

    // create model and load data to model
    clusterListModel = new QStandardItemModel(numClusterNodes, ClusterListNumCols);
    clusterListModel->setHeaderData(NameColNum, Qt::Horizontal, QObject::tr("Name"));
    clusterListModel->setHeaderData(AddressColNum, Qt::Horizontal, QObject::tr("Address"));
    clusterListModel->setHeaderData(PortColNum, Qt::Horizontal, QObject::tr("Port"));
    clusterListModel->setHeaderData(PasswdColNum, Qt::Horizontal, QObject::tr("Password"));

    loadSettingsToModel(availNodeNames, settings);

    // connect model to view
    ui->ListClustersView->setModel(clusterListModel);
    ui->ListClustersView->setSelectionMode( QAbstractItemView::SingleSelection );
    ui->ListClustersView->verticalHeader()->hide();


}


void SetupDialog::loadSettingsToModel(QStringList &availNodeNames, QSettings &settings)
{
    for (int row = 0; row < clusterListModel->rowCount(); ++row)
    {

        settings.beginGroup(availNodeNames[row]);
        QStandardItem *nodeNameItem = new QStandardItem(availNodeNames[row]);
        clusterListModel->setItem(row, NameColNum, nodeNameItem);
        QStandardItem *addressItem = new QStandardItem(settings.value("address", "").toString());
        clusterListModel->setItem(row, AddressColNum, addressItem);
        QStandardItem *portNumItem = new QStandardItem(settings.value("port", "").toString());
        clusterListModel->setItem(row, PortColNum, portNumItem);
        QStandardItem *passwordItem = new QStandardItem(settings.value("password", "").toString());
        clusterListModel->setItem(row, PasswdColNum, passwordItem);
        settings.endGroup();

    }


}


void SetupDialog::addClusterNode()
{

    QList<QStandardItem *> items;

    for (int i = 0; i < ClusterListNumCols; ++i)
    {
        items.append(new QStandardItem(""));
    }

    // add row to model
    //clusterListModel->appendRow(items);
    clusterListModel->insertRow(0, items);
    listDataChanged = true;


}


void SetupDialog::deleteClusterNode()
{

    QItemSelectionModel *select = ui->ListClustersView->selectionModel();

    if (select->hasSelection()) //check if has selection
    {
       QModelIndexList indexes = select->selectedIndexes();
       if (!indexes.empty())
       {
           clusterListModel->removeRows(indexes.last().row(), 1);
           indexes.removeLast();
           listDataChanged = true;
       }

    }

}


void SetupDialog::clusterListSave()
{

    if (listDataChanged)
    {
        QString fileName = CLUSTER_NODE_LIST_FILE;
        QSettings  settings(fileName, QSettings::IniFormat);

        settings.clear();
        for (int row = 0; row < clusterListModel->rowCount(); ++row)
        {
            QString s = clusterListModel->item(row, NameColNum)->text();
            settings.beginGroup(clusterListModel->item(row, NameColNum)->text());
            settings.setValue("address", clusterListModel->item(row, AddressColNum)->text());
            settings.setValue("port", clusterListModel->item(row, PortColNum)->text());
            settings.setValue("password", clusterListModel->item(row, PasswdColNum)->text());
            settings.endGroup();

        }

        emit clusterListChanged();

    }
    listDataChanged = false;
}


void SetupDialog::clusterListDataChanged(QStandardItem* /*item*/)
{
    listDataChanged = true;

}


QString SetupDialog::getUserCallsign()
{
    return callsign;
}

QString SetupDialog::getUserName()
{
    return name;
}

QString SetupDialog::getUserLocator()
{
    return locator;
}

QString SetupDialog::getUserQth()
{
    return qth;
}

// true if name exists
bool SetupDialog::doesClusterNameExist(QString clusterName)
{
    QList<QStandardItem *> nl = clusterListModel->findItems(clusterName);
    return !nl.isEmpty();
}

// gets connection info from datamodel
QStringList SetupDialog::getClusterInfo(QString clusterName)
{
    clusterInfo.clear();
    QList<QStandardItem*> foundLst;

    foundLst = clusterListModel->findItems(clusterName, Qt::MatchExactly, NameColNum);


    if(foundLst.count() == 1)
    {
        QModelIndex mi = clusterListModel->indexFromItem(foundLst[0]);  // always first item in foundlist!
        for (int col = 0; col < clusterListModel->columnCount(); ++col)
        {
            clusterInfo.append(clusterListModel->item(mi.row(), col)->text());
        }

    }



    return clusterInfo;
}


QStringList SetupDialog::getListOfClusterNames()
{

    clusterNameList.clear();
    for (int row = 0; row < clusterListModel->rowCount(); ++row)
    {
        clusterNameList.append(clusterListModel->item(row, 0)->text());
    }

    return clusterNameList;

}

void SetupDialog::setTabNum(int num)
{


    ui->ClusterSetUpTabWidget->setCurrentIndex(num);
}


bool SetupDialog::getRunStartFileFlag()
{
    return enableStartCmdFiles;
}


bool SetupDialog::getRunEndFileFlag()
{
   return enableEndCmdFiles;
}



