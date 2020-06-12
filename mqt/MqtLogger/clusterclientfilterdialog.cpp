////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Cluster Client Filter
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
///
//
//
/////////////////////////////////////////////////////////////////////////////

#include <QStringListModel>
#include <QMessageBox>
#include <QSignalMapper>
#include "cutils.h"
#include "list.h"
#include "clusterclientfilterdialog.h"
#include "callsigninputdialog.h"
#include "locatorinputdialog.h"
#include "ui_clusterclientfilterdialog.h"


int ClusterClientFilterDialog::mainTabIndex;
int ClusterClientFilterDialog::distanceTabIndex;

ClusterClientFilterDialog::ClusterClientFilterDialog(BaseContestLog *c, ClusterClientFilterSettings &filterSettings_, QWidget *parent) :

    QDialog(parent),
    ui(new Ui::ClusterClientFilterDialog),
    callsignListWidgetCurrentRow(-1),
    vhfButtonState(false),
    mWaveButtonState(false),
    modeButtonState(false)
    //enableHFSpots(false)
{
    ui->setupUi(this);
    QSettings settings;
    QByteArray geometry = settings.value("ClusterClientFilter/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    filterSettings = filterSettings_;
    ct = dynamic_cast<LoggerContestLog*>(c);


    // read enable hf spots flag
    //QString fileName = CLUSTER_SETTINGS_FILE;
    //QSettings config(fileName, QSettings::IniFormat);
    //config.beginGroup("HFSpots");
   // enableHFSpots = config.value("enable", false).toBool();
   // config.endGroup();


    initCheckFilterTab();
}

ClusterClientFilterDialog::~ClusterClientFilterDialog()
{
    delete ui;
}

void ClusterClientFilterDialog::doCloseEvent()
{
    QSettings settings;
    settings.setValue("ClusterClientFilter/geometry", saveGeometry());
}

void ClusterClientFilterDialog::initCheckFilterTab()
{


    setWindowTitle(tr("Cluster Spot Filters"));
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);



    bandChkBoxList << ui->_50MHzCheckBox << ui->_70MHzCheckBox << ui->_144MHzCheckBox << ui->_432MHzCheckBox
                   << ui->_1296MHzCheckBox << ui->_2300MHzCheckBox << ui->_3_4GHzCheckBox << ui->_5_6GHzCheckBox << ui->_10GHzCheckBox;

    QSignalMapper *signalMapperBandChkBox = new QSignalMapper(this);
    connect(signalMapperBandChkBox, SIGNAL(mapped(int)), this, SLOT(onBandChkBoxChecked(int)));

    for (int i = 0; i < bandChkBoxList.count(); i++)
    {
        signalMapperBandChkBox->setMapping(bandChkBoxList[i], i);
        connect(bandChkBoxList[i], SIGNAL(clicked()), signalMapperBandChkBox, SLOT(map()));
    }

    for (int i = 0; i < bandChkBoxList.count(); i++)
    {
        if (*filterSettings.bandFilters[i])
        {
            bandChkBoxList[i]->setCheckState(Qt::Checked);
        }
        else
        {
            bandChkBoxList[i]->setCheckState(Qt::Unchecked);
        }
    }

    modeChkBoxList << ui->noneModeChkBox << ui->cwModeChkBox << ui->usbModeChkBox << ui->fmModeChkBox << ui->rttyModeChkBox << ui->psk31ModeChkBox << ui->ft8ModeChkBox << ui->msk144ModeChkBox << ui->jt65ModeChkBox;

    for (int i = 0; i < modeChkBoxList.count(); i++)
    {
        if (*filterSettings.modeFilters[i])
        {
            modeChkBoxList[i]->setCheckState(Qt::Checked);
        }
        else
        {
            modeChkBoxList[i]->setCheckState(Qt::Unchecked);
        }
    }

    distanceLineEditsList << ui->spotDistanceEdit_50MHz << ui->spotDistanceEdit_70MHz << ui->spotDistanceEdit_144MHz << ui->spotDistanceEdit_432MHz
                          << ui->spotDistanceEdit_1296MHz << ui->spotDistanceEdit_2300MHz << ui->spotDistanceEdit_3_4GHz << ui->spotDistanceEdit_5_6GHz << ui->spotDistanceEdit_10GHz;


    for (int i = 0; i < filterSettings.distanceFilters.count(); i++)
    {
        distValue distItem;
        distanceLineEditsList[i]->setText(QString::number(*filterSettings.distanceFilters[i]));
        distItem.distChanged = false;
        distanceValues.append(distItem);
    }

    QSignalMapper *signalMapperDistEdit = new QSignalMapper(this);
    connect(signalMapperDistEdit, SIGNAL(mapped(int)), this, SLOT(onDistanceEditingFinished(int)));

    for (int i = 0; i < distanceLineEditsList.count(); i++)
    {
        signalMapperDistEdit->setMapping(distanceLineEditsList[i], i);
        connect(distanceLineEditsList[i], SIGNAL(editingFinished()), signalMapperDistEdit, SLOT(map()));
    }

    ignoreDistanceChkBoxList << ui->distFilterIgnoreCheckBox_50MHz << ui->distFilterIgnoreCheckBox_70MHz << ui->distFilterIgnoreCheckBox_144MHz << ui->distFilterIgnoreCheckBox_432MHz
                             << ui->distFilterIgnoreCheckBox_1296MHz << ui->distFilterIgnoreCheckBox_2300MHz << ui->distFilterIgnoreCheckBox_3_4GHz << ui->distFilterIgnoreCheckBox_5_6GHz << ui->distFilterIgnoreCheckBox_10GHz;

    QSignalMapper *signalMapperIgnoreDist = new QSignalMapper(this);
    connect(signalMapperIgnoreDist, SIGNAL(mapped(int)), this, SLOT(onIgnoreDistanceChecked(int)));

    for (int i = 0; i < ignoreDistanceChkBoxList.count(); i++)
    {
        signalMapperIgnoreDist->setMapping(ignoreDistanceChkBoxList[i], i);
        connect(ignoreDistanceChkBoxList[i], SIGNAL(clicked()), signalMapperIgnoreDist, SLOT(map()));
    }

    for (int i = 0; i < ignoreDistanceChkBoxList.count(); i++)
    {
        if (*filterSettings.ignoreDistanceFlags[i])
        {
            ignoreDistanceChkBoxList[i]->setCheckState(Qt::Checked);
        }
        else
        {
            ignoreDistanceChkBoxList[i]->setCheckState(Qt::Unchecked);
        }

    }

    ignoreEmptyDistanceChkBoxList << ui->ignoreEmptyDistanceValuesChkBox_50MHz << ui->ignoreEmptyDistanceValuesChkBox_70MHz << ui->ignoreEmptyDistanceValuesChkBox_144MHz << ui->ignoreEmptyDistanceValuesChkBox_432MHz
                                  << ui->ignoreEmptyDistanceValuesChkBox_1296MHz << ui->ignoreEmptyDistanceValuesChkBox_2300MHz << ui->ignoreEmptyDistanceValuesChkBox_3_4GHz << ui->ignoreEmptyDistanceValuesChkBox_5_6GHz << ui->ignoreEmptyDistanceValuesChkBox_10GHz;

    QSignalMapper *signalMapperIgnoreEmptyDist = new QSignalMapper(this);
    connect(signalMapperIgnoreEmptyDist, SIGNAL(mapped(int)), this, SLOT(onIgnoreEmptyDistanceChecked(int)));

    for (int i = 0; i < ignoreEmptyDistanceChkBoxList.count(); i++)
    {
        signalMapperIgnoreEmptyDist->setMapping(ignoreEmptyDistanceChkBoxList[i], i);
        connect(ignoreEmptyDistanceChkBoxList[i], SIGNAL(clicked()), signalMapperIgnoreEmptyDist, SLOT(map()));
    }

    for (int i = 0; i < ignoreEmptyDistanceChkBoxList.count(); i++)
    {
        if (*filterSettings.ignoreEmptyDistanceFlags[i])
        {
            ignoreEmptyDistanceChkBoxList[i]->setCheckState(Qt::Checked);
        }
        else
        {
            ignoreEmptyDistanceChkBoxList[i]->setCheckState(Qt::Unchecked);
        }

    }

    distanceLabelsList << ui->bandLabel_50MHz << ui->bandLabel_70MHz << ui->bandLabel_144MHz << ui->bandLabel_432MHz
                       << ui->bandLabel_1296MHz << ui->bandLabel_2300MHz << ui->bandLabel_3_4GHz << ui->bandLabel_5_6GHz << ui->bandLabel_10GHz;

    connect(ui->vhfSetAlDefaultDistPb, SIGNAL(clicked()), this, SLOT(onVhfSetDefDistPbClicked()));
    connect(ui->uhfSetAlDefaultDistPb, SIGNAL(clicked()), this, SLOT(onUhfSetDefDistPbClicked()));

    connect(ui->vhfSetAllIgnorePb, SIGNAL(clicked()), this, SLOT(onVhfSetAllIgnorePbClicked()));
    connect(ui->uhfSetAllIgnorePb, SIGNAL(clicked()), this, SLOT(onUhfSetAllIgnorePbClicked()));


    connect(ui->vhfClearAllIgnorePb, SIGNAL(clicked()), this, SLOT(onVhfClearAllIgnorePbClicked()));
    connect(ui->uhfClearAllIgnorePb, SIGNAL(clicked()), this, SLOT(onUhfClearAllIgnorePbClicked()));


    connect(ui->vhfSetAllEmptyDistPb, SIGNAL(clicked()), this, SLOT(onVhfSetAllEmptyPbClicked()));
    connect(ui->uhfSetAllEmptyDistPb, SIGNAL(clicked()), this, SLOT(onUhfSetAllEmptyPbClicked()));

    connect(ui->vhfClearAllEmptyDistPb, SIGNAL(clicked()), this, SLOT(onVhfClearAllEmptyDistPbClicked()));
    connect(ui->uhfClearAllEmptyDistPb, SIGNAL(clicked()), this, SLOT(onUhfClearAllEmptyDistPbClicked()));




    ui->ClusterClientFilterTab->setCurrentIndex(0);


    callsignListWidget = ui->callsignListWidget;
    callsignListWidget->addItems(filterSettings.unpackFilterList(filterSettings.callsignFilterList));


    //connect(ui->callsignListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(callsignCurrentRowChanged(int)));


    connect(ui->callsignAddButton, SIGNAL(clicked()), SLOT(callsignAddClicked()));
    connect(ui->callsignEditButton, SIGNAL(clicked()), SLOT(callsignEditClicked()));
    connect(ui->callsignDelButton, SIGNAL(clicked()), SLOT(callsignDelClicked()));
    connect(ui->callsignDelAllButton, SIGNAL(clicked()), SLOT(callsignDelAllClicked()));
    connect(ui->saveCallsignList, SIGNAL(clicked()), SLOT(onCallsignListSave()));
    connect(ui->importCallsignList, SIGNAL(clicked()), SLOT(onCallsignListImport()));

    locatorListWidget = ui->locatorListWidget;
    locatorListWidget->addItems(filterSettings.unpackFilterList(filterSettings.locatorFilterList));

    //connect(ui->locatorListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(locatorCurrentRowChanged(int)));

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

    connect(ui->ClusterClientFilterTab, SIGNAL(currentChanged(int)), this, SLOT(onFilterTabIndexChanged(int)));
    connect(ui->filterDistancesTab,  SIGNAL(currentChanged(int)), this, SLOT(onDistanceFilterTabIndexChanged(int)));

    setFilterTabCurrentIndex(mainTabIndex);
    setDistanceFilterTabCurrentIndex(distanceTabIndex);

    enableDistanceFields();


}


