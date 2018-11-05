#include "clusterclientfiltertab.h"
#include "ui_clusterclientfiltertab.h"



ClusterClientFilterTab::ClusterClientFilterTab(QWidget *parent) :
    QTabWidget(parent)
    ,ui(new Ui::ClusterClientFilterTab)
    ,bandFilterMask(0)
    ,editBandFilterMask(0)
    ,modeFilterMask(0)
    ,editModeFilterMask(0)
    ,vhfButtonState(false)
    ,mWaveButtonState(false)
    ,modeButtonState(false)
    ,filterTabChanged(false)
{
    ui->setupUi(this);

    initCheckFilterTab();

}

ClusterClientFilterTab::~ClusterClientFilterTab()
{
    delete ui;
}



void ClusterClientFilterTab::initCheckFilterTab()
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

    setCurrentIndex(0);


    connect(ui->vhfSelectBut, SIGNAL(clicked()), this, SLOT(vhfButtonSelected()));
    connect(ui->mWSelectBut, SIGNAL(clicked()), this, SLOT(mWaveButtonSelected()));
    connect(ui->modeSelectBut, SIGNAL(clicked()), this, SLOT(modeButtonSelected()));
    connect(ui->clearAllBut, SIGNAL(clicked()), this, SLOT(clearAllButtonSelected()));

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(filtersAccepted()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(filtersRejected()));

}






void ClusterClientFilterTab::filtersAccepted()
{
    // copy updated masks with edited values
    bandFilterMask = editBandFilterMask;
    modeFilterMask = editModeFilterMask;
    emit filtersChanged();
    close();
}


void ClusterClientFilterTab::filtersRejected()
{
    // restore settings on tab
    restoreTabSettings();
    close();
}


void ClusterClientFilterTab::restoreTabSettings()
{

    loadBandSettings(bandFilterMask);
    loadModeSettings(modeFilterMask);
}


void ClusterClientFilterTab::closeEvent (QCloseEvent *event)
{
    restoreTabSettings();
    QWidget::closeEvent(event);
}

void ClusterClientFilterTab::copyBandFilterMaskToEdit()
{
    editBandFilterMask = bandFilterMask;
}

void ClusterClientFilterTab::copyModeFilterMaskToEdit()
{
    editModeFilterMask = modeFilterMask;
}

void ClusterClientFilterTab::loadBandSettings(unsigned int bandMask)
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

void ClusterClientFilterTab::loadModeSettings(unsigned int modeMask)
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

void ClusterClientFilterTab::vhfButtonSelected()
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

void ClusterClientFilterTab::mWaveButtonSelected()
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

void ClusterClientFilterTab::modeButtonSelected()
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


void ClusterClientFilterTab::clearAllButtonSelected()
{
    clearAllFilters();
}


void ClusterClientFilterTab::clearAllFilters()
{
    clearVHFBands();
    clearMWaveBands();
    clearModes();

}



void ClusterClientFilterTab::clearVHFBands()
{
    editBandFilterMask = editBandFilterMask & ~_50M & ~_70M & ~_144M & ~_432M;
    for (int i = 0; i < vhfChkBoxList.count(); i++)
    {
        vhfChkBoxList[i]->setCheckState(Qt::Unchecked);
    }
}

void ClusterClientFilterTab::setVHFBands()
{
    editBandFilterMask = editBandFilterMask | _50M | _70M | _144M | _432M;
    for (int i = 0; i < vhfChkBoxList.count(); i++)
    {
        vhfChkBoxList[i]->setCheckState(Qt::Checked);
    }
}

void ClusterClientFilterTab::vhfChecked(int checkBoxNum)
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


void ClusterClientFilterTab::clearMWaveBands()
{
    editBandFilterMask = editBandFilterMask & ~_1296M & ~_2300M & ~_3_4G & ~_5_6G & ~_10G;
    for (int i = 0; i < mWaveChkBoxList.count(); i++)
    {
        mWaveChkBoxList[i]->setCheckState(Qt::Unchecked);
    }
}

void ClusterClientFilterTab::setMWaveBands()
{
    editBandFilterMask = editBandFilterMask | _1296M | _2300M | _3_4G | _5_6G | _10G;
    for (int i = 0; i < mWaveChkBoxList.count(); i++)
    {
        mWaveChkBoxList[i]->setCheckState(Qt::Checked);
    }
}




void ClusterClientFilterTab::mWaveChecked(int checkBoxNum)
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

void ClusterClientFilterTab::clearModes()
{
    editModeFilterMask = editModeFilterMask & ~CWMODE & ~PHONEMODE & ~RTTYMODE & ~MGMMODE;
    for (int i = 0; i <modeChkBoxList.count(); i++)
    {
        modeChkBoxList[i]->setCheckState(Qt::Unchecked);
    }
}

void ClusterClientFilterTab::setModes()
{
    editModeFilterMask = editModeFilterMask | CWMODE | PHONEMODE | RTTYMODE | MGMMODE;
    for (int i = 0; i < modeChkBoxList.count(); i++)
    {
        modeChkBoxList[i]->setCheckState(Qt::Checked);
    }
}


void ClusterClientFilterTab::modeChecked(int checkBoxNum)
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


unsigned int ClusterClientFilterTab::getBandFilterMask()
{
    return bandFilterMask;
}
unsigned int ClusterClientFilterTab::getModeFilterMask()
{
    return modeFilterMask;
}
