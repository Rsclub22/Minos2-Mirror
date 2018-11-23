#include <QStringListModel>
#include <QMessageBox>
#include "cutils.h"
#include "list.h"
#include "clusterclientfilterdialog.h"
#include "calllocinputdialog.h"
#include "ui_clusterclientfilterdialog.h"

ClusterClientFilterDialog::ClusterClientFilterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ClusterClientFilterDialog),
    callsignListWidgetCurrentRow(-1),
    editBandFilterMask(0),
    editModeFilterMask(0),
    vhfButtonState(false),
    mWaveButtonState(false),
    modeButtonState(false),
    bandTabChanged(false),
    callsignEditChanged(false),
    locatorEditChanged(false)
{
    ui->setupUi(this);
    initCheckFilterTab();
}

ClusterClientFilterDialog::~ClusterClientFilterDialog()
{
    delete ui;
}

void ClusterClientFilterDialog::initCheckFilterTab()
{


    setWindowTitle("Cluster Spot Filters");
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    vhfChkBoxList << ui->_50MHzCheckBox << ui->_70MHzCheckBox << ui->_144MHzCheckBox << ui->_432MHzCheckBox;

    mWaveChkBoxList << ui->_1296MHzCheckBox << ui->_2300MHzCheckBox << ui->_3_4GHzCheckBox << ui->_5_6GHzCheckBox << ui->_10GHzCheckBox;


    modeChkBoxList << ui->cwModeChkBox << ui->phoneModeChkBox << ui->rttyModeChkBox << ui->pskModeChkBox << ui->mgmModeChkBox;


    for (int i = 0; i < vhfChkBoxList.count(); i++)
    {

        connect(vhfChkBoxList[i], &QCheckBox::stateChanged, [this, i]() {vhfChecked(i);});

    }


    for (int i = 0; i < mWaveChkBoxList.count(); i++)
    {

        connect(mWaveChkBoxList[i], &QCheckBox::stateChanged, [this, i]() {mWaveChecked(i);});

    }

    for (int i = 0; i < modeChkBoxList.count(); i++)
    {

        connect(modeChkBoxList[i], &QCheckBox::stateChanged, [this, i]() {modeChecked(i);});

    }

    ui->ClusterClientFilterTab->setCurrentIndex(0);

    callsignListWidget = ui->callsignListWidget;
    callsignListWidget->addItems(filterSettings.unpackFilterList(filterSettings.callsignFilterList));


    connect(ui->callsignListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(callsignCurrentRowChanged(int)));


    connect(ui->callsignAddButton, SIGNAL(clicked()), SLOT(callsignAddClicked()));
    connect(ui->callsignEditButton, SIGNAL(clicked()), SLOT(callsignEditClicked()));
    connect(ui->callsignDelButton, SIGNAL(clicked()), SLOT(callsignDelClicked()));
    connect(ui->callsignDelAllButton, SIGNAL(clicked()), SLOT(callsignDelAllClicked()));
    connect(ui->saveCallsignList, SIGNAL(clicked()), SLOT(onCallsignListSave()));
    connect(ui->importCallsignList, SIGNAL(clicked()), SLOT(onCallsignListImport()));

    locatorListWidget = ui->locatorListWidget;
    locatorListWidget->addItems(filterSettings.unpackFilterList(filterSettings.locatorFilterList));

    connect(ui->locatorListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(locatorCurrentRowChanged(int)));

    connect(ui->locatorAddButton, SIGNAL(clicked()), SLOT(locatorAddClicked()));
    connect(ui->locatorEditButton, SIGNAL(clicked()), SLOT(locatorEditClicked()));
    connect(ui->locatorDelButton, SIGNAL(clicked()), SLOT(locatorDelClicked()));
    connect(ui->locatorDelAllButton, SIGNAL(clicked()), SLOT(locatorDelAllClicked()));
    connect(ui->saveLocatorList, SIGNAL(clicked()), SLOT(onLocatorListSave()));
    connect(ui->importLocatorList, SIGNAL(clicked()), SLOT(onLocatorListImport()));

    connect(ui->vhfSelectBut, SIGNAL(clicked()), this, SLOT(vhfButtonSelected()));
    connect(ui->mWSelectBut, SIGNAL(clicked()), this, SLOT(mWaveButtonSelected()));
    connect(ui->modeSelectBut, SIGNAL(clicked()), this, SLOT(modeButtonSelected()));
    connect(ui->clearAllBut, SIGNAL(clicked()), this, SLOT(clearAllButtonSelected()));


    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(filtersAccepted()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(filtersRejected()));

}