void ClusterClientFilterDialog::onFilterTabIndexChanged(int idx)
{
    mainTabIndex = idx;
}

void ClusterClientFilterDialog::onDistanceFilterTabIndexChanged(int idx)
{
    distanceTabIndex = idx;
}


/*
void ClusterClientFilterDialog::setContest(BaseContestLog *c)
{
    ct = dynamic_cast<LoggerContestLog *>( c);

}
*/


void ClusterClientFilterDialog::filtersAccepted()
{


    if (bandFiltersChanged())
    {
        // copy updated masks with edited values
        copyBandFiltersToFilterSettings();
        bandfilterChanged = true;
    }

    if (modeFiltersChanged())
    {
        copyModeFiltersToFilterSettings();
        modefilterChanged = true;
    }

    if (callsignFiltersChanged())
    {

        filterSettings.callsignFilterList.clear();
        filterSettings.callsignFilterList = filterSettings.packFilterList(getItemsTextFromListWidget(callsignListWidget));
        callsignfilterChanged = true;
    }

    if (locatorFiltersChanged())
    {
        filterSettings.locatorFilterList.clear();
        filterSettings.locatorFilterList = filterSettings.packFilterList(getItemsTextFromListWidget(locatorListWidget));
        locatorfilterChanged = true;
    }

    if (distanceValuesChanged())
    {
        for (int i = 0; i < distanceValues.count(); i++)
        {
            if (distanceValues[i].distChanged && *filterSettings.bandFilters[i])
            {
                *filterSettings.distanceFilters[i] = distanceValues[i].distance;
                distancefilterChanged = true;
            }

        }
    }

    if (ignoreDistanceChanged())
    {
        for (int i = 0; i < distanceValues.count();i++)
        {
            if (distanceValues[i].ignoreDistChanged && *filterSettings.bandFilters[i])
            {
               *filterSettings.ignoreDistanceFlags[i] = distanceValues[i].ignoreDistance;
               ignoreDistChanged = true;
            }
        }
    }

    if (ignoreEmptyDistanceChanged())
    {
        for (int i = 0; i < distanceValues.count();i++)
        {
            if (distanceValues[i].ignoreEmptyDistanceChanged && *filterSettings.bandFilters[i])
            {
               *filterSettings.ignoreEmptyDistanceFlags[i] = distanceValues[i].ignoreEmptyDistance;
               ignoreEmptyDistChanged = true;
            }
        }
    }

    if (bandfilterChanged || modefilterChanged || callsignfilterChanged
            || locatorfilterChanged || distancefilterChanged
            || ignoreDistChanged || ignoreEmptyDistChanged)
    {
        trace(QString("Cluster Filters Changed - ContestUuid = %1").arg(contestUuid));
        trace(QString("BandFilter Changed = %1, ModeFilter Changed = %2, CallsignFilter Changed = %3, LocatorFilter Changed = %4, DistanceFilter Changed = %5").arg(bandfilterChanged  ? "True" : "False").arg(modefilterChanged  ? "True" : "False").arg(callsignfilterChanged  ? "True" : "False").arg(locatorfilterChanged  ? "True" : "False").arg(distancefilterChanged ? "True" : "False"));
        if (bandfilterChanged)
        {
            for (int i = 0; i < filterSettings.bandFilters.count(); i++)
            {
                trace(QString("Band Filter - band %1, setting = %2").arg(QString::number(i).arg(*filterSettings.bandFilters[i] ? "True" : "False")));
            }
        }
        if (modefilterChanged)
        {
            for (int i = 0; i < filterSettings.modeFilters.count(); i++)
            {
                trace(QString("Mode Filter - band %1, setting = %2").arg(QString::number(i).arg(*filterSettings.modeFilters[i] ? "True" : "False")));
            }

        }
        if (callsignfilterChanged)
        {
            trace(QString("Callsign List = %1").arg(filterSettings.callsignFilterList));
        }
        if (locatorfilterChanged)
        {
            trace(QString("Locator List = %1").arg(filterSettings.locatorFilterList));
        }
        if (distancefilterChanged)
        {
            for (int i = 0; i < distanceValues.count(); i++)
            {
                if (distanceValues[i].distChanged)
                {
                    trace(QString("Distance change for band %1, value %2").arg(QString::number(i)).arg(QString::number(distanceValues[i].distance)));
                }
            }
        }

        if (ignoreDistChanged)
        {
            for (int i = 0; i < distanceValues.count(); i++)
            {
                if (distanceValues[i].distChanged)
                {
                    trace(QString("Ignore Distance  change flag  for band %1, value %2").arg(QString::number(i)).arg(distanceValues[i].ignoreDistChanged ? "True" :"False"));
                }
            }
        }
        if (ignoreEmptyDistChanged)
        {
            for (int i = 0; i < distanceValues.count(); i++)
            {
                if (distanceValues[i].distChanged)
                {
                    trace(QString("Ignore Empty Distance change flag for band %1, value %2").arg(QString::number(i)).arg(distanceValues[i].ignoreEmptyDistanceChanged ? "True" :"False"));
                }
            }
        }




        saveClusterFilterToContest();
        settingsChanged = true;

    }

    //emit filtersChanged(bandfilterChanged, modefilterChanged, callsignfilterChanged, locatorfilterChanged);
    doCloseEvent();
    //close();
}


