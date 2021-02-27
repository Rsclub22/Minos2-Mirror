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


BandSelButtons::BandSelButtons(const QVector<QSharedPointer<BandInfo> > &_bands, QGridLayout *_bandSelGrid, QObject *parent) : QObject(parent)

{
    bands = _bands;
    bandSelGridLayout = _bandSelGrid;
    setupButtons();

    QStringList listOfBands;
    for (auto &b:bands)
    {
        listOfBands.append(b->uk);
    }
    presetFreqs.readSettings(listOfBands);

    presetFreqs.copyAllPrevFreqToLastFreqByMode(freqPresetData::PRESET_MODE_CW, listOfBands);
    presetFreqs.copyAllPrevFreqToLastFreqByMode(freqPresetData::PRESET_MODE_PHONE, listOfBands);
    presetFreqs.copyAllPrevFreqToLastFreqByMode(freqPresetData::PRESET_MODE_MGM, listOfBands);


}


void BandSelButtons::setupButtons()
{
    hfSelBut = new QToolButton();
    hfSelBut->setText("HF");
    hfSelBut->setVisible(false);
    connect(hfSelBut, &QToolButton::pressed, this, [=]() {onHfSelButtonPressed();});

    vhfSelBut = new QToolButton();
    vhfSelBut->setText("VHF");
    vhfSelBut->setVisible(false);
    connect(vhfSelBut, &QToolButton::pressed, this, [=]() {onVhfSelButtonPressed();});

    mwSelBut = new QToolButton();
    mwSelBut->setText("MW");
    mwSelBut->setVisible(false);
    connect(mwSelBut, &QToolButton::pressed, this,
            [=]() {onMwSelButtonPressed();});

    preSetFreqRadBut = new QRadioButton();
    preSetFreqRadBut->setText(tr("Preset"));

    connect(preSetFreqRadBut, &QRadioButton::pressed, this, [=]() {onPresetFreqRadButPressed();});

    prevFreqRadBut = new QRadioButton();
    prevFreqRadBut->setText(tr("Prev"));

    connect(prevFreqRadBut, &QRadioButton::pressed, this, [=]() {onPrevFreqRadButPressed();});

    bandOnlyRadBut = new QRadioButton();
    bandOnlyRadBut->setText(tr("Band"));

    connect(bandOnlyRadBut, &QRadioButton::pressed, this, [=]() {onBandOnlyRadButPressed();});


    for (int i = 0; i < 6; i++)
    {
        toolButList.append(new QToolButton());

    }

    bandSelGridLayout->addWidget(hfSelBut, 0, 0, Qt::AlignHCenter);
    bandSelGridLayout->addWidget(vhfSelBut, 0, 1, Qt::AlignHCenter);
    bandSelGridLayout->addWidget(mwSelBut, 0, 2, Qt::AlignHCenter);
    bandSelGridLayout->addWidget(toolButList[0], 1, 0, Qt::AlignHCenter);
    bandSelGridLayout->addWidget(toolButList[1], 1, 1, Qt::AlignHCenter);
    bandSelGridLayout->addWidget(toolButList[2], 1, 2, Qt::AlignHCenter);
    bandSelGridLayout->addWidget(toolButList[3], 2, 0, Qt::AlignHCenter);
    bandSelGridLayout->addWidget(toolButList[4], 2, 1, Qt::AlignHCenter);
    bandSelGridLayout->addWidget(toolButList[5], 2, 2, Qt::AlignHCenter);
    bandSelGridLayout->addWidget(preSetFreqRadBut, 0, 3, Qt::AlignLeft);
    bandSelGridLayout->addWidget(prevFreqRadBut, 1, 3, Qt::AlignLeft);
    bandSelGridLayout->addWidget(bandOnlyRadBut, 2, 3, Qt::AlignLeft);

    QString buttonStyle = bandSelButtonData::BUTTON_OFF_STYLE;
    for (auto &tb:toolButList)
    {

        connect(tb, &QToolButton::pressed, this, [=]() {onBandSelButtonPressed(tb);});

        tb->setVisible(false);
        tb->setStyleSheet(buttonStyle);
    }


    buildBandButtonLabels();


}