void ClusterClientFilterDialog::filtersAccepted()
{
    int filterChangeMask = 0;


    if (bandTabChanged)
    {
        bandTabChanged = false;
        // copy updated masks with edited values
        filterSettings.bandFilterMask = editBandFilterMask;
        filterSettings.modeFilterMask = editModeFilterMask;
        filterChangeMask |= FREQFILTERUP;
    }

    else if (callsignEditChanged)
    {
        callsignEditChanged = false;
        filterSettings.callsignFilterList.clear();
        // get list of callsigns
        filterSettings.callsignFilterList = filterSettings.packFilterList(getItemsTextFromListWidget(callsignListWidget));
        filterChangeMask |= CALLSIGNUP;
    }
    else if (locatorEditChanged)
    {
        locatorEditChanged = false;
        filterSettings.locatorFilterList.clear();
        filterSettings.locatorFilterList = filterSettings.packFilterList(getItemsTextFromListWidget(locatorListWidget));
        filterChangeMask |= LOCATORUP;
    }

    emit filtersChanged(filterChangeMask);
    close();
}


QStringList ClusterClientFilterDialog::getItemsTextFromListWidget(QListWidget* lw)
{
    QStringList l;
    if (lw->count() != 0)
    {
        for (int row = 0; row < lw->count(); row++)
        {
            QListWidgetItem* item = lw->item(row);
            l.append(item->text());
        }
    }

    return l;
}


void ClusterClientFilterDialog::filtersRejected()
{
    if (bandTabChanged)
    {
        restoreVHFBands();
        restoreMWBands();
        restoreModes();
    }
    else if (callsignEditChanged)
    {
        // restore the callsignListWidget
        callsignListWidget->clear();
        callsignListWidget->addItems(filterSettings.unpackFilterList(filterSettings.callsignFilterList));
        //for (int i = 0; i < filterSettings.callsignFilterList.count(); i++)
        //{
        //    callsignListWidget->addItem(filterSettings.callsignFilterList[i]);
        //}
    }
    else if (locatorEditChanged)
    {
        // restore the locatorListWidget
        locatorListWidget->clear();
        locatorListWidget->addItems(filterSettings.unpackFilterList(filterSettings.callsignFilterList));
        //for(int i = 0; i < filterSettings.locatorFilterList.count(); i++)
        //{
        //    locatorListWidget->addItem(filterSettings.locatorFilterList[i]);
       // }
    }

    // restore settings on tab
    restoreTabSettings();

    close();
}


void ClusterClientFilterDialog::setTabCurrentIndex(int i)
{
    ui->ClusterClientFilterTab->setCurrentIndex(i);
}

int ClusterClientFilterDialog::getTabCurrentIndex()
{
    return ui->ClusterClientFilterTab->currentIndex();
}

void ClusterClientFilterDialog::restoreTabSettings()
{

    loadBandSettings(filterSettings.bandFilterMask);
    loadModeSettings(filterSettings.modeFilterMask);
}


void ClusterClientFilterDialog::closeEvent (QCloseEvent *event)
{
    restoreTabSettings();
    QWidget::closeEvent(event);
}

void ClusterClientFilterDialog::copyBandFilterMaskToEdit()
{
    editBandFilterMask = filterSettings.bandFilterMask;
}

void ClusterClientFilterDialog::copyModeFilterMaskToEdit()
{
    editModeFilterMask = filterSettings.modeFilterMask;
}

void ClusterClientFilterDialog::loadBandSettings(unsigned int bandMask)
{
    for (int i = 0; i < NUM_VHFMASKS; i++)
    {
        if (bandMask & vhfBandMasks[i])
        {
            vhfChkBoxList[i]->setChecked(true);
        }
        else
        {
            vhfChkBoxList[i]->setChecked(false);
        }
    }

    for (int i = 0; i < NUM_MWAVEMASKS; i++)
    {
        if (bandMask & mWaveBandMasks[i])
        {
            mWaveChkBoxList[i]->setChecked(true);
        }
        else
        {
            mWaveChkBoxList[i]->setChecked(false);
        }
    }
}

