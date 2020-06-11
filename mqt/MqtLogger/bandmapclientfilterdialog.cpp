///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Bandmap Filter Dialog
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019 - 2020
//
//
//
/////////////////////////////////////////////////////////////////////////////



#include "bandmapclientfilterdialog.h"
#include "ui_bandmapclientfilterdialog.h"
#include "BandList.h"

int BandmapClientFilterDialog::mainTabIndex;

BandmapClientFilterDialog::BandmapClientFilterDialog(BaseContestLog *c, BandmapClientFilterSettings &filterSettings_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BandmapClientFilterDialog),
    modeButtonState(false),
    distanceChanged(false),
    distanceChkBoxChanged(false),
    distanceEmptyChkBoxChanged(false),
    settingsChanged(false)

{
    ui->setupUi(this);
    QSettings settings;
    QByteArray geometry = settings.value("BandmapClientFilter/geometry").toByteArray();
    if (geometry.size() > 0)
    {
        restoreGeometry(geometry);
    }

    filterSettings = filterSettings_;
    ct = dynamic_cast<LoggerContestLog *>(c);

    initCheckFilterTab();
}

BandmapClientFilterDialog::~BandmapClientFilterDialog()
{
    delete ui;
}

void BandmapClientFilterDialog::modeButtonSelected()
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



void BandmapClientFilterDialog::closeEvent (QCloseEvent *event)
{
    doCloseEvent();
    QWidget::closeEvent(event);
}


void BandmapClientFilterDialog::doCloseEvent()
{
    QSettings settings;
    settings.setValue("ClusterClientFilter/geometry", saveGeometry());
}


void BandmapClientFilterDialog::initCheckFilterTab()
{


    setWindowTitle("Bandmap Spot Filters");
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->bandmapFilterTab->setCurrentIndex(0);

    modeChkBoxList << ui->noneModeChkBox << ui->cwModeChkBox << ui->usbModeChkBox << ui->fmModeChkBox << ui->rttyModeChkBox << ui->psk31ModeChkBox << ui->ft8ModeChkBox << ui->msk144ModeChkBox << ui->jt65ModeChkBox;

   connect(ui->modeSelectBut, SIGNAL(clicked()), this, SLOT(modeButtonSelected()));

    connect(ui->spotDistanceEdit, SIGNAL(editingFinished()), this, SLOT(onDistanceEditFinished()));
    connect(ui->distFilterIgnoreCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onIgnoreDistanceChkBoxStateChanged(int)));
    connect(ui->ignoreEmptyDistanceValuesChkBox, SIGNAL(stateChanged(int)), this, SLOT(onIgnoreEmptyDistanceValuesChkBoxStateChanged(int)));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(filtersAccepted()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(filtersRejected()));

    connect(ui->bandmapFilterTab, SIGNAL(currentChanged(int)), this, SLOT(onFilterTabIndexChanged(int)));

    setFilterTabCurrentIndex(mainTabIndex);
    loadSettingsToDialogBox();

}

/*
void BandmapClientFilterDialog::setContest(BaseContestLog *c)
{
    ct = dynamic_cast<LoggerContestLog *>( c);
}
*/

void BandmapClientFilterDialog::onFilterTabIndexChanged(int idx)
{
    mainTabIndex = idx;
}



void BandmapClientFilterDialog::setFilterTabCurrentIndex(int idx)
{
    ui->bandmapFilterTab->setCurrentIndex(idx);
}

int BandmapClientFilterDialog::getFilterTabCurrentIndex()
{
    return ui->bandmapFilterTab->currentIndex();
}


void BandmapClientFilterDialog::loadSettingsToDialogBox()
{

    loadDistanceFilterEditBox();
    loadIgnoreDistanceChkBoxState();
    loadIgnoreEmptyDistanceValuesChkBoxState();
    copyModeFiltersToDialog();

}


void BandmapClientFilterDialog::filtersAccepted()
{
    bool modefilterChanged = false;



    if (modeFiltersChanged())
    {
        copyModeFiltersToFilterSettings();
        modefilterChanged = true;
    }


    if (modefilterChanged || distanceChanged || distanceChkBoxChanged || distanceEmptyChkBoxChanged)
    {

        trace(QString("Bandmap Filters Changed - ContestUuid = %1").arg(contestUuid));
        if (modefilterChanged)
        {
            trace(QString("Mode Filters CW = %1, USBMode = %2, FMMode = %3, RTTYMode = %4, PSK31Mode = %5, FT8Mode = %6, MSK144Mode = %7, JT65Mode = %8").arg(*filterSettings.modeFilters[CW_MODE]).arg(*filterSettings.modeFilters[USB_MODE]).arg(*filterSettings.modeFilters[FM_MODE]).arg(*filterSettings.modeFilters[RTTY_MODE]).arg(*filterSettings.modeFilters[PSK31_MODE]).arg(*filterSettings.modeFilters[FT8_MODE]).arg(*filterSettings.modeFilters[MSK144_MODE]).arg(*filterSettings.modeFilters[JT65_MODE]));
        }
        if (distanceChanged)
        {
            trace(QString("Bandmap Filter Distance Changed = %1").arg(QString::number(filterSettings.distanceFilter)));
        }
        if (distanceChkBoxChanged)
        {
            trace(QString("Bandmap Filter Distance Checkbox Changed Ignore Distance = %1").arg(filterSettings.ignoreDistanceFlag ? "true" : "false"));
        }
        if (distanceEmptyChkBoxChanged)
        {
            trace(QString("Bandmap Filter Distance Checkbox Changed Ignore Empty Distance = %1, Ignore Empty Distance = %2").arg(filterSettings.ignoreEmptyDistanceFlag ? "true" : "false"));
        }
        trace(QString("Save to log"));
        saveBandmapFilterToContest();
        settingsChanged = true;
    }

    //emit filtersChanged(modefilterChanged);
    doCloseEvent();
    //close();
}

