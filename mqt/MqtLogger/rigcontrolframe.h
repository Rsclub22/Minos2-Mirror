/////////////////////////////////////////////////////////////////////////////
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2019
//
//
//
/////////////////////////////////////////////////////////////////////////////



#ifndef RIGCONTROLFRAME_H
#define RIGCONTROLFRAME_H

#include "base_pch.h"
#include "MinosLoggerEvents.h"
#include <QShortcut>
#include "RPCCommandConstants.h"
#include "rigmemcommondata.h"
#include "rigcommon.h"
#include "radiodetails.h"
#include "checkoperatingfreq.h"
#include "bandselbuttons.h"

namespace Ui {
    class RigControlFrame;
}

const int BANDLIST_TIMEOUT_DUR = 3000;

class quickBandSelData
{

public:

    QString band;
    Frequency freq;


};




class RigControlFrame : public QFrame
{
    Q_OBJECT
    friend class RunButtonsFrame;
public:
    explicit RigControlFrame(QWidget *parent);
    ~RigControlFrame() override;

    Ui::RigControlFrame *ui;

    void setContest(BaseContestLog *);

    void transferDetails(memoryData::memData &m);
    void getDetails(memoryData::memData &m);
    void getRigDetails(memoryData::memData &m);


    void setRadioLoaded();
    void setRadioListFromTslf();


    void setMode(QString);
    void setVolume(int level);
    void setFreq(Frequency);
    void setRitFreq(ShortFreq);
    void setRitRadioStatus(bool);
    void setRadioName(QString, bool);
    void setRadioState(QString);
    void setRitEnableState(bool s);

    bool isRadioLoaded();

    void exitFreqEdit();

    QString getStrPassBandState(QString mode);
    int getIntPassBandState(QString mode);
    bool checkRadioState();
    void on_ContestPageChanged();

    void checkConnection();

    void setRadioVolumeState(bool state);

    void setTransVertOffset(double offset, PubSubName psn);
    void setTransVertSwitch(int switchNum, PubSubName psn);
    void setTransVertStatus(bool status, PubSubName psn);
    void setVolumeStatus(bool status, PubSubName psn);
    void setRitEnableStatus(bool status, PubSubName psn);
    void setRitMaxKHzFreq(int maxRitFreq, PubSubName psn);
    void setBandList(QString s, PubSubName psn);
    void createSupportedBandList(QString);

    void setTransVertEnabled(bool status, PubSubName psn);

    void closeContest();

    Frequency getSendFreq();
signals:
    void selectRadio(QString, QString, Frequency, QString);  // radio name, freq, mode
    void sendFreqControl(Frequency);
    void sendVolumeToRadio(int);
    void noRadioSendFreq(Frequency);
    void noRadioSendMode(QString);
    void sendModeToControl(QString);
    void sendRitFreq(ShortFreq);
    void ritStatus(bool);
    void radioIsConnected(bool);
    void radioHasError(QString);
    void radioDisconnected();

    void setFreqDisplay(Frequency, bool);

private slots:
    void on_FontChanged();

    void freqLineEditInFocus();
    void radioBandFreq(Frequency f);
    void noRadioSetFreq(Frequency);
    void noRadioSetMode(QString m);
    void freqEditSelected();
    void freqPlusShortCut_clicked(bool);
    void freqNegShortCut_clicked(bool);
    void freqPlus_ShortCut();
    void freqNeg_ShortCut();
    void changeMainRadioFreq();
    void changeRitRadioFreq(ShortFreq);
    void on_radioNameSel_activated(const QString &arg1);


    void exitRitFreqEdit();
    void ritButtonSelected();
    void ritClearButtonSelected(bool state);
    void ritFreqEditShortCutInFocus();
    void sendVolumeRadio(int level);


    void ritClearShortCutSelected();
    //void bandListTimeout();

    //void setRadioFreq();


    void checkRigDetailsAvail();
    void clusterUpdateRigFreq(Frequency freq);


    void freqStepComboChanged(const QString step);


