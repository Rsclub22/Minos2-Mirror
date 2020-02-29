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

const QString RUN_BUTTON_ON_FREQ_STYLE = QString("background-color: orange ; color:black ; border-style: outset; border-width: 1px; border-color: black; min-width: 5em; padding: 3px;\n");
const QString RUN_BUTTON_OFF_FREQ_STYLE = QString("background-color: yellow ; color:black ; border-style: outset; border-width: 1px; border-color: black; min-width: 5em; padding: 3px;\n");
const QString RUN_BUTTON_OFF_STYLE = QString("background-color: Gainsboro ; color:black ; border-style: outset; border-width: 1px; border-color: black; min-width: 5em; padding: 3px;\n");



const int NO_RUN_BUTTON_ON = -1;
const int RUN_BUTTON_1_ON = 0;
const int RUN_BUTTON_2_ON = 1;
const int NUM_RUNBUTTONS = 2;
const int CHECK_RUN_FREQ_POLLTIME = 1000;

class RigControlFrame;
class RunMemoryButton : public QObject
{
    Q_OBJECT

public:
    explicit RunMemoryButton(QToolButton *b, RigControlFrame *rcf, int no);
    ~RunMemoryButton();

    RigControlFrame *rigControlFrame;
    QToolButton* memButton;
    QMenu* memoryMenu;
    QShortcut* shortKey;
    QShortcut* shiftShortKey;
    QAction* runOffAction;
    QAction* readAction;
    QAction* writeAction;
    QAction* editAction;
    QAction* clearAction;


    int getMemNo(){return memNo;}

    void showButtonOnOff(bool state);
    void showRunToolButtonOffFreq();
    void showRunToolButtonOnFreq();

private slots:
    void memoryUpdate();

    void memoryShortCutSelected();
    void readActionSelected();
    void editActionSelected();
    void writeActionSelected();
    void clearActionSelected();
    void buttonSelected();
    void runOffActionSelected();
signals:
    void clearActionSelected(int);
    void buttonActivated(int);


private:
    int memNo;
};

class quickBandSelData
{

public:

    QString band;
    QString freq;


};

class RigControlFrame : public QFrame
{
    Q_OBJECT

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
    void runButtonUpdate(int);
    void runButReadActSel(int buttonNumber);
    void runButWriteActSel(int buttonNumber);
    void runButEditActSel(int buttonNumber);

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



    void runButOffActionSelected(int buttonNumber);
    void setRunButtonActive(int buttonNumber);

    void runModeOff(int buttonNumber);

    void switchRunButton(int buttonNumber);

    void setRunFreq(int buttonNumber);

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
    void sendRunOnFlag(QString, bool);
    void sendRunOffFreqFlag(QString, bool);
    //void sendCQFreq(QString, bool);







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
    void sendFreq(QString f);


    void ritClearShortCutSelected();
    //void bandListTimeout();

    void setRadioFreq();


    void checkRigDetailsAvail();
    void clusterUpdateRigFreq(QString freq);


    void freqStepComboChanged(const QString step);
    void runButActivated(int buttonNumber);
    void on_ChkRunFreq();
public slots:
    void returnChangeRadioFreq();
    void runButClearActSel(int buttonNumber);
private:
    virtual bool eventFilter(QObject *obj, QEvent *event) override;


    // memory buttons
    memoryData::memData getRunMemoryData(int memoryNumber);

    void setRunMemoryData(int memoryNumber, memoryData::memData m);

    LoggerContestLog *ct = nullptr;

    QMap<int, RunMemoryButton *> runButtonMap;
    QMap<int, QCheckBox*> ignoreRunChkBoxMap;

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

    int runButtonOnNum = NO_RUN_BUTTON_ON;

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


    bool runButtonOnFlag;
    bool radioOffRunFreq;
    QString curRunFreq;
    QTimer *chkRunFreqTimer;


    bool oldRadioOffRunFreq = false;  // used by on_ChkRunFreq()


    void sendModeToRadio(QString);
    void freqLineEditBkgnd(bool status);
    void freqLineEditFrameColour(bool status);

    void ritLineEditInFocus();
    void ritFreqLineEditFrameColour(bool status);

    void initRigFrame(QWidget *parent);
    void initPassBandRadioButtons();
    void noRadioSendOutFreq(QString f);
    void noRadioSendOutMode(QString m);

    void initRunMemoryButton();
    void loadRunButtonLabels();

    void traceMsg(QString msg);

    void loadMemories();
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
    bool chkRadioFreqOnRunFreq();
};





#endif // RIGCONTROLFRAME_H
