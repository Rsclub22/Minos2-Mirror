
#include "LoggerContest.h"
#include "bandselbuttons.h"
#include "BandSwitchFrame.h"
#include "ui_BandSwitchFrame.h"

BandSwitchFrame::BandSwitchFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::BandSwitchFrame)
{
    ui->setupUi(this);
    initBandSelButtons();

}
void BandSwitchFrame::setRigControl(RigControlFrame *rc)
{
    rigControl = rc;
    connect(bandSelButtons , &BandSelButtons::sendPresetFreq, rigControl, &RigControlFrame::radioBandFreq);
    connect(bandSelButtons, &BandSelButtons::sendBandChange, rigControl, &RigControlFrame::onRadioBandChange);

}

int BandSwitchFrame::setButtonOnOff(QString band, bool on)
{
    return bandSelButtons->setButtonOnOff(band, on);
}

void BandSwitchFrame::setAllButtonsOff()
{
    bandSelButtons->setAllButtonsOff();
}

void BandSwitchFrame::selectSupportedBands(const QStringList &listOfBands)
{
    bandSelButtons->selectSupportedBands(listOfBands);
}

int BandSwitchFrame::selectButtonGroupAndActiveBand(const QString band)
{
    return bandSelButtons->selectButtonGroupAndActiveBand(band);
}

int BandSwitchFrame::selectButtonGroupAndActiveBand(const Frequency &freq)
{
    return bandSelButtons->selectButtonGroupAndActiveBand(freq);
}

void BandSwitchFrame::readPresetFreqsFromIni(const QVector<QSharedPointer<BandInfo> > bands)
{
    bandSelButtons->readPresetFreqsFromIni(bands);
}

void BandSwitchFrame::initBandSelButtons()
{

    QVector<QSharedPointer<BandInfo> > bands;
    BandList::getBandList().loadAllBands(bands);
    bandSelButtons = new BandSelButtons(bands, ui->bandSelGridLayout);

}
BandSwitchFrame::~BandSwitchFrame()
{
    delete ui;
    ct =nullptr;
}

void BandSwitchFrame::setContest(BaseContestLog *contest)
{
    ct = contest;
    bandSelButtons->setContest(contest);
    QString contestBand;
    QString mode;
    if (ct)
    {
        contestBand = ct->currentBand.getValue();
        mode = ct->currentMode.getValue();
    }

    bandSelButtons->setMode(mode);
    bandSelButtons->setContestBand(contestBand);

    if (ct)
    {
        ui->bandSwitchButtomFrame->setVisible(ct->contestBands.getValue() == allHF);
    }
}

void BandSwitchFrame::setContestBand(QString band)
{
    QString contestBand = band;
    bandSelButtons->setContestBand(contestBand);
}

void BandSwitchFrame::setMode(QString _curMmode)
{
    bandSelButtons->setMode(_curMmode);
}

void BandSwitchFrame::setPreviousFreq(QString mode, Frequency freq)
{
    bandSelButtons->setPreviousFreq(mode, freq);
}

Frequency BandSwitchFrame::getPresetFreq(const QString band, const QString mode)
{
    return bandSelButtons->getPresetFreq(band, mode);
}