    void onCheckContestBandMatch();

public slots:
    void returnChangeRadioFreq();
private:
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

    LoggerContestLog *ct = nullptr;
    BandSelButtons *bandSelButtons = nullptr;

    //QVector<quickBandSelData> listOfBands;
    QStringList listOfBands;

    QMap<PubSubName, RadioDetails> allRadioDetails;

    QShortcut* freqEditShortKey;

    QShortcut *freqPlusShortCut;
    QShortcut *freqNegShortCut;

    QShortcut *ritOnOffShortCut;
    QShortcut *ritClearShortCut;
    QShortcut *ritFreqEditShortCut;

    bool rigFrameStartFlag;
    bool radioLoaded;
    bool radioConnected;
    bool radioError;
    bool freqEditOn;
    Frequency curFreq;
    Frequency lastFreq;
    Frequency sendFreq;
    Frequency disconnectFreq;
    Frequency curFStepButtonsFreq;
    QString curMode;

    bool ritEnable;
    bool ritOn;
    bool ritEditOn;
    int maxRitFreq;
    bool ritKHzFlag;
    //QString curRit;

    QString contestBand;
    Frequency contestBandFLow;
    Frequency contestBandFHigh;

    QStringList listOfRadios;
    RadioDetails selRadioDetails;
    PubSubName selRadioName;
    QString radioName;
    QString rigAppName;
    QString radioState;

    bool onContestPageChangedFlag;
    bool ignorePreviousFreqFlag;
    bool ignorePresetFreqFlag;

    QTimer *launchRadioSelectTimer = nullptr;
    int launchRadioSelectCount;


    CheckOperatingFreq *operatingFreq;
    bool operatingFreqPlanOk;

    QPalette *freqDisplayPalette;
    bool legalFreq = true;

    void sendRigFreq(Frequency f);
    Frequency getCurFreq() const;

    void sendModeToRadio(QString);
    void freqLineEditBkgnd(bool status);
    void freqLineEditFrameColour(bool status);

    void ritLineEditInFocus();
    void ritFreqLineEditFrameColour(bool status);

    void initRigFrame(QWidget *parent);
    void initPassBandRadioButtons();
    void noRadioSendOutFreq(Frequency f);
    void noRadioSendOutMode(QString m);

    void traceMsg(QString msg);

    void mgmLabelVisible(bool state);
    bool checkValidFreq(Frequency freq);

    Frequency calcNewFreq(Frequency incFreq);

    int calcMinosMode(QString mode);

    void freqPlusMinusButton(Frequency freq);


    void showRitButOn();
    void showRitButOff();

    void setRadioList();

    void setVolControlVisible(bool value);

    void ritButtonOn();
    void ritButtonOff();

    void setRadioBandWarning(QString s);
    void setRadioTxVertEnabled(bool s);
    void setRadioTxVertStatus(bool status);
    void transVertIndicatorOn();
    void transVertIndicatorOff();

    bool isFreqLegal(const Frequency &freq, const QString band, const QString mode);
    bool checkFreqIsLegal(const Frequency &freq, const QString mode);
    void setFreqTextLegalColour(const Frequency freq, QString mode);
    void setFreqStepCombo(QString mode);
    double getStepFreqFromComboText(const QString step);
    void clearFreqInputFocus();
    bool readIgnorePresetFreqFlag();
    bool readIgnorePreviousFreqFlag();
    void setRitMaxKHzFreq(int maxRitFreq);
    //int setBandSelComboIndex(QString band);
    void restoreRadioFreq();
    void displayFreqOnFreqEditDisplay(const Frequency &freq);
    void setRadioFreq(Frequency &sendFreq, bool &rigFrameStartFlag);
    int setBandSelButtonFromFreq(const Frequency &freq);
    //int setBandSelComboFromFreq(const Frequency &freq);
    bool checkFreqOK(const Frequency &freq);
    bool checkContestBandMatch(const Frequency &freq);
    void setContestBandLimits(QString band);
    void initBandSelButtons();
};





#endif // RIGCONTROLFRAME_H