void BandmapClientFilterDialog::filtersRejected()
{


    //restoreModes();
    doCloseEvent();
    //close();
}


void BandmapClientFilterDialog::onDistanceEditFinished()
{
    bool ok;
    int distance = 0;
    if(!ui->spotDistanceEdit->text().isEmpty())
    {
        distance = ui->spotDistanceEdit->text().toInt(&ok);
        if (ok)
        {
            if (distance != filterSettings.distanceFilter)
            {
                filterSettings.distanceFilter = distance;
                distanceChanged = true;
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





void BandmapClientFilterDialog::loadDistanceFilterEditBox()
{

    ui->spotDistanceEdit->setText(QString::number(filterSettings.distanceFilter));
}

void BandmapClientFilterDialog::onIgnoreDistanceChkBoxStateChanged(int state)
{
    Q_UNUSED(state)
    if (ui->distFilterIgnoreCheckBox->isChecked() != filterSettings.ignoreDistanceFlag)
    {
        filterSettings.ignoreDistanceFlag = ui->distFilterIgnoreCheckBox->isChecked();
        distanceChkBoxChanged = true;
    }



}

void BandmapClientFilterDialog::loadIgnoreDistanceChkBoxState()
{
    if (filterSettings.ignoreDistanceFlag)
    {
        ui->distFilterIgnoreCheckBox->setCheckState(Qt::Checked );
    }
    else
    {
        ui->distFilterIgnoreCheckBox->setCheckState(Qt::Unchecked);
    }
}
void BandmapClientFilterDialog::onIgnoreEmptyDistanceValuesChkBoxStateChanged(int state)
{
    Q_UNUSED(state)

    if (ui->ignoreEmptyDistanceValuesChkBox->isChecked() != filterSettings.ignoreEmptyDistanceFlag)
    {
        filterSettings.ignoreEmptyDistanceFlag = ui->ignoreEmptyDistanceValuesChkBox->isChecked();
        distanceEmptyChkBoxChanged = true;
    }
}
void BandmapClientFilterDialog::loadIgnoreEmptyDistanceValuesChkBoxState()
{
    if (filterSettings.ignoreEmptyDistanceFlag)
    {
        ui->ignoreEmptyDistanceValuesChkBox->setCheckState(Qt::Checked );
    }
    else
    {
        ui->ignoreEmptyDistanceValuesChkBox->setCheckState(Qt::Unchecked);
    }
}


void BandmapClientFilterDialog::saveBandmapFilterToContest()
{
    ct->saveBandmapFilter(filterSettings);
}


bool BandmapClientFilterDialog::modeFiltersChanged()
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

void BandmapClientFilterDialog::copyModeFiltersToFilterSettings()
{
    for (int i = 0; i < modeChkBoxList.count(); i++)
    {
        *filterSettings.modeFilters[i] = modeChkBoxList[i]->isChecked();
    }
}

void BandmapClientFilterDialog::copyModeFiltersToDialog()
{
    for (int i = 0; i < modeChkBoxList.count(); i++)
    {
        modeChkBoxList[i]->setChecked(*filterSettings.modeFilters[i]);
    }
}





void BandmapClientFilterDialog::clearModes()
{
    for (int i = 0; i <modeChkBoxList.count(); i++)
    {
        modeChkBoxList[i]->setCheckState(Qt::Unchecked);

    }

}

void BandmapClientFilterDialog::setModes()
{
    for (int i = 0; i < modeChkBoxList.count(); i++)
    {
        modeChkBoxList[i]->setCheckState(Qt::Checked);

    }

}

/*
void BandmapClientFilterDialog::setModeFilter(bool state, int mode)
{
    if (mode >= 0 && mode < clusterModes.count())
    {
        *filterSettings.modeFilters[mode] = state;
    }
}


void BandmapClientFilterDialog::restoreModes()
{

    for (int i = 0; i < clusterModes.count(); i++)
    {

        modeChkBoxList[i]->setChecked(filterSettings.modeFilters[i]);

    }
}

*/
