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



#include "bandseltab.h"


BandSelTab::BandSelTab(QWidget *parent)
    : QTabWidget(parent)
{

    QList<QWidget *> widgets = this->findChildren<QWidget *>("hfBandSelTab");
    QList<QToolButton *> allPButtons = this->findChildren<QToolButton *>();

    initToolButtonTables();
}


void BandSelTab::initToolButtonTables()
{


    bandToolButList.insert("1.8 MHz", findChild<QToolButton *>("_1_8MHzToolBut"));
    bandToolButList.insert("3.5 MHz", findChild<QToolButton *>("_3_5MHzToolBut"));
    bandToolButList.insert("7 MHz", findChild<QToolButton *>("_7MHzToolBut"));
    bandToolButList.insert("14 MHz", findChild<QToolButton *>("_14MHzToolBut"));
    bandToolButList.insert("21 MHz", findChild<QToolButton *>("_21MHzToolBut"));
    bandToolButList.insert("28 MHz", findChild<QToolButton *>("_28MHzToolBut"));
    bandToolButList.insert("50 MHz", findChild<QToolButton *>("_50MHzToolBut"));
    bandToolButList.insert("70 MHz", findChild<QToolButton *>("_70MHzToolBut"));
    bandToolButList.insert("144 MHz", findChild<QToolButton *>("_144MHzToolBut"));
    bandToolButList.insert("432 MHz", findChild<QToolButton *>("_432MHzToolBut"));
    bandToolButList.insert("1296 MHz", findChild<QToolButton *>("_1296MHzToolBut"));
    bandToolButList.insert("2_3 GHz", findChild<QToolButton *>("_2_3GHzToolBut"));
    bandToolButList.insert("5_6 MHz", findChild<QToolButton *>("_5_6GHzToolBut"));
    bandToolButList.insert("10 GHz", findChild<QToolButton *>("_10GHzToolBut"));

    QMapIterator<QString, QToolButton*> btb(bandToolButList);
    while (btb.hasNext())
     {
        btb.next();
        btb.value()->setVisible(false);
        connect(btb.value(), &QToolButton::pressed,
                [=]() {onBandSelButtonPressed(btb.key());});

    }



}


void BandSelTab::onBandSelButtonPressed(QString key)
{
    int a = 0;
}



void BandSelTab::selectSupportedBands(const QStringList &listOfBands)
{
    setAllButtonsOff();
    setAllButtonsVisible(false);
    for (auto b:listOfBands)
    {
        setButtonVisible(b, true);
    }
}



void BandSelTab::setAllButtonsVisible(bool visible)
{
    QMapIterator<QString, QToolButton*> btb(bandToolButList);
    while (btb.hasNext())
     {
        btb.next();
        setButtonOnOff(btb.key(), visible);


    }
}



void BandSelTab::setButtonVisible(QString band, bool visible)
{

    bandToolButList.value(band)->setVisible(visible);

}

QString BandSelTab::getCurrentButtonOn_Band()
{
    return selectedBand;
}



void BandSelTab::setAllButtonsOff()
{


    QMapIterator<QString, QToolButton*> btb(bandToolButList);
    while (btb.hasNext())
     {
        btb.next();
        setButtonOnOff(btb.key(), false);


    }
}





int BandSelTab::setButtonOnOff(QString band, bool on)
{
    if (bandToolButList.contains(band))
    {
        QString buttonStyle;
        if (on)
        {
            selectedBand = band;
            buttonStyle = bandSelData::BUTTON_ON_STYLE;
        }
        else
        {
            selectedBand.clear();
            buttonStyle = bandSelData::BUTTON_OFF_STYLE;
        }

        bandToolButList.value(band)->setStyleSheet( buttonStyle);

        return 0;
    }
    else
    {
        return -1;
    }




}


void BandSelTab::setHf(bool allowHf)
{

}
