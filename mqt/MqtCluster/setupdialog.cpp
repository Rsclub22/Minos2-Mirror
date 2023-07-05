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
#include <QMessageBox>
#include "regsettings.h"
#include "cutils.h"
#include "clustercommon.h"
#include "CallsignLineEdit.h"

#include "setupdialog.h"
#include "ui_setupdialog.h"

const char * CLUSTER_NODE_LIST_FILE = "./Configuration/Cluster/ClusterSites.ini";

SetupDialog::SetupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetupDialog),
    listDataChanged(false),
    timeToLiveChanged(false),
    sendSpotToDXCluster(false),
    sendSpotsToDXClusterChanged(false),
    personalDataChanged(false),
    useQrzForQraFlag(false),
    useQrzForQraChanged(false)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("ClusterSetup/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    // General Tab
    connect(ui->timeToLive, &QLineEdit::editingFinished, this, [=](){timeToliveEditFinished();});
    connect(ui->sendSpotsToDXClusterChkBox, &QCheckBox::stateChanged, this, [=](int state){sendSpotsToDXClusterChkBoxChanged(state);});
    connect(ui->useQrzCheckBox, &QCheckBox::stateChanged, this, [=](int state){onQrzCheckBoxChkBoxClicked(state);});
    readGeneralSettings();
    loadGeneralToSetupTab();

    // Personal Tab
    ui->callsignEdit->setValidator(&ucValidator);
    ui->locatorEdit->setValidator(&ucValidator);

    connect(ui->callsignEdit, &CallsignLineEdit::callsignFinished, this, &SetupDialog::callsignFinished);
    connect(ui->nameEdit, &QLineEdit::editingFinished, this, &SetupDialog::nameEditFinshed);
    connect(ui->locatorEdit, &LocatorLineEdit::locatorFinished, this, &SetupDialog::locatorFinished);
    connect(ui->qthEdit, &QLineEdit::editingFinished, this, &SetupDialog::qthEditFinished);


    //readPersonal();
    //loadPersonalToSetupTab();



    // Cluster Node List Tab
    loadClusterListToModel();       // this also creates the model
    connect(ui->clustersListAddPbutton, &QPushButton::clicked, this, &SetupDialog::addClusterNode);
    connect(ui->clustersListDelPbutton, &QPushButton::clicked, this, &SetupDialog::deleteClusterNode);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SetupDialog::saveButtonPushed);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &SetupDialog::cancelButtonPushed);

    connect(clusterListModel, &QStandardItemModel::itemChanged, this, &SetupDialog::clusterListDataChanged);

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

    RegSettings settings;
    settings.getSettings().setValue("ClusterSetup/geometry", saveGeometry());
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
    if (timeToLiveChanged || sendSpotsToDXClusterChanged
        || useQrzForQraChanged)
    {
        timeToLive = ui->timeToLive->text().trimmed();
        QSettings config(CLUSTER_SETTINGS_FILE, QSettings::IniFormat);

        if (timeToLive != config.value("timeToLive", "").toString())
        {
            config.beginGroup("TimeToLive");
            config.setValue("timeToLive", timeToLive);
            config.endGroup();
        }

        if (sendSpotToDXCluster != config.value("enableSendToDXCluster", false).toBool())
        {
            config.beginGroup("EnableSendSpotsToDXCluster");
            config.setValue("enableSendToDXCluster", sendSpotToDXCluster);
            config.endGroup();
            emit sendSpotToTxEnabled(sendSpotToDXCluster);
        }

        if (useQrzForQraFlag != config.value("enableGetQraFromQrz", false).toBool())
        {
            config.beginGroup("UseQRZServer");
            config.setValue("enableGetQraFromQrz", useQrzForQraFlag);
            config.endGroup();

        }
    }
}


void SetupDialog::createDefaultGeneralSettingsFile()
{
    QSettings config(CLUSTER_SETTINGS_FILE, QSettings::IniFormat);

    config.beginGroup("Personal");

    config.setValue("Callsign", "");
    config.setValue("Name", "");
    config.setValue("Locator", "");
    config.setValue("Qth", "");

    config.endGroup();
    config.beginGroup("TimeToLive");
    config.setValue("timeToLive", 30);

    config.endGroup();
}

void SetupDialog::readGeneralSettings()
{
    QSettings config(CLUSTER_SETTINGS_FILE, QSettings::IniFormat);
    config.beginGroup("TimeToLive");
    timeToLive = config.value("timeToLive", "").toString();
    if (timeToLive == "0")      // 0 as no purge removed.
    {
        timeToLive = "10";
    }
    config.endGroup();
    config.beginGroup("CommandFile");
    //enableStartCmdFiles = config.value("enableStartCommandFile", false).toBool();
    //enableEndCmdFiles = config.value("enableEndCommandFile", false).toBool();
    config.endGroup();
    config.beginGroup("EnableSendSpotsToDXCluster");
    sendSpotToDXCluster = config.value("enableSendToDXCluster", false).toBool();
    config.endGroup();
    config.beginGroup("General");
    //bandFilterOnSaveFlag = config.value("bandFilterSaveOnClose", true).toBool();
    config.endGroup();
    config.beginGroup("UseQRZServer");
    useQrzForQraFlag =  config.value("enableGetQraFromQrz", false).toBool();
    config.endGroup();

}





void SetupDialog::loadGeneralToSetupTab()
{

    ui->timeToLive->setText(timeToLive);
    ui->sendSpotsToDXClusterChkBox->setChecked(sendSpotToDXCluster);
    ui->useQrzCheckBox->setChecked(useQrzForQraFlag);

}




QString SetupDialog::getTimeToLive()
{
    return timeToLive;
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

        QSettings config(CLUSTER_SETTINGS_FILE, QSettings::IniFormat);

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
    QSettings config(CLUSTER_SETTINGS_FILE, QSettings::IniFormat);

    config.beginGroup("Personal");

    callsign = config.value("Callsign", "").toString();
    name = config.value("Name", "").toString();
    locator = config.value("Locator", "").toString();
    qth = config.value("Qth", "").toString();

    config.endGroup();

}


void SetupDialog::saveCurrentNodeName(QString nodeName)
{
    QSettings config(CLUSTER_SETTINGS_FILE, QSettings::IniFormat);

    config.beginGroup("CurrentNodeName");
    config.setValue("currentNodeName", nodeName);
    config.endGroup();
}


QString SetupDialog::getCurrentNodeName()
{
    QSettings config(CLUSTER_SETTINGS_FILE, QSettings::IniFormat);

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

    QSettings settings(CLUSTER_NODE_LIST_FILE, QSettings::IniFormat);

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


//bool SetupDialog::getRunStartFileFlag()
//{
//    return enableStartCmdFiles;
//}


//bool SetupDialog::getRunEndFileFlag()
//{
//   return enableEndCmdFiles;
//}



