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


#include "LoggerContest.h"

#include "MinosLoggerEvents.h"
#include "bandselbuttons.h"
#include "BandList.h"


BandSelButtons::BandSelButtons(const QVector<QSharedPointer<BandInfo> > &_bands, QGridLayout *_bandSelGrid, QObject *parent) : QObject(parent)

{
    bands = _bands;
    bandSelGridLayout = _bandSelGrid;
    setupButtons();


    readPresetFreqsFromIni(bands);

    presetFreqs.copyAllPrevFreqToLastFreqByMode(freqPresetData::PRESET_MODE_CW, bands);
    presetFreqs.copyAllPrevFreqToLastFreqByMode(freqPresetData::PRESET_MODE_PHONE, bands);
    presetFreqs.copyAllPrevFreqToLastFreqByMode(freqPresetData::PRESET_MODE_MGM, bands);

    BandList &blist = BandList::getBandList();

    for (auto &b: blist.bandList)
    {
        if (b->getType() == "HF")
        {
            availHfBands.append(b->uk);
        }
    }
}


void BandSelButtons::setupButtons()
{

    for (int i = 0; i < 6; i++)
    {
        toolButList.append(new QToolButton());

    }

    bandSelGridLayout->addWidget(toolButList[0], 1, 0, Qt::AlignHCenter);
    bandSelGridLayout->addWidget(toolButList[1], 1, 1, Qt::AlignHCenter);
    bandSelGridLayout->addWidget(toolButList[2], 1, 2, Qt::AlignHCenter);
    bandSelGridLayout->addWidget(toolButList[3], 2, 0, Qt::AlignHCenter);
    bandSelGridLayout->addWidget(toolButList[4], 2, 1, Qt::AlignHCenter);
    bandSelGridLayout->addWidget(toolButList[5], 2, 2, Qt::AlignHCenter);

    QString buttonStyle = bandSelButtonData::BUTTON_OFF_STYLE;
    for (auto &tb:toolButList)
    {

        connect(tb, &QToolButton::pressed, this, [=]() {onBandSelButtonPressed(tb);});

        tb->setVisible(false);
        tb->setStyleSheet(buttonStyle);
    }


    buildBandButtonLabels();


}

void BandSelButtons::readPresetFreqsFromIni(const QVector <QSharedPointer <BandInfo> > bands)
{
    presetFreqs.readSettings(bands);
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

    ct->currentBand.setValue(band);
    ct->commonSave(false);
    setContestBand(band);

    MinosLoggerEvents::SendContestBandChanged(ct);

    freq = presetFreqs.getLastFreq(convertModeForPresets(curMode), band);
    emit sendPresetFreq(freq);
}

void BandSelButtons::selectSupportedBands(const QStringList &listOfBands)
{
    availHfBands.clear();

    for (auto &b:listOfBands)
    {
        if (getBandType(b) == bandSelButtonData::HF_BAND_TYPE)
        {
            availHfBands.append(b);
        }
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
    if (availHfBands.contains(band) )
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
       setToolTip(foundBand, freq.extractKhz());
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
}

void BandSelButtons::setContest(BaseContestLog *contest)
{
    ct = contest;
//    if (ct)
//    {
//        setMode(contest->currentMode.getValue());
//        setContestBand(ct->currentBand.getValue());
//    }
}
void BandSelButtons::setContestBand(QString contestBand_)
{
    contestBand = contestBand_;
    if (ct && ct->contestBands.getValue() == allHF)
    {
        selectButtonGroupAndActiveBand(contestBand_);
    }
}

QString BandSelButtons::convertModeForPresets(const QString mode)
{
    if (mode == hamlibData::USB || mode == hamlibData::LSB || mode == hamlibData::FM  || mode == "PH")
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