void ClusterClientFilterDialog::loadModeSettings(unsigned int modeMask)
{
    for (int i = 0; i < NUM_MODEMASKS; i++)
    {
        if (modeMask & modeMasks[i])
        {
            modeChkBoxList[i]->setChecked(true);
        }
        else
        {
            modeChkBoxList[i]->setChecked(false);
        }
    }
}

void ClusterClientFilterDialog::vhfButtonSelected()
{
    if (!vhfButtonState)
    {
        vhfButtonState = true;
        setVHFBands();
    }
    else
    {
        vhfButtonState = false;
        clearVHFBands();
    }
}

void ClusterClientFilterDialog::mWaveButtonSelected()
{
    if (!mWaveButtonState)
    {
        mWaveButtonState = true;
        setMWaveBands();
    }
    else
    {
        mWaveButtonState = false;
        clearMWaveBands();
    }
}

void ClusterClientFilterDialog::modeButtonSelected()
{
    if (!modeButtonState)
    {
        modeButtonState = true;
        setModes();
    }
    else
    {
        modeButtonState = false;
        clearModes();
    }
}


void ClusterClientFilterDialog::clearAllButtonSelected()
{
    clearAllFilters();
}


void ClusterClientFilterDialog::clearAllFilters()
{
    clearVHFBands();
    clearMWaveBands();
    clearModes();

}



void ClusterClientFilterDialog::clearVHFBands()
{
    editBandFilterMask = editBandFilterMask & ~_50M & ~_70M & ~_144M & ~_432M;
    for (int i = 0; i < vhfChkBoxList.count(); i++)
    {
        vhfChkBoxList[i]->setCheckState(Qt::Unchecked);
    }
    bandTabChanged = true;
}

void ClusterClientFilterDialog::setVHFBands()
{
    editBandFilterMask = editBandFilterMask | _50M | _70M | _144M | _432M;
    for (int i = 0; i < vhfChkBoxList.count(); i++)
    {
        vhfChkBoxList[i]->setCheckState(Qt::Checked);
    }
    bandTabChanged = true;
}

void ClusterClientFilterDialog::vhfChecked(int checkBoxNum)
{
    if (checkBoxNum < vhfChkBoxList.count())
    {
        if (vhfChkBoxList[checkBoxNum]->checkState() == Qt::Checked)
        {
            editBandFilterMask |= vhfBandMasks[checkBoxNum];
        }
        else if (mWaveChkBoxList[checkBoxNum]->checkState() == Qt::Unchecked)
        {
            editBandFilterMask &= ~vhfBandMasks[checkBoxNum];
        }

        bandTabChanged = true;

    }


}

void ClusterClientFilterDialog::restoreVHFBands()
{

    for (int i = 0; i < NUM_VHFMASKS; i++)
    {
        if (filterSettings.bandFilterMask & vhfBandMasks[i])
        {
            vhfChkBoxList[i]->setChecked(true);
        }
        else
        {
            vhfChkBoxList[i]->setChecked(false);
        }

    }

}



void ClusterClientFilterDialog::clearMWaveBands()
{
    editBandFilterMask = editBandFilterMask & ~_1296M & ~_2300M & ~_3_4G & ~_5_6G & ~_10G;
    for (int i = 0; i < mWaveChkBoxList.count(); i++)
    {
        mWaveChkBoxList[i]->setCheckState(Qt::Unchecked);
    }
    bandTabChanged = true;
}

void ClusterClientFilterDialog::setMWaveBands()
{
    editBandFilterMask = editBandFilterMask | _1296M | _2300M | _3_4G | _5_6G | _10G;
    for (int i = 0; i < mWaveChkBoxList.count(); i++)
    {
        mWaveChkBoxList[i]->setCheckState(Qt::Checked);
    }
    bandTabChanged = true;
}




void ClusterClientFilterDialog::mWaveChecked(int checkBoxNum)
{
    if (checkBoxNum < mWaveChkBoxList.count())
    {
        if (mWaveChkBoxList[checkBoxNum]->checkState() == Qt::Checked)
        {
            editBandFilterMask |= mWaveBandMasks[checkBoxNum];
        }
        else if (mWaveChkBoxList[checkBoxNum]->checkState() == Qt::Unchecked)
        {
            editBandFilterMask &= ~mWaveBandMasks[checkBoxNum];
        }
    }
    bandTabChanged = true;

}


