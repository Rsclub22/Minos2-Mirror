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

ClusterClientFilterDialog::ClusterClientFilterDialog(BaseContestLog *c, const ClusterClientFilterSettings &filterSettings_, const QVector<QSharedPointer<BandInfo> > &bands_, const QStringList &clustermodes_, QWidget *parent) :

    QDialog(parent),
    ui(new Ui::ClusterClientFilterDialog),
    callsignListWidgetCurrentRow(-1),
    hfButtonState(false),
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

    bands = bands_;
    clustermodes = clustermodes_;


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

    ClusterClientBandFilterDialogDetails ccfd;
    for (int i = 0; i < bands.count(); i++)
    {
        ccfd.bandChkBox = allBandChkBoxList[i];
        ccfd.bandType = bands[i].data()->getType();
        bandCheckBoxes.insert(bands[i].data()->uk, ccfd);
    }

    for (int i = 0; i < allBandChkBoxList.count(); i++)
    {
        connect(allBandChkBoxList[i], &QCheckBox::clicked, this, [=](){onBandChkBoxChecked(allBandChkBoxList[i]);});
    }


    for (auto &b: bands)
    {
        QString band = b.data()->uk;

        bandCheckBoxes.value(band).bandChkBox->setChecked(filterSettings.getBandFilter(band));

    }



    modeChkBoxList << ui->noneModeChkBox << ui->cwModeChkBox << ui->usbModeChkBox << ui->fmModeChkBox << ui->rttyModeChkBox << ui->psk31ModeChkBox << ui->ft8ModeChkBox << ui->msk144ModeChkBox << ui->jt65ModeChkBox;

    for (int i = 0; i < clustermodes.count(); i++)
    {
        modeCheckBoxes.insert(clustermodes[i], modeChkBoxList[i]);
    }

    for (auto &m:clustermodes)
    {

        modeCheckBoxes.value(m)->setChecked(filterSettings.getModeFilter(m));

    }



    allDistanceLineEditsList << ui->spotDistanceEdit_1_8MHz << ui->spotDistanceEdit_3_5MHz << ui->spotDistanceEdit_7MHz
                          << ui->spotDistanceEdit_14MHz << ui->spotDistanceEdit_21MHz << ui->spotDistanceEdit_28MHz
                          << ui->spotDistanceEdit_50MHz << ui->spotDistanceEdit_70MHz << ui->spotDistanceEdit_144MHz << ui->spotDistanceEdit_432MHz
                          << ui->spotDistanceEdit_1296MHz << ui->spotDistanceEdit_2300MHz << ui->spotDistanceEdit_3_4GHz << ui->spotDistanceEdit_5_6GHz << ui->spotDistanceEdit_10GHz;


    allDistanceLabelsList << ui->bandLabel_1_8MHz << ui->bandLabel_3_5MHz << ui->bandLabel_7MHz
                          << ui->bandLabel_14MHz << ui->bandLabel_21MHz << ui->bandLabel_28MHz
                          << ui->bandLabel_50MHz << ui->bandLabel_70MHz << ui->bandLabel_144MHz << ui->bandLabel_432MHz
                          << ui->bandLabel_1296MHz << ui->bandLabel_2300MHz << ui->bandLabel_3_4GHz << ui->bandLabel_5_6GHz << ui->bandLabel_10GHz;

    allIgnoreDistanceChkBoxList << ui->distFilterIgnoreCheckBox_1_8MHz << ui->distFilterIgnoreCheckBox_3_5MHz << ui->distFilterIgnoreCheckBox_7MHz
                             << ui->distFilterIgnoreCheckBox_14MHz << ui->distFilterIgnoreCheckBox_21MHz << ui->distFilterIgnoreCheckBox_28MHz
                             << ui->distFilterIgnoreCheckBox_50MHz << ui->distFilterIgnoreCheckBox_70MHz << ui->distFilterIgnoreCheckBox_144MHz << ui->distFilterIgnoreCheckBox_432MHz
                             << ui->distFilterIgnoreCheckBox_1296MHz << ui->distFilterIgnoreCheckBox_2300MHz << ui->distFilterIgnoreCheckBox_3_4GHz << ui->distFilterIgnoreCheckBox_5_6GHz << ui->distFilterIgnoreCheckBox_10GHz;

    allIgnoreEmptyDistanceChkBoxList << ui->ignoreEmptyDistanceValuesChkBox_1_8MHz << ui->ignoreEmptyDistanceValuesChkBox_3_5MHz  << ui->ignoreEmptyDistanceValuesChkBox_7MHz
                                  << ui->ignoreEmptyDistanceValuesChkBox_14MHz << ui->ignoreEmptyDistanceValuesChkBox_21MHz << ui->ignoreEmptyDistanceValuesChkBox_28MHz
                                  << ui->ignoreEmptyDistanceValuesChkBox_50MHz << ui->ignoreEmptyDistanceValuesChkBox_70MHz << ui->ignoreEmptyDistanceValuesChkBox_144MHz << ui->ignoreEmptyDistanceValuesChkBox_432MHz
                                  << ui->ignoreEmptyDistanceValuesChkBox_1296MHz << ui->ignoreEmptyDistanceValuesChkBox_2300MHz << ui->ignoreEmptyDistanceValuesChkBox_3_4GHz << ui->ignoreEmptyDistanceValuesChkBox_5_6GHz << ui->ignoreEmptyDistanceValuesChkBox_10GHz;



    ClusterClientDistanceFilterDetails ccdfd;
    for (int i = 0; i < bands.count(); i++)
    {
        ccdfd.bandLineEdit = allDistanceLineEditsList[i];
        ccdfd.bandLabel = allDistanceLabelsList[i];
        ccdfd.distFilterIgnoreCheckBox = allIgnoreDistanceChkBoxList[i];
        ccdfd.distFilterIgnoreEmptyCheckBox = allIgnoreDistanceChkBoxList[i];
        ccdfd.bandType = bands[i].data()->getType();
        bandDistanceWidgets.insert(bands[i].data()->uk, ccdfd);
    }


    for (auto &b: bands)
    {

        QString band = b.data()->uk;

        bandDistanceWidgets.value(band).bandLineEdit->setText(QString::number(filterSettings.getDistanceFilter(band)));

        if (filterSettings.getIgnoreDistanceFlag(band))
        {
            bandDistanceWidgets.value(band).distFilterIgnoreCheckBox->setChecked(true);
        }
        else
        {
            bandDistanceWidgets.value(band).distFilterIgnoreCheckBox->setChecked(false);
        }

        if (filterSettings.getIgnoreEmptyDistanceFlag(band))
        {
           bandDistanceWidgets.value(band).distFilterIgnoreCheckBox->setChecked(true);
        }
        else
        {
           bandDistanceWidgets.value(band).distFilterIgnoreCheckBox->setChecked(false);
        }

    }

    for (int i = 0; i < allDistanceLineEditsList.count(); i++)
    {
        // set the distance enable/disable
        connect(allDistanceLineEditsList[i], &QLineEdit::editingFinished,  this, [=](){onDistanceEditingFinished(allDistanceLineEditsList[i]);});
    }



    //for (int i = 0; i < allIgnoreDistanceChkBoxList.count(); i++)
    //{
    //    connect(allIgnoreDistanceChkBoxList[i], &QCheckBox::clicked, this, [=](){onIgnoreDistanceChecked(allIgnoreDistanceChkBoxList[i]);});
    //}




    //for (int i = 0; i < allIgnoreEmptyDistanceChkBoxList.count(); i++)
    //{
    //    connect(allIgnoreEmptyDistanceChkBoxList[i], &QCheckBox::clicked, this, [=](){onIgnoreEmptyDistanceChecked(allIgnoreEmptyDistanceChkBoxList[i]);});
    //}




    connect(ui->hfSetAlDefaultDistPb, &QPushButton::clicked, this, [=](){onHfSetDefDistPbClicked();});
    connect(ui->vhfSetAlDefaultDistPb, &QPushButton::clicked, this, [=](){onVhfSetDefDistPbClicked();});
    connect(ui->mwSetAlDefaultDistPb, &QPushButton::clicked, this, [=](){onMwSetDefDistPbClicked();});

    connect(ui->hfSetAllIgnorePb, &QPushButton::clicked, this, [=](){onHfSetAllIgnorePbClicked();});
    connect(ui->vhfSetAllIgnorePb, &QPushButton::clicked, this, [=](){onVhfSetAllIgnorePbClicked();});
    connect(ui->mwSetAllIgnorePb, &QPushButton::clicked, this, [=](){onMwSetAllIgnorePbClicked();});

    connect(ui->hfClearAllIgnorePb, &QPushButton::clicked, this, [=](){onHfClearAllIgnorePbClicked();});
    connect(ui->vhfClearAllIgnorePb, &QPushButton::clicked, this, [=](){onVhfClearAllIgnorePbClicked();});
    connect(ui->mwClearAllIgnorePb, &QPushButton::clicked, this, [=](){onMwClearAllIgnorePbClicked();});

    connect(ui->hfSetAllEmptyDistPb, &QPushButton::clicked, this, [=](){onHfSetAllEmptyPbClicked();});
    connect(ui->vhfSetAllEmptyDistPb, &QPushButton::clicked, this, [=](){onVhfSetAllEmptyPbClicked();});
    connect(ui->mwSetAllEmptyDistPb, &QPushButton::clicked, this, [=](){onMwSetAllEmptyPbClicked();});

    connect(ui->hfClearAllEmptyDistPb, &QPushButton::clicked, this, [=](){onHfClearAllEmptyDistPbClicked();});
    connect(ui->vhfClearAllEmptyDistPb, &QPushButton::clicked, this, [=](){onVhfClearAllEmptyDistPbClicked();});
    connect(ui->mwClearAllEmptyDistPb, &QPushButton::clicked, this, [=](){onMwClearAllEmptyDistPbClicked();});



    callsignListWidget = ui->callsignListWidget;
    callsignListWidget->addItems(filterSettings.unpackFilterList(filterSettings.callsignFilterList));


    //connect(ui->callsignListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(callsignCurrentRowChanged(int)));


    connect(ui->callsignAddButton, &QPushButton::clicked, this, [=](){callsignAddClicked();});
    connect(ui->callsignEditButton, &QPushButton::clicked, this, [=](){callsignEditClicked();});
    connect(ui->callsignDelButton, &QPushButton::clicked, this, [=](){callsignDelClicked();});
    connect(ui->callsignDelAllButton, &QPushButton::clicked, this, [=](){callsignDelAllClicked();});
    connect(ui->saveCallsignList, &QPushButton::clicked, this, [=](){onCallsignListSave();});
    connect(ui->importCallsignList, &QPushButton::clicked, this, [=](){onCallsignListImport();});

    locatorListWidget = ui->locatorListWidget;
    locatorListWidget->addItems(filterSettings.unpackFilterList(filterSettings.locatorFilterList));

    //connect(ui->locatorListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(locatorCurrentRowChanged(int)));

    connect(ui->locatorAddButton, &QPushButton::clicked, this, [=](){locatorAddClicked();});
    connect(ui->locatorEditButton, &QPushButton::clicked, this, [=](){locatorEditClicked();});
    connect(ui->locatorDelButton, &QPushButton::clicked, this, [=](){locatorDelClicked();});
    connect(ui->locatorDelAllButton, &QPushButton::clicked, this, [=](){locatorDelAllClicked();});
    connect(ui->saveLocatorList, &QPushButton::clicked, this, [=](){onLocatorListSave();});
    connect(ui->importLocatorList, &QPushButton::clicked, this, [=](){onLocatorListImport();});

    connect(ui->hfSelectBut, &QPushButton::clicked, this, [=](){hfButtonSelected();});
    connect(ui->vhfSelectBut, &QPushButton::clicked, this, [=](){vhfButtonSelected();});
    connect(ui->mWSelectBut, &QPushButton::clicked, this, [=](){mWaveButtonSelected();});
    connect(ui->modeSelectBut, &QPushButton::clicked, this, [=](){modeButtonSelected();});
    connect(ui->clearAllBut,   &QPushButton::clicked, this, [=](){clearAllButtonSelected();});


    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [=](){filtersAccepted();});
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, [=](){filtersRejected();});

    connect(ui->ClusterClientFilterTab, &QTabWidget::currentChanged,  this, [=](int index){onFilterTabIndexChanged(index);});
    connect(ui->filterDistancesTab,  &QTabWidget::currentChanged,  this, [=](int index){onDistanceFilterTabIndexChanged(index);});

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
    for (auto &b: bands)
    {
        QString band = b.data()->uk;

        if (filterSettings.getBandFilter(band) != bandCheckBoxes.value(band).bandChkBox->isChecked())
        {
            filterSettings.setBandFilter(band, bandCheckBoxes.value(band).bandChkBox->isChecked());
            trace(QString("BandFilter Changed = %1 , State = %2, ContestUuid = %3").arg(band).arg(bandCheckBoxes.value(band).bandChkBox->isChecked()  ? "True" : "False").arg(contestUuid));

            bandFilterChangedFlag = true;
        }
    }


    //check modefilter changed
    for (auto &m: clustermodes)
    {
        if (filterSettings.getModeFilter(m) != modeCheckBoxes.value(m)->isChecked())
        {
            filterSettings.setModeFilter(m, modeCheckBoxes.value(m)->isChecked());
            trace(QString("ModeFilter Changed = %1 , State = %2, ContestUuid = %3").arg(m).arg(modeCheckBoxes.value(m)->isChecked()  ? "True" : "False").arg(contestUuid));

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


    for (auto &b: bands)
    {
        QString band = b.data()->uk;
        if (filterSettings.getBandFilter(band))
        {
            if(bandDistanceWidgets.value(band).bandLineEdit->text() != QString::number(filterSettings.getDistanceFilter(band)))
            {
                filterSettings.setDistanceFilter(band, bandDistanceWidgets.value(band).bandLineEdit->text().toInt());
                trace(QString("Distance Filter Changed Band = %1 , Distance = %2, ContestUuid = %3").arg(band).arg(bandDistanceWidgets.value(band).bandLineEdit->text()).arg(contestUuid));

                distanceFilterChangedFlag = true;
            }
        }
    }


    for (auto &b:bands)
    {
        QString band = b.data()->uk;

        if (filterSettings.getBandFilter(band))
        {
            if(bandDistanceWidgets.value(band).distFilterIgnoreCheckBox->isChecked() != filterSettings.getIgnoreDistanceFlag(band))
            {
                filterSettings.setIgnoreDistanceFlag(band, bandDistanceWidgets.value(band).distFilterIgnoreCheckBox->isChecked());
                trace(QString("Ignore Distance Checked Band = %1 , State = %2, ContestUuid = %3").arg(band).arg(bandDistanceWidgets.value(band).distFilterIgnoreCheckBox->isChecked()  ? "True" : "False").arg(contestUuid));

                ignoreDistanceChangedFlag = true;
            }
        }
    }


    for (auto &b:bands)
    {
        QString band = b.data()->uk;

        if (filterSettings.getBandFilter(band))
        {
            if(bandDistanceWidgets.value(band).distFilterIgnoreEmptyCheckBox->isChecked() != filterSettings.getIgnoreEmptyDistanceFlag(band))
            {
                filterSettings.setIgnoreEmptyDistanceFlag(band, bandDistanceWidgets.value(band).distFilterIgnoreEmptyCheckBox->isChecked());
                trace(QString("Ignore Empty Distance Checked Band = %1 , State = %2, ContestUuid = %3").arg(band, bandDistanceWidgets.value(band).distFilterIgnoreEmptyCheckBox->isChecked()  ? "True" : "False", contestUuid));

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
    for (auto &m:clustermodes)
    {
        if (filterSettings.getModeFilter(m) != modeCheckBoxes.value(m)->isChecked())
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
    for (auto &b: bands)
    {
        bandCheckBoxes.value(b.data()->uk).bandChkBox->setChecked(filterSettings.getBandFilter(b.data()->uk));
    }

}




void ClusterClientFilterDialog::setHFVisible(bool state)
{
    ui->hfSelectBut->setVisible(state);



    for (auto &b:bands)
    {
        if (b->getType() == "HF")
        {
            bandCheckBoxes.value(b.data()->uk).bandChkBox->setVisible(state);
            bandDistanceWidgets.value(b.data()->uk).bandLabel->setVisible(state);
            bandDistanceWidgets.value(b.data()->uk).bandLineEdit->setVisible(state);
            bandDistanceWidgets.value(b.data()->uk).distFilterIgnoreCheckBox->setVisible(state);
            bandDistanceWidgets.value(b.data()->uk).distFilterIgnoreEmptyCheckBox->setVisible(state);
        }
    }


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
        //QString n = ui->filterDistancesTab->tabText(0);
        if (ui->filterDistancesTab->tabText(0) == hfTabName)
        {
            ui->filterDistancesTab->removeTab(0);
        }
    }


}


void ClusterClientFilterDialog::hfButtonSelected()
{
    if (!hfButtonState)
    {
        hfButtonState = true;
        setBandsCheckBox(HF_BANDTYPE, hfButtonState);
    }
    else
    {
        hfButtonState = false;
        setBandsCheckBox(HF_BANDTYPE, hfButtonState);
    }
}




void ClusterClientFilterDialog::vhfButtonSelected()
{
    if (!vhfButtonState)
    {
        vhfButtonState = true;
        setBandsCheckBox(VHF_BANDTYPE, vhfButtonState);
    }
    else
    {
        vhfButtonState = false;
        setBandsCheckBox(VHF_BANDTYPE, vhfButtonState);
    }
}

void ClusterClientFilterDialog::mWaveButtonSelected()
{
    if (!mWaveButtonState)
    {
        mWaveButtonState = true;
        setBandsCheckBox(MW_BANDTYPE, mWaveButtonState);
    }
    else
    {
        mWaveButtonState = false;
        setBandsCheckBox(MW_BANDTYPE, mWaveButtonState);
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


void ClusterClientFilterDialog::setBandFilter(QString band, bool state)
{

    filterSettings.setBandFilter(band, state);

}


void ClusterClientFilterDialog::setModeFilter(QString mode, bool state)
{
    filterSettings.setModeFilter(mode, state);
}

void ClusterClientFilterDialog::clearAllButtonSelected()
{
    clearAllFilters();
}


void ClusterClientFilterDialog::clearAllFilters()
{
    setBandsCheckBox(HF_BANDTYPE, false);
    hfButtonState = false;
    setBandsCheckBox(VHF_BANDTYPE, false);
    vhfButtonState = false;
    setBandsCheckBox(MW_BANDTYPE, false);
    mWaveButtonState = false;
    clearModes();
    modeButtonState = false;

}




void ClusterClientFilterDialog::setBandsCheckBox(QString bandType, bool state)
{
    for (auto &b:bands)
    {
        if (b->getType() == bandType)
        {
            bandCheckBoxes.value(b.data()->uk).bandChkBox->setChecked(state);
        }
    }

}

void ClusterClientFilterDialog::restoreBands()
{

    for (auto &b: bands)
    {
        QString band = b.data()->uk;

        if (filterSettings.getBandFilter(band))
        {
           bandCheckBoxes.value(band).bandChkBox->setChecked(true);
        }
        else
        {
           bandCheckBoxes.value(band).bandChkBox->setChecked(false);
        }

    }
}







void ClusterClientFilterDialog::clearModes()
{
    for (auto &m:clustermodes)
    {
        modeCheckBoxes.value(m)->setChecked(false);
    }

}

void ClusterClientFilterDialog::setModes()
{
    for (auto &m:clustermodes)
    {
        modeCheckBoxes.value(m)->setChecked(true);
    }

}


void ClusterClientFilterDialog::restoreModes()
{

    for (auto &m: clustermodes)
    {
        modeCheckBoxes.value(m)->setChecked(filterSettings.getModeFilter(m));

    }

}


bool  ClusterClientFilterDialog::checkBandMatch(QString band)
{
    return filterSettings.getBandFilter(band);
}

bool ClusterClientFilterDialog::checkModeMatch(QString mode)
{
    return filterSettings.getModeFilter(mode);
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

    for (auto &b: bands)
    {
        QString band = b.data()->uk;

        if (bandCheckBoxes.value(band).bandChkBox->isChecked())
        {
            setDistanceFields(band, true);
        }
        else
        {
           setDistanceFields(band, false);
        }
    }



}

void ClusterClientFilterDialog::setDistanceFields(QString band, bool state)
{
    bandDistanceWidgets.value(band).bandLabel->setEnabled(state);
    bandDistanceWidgets.value(band).bandLineEdit->setEnabled(state);
    bandDistanceWidgets.value(band).distFilterIgnoreCheckBox->setEnabled(state);
    bandDistanceWidgets.value(band).distFilterIgnoreEmptyCheckBox->setEnabled(state);

}

void ClusterClientFilterDialog::onBandChkBoxChecked(QCheckBox * bandChkBox)
{
    Q_UNUSED(bandChkBox)
    enableDistanceFields();

}



void ClusterClientFilterDialog::onDistanceEditingFinished(QLineEdit *distanceLineEdit)
{

    QString band = findBandQLineEdit(distanceLineEdit);

    if (band.isEmpty())
    {
        bool ok;
        int distance = 0;
        if(!bandDistanceWidgets.value(band).bandLineEdit->text().isEmpty())
        {
            distance = bandDistanceWidgets.value(band).bandLineEdit->text().toInt(&ok);
            if (ok && (distance >= MIN_FILTER_DISTANCE && distance <= MAX_FILTER_DISTANCE))
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



}

QString ClusterClientFilterDialog::findBandQLineEdit(QLineEdit *distanceLineEdit)
{
    QMapIterator<QString, ClusterClientDistanceFilterDetails> i(bandDistanceWidgets);
    while (i.hasNext())
    {
        i.next();
        if (i.value().bandLineEdit == distanceLineEdit)
        {
            return i.key();
        }

    }

    return "";
}

void ClusterClientFilterDialog::onIgnoreDistanceChecked(QCheckBox *ignoreDistChkBox)
{
    Q_UNUSED(ignoreDistChkBox)
    //if (ignoreDistanceChkBoxList[idx]->isChecked() != *filterSettings.ignoreDistanceFlags[idx])
    //{
    //    distanceValues[idx].ignoreDistance = ignoreDistanceChkBoxList[idx]->isChecked();
    //    distanceValues[idx].ignoreDistChanged = true;
    //}
}

void ClusterClientFilterDialog::onIgnoreEmptyDistanceChecked(QCheckBox* ignoreEmptyCheckBox)
{
    Q_UNUSED(ignoreEmptyCheckBox)
    //if (ignoreEmptyDistanceChkBoxList[idx]->isChecked() != *filterSettings.ignoreEmptyDistanceFlags[idx])
    //{
    //    distanceValues[idx].ignoreEmptyDistance = ignoreEmptyDistanceChkBoxList[idx]->isChecked();
    //    distanceValues[idx].ignoreEmptyDistanceChanged = true;
    //}
}

void ClusterClientFilterDialog::onHfSetDefDistPbClicked()
{

    setDefDistances(HF_BANDTYPE);

}
void ClusterClientFilterDialog::onVhfSetDefDistPbClicked()
{
    setDefDistances(VHF_BANDTYPE);
}

void ClusterClientFilterDialog::onMwSetDefDistPbClicked()
{
    setDefDistances(MW_BANDTYPE);
}

void ClusterClientFilterDialog::setDefDistances(QString bandType)
{
    QSettings settings(CLUSTER_FILTER_FILE, QSettings::IniFormat);
    settings.beginGroup("Default Distance");

    ClusterFilterDefaultDistIniName defaultDistIniNames;
    defaultDistIniNames.initClusterFilterIdAndNames(bands);



    for (auto &b:bands)
    {
        QString band = b.data()->uk;

        if (b->getType() == bandType && bandCheckBoxes.contains(band))
        {

            if (bandCheckBoxes.value(band).bandChkBox->isChecked())
            {
                int hfDefault = settings.value(defaultDistIniNames.getDefaultDistIniName(band).defaultDistanceName, DEFAULT_FILTER_DISTANCE).toInt();
                bandDistanceWidgets.value(band).bandLineEdit->setText(QString::number(hfDefault));
            }

        }
        else
        {
            trace(QString("setDefDistances - band missing =%1").arg(band));
        }
    }

    settings.endGroup();

}




void ClusterClientFilterDialog::onHfSetAllIgnorePbClicked()
{
    setIgnoreDistCheckBox(HF_BANDTYPE, true);
}


void ClusterClientFilterDialog::onVhfSetAllIgnorePbClicked()
{
    setIgnoreDistCheckBox(VHF_BANDTYPE, true);
}

void ClusterClientFilterDialog::onMwSetAllIgnorePbClicked()
{
    setIgnoreDistCheckBox(MW_BANDTYPE, true);
}


void ClusterClientFilterDialog::onHfClearAllIgnorePbClicked()
{
    setIgnoreDistCheckBox(VHF_BANDTYPE, false);
}


void ClusterClientFilterDialog::onVhfClearAllIgnorePbClicked()
{
    setIgnoreDistCheckBox(VHF_BANDTYPE, false);
}

void ClusterClientFilterDialog::onMwClearAllIgnorePbClicked()
{
    setIgnoreDistCheckBox(MW_BANDTYPE, false);
}

void ClusterClientFilterDialog::setIgnoreDistCheckBox(QString bandType, bool state)
{
    for (auto &b:bands)
    {
        QString band = b.data()->uk;

        if(b->getType() == bandType)
        {
            if (bandCheckBoxes.value(band).bandChkBox->isChecked())
            {
               bandDistanceWidgets.value(band).distFilterIgnoreCheckBox->setChecked(state);
            }
        }

    }
}


void ClusterClientFilterDialog::onHfSetAllEmptyPbClicked()
{
    setEmptyDistCheckBox(VHF_BANDTYPE, true);
}


void ClusterClientFilterDialog::onVhfSetAllEmptyPbClicked()
{
    setEmptyDistCheckBox(VHF_BANDTYPE, true);
}

void ClusterClientFilterDialog::onMwSetAllEmptyPbClicked()
{
    setEmptyDistCheckBox(MW_BANDTYPE, true);
}

void ClusterClientFilterDialog::onHfClearAllEmptyDistPbClicked()
{
    setEmptyDistCheckBox(HF_BANDTYPE, false);
}




void ClusterClientFilterDialog::onVhfClearAllEmptyDistPbClicked()
{
    setEmptyDistCheckBox(VHF_BANDTYPE, false);
}

void ClusterClientFilterDialog::onMwClearAllEmptyDistPbClicked()
{
    setEmptyDistCheckBox(MW_BANDTYPE, false);
}

void ClusterClientFilterDialog::setEmptyDistCheckBox(QString bandType, bool state)
{
    for (auto &b:bands)
    {
        if(b->getType() == bandType)
        {
            if (bandCheckBoxes.value(b.data()->uk).bandChkBox->isChecked())
            {
               bandDistanceWidgets.value(b.data()->uk).distFilterIgnoreEmptyCheckBox->setChecked(state);
            }
        }

    }
}

