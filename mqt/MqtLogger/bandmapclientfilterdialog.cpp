///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Bandmap Filter Dialog
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019 - 2020
//
//
//
/////////////////////////////////////////////////////////////////////////////

#include <QMessageBox>
#include "MTrace.h"

#include "bandmapclientfilterdialog.h"
#include "ui_bandmapclientfilterdialog.h"

int BandmapClientFilterDialog::mainTabIndex;

BandmapClientFilterDialog::BandmapClientFilterDialog(BaseContestLog *c, BandmapClientFilterSettings &filterSettings_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BandmapClientFilterDialog),
    distanceChanged(false),
    distanceChkBoxChanged(false),
    distanceEmptyChkBoxChanged(false),
    settingsChanged(false)

{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

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
    if (areAnyModesSet())
    {
        clearModes();

    }
    else
    {
        setModes();

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
    settings.setValue("BandmapClientFilter/geometry", saveGeometry());
}


void BandmapClientFilterDialog::initCheckFilterTab()
{
    setWindowTitle("Bandmap Spot Filters");

    ui->bandmapFilterTab->setCurrentIndex(0);

    modeChkBoxList << ui->noneModeChkBox << ui->cwModeChkBox << ui->lsbModeChkBox << ui->usbModeChkBox << ui->fmModeChkBox << ui->rttyModeChkBox << ui->psk31ModeChkBox << ui->ft8ModeChkBox << ui->ft4ModeChkBox << ui->msk144ModeChkBox << ui->jt65ModeChkBox;

    if (clusterModes.count() == modeChkBoxList.count())
    {
        for (int i = 0; i < clusterModes.count(); i++)
        {
            modeCheckBoxes.insert(clusterModes[i], modeChkBoxList[i]);
        }
    }
    else
    {
        trace(QString("initCheckFilterTab: clusterModes %1 != modeChkBoxList %2").arg(clusterModes.count(), modeChkBoxList.count()));
    }


    ui->modeSelectBut->setToolTip(tr("Click to set/reset all modes"));
    connect(ui->modeSelectBut, &QPushButton::clicked, this, &BandmapClientFilterDialog::modeButtonSelected);

    connect(ui->spotDistanceEdit, &QLineEdit::editingFinished, this, &BandmapClientFilterDialog::onDistanceEditFinished);
    connect(ui->distFilterIgnoreCheckBox, &QCheckBox::stateChanged, this, &BandmapClientFilterDialog::onIgnoreDistanceChkBoxStateChanged);
    connect(ui->ignoreEmptyDistanceValuesChkBox, &QCheckBox::stateChanged, this, &BandmapClientFilterDialog::onIgnoreEmptyDistanceValuesChkBoxStateChanged);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &BandmapClientFilterDialog::filtersAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &BandmapClientFilterDialog::filtersRejected);

    connect(ui->bandmapFilterTab, &QTabWidget::currentChanged, this, &BandmapClientFilterDialog::onFilterTabIndexChanged);

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
            trace(QString("Mode Filters CW = %1, LSBMode = %2, USBMode = %3, FMMode = %4, RTTYMode = %5, PSK31Mode = %6, FT8Mode = %7, MSK144Mode = %8, JT65Mode = %9")
                  .arg(filterSettings.getModeFilter(CW_MODE) ? "true" : "false")
                  .arg(filterSettings.getModeFilter(LSB_MODE)  ? "true" : "false")
                  .arg(filterSettings.getModeFilter(USB_MODE)  ? "true" : "false")
                  .arg(filterSettings.getModeFilter(FM_MODE)  ? "true" : "false").arg(filterSettings.getModeFilter(RTTY_MODE)  ? "true" : "false")
                  .arg(filterSettings.getModeFilter(PSK31_MODE) ? "true" : "false").arg(filterSettings.getModeFilter(FT8_MODE)  ? "true" : "false")
                  .arg(filterSettings.getModeFilter(MSK144_MODE)  ? "true" : "false").arg(filterSettings.getModeFilter(JT65_MODE)  ? "true" : "false"));
        }
        if (distanceChanged)
        {
            trace(QString("Bandmap Filter Distance Changed = %1").arg(QString::number(filterSettings.getDistanceFilter())));
        }
        if (distanceChkBoxChanged)
        {
            trace(QString("Bandmap Filter Distance Checkbox Changed Ignore Distance = %1").arg(filterSettings.getIgnoreDistanceFlag() ? "true" : "false"));
        }
        if (distanceEmptyChkBoxChanged)
        {
            trace(QString("Bandmap Filter Distance Checkbox Changed Ignore Empty Distance = %1, Ignore Empty Distance = %2").arg(filterSettings.getIgnoreEmptyDistanceFlag() ? "true" : "false"));
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

    doCloseEvent();

}