void ClusterClientFilterDialog::restoreMWBands()
{

    for (int i = 0; i < NUM_MWAVEMASKS; i++)
    {
        if (filterSettings.bandFilterMask & mWaveBandMasks[i])
        {
            mWaveChkBoxList[i]->setChecked(true);
        }
        else
        {
            mWaveChkBoxList[i]->setChecked(false);
        }

    }

}

void ClusterClientFilterDialog::clearModes()
{
    editModeFilterMask = editModeFilterMask & ~CWMODE & ~PHONEMODE & ~RTTYMODE & ~MGMMODE;
    for (int i = 0; i <modeChkBoxList.count(); i++)
    {
        modeChkBoxList[i]->setCheckState(Qt::Unchecked);
    }
}

void ClusterClientFilterDialog::setModes()
{
    editModeFilterMask = editModeFilterMask | CWMODE | PHONEMODE | RTTYMODE | MGMMODE;
    for (int i = 0; i < modeChkBoxList.count(); i++)
    {
        modeChkBoxList[i]->setCheckState(Qt::Checked);
    }
}


void ClusterClientFilterDialog::modeChecked(int checkBoxNum)
{
    if (checkBoxNum < modeChkBoxList.count())
    {
        if (modeChkBoxList[checkBoxNum]->checkState() == Qt::Checked)
        {
           editModeFilterMask |= modeMasks[checkBoxNum];
        }
        else if (mWaveChkBoxList[checkBoxNum]->checkState() == Qt::Unchecked)
        {
            editModeFilterMask &= ~modeMasks[checkBoxNum];
        }
    }

}

void ClusterClientFilterDialog::restoreModes()
{

    for (int i = 0; i < NUM_MODEMASKS; i++)
    {
        if (filterSettings.modeFilterMask & modeMasks[i])
        {
            modeChkBoxList[i]->setChecked(true);
        }
        else
        {
            modeChkBoxList[i]->setChecked(false);
        }

    }

}


unsigned int ClusterClientFilterDialog::getBandFilterMask()
{
    return filterSettings.bandFilterMask;
}
unsigned int ClusterClientFilterDialog::getModeFilterMask()
{
    return filterSettings.modeFilterMask;
}






void ClusterClientFilterDialog::copyCallsignFilterListToListWidget()
{
    callsignListWidget->clear();
    foreach (QString str, filterSettings.unpackFilterList(filterSettings.callsignFilterList))
    {
        callsignListWidget->addItem(str);
    }
}

void ClusterClientFilterDialog::callsignAddClicked()
{

    CallLocInputDialog callsignDialog(this, QString(""), QString("Add Callsign Filter"), QString("Enter Callsign"));
    QString callsign;
    if (callsignDialog.exec() == QDialog::Accepted)
    {
        callsign = callsignDialog.getText();

        if (!callsign.isEmpty())
        {

            if (searchItem(callsign, callsignListWidget))
            {
                QMessageBox::information(this, tr("Add Callsign Filter"),
                                         tr("Callsign already exists in list!"),
                                          QMessageBox::Ok|QMessageBox::Default,
                                          QMessageBox::NoButton, QMessageBox::NoButton);
            }
            else
            {
                QListWidgetItem *newItem = new QListWidgetItem;
                newItem->setText(callsign);
                int row = callsignListWidget->count();
                callsignListWidget->insertItem(row, newItem);
                callsignEditChanged = true;
            }

        }
    }
}


bool ClusterClientFilterDialog::searchItem(QString text, QListWidget* listWidget)
{
    for (int i = 0; i < listWidget->count(); i++)
    {
        QListWidgetItem* item = listWidget->item(i);
        if (item->text() == text)
        {
            return true;
        }

    }

    return false;
}


void ClusterClientFilterDialog::callsignCurrentRowChanged(int currentRow)
{
    callsignListWidgetCurrentRow = currentRow;
}

void ClusterClientFilterDialog::locatorCurrentRowChanged(int currentRow)
{
    locatorListWidgetCurrentRow = currentRow;
}

