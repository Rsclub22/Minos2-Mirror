/////////////////////////////////////////////////////////////////////////////
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2017 - 2020
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2019
//
/////////////////////////////////////////////////////////////////////////////

#include "MinosLoggerEvents.h"
#include "BandList.h"
#include "contest.h"
#include "bandselbuttons.h"

namespace bandSelButtonData
{
const char * BUTTON_ON_STYLE = "background-color: Sandybrown ; border-style: outset; border-width: 1px; border-color: black";
const char * BUTTON_OFF_STYLE = "background-color: Gainsboro ; border-style: outset; border-width: 1px; border-color: black";
const char * BUTTON_NOT_CONTEST_STYLE = "background-color: Yellow ; border-style: outset; border-width: 1px; border-color: black";
}

BandSelButtons::BandSelButtons(const QVector<QSharedPointer<BandInfo> > &_bands, QGridLayout *_bandSelGrid, QObject *parent) : QObject(parent)

{
    bands = _bands;
    bandSelGridLayout = _bandSelGrid;
    setupButtons();

    readPresetFreqsFromIni(bands);

    presetFreqs.copyAllPrevFreqToLastFreqByMode(freqPresetData::PRESET_MODE_CW, bands);
    presetFreqs.copyAllPrevFreqToLastFreqByMode(freqPresetData::PRESET_MODE_PHONE, bands);
    presetFreqs.copyAllPrevFreqToLastFreqByMode(freqPresetData::PRESET_MODE_MGM, bands);
    presetFreqs.copyAllPrevFreqToLastFreqByMode(freqPresetData::PRESET_MODE_RTTY, bands);
    presetFreqs.copyAllPrevFreqToLastFreqByMode(freqPresetData::PRESET_MODE_PSK, bands);

    // availHFBands is those available in logger
    // we gt Rig Control's isdea later
    for (auto &b: bands)
    {
        if (b->getType() == HF_BANDTYPE && b->enabled && b->contestAllowed )
        {
            availHfBands.append(b->uk);
        }
    }
}


void BandSelButtons::setupButtons()
{
    int row = 0;
    int col = 0;
    for (const auto &b: QASCONST(bands))
    {
        if (b->getType() == HF_BANDTYPE)
        {
            QToolButton *bb = new QToolButton();
            bb->setText(b->uk);
            toolButList.append(bb);
            bb->setFocusPolicy(Qt::NoFocus);
            bandSelGridLayout->addWidget(bb, row, col, Qt::AlignHCenter);
            col++;
            if (col %3 == 0)
            {
                row++;
                col = 0;
            }
            connect(bb, &QToolButton::pressed, this, [=]() {onBandSelButtonPressed(bb);});

            bb->setVisible(false);
            QString buttonStyle = bandSelButtonData::BUTTON_OFF_STYLE;
            bb->setStyleSheet(buttonStyle);
        }
    }
}

void BandSelButtons::readPresetFreqsFromIni(const QVector <QSharedPointer <BandInfo> > bands)
{
    presetFreqs.readSettings(bands);
}

void BandSelButtons::onBandSelButtonPressed(QToolButton* button)
{
    Frequency freq;
    QString band = button->text();

    ct->setCurrentBand(band);
    ct->commonSave(false);
    setContestBand(band);

    MinosLoggerEvents::SendContestBandChanged(ct);

    QString m = curMode;
    freq = presetFreqs.getLastFreq(convertModeForPresets(m), band);
    emit sendPresetFreq(freq);
}

void BandSelButtons::selectSupportedBands(const QStringList &listOfBands)
{
    availHfBands.clear();

    for (auto &b:listOfBands)
    {
        if (getBandType(b) == HF_BANDTYPE)
        {
            availHfBands.append(b);
        }
    }
}

QToolButton *BandSelButtons::findToolButton(QString band)
{
    for (int i = 0; i < toolButList.count(); i++)
    {
        QString tb = toolButList[i]->text();
        if (tb == band)
        {
            return toolButList[i];
        }
    }
    return nullptr;
}

void BandSelButtons::setButtonsToBandType()
{
    setAllButtonsOff();
    setAllButtonsVisible(false);

    if (!ct)
    {
        return;
    }

    for (int i = 0; i < availHfBands.count(); i++)
    {
        QToolButton *t = findToolButton(availHfBands[i]);
        if (t)
        {
            t->setVisible(true);
        }
    }

    QStringList bll = ct->bandsList.getValue().split(";");
    for(const auto &bs: QASCONST(bll))
    {
        QStringList bsl = bs.split(" ");
        if (bsl.count() == 3)
        {
            QString btn = bsl[0] + " " + bsl[1];
            QToolButton *t = findToolButton(btn);
            if (t )
            {
                if (bsl[2] == "0")
                {
                    t->setVisible(false);
                }
            }
        }
    }

    if (!selectedBand.isEmpty())
    {
        QToolButton *t = findToolButton(selectedBand);
        setButtonOnOff(t, true);
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

bool BandSelButtons::isBandAvailable(QString band)
{
    if (availHfBands.contains(band) )
    {
        return true;
    }

    return false;
}

void BandSelButtons::setAllButtonsOff()
{
    for (auto t:QASCONST(toolButList))
    {
        setButtonOnOff(t, false);
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
    for (const auto &b: QASCONST(bands))
    {
        if (b->fLow <= freq && b->fHigh >= freq)
        {
            foundBand = b->uk;
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
    setButtonsToBandType();
    QToolButton *t = findToolButton(band);
    if (t)
    {
        setButtonOnOff(t, true);
        return 0;
    }
    return -1;
}


void BandSelButtons::setButtonOnOff(QToolButton *t, const bool on)
{
    if (t)
    {
        QString buttonStyle;
        if (on)
        {
            if (t->text() != contestBand)
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
        t->setStyleSheet( buttonStyle);
    }
}

void BandSelButtons::setToolTip(QString band, QString tipTxt)
{
    QToolButton *t = findToolButton(band);
    if (t)
    {
       t->setToolTip(tipTxt);
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
    for (const auto &b: QASCONST(bands))
    {
        if (b->getType() == bandType)
        {
            if (isBandAvailable(b->uk))
            {
                return true;
            }
        }
    }
    return false;
}


QString BandSelButtons::getBandType(const QString selectedBand)
{
    for (const auto &b: QASCONST(BandList::getBandList().bandList))
    {
        if (b->uk == selectedBand)
        {
            return b->getType();
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
}
void BandSelButtons::setContestBand(QString contestBand_)
{
    contestBand = contestBand_;
    if (ct && ct->isHF())
    {
        selectButtonGroupAndActiveBand(contestBand_);
    }
}

QString BandSelButtons::convertModeForPresets(const QString mode)
{
    if (mode == hamlibData::USB || mode == hamlibData::LSB || mode == hamlibData::FM  || mode == hamlibData::PH)
    {
        return "PHONE";
    }

    return mode;
}


Frequency BandSelButtons::getPresetFreq(const QString band, const QString m)
{
    Frequency f;
    QString mode = m;
    QString cmode = convertModeForPresets(mode);
    if (presetFreqs.contains(cmode, band))
    {
         f = presetFreqs.getPresetFreq(cmode, band);
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
