///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur RadRotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
//
//
/////////////////////////////////////////////////////////////////////////////



#include "bandmapclientfilterdialog.h"
#include "ui_bandmapclientfilterdialog.h"

BandmapClientFilterDialog::BandmapClientFilterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BandmapClientFilterDialog),
    modeButtonState(false)
{
    ui->setupUi(this);
    QSettings settings;
    QByteArray geometry = settings.value("ClusterClientFilter/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

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



    modeChkBoxList << ui->noneModeChkBox << ui->cwModeChkBox << ui->usbModeChkBox << ui->fmModeChkBox << ui->rttyModeChkBox << ui->psk31ModeChkBox << ui->ft8ModeChkBox << ui->msk144ModeChkBox << ui->jt65ModeChkBox;

   connect(ui->modeSelectBut, SIGNAL(clicked()), this, SLOT(modeButtonSelected()));


    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(filtersAccepted()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(filtersRejected()));

}


void BandmapClientFilterDialog::setContest(BaseContestLog *c)
{
    ct = dynamic_cast<LoggerContestLog *>( c);

}


void BandmapClientFilterDialog::filtersAccepted()
{
    bool modefilterChanged = false;



    if (modeFiltersChanged())
    {
        copyModeFiltersToFilterSettings();
        modefilterChanged = true;
    }


    if (modefilterChanged)
    {
        trace(QString("Bandmap Filters Changed - ContestUuid = %1").arg(contestUuid));
        trace(QString("Mode Filters CW = %1, USBMode = %2, FMMode = %3, RTTYMode = %4, PSK31Mode = %5, FT8Mode = %6, MSK144Mode = %7, JT65Mode = %8").arg(*filterSettings.modeFilters[CW_MODE]).arg(*filterSettings.modeFilters[USB_MODE]).arg(*filterSettings.modeFilters[FM_MODE]).arg(*filterSettings.modeFilters[RTTY_MODE]).arg(*filterSettings.modeFilters[PSK31_MODE]).arg(*filterSettings.modeFilters[FT8_MODE]).arg(*filterSettings.modeFilters[MSK144_MODE]).arg(*filterSettings.modeFilters[JT65_MODE]));
        trace(QString("Save to log"));
        saveBandmapFilterToContest();
    }

    emit filtersChanged(modefilterChanged);
    doCloseEvent();
    close();
}

void BandmapClientFilterDialog::filtersRejected()
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

    restoreModes();
    doCloseEvent();
    close();
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