void ClusterClientFilterDialog::callsignDelClicked()
{
    if (callsignListWidgetCurrentRow >= 0)
    {
        int status = QMessageBox::question( this,
        QString("Delete Callsign Filter"),
        QString("Please confirm you want to delete callsign %1 ?").arg(callsignListWidget->currentItem()->text()),
        QMessageBox::Yes|QMessageBox::Default,
        QMessageBox::No|QMessageBox::Escape,
        QMessageBox::NoButton);
        if (status == QMessageBox::Yes)
        {
            if (callsignListWidgetCurrentRow >= 0 && callsignListWidgetCurrentRow < callsignListWidget->count())
            {
                callsignListWidget->takeItem(callsignListWidgetCurrentRow);
                callsignEditChanged = true;
            }
        }

    }

}


void ClusterClientFilterDialog::callsignDelAllClicked()
{
    if (callsignListWidget->count() > 0)
    {
        int status = QMessageBox::question( this,
        QString("Delete All Callsign Filters"),
        QString("Please confirm you want to delete all callsigns?"),
        QMessageBox::Yes|QMessageBox::Default,
        QMessageBox::No|QMessageBox::Escape,
        QMessageBox::NoButton);
        if (status == QMessageBox::Yes)
        {
            callsignListWidget->clear();
            callsignEditChanged = true;
        }
    }
}

void ClusterClientFilterDialog::callsignEditClicked()
{
    if (callsignListWidgetCurrentRow >= 0)
    {
        int row = callsignListWidgetCurrentRow;
        QString currentCall = callsignListWidget->currentItem()->text();
        CallLocInputDialog callsignDialog(this, currentCall, QString("Edit Callsign Filter"), QString("Edit Callsign"));
        QString callsign;
        if (callsignDialog.exec() == QDialog::Accepted)
        {
            callsign = callsignDialog.getText();
            if (callsign != currentCall)
            {
                QListWidgetItem *editItem = new QListWidgetItem;
                editItem->setText(callsign);
                 callsignListWidget->takeItem(row);
                 callsignListWidget->insertItem(row, editItem);
                //callsignListWidget->setCurrentItem(editItem);
                callsignEditChanged = true;
            }

        }
    }
}


QStringList ClusterClientFilterDialog::getCallsignFilterList()
{
    QStringList cl = filterSettings.unpackFilterList(filterSettings.callsignFilterList);
    return cl;
}

QStringList ClusterClientFilterDialog::getLocatorFilterList()
{
    QStringList ll = filterSettings.unpackFilterList(filterSettings.locatorFilterList);
    return ll;
}


void ClusterClientFilterDialog::copyLocatorFilterListToListWidget()
{
    locatorListWidget->clear();
    foreach (QString str, filterSettings.unpackFilterList( filterSettings.locatorFilterList))
    {
        locatorListWidget->addItem(str);
    }
}


void ClusterClientFilterDialog::locatorAddClicked()
{
    CallLocInputDialog locatorDialog(this, QString(""), QString("Add Locator Filter"), QString("Enter Callsign"));
    QString locator;
    if (locatorDialog.exec() == QDialog::Accepted)
    {
        locator = locatorDialog.getText();

        if (!locator.isEmpty())
        {

            if (searchItem(locator, locatorListWidget))
            {
                QMessageBox::information(this, tr("Add Locator Filter"),
                                         tr("Locator already exists in list!"),
                                          QMessageBox::Ok|QMessageBox::Default,
                                          QMessageBox::NoButton, QMessageBox::NoButton);
            }
            else
            {
                QListWidgetItem *newItem = new QListWidgetItem;
                newItem->setText(locator);
                int row = locatorListWidget->count();
                locatorListWidget->insertItem(row, newItem);
                locatorEditChanged = true;
            }

        }
    }
}
void ClusterClientFilterDialog::locatorEditClicked()
{
    if (locatorListWidgetCurrentRow >= 0)
    {
        int row = locatorListWidgetCurrentRow;
        QString currentLocator = locatorListWidget->currentItem()->text();
        CallLocInputDialog callsignDialog(this, currentLocator, QString("Edit Locator Filter"), QString("Edit Locator"));
        QString locator;
        if (callsignDialog.exec() == QDialog::Accepted)
        {
            locator = callsignDialog.getText();
            if (locator != currentLocator)
            {
                QListWidgetItem *editItem = new QListWidgetItem;
                editItem->setText(locator);
                 locatorListWidget->takeItem(row);
                 locatorListWidget->insertItem(row, editItem);
                //callsignListWidget->setCurrentItem(editItem);
                locatorEditChanged = true;
            }

        }
    }
}

