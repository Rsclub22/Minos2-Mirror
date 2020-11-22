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
#include "ContestApp.h"
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
    modeButtonState(false),
    bandFilterChangedFlag(false),
    modeFilterChangedFlag(false),
    callsignFilterChangedFlag(false),
    locatorFilterChangedFlag(false),
    distanceFilterChangedFlag(false),
    ignoreDistanceChangedFlag(false),
    ignoreEmptyDistanceChangedFlag(false),
    settingsChangedFlag(false)
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



    allBandChkBoxList << ui->_1_8MHzCheckBox << ui->_3_5MHzCheckBox  << ui->_7MHzCheckBox
                   << ui->_14MHzCheckBox << ui->_21MHzCheckBox << ui->_28MHzCheckBox
                   << ui->_50MHzCheckBox << ui->_70MHzCheckBox << ui->_144MHzCheckBox << ui->_432MHzCheckBox
                   << ui->_1296MHzCheckBox << ui->_2300MHzCheckBox << ui->_3_4GHzCheckBox << ui->_5_6GHzCheckBox << ui->_10GHzCheckBox;


    for (int i = 0; i < allBandChkBoxList.count(); i++)
    {
        connect(allBandChkBoxList[i], &QCheckBox::clicked, [=](){onBandChkBoxChecked(i);});
    }

    hfBandChkBoxList << ui->_1_8MHzCheckBox << ui->_3_5MHzCheckBox  << ui->_7MHzCheckBox
                     << ui->_14MHzCheckBox << ui->_21MHzCheckBox << ui->_28MHzCheckBox;

    vhfBandChkBoxList << ui->_50MHzCheckBox << ui->_70MHzCheckBox << ui->_144MHzCheckBox << ui->_432MHzCheckBox;
    mwBandChkBoxList << ui->_1296MHzCheckBox << ui->_2300MHzCheckBox << ui->_3_4GHzCheckBox << ui->_5_6GHzCheckBox << ui->_10GHzCheckBox;



    for (int i = 0; i < allBandChkBoxList.count(); i++)
    {
        if (*filterSettings.allBandFilters[i])
        {
            allBandChkBoxList[i]->setCheckState(Qt::Checked);
        }
        else
        {
            allBandChkBoxList[i]->setCheckState(Qt::Unchecked);
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

    allDistanceLineEditsList << ui->spotDistanceEdit_1_8MHz << ui->spotDistanceEdit_3_5MHz << ui->spotDistanceEdit_7MHz
                          << ui->spotDistanceEdit_14MHz << ui->spotDistanceEdit_21MHz << ui->spotDistanceEdit_28MHz
                          << ui->spotDistanceEdit_50MHz << ui->spotDistanceEdit_70MHz << ui->spotDistanceEdit_144MHz << ui->spotDistanceEdit_432MHz
                          << ui->spotDistanceEdit_1296MHz << ui->spotDistanceEdit_2300MHz << ui->spotDistanceEdit_3_4GHz << ui->spotDistanceEdit_5_6GHz << ui->spotDistanceEdit_10GHz;

    hfDistanceLineEditsList << ui->spotDistanceEdit_1_8MHz << ui->spotDistanceEdit_3_5MHz << ui->spotDistanceEdit_7MHz
                            << ui->spotDistanceEdit_14MHz << ui->spotDistanceEdit_21MHz << ui->spotDistanceEdit_28MHz;

    vhfDistanceLineEditsList << ui->spotDistanceEdit_50MHz << ui->spotDistanceEdit_70MHz << ui->spotDistanceEdit_144MHz << ui->spotDistanceEdit_432MHz;

    mwDistanceLineEditsList << ui->spotDistanceEdit_1296MHz << ui->spotDistanceEdit_2300MHz << ui->spotDistanceEdit_3_4GHz << ui->spotDistanceEdit_5_6GHz << ui->spotDistanceEdit_10GHz;

    for (int i = 0; i < filterSettings.allDistanceFilters.count(); i++)
    {

        allDistanceLineEditsList[i]->setText(QString::number(*filterSettings.allDistanceFilters[i]));

    }

    for (int i = 0; i < allDistanceLineEditsList.count(); i++)
    {
        connect(allDistanceLineEditsList[i], &QLineEdit::editingFinished,  [=](){onDistanceEditingFinished(i);});
    }

    allIgnoreDistanceChkBoxList << ui->distFilterIgnoreCheckBox_1_8MHz << ui->distFilterIgnoreCheckBox_3_5MHz << ui->distFilterIgnoreCheckBox_7MHz
                             << ui->distFilterIgnoreCheckBox_14MHz << ui->distFilterIgnoreCheckBox_21MHz << ui->distFilterIgnoreCheckBox_28MHz
                             << ui->distFilterIgnoreCheckBox_50MHz << ui->distFilterIgnoreCheckBox_70MHz << ui->distFilterIgnoreCheckBox_144MHz << ui->distFilterIgnoreCheckBox_432MHz
                             << ui->distFilterIgnoreCheckBox_1296MHz << ui->distFilterIgnoreCheckBox_2300MHz << ui->distFilterIgnoreCheckBox_3_4GHz << ui->distFilterIgnoreCheckBox_5_6GHz << ui->distFilterIgnoreCheckBox_10GHz;

   hfIgnoreDistanceChkBoxList << ui->distFilterIgnoreCheckBox_1_8MHz << ui->distFilterIgnoreCheckBox_3_5MHz << ui->distFilterIgnoreCheckBox_7MHz
                              << ui->distFilterIgnoreCheckBox_14MHz << ui->distFilterIgnoreCheckBox_21MHz << ui->distFilterIgnoreCheckBox_28MHz;

   vhfIgnoreDistanceChkBoxList << ui->distFilterIgnoreCheckBox_50MHz << ui->distFilterIgnoreCheckBox_70MHz << ui->distFilterIgnoreCheckBox_144MHz << ui->distFilterIgnoreCheckBox_432MHz;

   mwIgnoreDistanceChkBoxList << ui->distFilterIgnoreCheckBox_1296MHz << ui->distFilterIgnoreCheckBox_2300MHz << ui->distFilterIgnoreCheckBox_3_4GHz << ui->distFilterIgnoreCheckBox_5_6GHz << ui->distFilterIgnoreCheckBox_10GHz;



    for (int i = 0; i < allIgnoreDistanceChkBoxList.count(); i++)
    {
        connect(allIgnoreDistanceChkBoxList[i], &QCheckBox::clicked, [=](){onIgnoreDistanceChecked(i);});
    }

    for (int i = 0; i < allIgnoreDistanceChkBoxList.count(); i++)
    {
        if (*filterSettings.allIgnoreDistanceFlags[i])
        {

            allIgnoreDistanceChkBoxList[i]->setCheckState(Qt::Checked);

        }
        else
        {
            allIgnoreDistanceChkBoxList[i]->setCheckState(Qt::Unchecked);
        }

    }

    allIgnoreEmptyDistanceChkBoxList << ui->ignoreEmptyDistanceValuesChkBox_1_8MHz << ui->ignoreEmptyDistanceValuesChkBox_3_5MHz  << ui->ignoreEmptyDistanceValuesChkBox_7MHz
                                  << ui->ignoreEmptyDistanceValuesChkBox_14MHz << ui->ignoreEmptyDistanceValuesChkBox_21MHz << ui->ignoreEmptyDistanceValuesChkBox_28MHz
                                  << ui->ignoreEmptyDistanceValuesChkBox_50MHz << ui->ignoreEmptyDistanceValuesChkBox_70MHz << ui->ignoreEmptyDistanceValuesChkBox_144MHz << ui->ignoreEmptyDistanceValuesChkBox_432MHz
                                  << ui->ignoreEmptyDistanceValuesChkBox_1296MHz << ui->ignoreEmptyDistanceValuesChkBox_2300MHz << ui->ignoreEmptyDistanceValuesChkBox_3_4GHz << ui->ignoreEmptyDistanceValuesChkBox_5_6GHz << ui->ignoreEmptyDistanceValuesChkBox_10GHz;

    hfIgnoreEmptyDistanceChkBoxList << ui->ignoreEmptyDistanceValuesChkBox_1_8MHz << ui->ignoreEmptyDistanceValuesChkBox_3_5MHz  << ui->ignoreEmptyDistanceValuesChkBox_7MHz
                                    << ui->ignoreEmptyDistanceValuesChkBox_14MHz << ui->ignoreEmptyDistanceValuesChkBox_21MHz << ui->ignoreEmptyDistanceValuesChkBox_28MHz;

    vhfIgnoreEmptyDistanceChkBoxList << ui->ignoreEmptyDistanceValuesChkBox_50MHz << ui->ignoreEmptyDistanceValuesChkBox_70MHz << ui->ignoreEmptyDistanceValuesChkBox_144MHz << ui->ignoreEmptyDistanceValuesChkBox_432MHz;

    mwIgnoreEmptyDistanceChkBoxList << ui->ignoreEmptyDistanceValuesChkBox_1296MHz << ui->ignoreEmptyDistanceValuesChkBox_2300MHz << ui->ignoreEmptyDistanceValuesChkBox_3_4GHz << ui->ignoreEmptyDistanceValuesChkBox_5_6GHz << ui->ignoreEmptyDistanceValuesChkBox_10GHz;



    for (int i = 0; i < allIgnoreEmptyDistanceChkBoxList.count(); i++)
    {
        connect(allIgnoreEmptyDistanceChkBoxList[i], &QCheckBox::clicked, [=](){onIgnoreEmptyDistanceChecked(i);});
    }

    for (int i = 0; i < allIgnoreEmptyDistanceChkBoxList.count(); i++)
    {
        if (*filterSettings.allIgnoreEmptyDistanceFlags[i])
        {
            allIgnoreEmptyDistanceChkBoxList[i]->setCheckState(Qt::Checked);
        }
        else
        {
            allIgnoreEmptyDistanceChkBoxList[i]->setCheckState(Qt::Unchecked);
        }
    }

    allDistanceLabelsList << ui->bandLabel_1_8MHz << ui->bandLabel_3_5MHz << ui->bandLabel_7MHz
                          << ui->bandLabel_14MHz << ui->bandLabel_21MHz << ui->bandLabel_28MHz
                          << ui->bandLabel_50MHz << ui->bandLabel_70MHz << ui->bandLabel_144MHz << ui->bandLabel_432MHz
                          << ui->bandLabel_1296MHz << ui->bandLabel_2300MHz << ui->bandLabel_3_4GHz << ui->bandLabel_5_6GHz << ui->bandLabel_10GHz;


    hfDistanceLabelsList << ui->bandLabel_1_8MHz << ui->bandLabel_3_5MHz << ui->bandLabel_7MHz
                         << ui->bandLabel_14MHz << ui->bandLabel_21MHz << ui->bandLabel_28MHz;

    vhfDistanceLabelsList << ui->bandLabel_14MHz << ui->bandLabel_21MHz << ui->bandLabel_28MHz
                          << ui->bandLabel_50MHz << ui->bandLabel_70MHz << ui->bandLabel_144MHz << ui->bandLabel_432MHz;

    mwDistanceLabelsList << ui->bandLabel_1296MHz << ui->bandLabel_2300MHz << ui->bandLabel_3_4GHz
                         << ui->bandLabel_5_6GHz << ui->bandLabel_10GHz;


    connect(ui->hfSetAlDefaultDistPb, &QPushButton::clicked, [=](){onHfSetDefDistPbClicked();});
    connect(ui->vhfSetAlDefaultDistPb, &QPushButton::clicked, [=](){onVhfSetDefDistPbClicked();});
    connect(ui->mwSetAlDefaultDistPb, &QPushButton::clicked, [=](){onMwSetDefDistPbClicked();});

    connect(ui->hfSetAllIgnorePb, &QPushButton::clicked, [=](){onHfSetAllIgnorePbClicked();});
    connect(ui->vhfSetAllIgnorePb, &QPushButton::clicked, [=](){onVhfSetAllIgnorePbClicked();});
    connect(ui->mwSetAllIgnorePb, &QPushButton::clicked, [=](){onMwSetAllIgnorePbClicked();});

    connect(ui->hfClearAllIgnorePb, &QPushButton::clicked, [=](){onHfClearAllIgnorePbClicked();});
    connect(ui->vhfClearAllIgnorePb, &QPushButton::clicked, [=](){onVhfClearAllIgnorePbClicked();});
    connect(ui->mwClearAllIgnorePb, &QPushButton::clicked, [=](){onMwClearAllIgnorePbClicked();});

    connect(ui->hfSetAllEmptyDistPb, &QPushButton::clicked, [=](){onHfSetAllEmptyPbClicked();});
    connect(ui->vhfSetAllEmptyDistPb, &QPushButton::clicked, [=](){onVhfSetAllEmptyPbClicked();});
    connect(ui->mwSetAllEmptyDistPb, &QPushButton::clicked, [=](){onMwSetAllEmptyPbClicked();});

    connect(ui->hfClearAllEmptyDistPb, &QPushButton::clicked, [=](){onHfClearAllEmptyDistPbClicked();});
    connect(ui->vhfClearAllEmptyDistPb, &QPushButton::clicked, [=](){onVhfClearAllEmptyDistPbClicked();});
    connect(ui->mwClearAllEmptyDistPb, &QPushButton::clicked, [=](){onMwClearAllEmptyDistPbClicked();});



    callsignListWidget = ui->callsignListWidget;
    callsignListWidget->addItems(filterSettings.unpackFilterList(filterSettings.callsignFilterList));


    //connect(ui->callsignListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(callsignCurrentRowChanged(int)));


    connect(ui->callsignAddButton, &QPushButton::clicked, [=](){callsignAddClicked();});
    connect(ui->callsignEditButton, &QPushButton::clicked, [=](){callsignEditClicked();});
    connect(ui->callsignDelButton, &QPushButton::clicked, [=](){callsignDelClicked();});
    connect(ui->callsignDelAllButton, &QPushButton::clicked, [=](){callsignDelAllClicked();});
    connect(ui->saveCallsignList, &QPushButton::clicked, [=](){onCallsignListSave();});
    connect(ui->importCallsignList, &QPushButton::clicked, [=](){onCallsignListImport();});

    locatorListWidget = ui->locatorListWidget;
    locatorListWidget->addItems(filterSettings.unpackFilterList(filterSettings.locatorFilterList));

    //connect(ui->locatorListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(locatorCurrentRowChanged(int)));

    connect(ui->locatorAddButton, &QPushButton::clicked, [=](){locatorAddClicked();});
    connect(ui->locatorEditButton, &QPushButton::clicked, [=](){locatorEditClicked();});
    connect(ui->locatorDelButton, &QPushButton::clicked, [=](){locatorDelClicked();});
    connect(ui->locatorDelAllButton, &QPushButton::clicked, [=](){locatorDelAllClicked();});
    connect(ui->saveLocatorList, &QPushButton::clicked, [=](){onLocatorListSave();});
    connect(ui->importLocatorList, &QPushButton::clicked, [=](){onLocatorListImport();});

    connect(ui->vhfSelectBut, &QPushButton::clicked, [=](){vhfButtonSelected();});
    connect(ui->mWSelectBut, &QPushButton::clicked, [=](){mWaveButtonSelected();});
    connect(ui->modeSelectBut, &QPushButton::clicked, [=](){modeButtonSelected();});
    connect(ui->clearAllBut,   &QPushButton::clicked, [=](){clearAllButtonSelected();});


    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=](){filtersAccepted();});
    connect(ui->buttonBox, &QDialogButtonBox::rejected, [=](){filtersRejected();});

    connect(ui->ClusterClientFilterTab, &QTabWidget::currentChanged,  [=](int index){onFilterTabIndexChanged(index);});
    connect(ui->filterDistancesTab,  &QTabWidget::currentChanged,  [=](int index){onDistanceFilterTabIndexChanged(index);});

    setFilterTabCurrentIndex(mainTabIndex);
    setDistanceFilterTabCurrentIndex(distanceTabIndex);

    enableDistanceFields();

    bool allowHF = false;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpAllowHF, allowHF );
    setHFVisible(allowHF);
    if (allowHF)
    {
        ui->ClusterClientFilterTab->setCurrentIndex(0);
    }
    else
    {
        ui->ClusterClientFilterTab->setCurrentIndex(1);
    }


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
    bool bandFilterChangedFlag = false;
    bool modeFilterChangedFlag = false;
    bool callsignFilterChangedFlag = false;
    bool locatorFilterChangedFlag = false;
    bool distanceFilterChangedFlag = false;
    bool ignoreDistanceChangedFlag = false;
    bool ignoreEmptyDistanceChangedFlag = false;

    // check bandfilter changed
    for (int i = 0; i < allBandChkBoxList.count(); i++)
    {
        if (*filterSettings.allBandFilters[i] != allBandChkBoxList[i]->isChecked())
        {
            *filterSettings.allBandFilters[i] = allBandChkBoxList[i]->isChecked();
            trace(QString("BandFilter Changed = %1 , State = %2, ContestUuid = %3").arg(i).arg(allBandChkBoxList[i]->isChecked()  ? "True" : "False").arg(contestUuid));

            bandFilterChangedFlag = true;
        }

    }

    //check modefilter changed
    for (int i = 0; i < modeChkBoxList.count(); i++)
    {
        if (*filterSettings.modeFilters[i] != modeChkBoxList[i]->isChecked())
        {
            *filterSettings.modeFilters[i] = modeChkBoxList[i]->isChecked();
            trace(QString("ModeFilter Changed = %1 , State = %2, ContestUuid = %3").arg(i).arg(modeChkBoxList[i]->isChecked()  ? "True" : "False").arg(contestUuid));

            modeFilterChangedFlag = true;
        }
    }

    if (callsignFiltersChanged())
    {

        filterSettings.callsignFilterList.clear();
        filterSettings.callsignFilterList = filterSettings.packFilterList(getItemsTextFromListWidget(callsignListWidget));
        callsignFilterChangedFlag = true;
    }

    if (locatorFiltersChanged())
    {
        filterSettings.locatorFilterList.clear();
        filterSettings.locatorFilterList = filterSettings.packFilterList(getItemsTextFromListWidget(locatorListWidget));
        locatorFilterChangedFlag = true;
    }


    for (int i = 0; i < allDistanceLineEditsList.count(); i++)
    {
        if (*filterSettings.allBandFilters[i])
        {

            if (allDistanceLineEditsList[i]->text() != QString::number(*filterSettings.allDistanceFilters[i]))
            {
                *filterSettings.allDistanceFilters[i] = allDistanceLineEditsList[i]->text().trimmed().toInt();
                trace(QString("Distance Filter Changed Band = %1 , Distance = %2, ContestUuid = %3").arg(i).arg(allDistanceLineEditsList[i]->text()).arg(contestUuid));

                distanceFilterChangedFlag = true;
            }
        }
    }



    for (int i = 0; i < allIgnoreDistanceChkBoxList.count(); i++)
    {
        if (*filterSettings.allBandFilters[i])
        {
            if (allIgnoreDistanceChkBoxList[i]->isChecked() != *filterSettings.allIgnoreDistanceFlags[i])
            {
                *filterSettings.allIgnoreDistanceFlags[i] = allIgnoreDistanceChkBoxList[i]->isChecked();
                trace(QString("Ignore Distance Checked Band = %1 , State = %2, ContestUuid = %3").arg(i).arg(allIgnoreDistanceChkBoxList[i]->isChecked()  ? "True" : "False").arg(contestUuid));

                ignoreDistanceChangedFlag = true;
            }

        }

    }



    for (int i = 0; i < allIgnoreEmptyDistanceChkBoxList.count();i++)
    {
        if (*filterSettings.allBandFilters[i])
        {
            if (allIgnoreEmptyDistanceChkBoxList[i]->isChecked() != *filterSettings.allIgnoreEmptyDistanceFlags[i])
            {
                *filterSettings.allIgnoreEmptyDistanceFlags[i] = allIgnoreEmptyDistanceChkBoxList[i]->isChecked();
                trace(QString("Ignore Empty Distance Checked Band = %1 , State = %2, ContestUuid = %3").arg(i).arg(allIgnoreEmptyDistanceChkBoxList[i]->isChecked()  ? "True" : "False").arg(contestUuid));

                ignoreEmptyDistanceChangedFlag = true;
            }

        }
    }


    if (bandFilterChangedFlag || modeFilterChangedFlag
            || callsignFilterChangedFlag || locatorFilterChangedFlag
            || distanceFilterChangedFlag || ignoreDistanceChangedFlag || ignoreEmptyDistanceChangedFlag )
    {
        trace(QString("Cluster Filters Changed - Save to Contest ContestUuid = %1").arg(contestUuid));

        saveClusterFilterToContest();
        settingsChangedFlag = true;

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




void ClusterClientFilterDialog::filtersRejected()
{

    doCloseEvent();

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



void ClusterClientFilterDialog::copyBandFiltersToDialog()
{
    for (int i = 0; i < allBandChkBoxList.count(); i++)
    {
       allBandChkBoxList[i]->setChecked(*filterSettings.allBandFilters[i]);
    }
}




void ClusterClientFilterDialog::setHFVisible(bool state)
{
    ui->hfSelectBut->setVisible(state);
    ui->_1_8MHzCheckBox->setVisible(state);
    ui->_3_5MHzCheckBox->setVisible(state);
    ui->_7MHzCheckBox->setVisible(state);
    ui->_14MHzCheckBox->setVisible(state);
    ui->_21MHzCheckBox->setVisible(state);
    ui->_28MHzCheckBox->setVisible(state);


    //ui->HF_DistanceTab->setVisible(state);  // only works in Qt 5.15
    QString hfTabName = "HF";
    if (state)
    {
        // set hf Tab "visible"
        if (ui->filterDistancesTab->tabText(0) != hfTabName)
        {
            QWidget *hfTab = ui->filterDistancesTab->findChild<QWidget *>(hfTabName);
            if (hfTab)
            {
               ui->filterDistancesTab->insertTab(0, hfTab,hfTabName);
            }
        }
    }
    else
    {
        // set hf tab "invisible"
        QString n = ui->filterDistancesTab->tabText(0);
        if (ui->filterDistancesTab->tabText(0) == hfTabName)
        {
            ui->filterDistancesTab->removeTab(0);
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


void ClusterClientFilterDialog::setBandFilter(int band)
{
    if (band > NO_BANDS && band < allBandChkBoxList.count())
    {
        *filterSettings.allBandFilters[band] = true;
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
    for (int i = 0; i < vhfBandChkBoxList.count(); i++)
    {
        vhfBandChkBoxList[i]->setCheckState(Qt::Unchecked);

    }

}

void ClusterClientFilterDialog::setVHFBands()
{
    for (int i = 0; i < vhfBandChkBoxList.count(); i++)
    {
        vhfBandChkBoxList[i]->setCheckState(Qt::Checked);

    }

}



void ClusterClientFilterDialog::restoreBands()
{

    for (int i = 0; allBandChkBoxList.count(); i++)
    {
        allBandChkBoxList[i]->setChecked(*filterSettings.allBandFilters[i]);

    }



}



void ClusterClientFilterDialog::clearMWaveBands()
{
    for (int i = 0; i < mwBandChkBoxList.count(); i++)
    {
        mwBandChkBoxList[i]->setCheckState(Qt::Unchecked);

    }

}

void ClusterClientFilterDialog::setMWaveBands()
{
    for (int i = 0; i < mwBandChkBoxList.count(); i++)
    {
        mwBandChkBoxList[i]->setCheckState(Qt::Checked);

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
    return *filterSettings.allBandFilters[bandNum];
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
    for (auto const &str: filterSettings.unpackFilterList(filterSettings.callsignFilterList))
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
    for(auto const &str: filterSettings.unpackFilterList( filterSettings.locatorFilterList))
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
    for (int i = 0; (i < allBandChkBoxList.count() && i < allDistanceLineEditsList.count()); i++)
    {
        if (allBandChkBoxList[i]->checkState() == Qt::Checked)
        {
            allDistanceLineEditsList[i]->setEnabled(true);
            allIgnoreDistanceChkBoxList[i]->setEnabled(true);
            allIgnoreEmptyDistanceChkBoxList[i]->setEnabled(true);
            allDistanceLabelsList[i]->setEnabled(true);
        }
        else
        {
            allDistanceLineEditsList[i]->setEnabled(false);
            allIgnoreDistanceChkBoxList[i]->setEnabled(false);
            allIgnoreEmptyDistanceChkBoxList[i]->setEnabled(false);
            allDistanceLabelsList[i]->setEnabled(false);
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
    if(!allDistanceLineEditsList[idx]->text().isEmpty())
    {
        distance = allDistanceLineEditsList[idx]->text().toInt(&ok);
        if (ok)
        {
            return;

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
    //if (ignoreDistanceChkBoxList[idx]->isChecked() != *filterSettings.ignoreDistanceFlags[idx])
    //{
    //    distanceValues[idx].ignoreDistance = ignoreDistanceChkBoxList[idx]->isChecked();
    //    distanceValues[idx].ignoreDistChanged = true;
    //}
}

void ClusterClientFilterDialog::onIgnoreEmptyDistanceChecked(int idx)
{
    //if (ignoreEmptyDistanceChkBoxList[idx]->isChecked() != *filterSettings.ignoreEmptyDistanceFlags[idx])
    //{
    //    distanceValues[idx].ignoreEmptyDistance = ignoreEmptyDistanceChkBoxList[idx]->isChecked();
    //    distanceValues[idx].ignoreEmptyDistanceChanged = true;
    //}
}

void ClusterClientFilterDialog::onHfSetDefDistPbClicked()
{
    ClusterFilterIdAndNames clustId;
    for (int i = 0; i < clustId.getHfDefaultFilterNameCount(); i++)
    {
        if (hfBandChkBoxList[i]->isChecked())
        {
            int hfDefault = 0;
            TContestApp::getContestApp() ->loggerBundle.getIntProfile( clustId.getHfDefaultFilterId(i), hfDefault );
            hfDistanceLineEditsList[i]->setText(QString::number(hfDefault));

        }
    }

}


void ClusterClientFilterDialog::onVhfSetDefDistPbClicked()
{
    ClusterFilterIdAndNames clustId;
    for (int i = 0; i < clustId.getVhfDefaultFilterNameCount(); i++)
    {
        if (vhfBandChkBoxList[i]->isChecked())
        {
            int vhfDefault = 0;
            TContestApp::getContestApp() ->loggerBundle.getIntProfile( clustId.getVhfDefaultFilterId(i), vhfDefault );
            vhfDistanceLineEditsList[i]->setText(QString::number(vhfDefault));

        }
    }

}





void ClusterClientFilterDialog::onMwSetDefDistPbClicked()
{
    ClusterFilterIdAndNames clustId;
    for (int i = 0; i < clustId.getMwDefaultFilterNameCount(); i++)
    {
        if (mwBandChkBoxList[i]->isChecked())
        {
            int mwDefault = 0;
            TContestApp::getContestApp() ->loggerBundle.getIntProfile( clustId.getMwDefaultFilterId(i), mwDefault );
            mwDistanceLineEditsList[i]->setText(QString::number(mwDefault));

        }
    }
}



void ClusterClientFilterDialog::onHfSetAllIgnorePbClicked()
{
    for (int i = 0; i < hfIgnoreDistanceChkBoxList.count(); i++)
    {
        if (hfBandChkBoxList[i]->isChecked())
        {
            hfIgnoreDistanceChkBoxList[i]->setChecked(true);
        }

    }
}




void ClusterClientFilterDialog::onVhfSetAllIgnorePbClicked()
{
    for (int i = 0; i < vhfIgnoreDistanceChkBoxList.count(); i++)
    {
        if (vhfBandChkBoxList[i]->isChecked())
        {
            vhfIgnoreDistanceChkBoxList[i]->setChecked(true);
        }

    }
}

void ClusterClientFilterDialog::onMwSetAllIgnorePbClicked()
{
    for (int i = 0; i < mwIgnoreDistanceChkBoxList.count(); i++)
    {
        if (mwBandChkBoxList[i]->isChecked())
        {
            mwIgnoreDistanceChkBoxList[i]->setChecked(true);
        }
    }
}


void ClusterClientFilterDialog::onHfClearAllIgnorePbClicked()
{
    for (int i = 0; i < hfIgnoreDistanceChkBoxList.count(); i++)
    {
        if (hfBandChkBoxList[i]->isChecked())
        {
            hfIgnoreDistanceChkBoxList[i]->setChecked(false);
        }
    }
}


void ClusterClientFilterDialog::onVhfClearAllIgnorePbClicked()
{
    for (int i = 0; i < vhfIgnoreDistanceChkBoxList.count(); i++)
    {
        if (vhfBandChkBoxList[i]->isChecked())
        {
            vhfIgnoreDistanceChkBoxList[i]->setChecked(false);
        }
    }
}

void ClusterClientFilterDialog::onMwClearAllIgnorePbClicked()
{
    for (int i = 4; i < mwIgnoreDistanceChkBoxList.count(); i++)
    {
        if (mwBandChkBoxList[i]->isChecked())
        {
            mwIgnoreDistanceChkBoxList[i]->setChecked(false);
        }
    }
}


void ClusterClientFilterDialog::onHfSetAllEmptyPbClicked()
{
    for (int i = 0; i < hfIgnoreEmptyDistanceChkBoxList.count(); i++)
    {
        if (hfBandChkBoxList[i]->isChecked())
        {
            hfIgnoreEmptyDistanceChkBoxList[i]->setChecked(true);
        }

    }
}


void ClusterClientFilterDialog::onVhfSetAllEmptyPbClicked()
{
    for (int i = 0; i < vhfIgnoreEmptyDistanceChkBoxList.count(); i++)
    {
        if (vhfBandChkBoxList[i]->isChecked())
        {
            vhfIgnoreEmptyDistanceChkBoxList[i]->setChecked(true);
        }

    }
}

void ClusterClientFilterDialog::onMwSetAllEmptyPbClicked()
{
    for (int i = 4; i < mwIgnoreEmptyDistanceChkBoxList.count(); i++)
    {
        if (mwBandChkBoxList[i]->isChecked())
        {
            mwIgnoreEmptyDistanceChkBoxList[i]->setChecked(true);
        }
    }
}

void ClusterClientFilterDialog::onHfClearAllEmptyDistPbClicked()
{
    for (int i = 0; i < hfIgnoreEmptyDistanceChkBoxList.count(); i++)
    {
        if (hfBandChkBoxList[i]->isChecked())
        {
            hfIgnoreEmptyDistanceChkBoxList[i]->setChecked(false);
        }
    }
}




void ClusterClientFilterDialog::onVhfClearAllEmptyDistPbClicked()
{
    for (int i = 0; i < vhfIgnoreEmptyDistanceChkBoxList.count(); i++)
    {
        if (vhfBandChkBoxList[i]->isChecked())
        {
            vhfIgnoreEmptyDistanceChkBoxList[i]->setChecked(false);
        }
    }
}

void ClusterClientFilterDialog::onMwClearAllEmptyDistPbClicked()
{
    for (int i = 4; i < mwIgnoreEmptyDistanceChkBoxList.count(); i++)
    {
        if (mwBandChkBoxList[i]->isChecked())
        {
            mwIgnoreEmptyDistanceChkBoxList[i]->setChecked(false);
        }
    }
}

