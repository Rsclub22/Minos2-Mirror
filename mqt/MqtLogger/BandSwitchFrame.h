#ifndef BANDSWITCHFRAME_H
#define BANDSWITCHFRAME_H

#include "rigcontrolframe.h"

namespace Ui {
class BandSwitchFrame;
}

class BandSelButtons;

class BandSwitchFrame : public QFrame
{
    Q_OBJECT

public:
    BandSelButtons *bandSelButtons = nullptr;

    explicit BandSwitchFrame(QWidget *parent = nullptr);
    ~BandSwitchFrame();
    void setContest(BaseContestLog *contest);
    void setContestBand(QString band);
    void setMode(QString _curMmode);
    void setPreviousFreq(QString mode, Frequency freq);
    Frequency getPresetFreq(const QString band, const QString mode);


    void initBandSelButtons();
    void setRigControl(RigControlFrame *rc);

    int setButtonOnOff(QString band, bool on);
    void selectSupportedBands(const QStringList &listOfBands);

    int selectButtonGroupAndActiveBand(const QString band);
    int selectButtonGroupAndActiveBand(const Frequency &freq);

    void readPresetFreqsFromIni(const QVector<QSharedPointer<BandInfo> > bands);

private:
    Ui::BandSwitchFrame *ui;
    BaseContestLog *ct = nullptr;
    RigControlFrame *rigControl = nullptr;

};

#endif // BANDSWITCHFRAME_H
