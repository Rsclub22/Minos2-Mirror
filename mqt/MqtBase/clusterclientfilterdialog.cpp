#include <QStringListModel>
#include "cutils.h"
#include "clusterclientfilterdialog.h"
#include "ui_clusterclientfilterdialog.h"

ClusterClientFilterDialog::ClusterClientFilterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ClusterClientFilterDialog),
    bandFilterMask(0),
    editBandFilterMask(0),
    modeFilterMask(0),
    editModeFilterMask(0),
    vhfButtonState(false),
    mWaveButtonState(false),
    modeButtonState(false),
    filterTabChanged(false),
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

    callsignListModel = new StringListModel(callsignFilterListTemp);
    connect(ui->callsignListWidget, SIGNAL(currentTextChanged(const QString&)()), this, SLOT(currentTextChanged(const QString&)()));
    connect(ui->callsignAddButton, SIGNAL(clicked()), SLOT(callsignAddClicked()));
    //ui->callsignEdit->setValidator(new UpperCaseValidator(true));

    connect(ui->vhfSelectBut, SIGNAL(clicked()), this, SLOT(vhfButtonSelected()));
    connect(ui->mWSelectBut, SIGNAL(clicked()), this, SLOT(mWaveButtonSelected()));
    connect(ui->modeSelectBut, SIGNAL(clicked()), this, SLOT(modeButtonSelected()));
    connect(ui->clearAllBut, SIGNAL(clicked()), this, SLOT(clearAllButtonSelected()));

    connect(ui->locatorEdit, SIGNAL(editingFinished()), this, SLOT(locatorEditFinished()));

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(filtersAccepted()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(filtersRejected()));

}






void ClusterClientFilterDialog::filtersAccepted()
{
    // copy updated masks with edited values
    bandFilterMask = editBandFilterMask;
    modeFilterMask = editModeFilterMask;
    emit filtersChanged();
    close();
}


void ClusterClientFilterDialog::filtersRejected()
{
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

    loadBandSettings(bandFilterMask);
    loadModeSettings(modeFilterMask);
}


void ClusterClientFilterDialog::closeEvent (QCloseEvent *event)
{
    restoreTabSettings();
    QWidget::closeEvent(event);
}

void ClusterClientFilterDialog::copyBandFilterMaskToEdit()
{
    editBandFilterMask = bandFilterMask;
}

void ClusterClientFilterDialog::copyModeFilterMaskToEdit()
{
    editModeFilterMask = modeFilterMask;
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
}

void ClusterClientFilterDialog::setVHFBands()
{
    editBandFilterMask = editBandFilterMask | _50M | _70M | _144M | _432M;
    for (int i = 0; i < vhfChkBoxList.count(); i++)
    {
        vhfChkBoxList[i]->setCheckState(Qt::Checked);
    }
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
    }


}


void ClusterClientFilterDialog::clearMWaveBands()
{
    editBandFilterMask = editBandFilterMask & ~_1296M & ~_2300M & ~_3_4G & ~_5_6G & ~_10G;
    for (int i = 0; i < mWaveChkBoxList.count(); i++)
    {
        mWaveChkBoxList[i]->setCheckState(Qt::Unchecked);
    }
}

void ClusterClientFilterDialog::setMWaveBands()
{
    editBandFilterMask = editBandFilterMask | _1296M | _2300M | _3_4G | _5_6G | _10G;
    for (int i = 0; i < mWaveChkBoxList.count(); i++)
    {
        mWaveChkBoxList[i]->setCheckState(Qt::Checked);
    }
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


unsigned int ClusterClientFilterDialog::getBandFilterMask()
{
    return bandFilterMask;
}
unsigned int ClusterClientFilterDialog::getModeFilterMask()
{
    return modeFilterMask;
}


void ClusterClientFilterDialog::currentTextChanged(const QString& text)
{
/*
    callsignFilterListTemp.clear();
    if (!ui->callsignEdit->text().isEmpty())
    {
        if (ui->callsignEdit->text().contains(FILTER_DELIMITER))
        {
            callsignFilterListTemp = ui->callsignEdit->text().split(FILTER_DELIMITER, QString::SkipEmptyParts);
            for (int i = 0; i < callsignFilterListTemp.count(); i++)
            {
                callsignFilterListTemp[i] = callsignFilterListTemp[i].trimmed();
            }
        }
        else
        {
            callsignFilterListTemp.append(ui->callsignEdit->text().trimmed());
        }

        callsignEditChanged = true;
    }
    else
    {
        if (!callsignFilterList.isEmpty())
        {
            callsignEditChanged = true;
        }
    }
*/
}


void ClusterClientFilterDialog::callsignAddClicked()
{

    QListWidgetItem *newItem = new QListWidgetItem;
    newItem->setText("");
    int row = callsignListWidget->count();
    callsignListWidget->insertItem(row, newItem);
}


void ClusterClientFilterDialog::locatorEditFinished()
{

}