void BandSelButtons::buildBandButtonLabels()
{
    for (auto &b:bands)
    {
        QString band = b.data()->uk;
        QStringList bl = band.split('\x20');
        if(bl.count() == 2)
        {
            QString label = bl[0];
            bandToButtonLabels.insert(band, label);
            buttonLabelsToBand.insert(label, band);
        }

    }
}


void BandSelButtons::onBandSelButtonPressed(QToolButton* button)
{
    Frequency freq;
    QString band = buttonLabelsToBand.value(button->text());
    if (preSetFreqRadButIsChecked())
    {
        freq = presetFreqs.getPresetFreq(convertModeForPresets(curMode), band);
        emit sendPresetFreq(freq);
    }
    else if (prevFreqRadButIsChecked())
    {
        freq = presetFreqs.getLastFreq(convertModeForPresets(curMode), band);
        emit sendPresetFreq(freq);
    }
    else if (bandOnlyRadButIsChecked())
    {
        emit sendBandChange(band);
    }

    selectButtonGroupAndActiveBand(band);

}


void BandSelButtons::onHfSelButtonPressed()
{
    setButtonsToBandType(bandSelButtonData::HF_BAND_TYPE);
}

void BandSelButtons::onVhfSelButtonPressed()
{
    setButtonsToBandType(bandSelButtonData::VHF_BAND_TYPE);
}

void BandSelButtons::onMwSelButtonPressed()
{
    setButtonsToBandType(bandSelButtonData::MW_BAND_TYPE);
}

void BandSelButtons::onPresetFreqRadButPressed()
{
    if (!curMode.isEmpty())
    {
       setPresetFreqToolTip(curMode);
    }

}

void BandSelButtons::onPrevFreqRadButPressed()
{
    if (!curMode.isEmpty())
    {
        setPrevFreqToolTip(curMode);
    }
}


void BandSelButtons::onBandOnlyRadButPressed()
{
    setBandsToolTip();
}

void BandSelButtons::selectSupportedBands(const QStringList &listOfBands)
{


    availHfBands.clear();
    availVhfBands.clear();
    availMwBands.clear();

    for (auto &b:listOfBands)
    {
        if (getBandType(b) == bandSelButtonData::HF_BAND_TYPE)
        {
            availHfBands.append(b);
        }
        else if (getBandType(b) == bandSelButtonData::VHF_BAND_TYPE)
        {
            availVhfBands.append(b);
        }
        else if(getBandType(b) == bandSelButtonData::MW_BAND_TYPE)
        {
            availMwBands.append(b);
        }
    }

    if (!availHfBands.isEmpty())
    {
        hfSelBut->setVisible(true);
    }

    if (!availVhfBands.isEmpty())
    {
        vhfSelBut->setVisible(true);
    }

    if (!availMwBands.isEmpty())
    {
        mwSelBut->setVisible(true);
    }


}



void BandSelButtons::setButtonsToBandType(QString bandType)
{
    setAllButtonsOff();
    setAllButtonsVisible(false);
    clearAllButtonLabels();
    bandToolButList.clear();

    if (bandType == bandSelButtonData::HF_BAND_TYPE && !availHfBands.isEmpty())
    {
        for (int i = 0; i < availHfBands.count(); i++)
        {
            toolButList[i]->setText(bandToButtonLabels.value(availHfBands[i]));
            toolButList[i]->setVisible(true);
            bandToolButList.insert(availHfBands[i], toolButList[i]);
        }

        hfSelBut->setStyleSheet(bandSelButtonData::TYPE_BUTTON_ON_STYLE);
        vhfSelBut->setStyleSheet(bandSelButtonData::TYPE_BUTTON_OFF_STYLE);
        mwSelBut->setStyleSheet(bandSelButtonData::TYPE_BUTTON_OFF_STYLE);


    }
    else if (bandType == bandSelButtonData::VHF_BAND_TYPE && !availVhfBands.isEmpty())
    {
        for (int i = 0; i < availVhfBands.count(); i++)
        {
            toolButList[i]->setText(bandToButtonLabels.value(availVhfBands[i]));
            toolButList[i]->setVisible(true);
            bandToolButList.insert(availVhfBands[i], toolButList[i]);
        }

        vhfSelBut->setStyleSheet(bandSelButtonData::TYPE_BUTTON_ON_STYLE);
        hfSelBut->setStyleSheet(bandSelButtonData::TYPE_BUTTON_OFF_STYLE);
        mwSelBut->setStyleSheet(bandSelButtonData::TYPE_BUTTON_OFF_STYLE);

    }
    else if (bandType == bandSelButtonData::MW_BAND_TYPE && !availMwBands.isEmpty())
    {
        for (int i = 0; i < availMwBands.count(); i++)
        {
            toolButList[i]->setText(bandToButtonLabels.value(availMwBands[i]));
            toolButList[i]->setVisible(true);
            bandToolButList.insert(availMwBands[i], toolButList[i]);
        }

        mwSelBut->setStyleSheet(bandSelButtonData::TYPE_BUTTON_ON_STYLE);
        hfSelBut->setStyleSheet(bandSelButtonData::TYPE_BUTTON_OFF_STYLE);
        vhfSelBut->setStyleSheet(bandSelButtonData::TYPE_BUTTON_OFF_STYLE);

    }

    if (!selectedBand.isEmpty())
    {
        setButtonOnOff(selectedBand, true);
    }

}





