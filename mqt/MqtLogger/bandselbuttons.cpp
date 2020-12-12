/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2017 - 2020
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2019
//
//
//
/////////////////////////////////////////////////////////////////////////////



#include "bandselbuttons.h"
#include "BandList.h"


BandSelButtons::BandSelButtons(const QVector<QSharedPointer<BandInfo> > &_bands, const QList<QToolButton *> &bandButtons, const BandSelTabWidget &_bandSelTabWidget, QObject *parent) : QObject(parent)

{
    bands = _bands;
    bandSelTabWidget = _bandSelTabWidget;


    if (bands.count() == bandButtons.count())
    {
        for (int i = 0; i < bands.count(); i++ )
        {
            bandToolButList.insert(convertBandKey(bands[i].data()->uk), bandButtons[i]);
        }

        QMapIterator<QString, QToolButton*> btb(bandToolButList);
        while (btb.hasNext())
        {
            btb.next();
            //btb.value()->setVisible(false);
            btb.value()->hide();
            connect(btb.value(), &QToolButton::pressed,
                    [=]() {onBandSelButtonPressed(btb.key());});

        }

    }
    else
    {
        trace(QString("BandSelTab: initBandButtons error bands = %1, bandButtons = %2").arg(bands.count()).arg(bandButtons.count()));
    }


    FreqPresetDialog::readSettings(presetFreqs, bands);

    setToolTip("1.8 MHz", "1.8100");

}


void BandSelButtons::onBandSelButtonPressed(QString key)
{
    int a = 0;
}



void BandSelButtons::selectSupportedBands(const QStringList &listOfBands)
{

    activeBands.clear();
    for (auto b:listOfBands)
    {
        activeBands.append(convertBandKey(b));
    }

    for (int i = 0; i < bandSelTabWidget.mainTab->count(); i++)
    {
        bandSelTabWidget.mainTab->removeTab(i);
    }

    QWidget *hfTab = bandSelTabWidget.mainTab->findChild<QWidget *>("hfBandSelTab");
    if (hfTab)
    {
        bandSelTabWidget.mainTab->insertTab(0, hfTab, bandSelButtonData::HF_TAB_NAME);
    }
    QWidget *vhfTab = bandSelTabWidget.mainTab->findChild<QWidget *>("vhfBandSelTab");
    if (vhfTab)
    {
        bandSelTabWidget.mainTab->insertTab(1, vhfTab, bandSelButtonData::VHF_TAB_NAME);

    }
    QWidget *mwTab = bandSelTabWidget.mainTab->findChild<QWidget *>("mwBandSelTab");
    if (mwTab)
    {
        bandSelTabWidget.mainTab->insertTab(2, mwTab, bandSelButtonData::MW_TAB_NAME);
    }


    setAllButtonsOff();
    setAllButtonsVisible(false);

    // select buttons for active radio bands
    for (auto b:listOfBands)
    {
        setButtonVisible(convertBandKey(b), true);

    }

    // remove unwanted tabs
    if (!checkHfButtonsVisible())
    {
        removeBandSelTab(bandSelButtonData::HF_TAB_NAME);
    }
    else if (!checkVhfButtonsVisible())
    {
        removeBandSelTab(bandSelButtonData::VHF_TAB_NAME);
    }
    if (!checkMWaveButtonsVisible())
    {
       removeBandSelTab(bandSelButtonData::MW_TAB_NAME);

    }






}



void BandSelButtons::setTabToCurrentBandType(QString selectedBand)
{
    // set to current band tab
    QString bandType = selectedBandType(selectedBand);
    if (bandType == bandSelButtonData::HF_BAND_TYPE)
    {
        if (bandSelTabWidget.mainTab->currentWidget() != bandSelTabWidget.hfTab)
        {
           bandSelTabWidget.mainTab->setCurrentWidget(bandSelTabWidget.hfTab);
        }

    }
    else if (bandType == bandSelButtonData::VHF_BAND_TYPE)
    {
        QWidget* w = bandSelTabWidget.mainTab->currentWidget();
        if (bandSelTabWidget.mainTab->currentWidget() != bandSelTabWidget.vhfTab)
        {
            bandSelTabWidget.mainTab->setCurrentWidget(bandSelTabWidget.vhfTab);
        }
    }
    else if (bandType == bandSelButtonData::MW_BAND_TYPE)
    {
        if (bandSelTabWidget.mainTab->currentWidget() != bandSelTabWidget.mwTab)
        {
            bandSelTabWidget.mainTab->setCurrentWidget(bandSelTabWidget.mwTab);
        }
    }
}

void BandSelButtons::removeBandSelTab(QString tabLabel)
{
    for (int i = 0; i < bandSelTabWidget.mainTab->count(); i++)
    {

      if (bandSelTabWidget.mainTab->tabText(i) == tabLabel)
      {
          bandSelTabWidget.mainTab->removeTab(i);
      }
    }
}



void BandSelButtons::setAllButtonsVisible(bool visible)
{
    QMapIterator<QString, QToolButton*> btb(bandToolButList);
    while (btb.hasNext())
     {
        btb.next();
        setButtonOnOff(btb.key(), visible);


    }
}


void BandSelButtons::setButtonVisible(QString band, bool visible)
{

    bandToolButList.value(convertBandKey(band))->setVisible(visible);

}

bool BandSelButtons::isBandAvailable(QString band)
{
    if (activeBands.contains(convertBandKey(band)))
    {
        return true;
    }

    return false;
}

QString BandSelButtons::getCurrentButtonOn_Band()
{
    return selectedBand;
}



void BandSelButtons::setAllButtonsOff()
{


    QMapIterator<QString, QToolButton*> btb(bandToolButList);
    while (btb.hasNext())
     {
        btb.next();
        setButtonOnOff(btb.key(), false);


    }
}





int BandSelButtons::setButtonOnOff(QString band, bool on)
{
    if (bandToolButList.contains(convertBandKey(band)))
    {
        QString buttonStyle;
        if (on)
        {
            selectedBand = band;
            buttonStyle = bandSelButtonData::BUTTON_ON_STYLE;
            setTabToCurrentBandType(selectedBand);
        }
        else
        {
            selectedBand.clear();
            buttonStyle = bandSelButtonData::BUTTON_OFF_STYLE;
        }

        bandToolButList.value(convertBandKey(band))->setStyleSheet( buttonStyle);

        return 0;
    }
    else
    {
        return -1;
    }
}

void BandSelButtons::setToolTip(QString band, QString tipTxt)
{
    bandToolButList.value(convertBandKey(band))->setToolTip(tipTxt);
}

void BandSelButtons::setHf(bool allowHf)
{

}


bool BandSelButtons::checkHfButtonsVisible()
{
    return checkButtonsAvailable(bandSelButtonData::HF_BAND_TYPE);

}

bool BandSelButtons::checkVhfButtonsVisible()
{
    return checkButtonsAvailable(bandSelButtonData::VHF_BAND_TYPE);

}

bool BandSelButtons::checkMWaveButtonsVisible()
{
    return checkButtonsAvailable(bandSelButtonData::MW_BAND_TYPE);

}

bool BandSelButtons::checkButtonsAvailable(QString bandType)
{

    for (auto b:bands)
    {

        if (b.data()->getType() == bandType)
        {
            if (isBandAvailable(b.data()->uk))
            {
                return true;
            }

        }
    }

    return false;
}


QString BandSelButtons::selectedBandType(const QString selectedBand)
{
    for (auto b:bands)
    {
        if (b.data()->uk == selectedBand)
        {
            return b.data()->getType();
        }

    }

    return "";
}
