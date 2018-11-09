#include <QStringListModel>
#include <QMessageBox>
#include "cutils.h"
#include "clusterclientfilterdialog.h"
#include "calllocinputdialog.h"
#include "ui_clusterclientfilterdialog.h"

ClusterClientFilterDialog::ClusterClientFilterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ClusterClientFilterDialog),
    callsignListWidgetCurrentRow(-1),
    bandFilterMask(0),
    editBandFilterMask(0),
    modeFilterMask(0),
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
    callsignListWidget->addItems(callsignFilterList);


    connect(ui->callsignListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(callsignCurrentRowChanged(int)));


    connect(ui->callsignAddButton, SIGNAL(clicked()), SLOT(callsignAddClicked()));
    connect(ui->callsignEditButton, SIGNAL(clicked()), SLOT(callsignEditClicked()));
    connect(ui->callsignDelButton, SIGNAL(clicked()), SLOT(callsignDelClicked()));

    locatorListWidget = ui->locatorListWidget;
    locatorListWidget->addItems(locatorFilterList);

    connect(ui->locatorListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(locatorCurrentRowChanged(int)));

    connect(ui->locatorAddButton, SIGNAL(clicked()), SLOT(locatorAddClicked()));
    connect(ui->locatorEditButton, SIGNAL(clicked()), SLOT(locatorEditClicked()));
    connect(ui->locatorDelButton, SIGNAL(clicked()), SLOT(locatorDelClicked()));

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
        bandFilterMask = editBandFilterMask;
        modeFilterMask = editModeFilterMask;
        filterChangeMask |= FREQFILTERUP;
    }

    else if (callsignEditChanged)
    {
        callsignEditChanged = false;
        callsignFilterList.clear();
        for (int row = 0; row < callsignListWidget->count(); row++)
        {
            QListWidgetItem* item = callsignListWidget->item(row);
            callsignFilterList.append(item->text());
        }
        filterChangeMask |= CALLSIGNUP;
    }
    else if (locatorEditChanged)
    {
        locatorEditChanged = false;
        locatorFilterList.clear();
        for (int row = 0; row < locatorListWidget->count(); row++)
        {
            QListWidgetItem* item = locatorListWidget->item(row);
            locatorFilterList.append(item->text());
        }
        filterChangeMask |= LOCATORUP;
    }

    emit filtersChanged(filterChangeMask);
    close();
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
        for (int i = 0; i < callsignFilterList.count(); i++)
        {
            callsignListWidget->addItem(callsignFilterList[i]);
        }
    }
    else if (locatorEditChanged)
    {
        // restore the locatorListWidget
        locatorListWidget->clear();
        for(int i = 0; i < locatorFilterList.count(); i++)
        {
            locatorListWidget->addItem(locatorFilterList[i]);
        }
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
        if (bandFilterMask & vhfBandMasks[i])
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
        if (bandFilterMask & mWaveBandMasks[i])
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
        if (modeFilterMask & modeMasks[i])
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

void ClusterClientFilterDialog::copyCallsignFilterListToListWidget()
{
    callsignListWidget->clear();
    foreach (QString str, callsignFilterList)
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
    if (callsignListWidgetCurrentRow >=0)
    {
        int status = QMessageBox::question( this,
        QString("Delete Callsign Filter"),
        QString("Do you want to delete callsign %1 ?").arg(callsignListWidget->currentItem()->text()),
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
    return callsignFilterList;
}

QStringList ClusterClientFilterDialog::getLocatorFilterList()
{
    return locatorFilterList;
}


void ClusterClientFilterDialog::copyLocatorFilterListToListWidget()
{
    locatorListWidget->clear();
    foreach (QString str, locatorFilterList)
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