void ClusterClientFilterDialog::saveClusterFilterToContest()
{
    ct->saveClusterFilter(filterSettings);
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

bool ClusterClientFilterDialog::bandFiltersChanged()
{
    for (int i = 0; i < bandChkBoxList.count(); i++)
    {
        if (*filterSettings.bandFilters[i] != bandChkBoxList[i]->isChecked())
        {
            return true;
        }
    }

    return false;
}

bool ClusterClientFilterDialog::modeFiltersChanged()
{
    for (int i = 0; i < modeChkBoxList.count(); i++)
    {
        if (*filterSettings.modeFilters[i] != modeChkBoxList[i]->isChecked())
        {
            return true;
        }
    }

    return false;
}

bool ClusterClientFilterDialog::callsignFiltersChanged()
{
    QString editedCalls = filterSettings.packFilterList(getItemsTextFromListWidget(callsignListWidget));
    if (editedCalls != filterSettings.callsignFilterList)
    {
        return true;
    }
    return false;
}

bool ClusterClientFilterDialog::locatorFiltersChanged()
{
    QString editedlocators = filterSettings.packFilterList(getItemsTextFromListWidget(locatorListWidget));
    if (editedlocators != filterSettings.locatorFilterList)
    {
        return true;
    }
    return false;
}

bool ClusterClientFilterDialog::distanceValuesChanged()
{
    for (int i = 0; i < distanceValues.count(); i++)
    {
        if (distanceValues[i].distChanged)
        {
            return true;
        }
    }

    return false;
}

bool ClusterClientFilterDialog::ignoreDistanceChanged()
{
    for (int i = 0; i < distanceValues.count(); i++)
    {
        if (distanceValues[i].ignoreDistChanged)
        {
            return true;
        }
    }

    return false;
}

bool ClusterClientFilterDialog::ignoreEmptyDistanceChanged()
{
    for (int i = 0; i < distanceValues.count(); i++)
    {
        if (distanceValues[i].ignoreEmptyDistanceChanged)
        {
            return true;
        }
    }

    return false;
}



void ClusterClientFilterDialog::filtersRejected()
{
 /*
    if (bandFiltersChanged())
    {
        restoreBands();
        restoreModes();
    }
    else if (callsignEditChanged)
    {
        // restore the callsignListWidget
        callsignListWidget->clear();
        callsignListWidget->addItems(filterSettings.unpackFilterList(filterSettings.callsignFilterList));

    }
    else if (locatorEditChanged)
    {
        // restore the locatorListWidget
        locatorListWidget->clear();
        locatorListWidget->addItems(filterSettings.unpackFilterList(filterSettings.callsignFilterList));

    }

    // restore settings on tab
*/
    //restoreTabSettings();
    doCloseEvent();
    //close();
}


void ClusterClientFilterDialog::setFilterTabCurrentIndex(int idx)
{
    ui->ClusterClientFilterTab->setCurrentIndex(idx);
}

int ClusterClientFilterDialog::getFilterTabCurrentIndex()
{
    return ui->ClusterClientFilterTab->currentIndex();
}

void ClusterClientFilterDialog::setDistanceFilterTabCurrentIndex(int idx)
{
    ui->filterDistancesTab->setCurrentIndex(idx);
}

int ClusterClientFilterDialog::getDistanceFilterTabCurrentIndex()
{
    return ui->filterDistancesTab->currentIndex();
}

void ClusterClientFilterDialog::closeEvent (QCloseEvent *event)
{

    doCloseEvent();
    QWidget::closeEvent(event);
}

void ClusterClientFilterDialog::copyBandFiltersToFilterSettings()
{
    for (int i = 0; i < bandChkBoxList.count(); i++)
    {
        *filterSettings.bandFilters[i] = bandChkBoxList[i]->isChecked();
    }
}

void ClusterClientFilterDialog::copyBandFiltersToDialog()
{
    for (int i = 0; i < bandChkBoxList.count(); i++)
    {
        bandChkBoxList[i]->setChecked(*filterSettings.bandFilters[i]);
    }
}

void ClusterClientFilterDialog::copyModeFiltersToFilterSettings()
{
    for (int i = 0; i < modeChkBoxList.count(); i++)
    {
        *filterSettings.modeFilters[i] = modeChkBoxList[i]->isChecked();
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


void ClusterClientFilterDialog::setBandFilter(int band)
{
    if (band > NO_BANDS && band < NUMBANDS)
    {
        *filterSettings.bandFilters[band] = true;
    }
}


void ClusterClientFilterDialog::setModeFilter(bool state, int mode)
{
    if (mode >= 0 && mode < clusterModes.count())
    {
        *filterSettings.modeFilters[mode] = state;
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
    for (int i =  VHFBANDSTART; i < VHFBANDEND; i++)
    {
        bandChkBoxList[i]->setCheckState(Qt::Unchecked);

    }

}

void ClusterClientFilterDialog::setVHFBands()
{
    for (int i = VHFBANDSTART; i < VHFBANDEND; i++)
    {
        bandChkBoxList[i]->setCheckState(Qt::Checked);

    }

}



void ClusterClientFilterDialog::restoreBands()
{

    for (int i = 0; i < NUMBANDS; i++)
    {
        bandChkBoxList[i]->setChecked(*filterSettings.bandFilters[i]);

    }



}



void ClusterClientFilterDialog::clearMWaveBands()
{
    for (int i = MWBANDSTART; i < MWBANDEND; i++)
    {
        bandChkBoxList[i]->setCheckState(Qt::Unchecked);

    }

}

void ClusterClientFilterDialog::setMWaveBands()
{
    for (int i = MWBANDSTART; i < MWBANDEND; i++)
    {
        bandChkBoxList[i]->setCheckState(Qt::Checked);

    }

}




void ClusterClientFilterDialog::clearModes()
{
    for (int i = 0; i <modeChkBoxList.count(); i++)
    {
        modeChkBoxList[i]->setCheckState(Qt::Unchecked);

    }

}

void ClusterClientFilterDialog::setModes()
{
    for (int i = 0; i < modeChkBoxList.count(); i++)
    {
        modeChkBoxList[i]->setCheckState(Qt::Checked);

    }

}


void ClusterClientFilterDialog::restoreModes()
{

    for (int i = 0; i < clusterModes.count(); i++)
    {

        modeChkBoxList[i]->setChecked(filterSettings.modeFilters[i]);

    }
}


bool  ClusterClientFilterDialog::checkBandMatch(int bandNum)
{
    return *filterSettings.bandFilters[bandNum];
}

bool ClusterClientFilterDialog::checkModeMatch(int bandNum)
{
    return *filterSettings.modeFilters[bandNum];
}






void ClusterClientFilterDialog::copyCallsignFilterListToListWidget()
{
    //LoggerContestLog *c = dynamic_cast<LoggerContestLog *>( ct );
    //ClusterClientFilterSettings ccfs = ct->clusterFilterSettings.getValue();
    callsignListWidget->clear();
    foreach (QString str, filterSettings.unpackFilterList(filterSettings.callsignFilterList))
    {
        callsignListWidget->addItem(str);
    }
}

void ClusterClientFilterDialog::callsignAddClicked()
{

    CallsignInputDialog callsignDialog(this, QString(""), tr("Add Callsign Filter"), tr("Enter Callsign"));
    QString callsign;
    bool useCallsign = false;

    if (callsignDialog.exec() == QDialog::Accepted)
    {
        callsign = callsignDialog.getText();

        if (!callsign.isEmpty())
        {
           if (!callsignDialog.isValid())
           {
                int ret = QMessageBox::information(this, tr("Add Callsign Filter"),
                                         tr("Callsign may be invalid, do you still want to use the callsign?"),
                                          QMessageBox::Yes|QMessageBox::No);

                if (ret == QMessageBox::Yes)
                {
                    useCallsign = true;
                }



           }



            if (callsignDialog.isValid() || useCallsign)
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

                }


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

/*
void ClusterClientFilterDialog::callsignCurrentRowChanged(int currentRow)
{
    callsignListWidgetCurrentRow = currentRow;
}

void ClusterClientFilterDialog::locatorCurrentRowChanged(int currentRow)
{
    locatorListWidgetCurrentRow = currentRow;
}
*/

void ClusterClientFilterDialog::callsignDelClicked()
{
    QList<QListWidgetItem *> selItems = callsignListWidget->selectedItems();

    if (selItems.count() == 1)
    {
        int status = QMessageBox::question( this,
        tr("Delete Callsign Filter"),
        tr("Please confirm you want to delete callsign %1 ?").arg(callsignListWidget->currentItem()->text()),
        QMessageBox::Yes|QMessageBox::Default,
        QMessageBox::No|QMessageBox::Escape,
        QMessageBox::NoButton);
        if (status == QMessageBox::Yes)
        {
            int row = callsignListWidget->row(selItems[0]);
            if (row >= 0 && row < callsignListWidget->count())
            {
                callsignListWidget->takeItem(row);

            }
        }

    }

}


void ClusterClientFilterDialog::callsignDelAllClicked()
{
    if (callsignListWidget->count() > 0)
    {
        int status = QMessageBox::question( this,
        tr("Delete All Callsign Filters"),
        tr("Please confirm you want to delete all callsigns?"),
        QMessageBox::Yes|QMessageBox::Default,
        QMessageBox::No|QMessageBox::Escape,
        QMessageBox::NoButton);
        if (status == QMessageBox::Yes)
        {
            callsignListWidget->clear();

        }
    }
}

void ClusterClientFilterDialog::callsignEditClicked()
{
    QList<QListWidgetItem *> selItems = callsignListWidget->selectedItems();

    if (selItems.count() == 1)
    {
        int row = callsignListWidget->row(selItems[0]);
        QString currentCall = callsignListWidget->currentItem()->text();
        CallsignInputDialog callsignDialog(this, currentCall, tr("Edit Callsign Filter"), tr("Edit Callsign"));
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
    //LoggerContestLog *c = dynamic_cast<LoggerContestLog *>( ct );
    //ClusterClientFilterSettings ccfs = ct->clusterFilterSettings.getValue();
    locatorListWidget->clear();
    foreach (QString str, filterSettings.unpackFilterList( filterSettings.locatorFilterList))
    {
        locatorListWidget->addItem(str);
    }
}


void ClusterClientFilterDialog::locatorAddClicked()
{
    LocatorInputDialog locatorDialog(this, QString(""), tr("Add Locator Filter"), tr("Enter Locator"));
    locatorDialog.allowLoc4(true);

    QString locator;

    bool useLocator = false;

    if (locatorDialog.exec() == QDialog::Accepted)
    {
        locator = locatorDialog.getText();

        if (!locator.isEmpty())
        {


            if (!locatorDialog.isValid())
            {
                 int ret = QMessageBox::information(this, tr("Add Locator Filter"),
                                          tr("The locator may be invalid, do you still want to use it?"),
                                           QMessageBox::Yes|QMessageBox::No);

                 if (ret == QMessageBox::Yes)
                 {
                     useLocator = true;
                 }



            }





            if (locatorDialog.isValid() || useLocator)
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

                }
            }


        }
    }
}
void ClusterClientFilterDialog::locatorEditClicked()
{

    QList<QListWidgetItem *> selItems = locatorListWidget->selectedItems();

    if (selItems.count() == 1)
    {
        int row = locatorListWidget->row(selItems[0]);
        QString currentLocator = locatorListWidget->currentItem()->text();
        LocatorInputDialog callsignDialog(this, currentLocator, tr("Edit Locator Filter"), tr("Edit Locator"));
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

            }

        }
    }
}

void ClusterClientFilterDialog::locatorDelClicked()
{
    QList<QListWidgetItem *> selItems = locatorListWidget->selectedItems();

    if (selItems.count() == 1)
    {
        int status = QMessageBox::question( this,
        tr("Delete Locator Filter"),
        tr("Do you want to delete locator %1 ?").arg(locatorListWidget->currentItem()->text()),
        QMessageBox::Yes|QMessageBox::Default,
        QMessageBox::No|QMessageBox::Escape,
        QMessageBox::NoButton);
        if (status == QMessageBox::Yes)
        {
            int row = locatorListWidget->row(selItems[0]);
            if (row >= 0 && row < locatorListWidget->count())
            {
                locatorListWidget->takeItem(row);

            }
        }

    }
}


void ClusterClientFilterDialog::locatorDelAllClicked()
{
    if (locatorListWidget->count() > 0)
    {
        int status = QMessageBox::question( this,
        tr("Delete All Locator Filters"),
        tr("Please confirm you want to delete all locators?"),
        QMessageBox::Yes|QMessageBox::Default,
        QMessageBox::No|QMessageBox::Escape,
        QMessageBox::NoButton);
        if (status == QMessageBox::Yes)
        {
            locatorListWidget->clear();

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
                if (i != listOfFilters.count() - 1)
                {
                   stream << FILTER_DELIMITER;
                }
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
                stream << Qt::endl;
#else
                stream << endl;
#endif

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


//bool ClusterClientFilterDialog::getEnableHFSpotsFlag()
//{
//    return enableHFSpots;
//}


void ClusterClientFilterDialog::enableDistanceFields()
{
    for (int i = 0; (i < bandChkBoxList.count() && i < distanceLineEditsList.count()); i++)
    {
        if (bandChkBoxList[i]->checkState() == Qt::Checked)
        {
            distanceLineEditsList[i]->setEnabled(true);
            ignoreDistanceChkBoxList[i]->setEnabled(true);
            ignoreEmptyDistanceChkBoxList[i]->setEnabled(true);
            distanceLabelsList[i]->setEnabled(true);
        }
        else
        {
            distanceLineEditsList[i]->setEnabled(false);
            ignoreDistanceChkBoxList[i]->setEnabled(false);
            ignoreEmptyDistanceChkBoxList[i]->setEnabled(false);
            distanceLabelsList[i]->setEnabled(false);
        }
    }

}


void ClusterClientFilterDialog::onBandChkBoxChecked(int idx)
{
    Q_UNUSED(idx)
    enableDistanceFields();

}



void ClusterClientFilterDialog::onDistanceEditingFinished(int idx)
{
    bool ok;
    int distance = 0;
    if(!distanceLineEditsList[idx]->text().isEmpty())
    {
        distance = distanceLineEditsList[idx]->text().toInt(&ok);
        if (ok)
        {
            if (distance != *filterSettings.distanceFilters[idx])
            {
                distanceValues[idx].distance = distance;
                distanceValues[idx].distChanged = true;
            }
        }
        else
        {
            QMessageBox::information(this, tr("Distance Filter"),
                                     tr("Please enter a number between %1 and %2!").arg(MIN_FILTER_DISTANCE).arg(MAX_FILTER_DISTANCE),
                                      QMessageBox::Ok|QMessageBox::Default,
                                      QMessageBox::NoButton, QMessageBox::NoButton);
        }
    }
}

void ClusterClientFilterDialog::onIgnoreDistanceChecked(int idx)
{
    if (ignoreDistanceChkBoxList[idx]->isChecked() != *filterSettings.ignoreDistanceFlags[idx])
    {
        distanceValues[idx].ignoreDistance = ignoreDistanceChkBoxList[idx]->isChecked();
        distanceValues[idx].ignoreDistChanged = true;
    }
}

void ClusterClientFilterDialog::onIgnoreEmptyDistanceChecked(int idx)
{
    if (ignoreEmptyDistanceChkBoxList[idx]->isChecked() != *filterSettings.ignoreEmptyDistanceFlags[idx])
    {
        distanceValues[idx].ignoreEmptyDistance = ignoreEmptyDistanceChkBoxList[idx]->isChecked();
        distanceValues[idx].ignoreEmptyDistanceChanged = true;
    }
}

void ClusterClientFilterDialog::onVhfSetDefDistPbClicked()
{
    setDefaultDistValues(0, 4, true);
}

void ClusterClientFilterDialog::onUhfSetDefDistPbClicked()
{
    setDefaultDistValues(4, distanceLineEditsList.count(), true);
}

void ClusterClientFilterDialog::setDefaultDistValues(int start, int end, bool status)
{
    QSettings config(CLUSTER_FILTER_FILE, QSettings::IniFormat);
    config.beginGroup("distanceFilter");

    for (int i = start; i < end; i++)
    {
        if (bandChkBoxList[i]->isChecked())
        {
            distanceValues[i].distance = config.value(distanceIniNames[i], DEFAULT_FILTER_DISTANCE).toInt();
            distanceLineEditsList[i]->setText(QString::number(distanceValues[i].distance));
            distanceValues[i].distChanged = status;
        }
    }

    config.endGroup();
}

void ClusterClientFilterDialog::onVhfSetAllIgnorePbClicked()
{
    for (int i = 0; i < 4; i++)
    {
        if (bandChkBoxList[i]->isChecked())
        {
            distanceValues[i].ignoreDistance = true;
            distanceValues[i].ignoreDistChanged = true;
            ignoreDistanceChkBoxList[i]->setChecked(true);
        }

    }
}

void ClusterClientFilterDialog::onUhfSetAllIgnorePbClicked()
{
    for (int i = 4; i < distanceLineEditsList.count(); i++)
    {
        if (bandChkBoxList[i]->isChecked())
        {
            distanceValues[i].ignoreDistance = true;
            distanceValues[i].ignoreDistChanged = true;
            ignoreDistanceChkBoxList[i]->setChecked(true);
        }
    }
}

void ClusterClientFilterDialog::onVhfClearAllIgnorePbClicked()
{
    for (int i = 0; i < 4; i++)
    {
        if (bandChkBoxList[i]->isChecked())
        {
            distanceValues[i].ignoreDistance = false;
            distanceValues[i].ignoreDistChanged = true;
            ignoreDistanceChkBoxList[i]->setChecked(false);
        }
    }
}

void ClusterClientFilterDialog::onUhfClearAllIgnorePbClicked()
{
    for (int i = 4; i < distanceLineEditsList.count(); i++)
    {
        if (bandChkBoxList[i]->isChecked())
        {
            distanceValues[i].ignoreDistance = false;
            distanceValues[i].ignoreDistChanged = true;
            ignoreDistanceChkBoxList[i]->setChecked(false);
        }
    }
}


void ClusterClientFilterDialog::onVhfSetAllEmptyPbClicked()
{
    for (int i = 0; i < 4; i++)
    {
        if (bandChkBoxList[i]->isChecked())
        {
            distanceValues[i].ignoreEmptyDistance = true;
            distanceValues[i].ignoreEmptyDistanceChanged = true;
            ignoreEmptyDistanceChkBoxList[i]->setChecked(true);
        }

    }
}

void ClusterClientFilterDialog::onUhfSetAllEmptyPbClicked()
{
    for (int i = 4; i < distanceLineEditsList.count(); i++)
    {
        if (bandChkBoxList[i]->isChecked())
        {
            distanceValues[i].ignoreEmptyDistance = true;
            distanceValues[i].ignoreEmptyDistanceChanged = true;
            ignoreEmptyDistanceChkBoxList[i]->setChecked(true);
        }
    }
}

void ClusterClientFilterDialog::onVhfClearAllEmptyDistPbClicked()
{
    for (int i = 0; i < 4; i++)
    {
        if (bandChkBoxList[i]->isChecked())
        {
            distanceValues[i].ignoreEmptyDistance = false;
            distanceValues[i].ignoreEmptyDistanceChanged = true;
            ignoreEmptyDistanceChkBoxList[i]->setChecked(false);
        }
    }
}

void ClusterClientFilterDialog::onUhfClearAllEmptyDistPbClicked()
{
    for (int i = 4; i < distanceLineEditsList.count(); i++)
    {
        if (bandChkBoxList[i]->isChecked())
        {
            distanceValues[i].ignoreEmptyDistance = false;
            distanceValues[i].ignoreEmptyDistanceChanged = true;
            ignoreEmptyDistanceChkBoxList[i]->setChecked(false);
        }
    }
}