void BandSelButtons::clearAllButtonLabels()
{
    for (auto &tb:toolButList)
    {
        tb->setText("  ");
    }
}


void BandSelButtons::setAllButtonsVisible(bool visible)
{
    for (auto &tb:toolButList)
    {
        tb->setVisible(visible);
    }

}


void BandSelButtons::setButtonVisible(QString band, bool visible)
{
    if (bandToolButList.contains(band))
    {
       bandToolButList.value(band)->setVisible(visible);
    }


}

bool BandSelButtons::isBandAvailable(QString band)
{
    if (availHfBands.contains(band) || availVhfBands.contains(band) || availMwBands.contains(band))
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

    for (QMap<QString, QToolButton*>::const_iterator i = bandToolButList.constBegin(); i != bandToolButList.constEnd(); i++)
    {

        setButtonOnOff(i.key(), false);

    }
}


void BandSelButtons::setPreviousFreq(QString mode, Frequency freq)
{
    QString foundBand;
    if (findBand(freq, bands, foundBand))
    {
       presetFreqs.setLastFreq(convertModeForPresets(mode), foundBand, freq);
       if (prevFreqRadBut->isChecked())
       {
           setToolTip(foundBand, freq.extractKhz());
       }

    }

}

bool BandSelButtons::findBand(const Frequency &freq, QVector<QSharedPointer<BandInfo> > &bands, QString &foundBand )
{
    for (auto const &b: bands)
    {
        if (b->fLow <= freq && b->fHigh >= freq)
        {
            foundBand = b.data()->uk;
            return true;
        }
        else
        {
            foundBand = "";

        }
    }

    return false;
}


int BandSelButtons::selectButtonGroupAndActiveBand(const Frequency &freq)
{
    int retCode = -1;

    QString band;
    bool ok = findBand(freq, bands, band);
    if (ok && !band.isEmpty())
    {
       retCode = selectButtonGroupAndActiveBand(band);
    }

    return retCode;
}


int BandSelButtons::selectButtonGroupAndActiveBand(const QString band)
{
    selectedBand = band;
    setButtonsToBandType(getBandType(band));
    return setButtonOnOff(band, true);
}


int BandSelButtons::setButtonOnOff(const QString band, const bool on)
{
    if (bandToolButList.contains(band))
    {
        QString buttonStyle;
        if (on)
        {
            if (band != contestBand)
            {
                buttonStyle = bandSelButtonData::BUTTON_NOT_CONTEST_STYLE;
            }
            else
            {
               buttonStyle = bandSelButtonData::BUTTON_ON_STYLE;
            }

        }
        else
        {
            buttonStyle = bandSelButtonData::BUTTON_OFF_STYLE;
        }

        bandToolButList.value(band)->setStyleSheet( buttonStyle);

        return 0;
    }
    else
    {
        return -1;
    }
}

void BandSelButtons::setToolTip(QString band, QString tipTxt)
{
    if (bandToolButList.contains(band))
    {
       bandToolButList.value(band)->setToolTip(tipTxt);
    }

}

void BandSelButtons::setBandsToolTip()
{
    for(auto &b:bands)
    {
       setToolTip(b->uk, b->uk);
    }
}

