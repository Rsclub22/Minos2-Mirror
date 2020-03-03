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
#include "radiodetails.h"
#include "checkoperatingfreq.h"

namespace Ui {
    class RigControlFrame;
}

const int BANDLIST_TIMEOUT_DUR = 3000;

const QString RUN_BUTTON_ON_FREQ_STYLE = QString("background-color: orange ; color:black ; border-style: outset; border-width: 1px; border-color: black;\n");
const QString RUN_BUTTON_OFF_FREQ_STYLE = QString("background-color: yellow ; color:black ; border-style: outset; border-width: 1px; border-color: black;\n");
const QString RUN_BUTTON_OFF_STYLE = QString("background-color: Gainsboro ; color:black ; border-style: outset; border-width: 1px; border-color: black;\n");



const int NO_RUN_BUTTON_ON = -1;
const int RUN_BUTTON_1_ON = 0;
const int RUN_BUTTON_2_ON = 1;
const int NUM_RUNBUTTONS = 2;
const int CHECK_RUN_FREQ_POLLTIME = 1000;

class quickBandSelData
{

public:

    QString band;
    QString freq;


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
    void setRadioList();

    void setMode(QString);
    void setVolume(int level);
    void setFreq(QString);
    void setRitFreq(QString);
    void setRitRadioStatus(bool);
    void setRadioName(QString, QString mode);
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
    void setBandList(QString s, PubSubName psn);
    void createActiveBandList(QString);

    void setTransVertEnabled(bool status, PubSubName psn);

    void closeContest();

    void setIgnorePresetFreqFlag(bool status, PubSubName psn);
    void setIgnorePreviousFreqFlag(bool status, PubSubName psn);
signals:
    void selectRadio(QString, QString);
    void sendRadioName(QString);
    void sendFreqControl(QString);
    void sendVolumeToRadio(int);
    void noRadioSendFreq(QString);
    void noRadioSendMode(QString);
    void sendModeToControl(QString);
    void sendRitFreq(int);
    void ritStatus(bool);
    void radioIsConnected(bool);
    void radioHasError(QString);
    void radioDisconnected();

private slots:
    void on_FontChanged();

    void freqLineEditInFocus();
    void radioBandFreq(int index);
    void noRadioSetFreq(QString);
    void noRadioSetMode(QString m);
    void freqEditSelected();
    void freqPlusShortCut_clicked(bool);
    void freqNegShortCut_clicked(bool);
    void freqPlus_ShortCut();
    void freqNeg_ShortCut();
    void changeMainRadioFreq();
    void changeRitRadioFreq(int);
    void on_radioNameSel_activated(const QString &arg1);


    void exitRitFreqEdit();
    void ritButtonSelected();
    void ritClearButtonSelected(bool state);
    void ritFreqEditShortCutInFocus();
    void sendVolumeRadio(int level);


    void ritClearShortCutSelected();
    //void bandListTimeout();

    void setRadioFreq();


    void checkRigDetailsAvail();
    void clusterUpdateRigFreq(QString freq);


    void freqStepComboChanged(const QString step);
public slots:
    void returnChangeRadioFreq();
private:
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

    LoggerContestLog *ct = nullptr;

    QVector<quickBandSelData> listOfBands;

    QMap<PubSubName, RadioDetails> allRadioDetails;

    QShortcut* freqEditShortKey;

    QShortcut *freqPlusShortCut;
    QShortcut *freqNegShortCut;

    QShortcut *ritOnOffShortCut;
    QShortcut *ritClearShortCut;
    QShortcut *ritFreqEditShortCut;

    bool radioLoaded;
    bool radioConnected;
    bool radioError;
    bool freqEditOn;
    QString curFreq;
    double curFStepButtonsFreq;
    QString curMode;

    bool ritEnable;
    bool ritOn;
    bool ritEditOn;
    //QString curRit;

    bool ignorePresetFreqFlag;
    bool ignorePreviousFreqFlag;

    QStringList listOfRadios;
    RadioDetails selRadioDetails;
    PubSubName selRadioName;
    QString radioName;
    QString rigAppName;
    QString radioState;

    bool onContestPageChangedFlag;

    QTimer *launchRadioSelectTimer;
    int launchRadioSelectCount;

    QString lastFreq;

    CheckOperatingFreq *operatingFreq;
    bool operatingFreqPlanOk;

    QPalette *freqDisplayPalette;
    bool legalFreq = true;

    void sendFreq(QString f);
    QString getCurFreq() const;

    void sendModeToRadio(QString);
    void freqLineEditBkgnd(bool status);
    void freqLineEditFrameColour(bool status);

    void ritLineEditInFocus();
    void ritFreqLineEditFrameColour(bool status);

    void initRigFrame(QWidget *parent);
    void initPassBandRadioButtons();
    void noRadioSendOutFreq(QString f);
    void noRadioSendOutMode(QString m);

    void traceMsg(QString msg);

    void mgmLabelVisible(bool state);
    bool checkValidFreq(QString freq);

    QString calcNewFreq(double incFreq);

    int calcMinosMode(QString mode);

    void freqPlusMinusButton(double freq);


    void showRitButOn();
    void showRitButOff();

    void setVolControlVisible(bool value);

    void ritButtonOn();
    void ritButtonOff();

    void setRadioBandWarning(QString s);
    void setRadioTxVertEnabled(bool s);
    void setRadioTxVertStatus(bool status);
    void transVertIndicatorOn();
    void transVertIndicatorOff();

    bool isFreqLegal(const double freq, const QString band, const QString mode);
    bool checkFreqIsLegal(const double freq, const QString mode);
    void setFreqTextLegalColour(const QString freq, QString mode);
    void setFreqStepCombo(QString mode);
    double getStepFreqFromComboText(const QString step);
    void clearFreqInputFocus();
};





#endif // RIGCONTROLFRAME_H