void ClusterClientFilterDialog::locatorDelClicked()
{
    if (locatorListWidgetCurrentRow >=0)
    {
        int status = QMessageBox::question( this,
        QString("Delete Locator Filter"),
        QString("Do you want to delete locator %1 ?").arg(locatorListWidget->currentItem()->text()),
        QMessageBox::Yes|QMessageBox::Default,
        QMessageBox::No|QMessageBox::Escape,
        QMessageBox::NoButton);
        if (status == QMessageBox::Yes)
        {
            if (locatorListWidgetCurrentRow >= 0 && locatorListWidgetCurrentRow < locatorListWidget->count())
            {
                locatorListWidget->takeItem(locatorListWidgetCurrentRow);
                locatorEditChanged = true;
            }
        }

    }
}


void ClusterClientFilterDialog::locatorDelAllClicked()
{
    if (locatorListWidget->count() > 0)
    {
        int status = QMessageBox::question( this,
        QString("Delete All Locator Filters"),
        QString("Please confirm you want to delete all locators?"),
        QMessageBox::Yes|QMessageBox::Default,
        QMessageBox::No|QMessageBox::Escape,
        QMessageBox::NoButton);
        if (status == QMessageBox::Yes)
        {
            locatorListWidget->clear();
            locatorEditChanged = true;
        }
    }
}


void ClusterClientFilterDialog::onCallsignListSave()
{
    if (callsignListWidget->count() != 0)
    {
        saveFilterToFile(getItemsTextFromListWidget(callsignListWidget), "Callsign");
    }

}

void ClusterClientFilterDialog::onCallsignListImport()
{
    QStringList lof;
    importFilterToWidgetList(lof, "Callsign");
    if (!lof.isEmpty())
    {
        callsignListWidget->addItems(lof);
    }
}

void ClusterClientFilterDialog::onLocatorListSave()
{
    if (locatorListWidget->count() != 0)
    {
        saveFilterToFile(getItemsTextFromListWidget(locatorListWidget), "Locator");
    }
}

void ClusterClientFilterDialog::onLocatorListImport()
{
    QStringList lof;
    importFilterToWidgetList(lof, "Locator");
    if (!lof.isEmpty())
    {
        locatorListWidget->addItems(lof);
    }
}

void ClusterClientFilterDialog::saveFilterToFile(QStringList listOfFilters, QString type)
{
    if (!listOfFilters.isEmpty())
    {
        QString listDir;
        QString path;
        if (type == "Callsign")
        {
           listDir =  CLUSTER_CALLSIGNLIST_DIR;
        }
        else if (type == "Locator")
        {
            listDir = CLUSTER_LOCATORLIST_DIR;
        }
        path = CLUSTER_PATH + listDir;

        CreateDir(path);

        QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save %1").arg(type), path, tr("%1 List Files (*.txt)").arg(type));

        QFile file( fileName );

        if (file.exists(fileName))
        {
           file.remove(fileName);
        }

        if ( file.open(QIODevice::ReadWrite | QIODevice::Text) )
        {
            QTextStream stream( &file );
            int i = 0;
            while (i < listOfFilters.count())
            {
                stream << listOfFilters[i];
                if (i == listOfFilters.count() - 1)
                {
                    stream << endl;
                }
                else
                {
                   stream << FILTER_DELIMITER << endl;
                }

                i++;
            }
        }

        file.close();
    }
}

void ClusterClientFilterDialog::importFilterToWidgetList(QStringList &listOfFilters, QString type)
{
    QList<QStringList> lof;
    QString listDir;
    QString path;
    if (type == "Callsign")
    {
       listDir =  CLUSTER_CALLSIGNLIST_DIR;
    }
    else if (type == "Locator")
    {
        listDir = CLUSTER_LOCATORLIST_DIR;
    }
    path = CLUSTER_PATH + listDir;

    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open %1").arg(type), path, tr("%1 List Files (*.txt)").arg(type));

    CsvReader csv;
    csv.parseCsv(fileName, lof);

    for (int i = 0; i < lof.count(); i++)
    {
        QStringList l;
        l = lof[i];
        for (int x = 0; x < l.count(); x++)
        {
            QString s = l[x];
            if (s != "")
            {
               listOfFilters.append(s);
            }
        }
    }

}