void BandmapClientFilterDialog::onDistanceEditFinished()
{
    bool ok;
    int distance = ui->spotDistanceEdit->text().toInt(&ok);
    if (ok)
    {
        if (distance != filterSettings.getDistanceFilter())
        {
            filterSettings.setDistanceFilter(distance);
            distanceChanged = true;
        }
    }
    else
    {
        QMessageBox::information(this, tr("Distance Filter"),
                                 tr("Please enter a number between %1 and %2!").arg(MIN_FILTER_DISTANCE).arg(MAX_FILTER_DISTANCE),
                                  QMessageBox::Ok|QMessageBox::NoButton, QMessageBox::NoButton);
        ui->spotDistanceEdit->setText(QString::number(filterSettings.getDistanceFilter()));
    }
}





void BandmapClientFilterDialog::loadDistanceFilterEditBox()
{

    ui->spotDistanceEdit->setText(QString::number(filterSettings.getDistanceFilter()));
}

void BandmapClientFilterDialog::onIgnoreDistanceChkBoxStateChanged(int state)
{
    Q_UNUSED(state)
    if (ui->distFilterIgnoreCheckBox->isChecked() != filterSettings.getIgnoreDistanceFlag())
    {
        filterSettings.setIgnoreDistanceFlag(ui->distFilterIgnoreCheckBox->isChecked());
        distanceChkBoxChanged = true;
    }



}

void BandmapClientFilterDialog::loadIgnoreDistanceChkBoxState()
{
    if (filterSettings.getIgnoreDistanceFlag())
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

    if (ui->ignoreEmptyDistanceValuesChkBox->isChecked() != filterSettings.getIgnoreEmptyDistanceFlag())
    {
        filterSettings.setIgnoreEmptyDistanceFlag(ui->ignoreEmptyDistanceValuesChkBox->isChecked());
        distanceEmptyChkBoxChanged = true;
    }
}
void BandmapClientFilterDialog::loadIgnoreEmptyDistanceValuesChkBoxState()
{
    if (filterSettings.getIgnoreEmptyDistanceFlag())
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
    for (auto &m:clusterModes)
    {
        if (filterSettings.getModeFilter(m) != modeCheckBoxes.value(m)->isChecked())
        {
            return true;
        }
    }

    return false;
}

void BandmapClientFilterDialog::copyModeFiltersToFilterSettings()
{
    for (auto &m:clusterModes)
    {
        filterSettings.setModeFilter(m, modeCheckBoxes.value(m)->isChecked());
    }
}

void BandmapClientFilterDialog::copyModeFiltersToDialog()
{
    for (auto &m:clusterModes)
    {
        modeCheckBoxes.value(m)->setChecked(filterSettings.getModeFilter(m));
    }
}





void BandmapClientFilterDialog::clearModes()
{
    for (auto &m:clusterModes)
    {
        modeCheckBoxes.value(m)->setCheckState(Qt::Unchecked);

    }

}

void BandmapClientFilterDialog::setModes()
{
    for (auto &m:clusterModes)
    {
        modeCheckBoxes.value(m)->setCheckState(Qt::Checked);

    }

}

bool BandmapClientFilterDialog::areAnyModesSet()
{
    for (auto &m:clusterModes)
    {
        if (modeCheckBoxes.value(m)->isChecked())
        {
            return true;
        }
    }

    return false;
}