void BandSelButtons::setPresetFreqToolTip(const QString mode)
{
    QString m = convertModeForPresets(mode);
    for (auto &b:bands)
    {
       if (presetFreqs.contains(m, b->uk))
       {
           Frequency f = presetFreqs.getPresetFreq(m, b->uk);
           if (f.isClear())
           {
               setToolTip(b->uk, "***");
           }
           else
           {
               setToolTip(b->uk, f.extractKhz());
           }
       }


    }
}

void BandSelButtons::setPrevFreqToolTip(QString mode)
{
    QString m = convertModeForPresets(mode);
    for (auto &b:bands)
    {
        if (presetFreqs.contains(m, b->uk))
        {
           Frequency f = presetFreqs.getLastFreq(m, b->uk);
           if (f.isClear())
           {
               setToolTip(b->uk, "***");
           }
           else
           {
                setToolTip(b->uk, f.extractKhz());
           }
        }
    }
}


void BandSelButtons::setHf(bool allowHf)
{
    for (auto &b:bands)
    {
        if (b.data()->getType() == bandSelButtonData::HF_BAND_TYPE)
        {

            setButtonVisible(b.data()->uk, allowHf);
        }

    }
        hfSelBut->setVisible(allowHf);
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

    for (auto &b:bands)
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


QString BandSelButtons::getBandType(const QString selectedBand)
{
    for (auto &b:bands)
    {
        if (b.data()->uk == selectedBand)
        {
            return b.data()->getType();
        }

    }

    return "";
}


void BandSelButtons::setMode(QString mode)
{
    curMode = mode;
    setBandSelButtonsFromMode(curMode);
}



void BandSelButtons::setBandSelButtonsFromMode(QString curMode)
{
    // sets tooltips
    if (!curMode.isEmpty())
    {
        if(preSetFreqRadBut->isChecked())
        {

            setPresetFreqToolTip(curMode);
        }
        else if (prevFreqRadBut->isChecked())
        {
            setPrevFreqToolTip(curMode);
        }
        else if (bandOnlyRadBut->isChecked())
        {
            setBandsToolTip();
        }
    }
}


void BandSelButtons::setContest(QString contestBand_)
{
    contestBand = contestBand_;
}

QString BandSelButtons::convertModeForPresets(const QString mode)
{
    if (mode == "USB" || mode == "LSB" || mode == "FM")
    {
        return "PHONE";
    }

    return mode;
}


Frequency BandSelButtons::getPresetFreq(const QString band, const QString mode)
{
    Frequency f;
    if (presetFreqs.contains(convertModeForPresets(mode), band))
    {
         f = presetFreqs.getPresetFreq(convertModeForPresets(mode), band);
    }

    return f;
}

Frequency BandSelButtons::getLastFreq(const QString band, const QString mode)
{
    Frequency f;
    if (presetFreqs.contains(mode, band))
    {
         f = presetFreqs.getLastFreq(convertModeForPresets(mode), band);
    }
    return f;
}

void BandSelButtons::setPreSetFreqRadioButVisible(bool visible)
{
    preSetFreqRadBut->setVisible(visible);
}

void BandSelButtons::setPrevFreqRadioButVisible(bool visible)
{
    prevFreqRadBut->setVisible(visible);
}

void BandSelButtons::setbandOnlyButVisible(bool visible)
{
    bandOnlyRadBut->setVisible(visible);
}

bool BandSelButtons::preSetFreqRadButIsChecked()
{
    return preSetFreqRadBut->isChecked();
}
bool BandSelButtons::prevFreqRadButIsChecked()
{
    return prevFreqRadBut->isChecked();
}
bool BandSelButtons::bandOnlyRadButIsChecked()
{
    return bandOnlyRadBut->isChecked();
}
void BandSelButtons::setPresetFreqRadioButChecked(bool checked)
{
    preSetFreqRadBut->setChecked(checked);
}

void BandSelButtons::setBandOnlyRadioButChecked(bool checked)
{
    bandOnlyRadBut->setChecked(checked);
}

void BandSelButtons::setPrevFreqRadioButChecked(bool checked)
{
    prevFreqRadBut->setChecked(checked);
}


