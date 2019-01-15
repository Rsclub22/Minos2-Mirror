
/////////////////////////////////////////////////////////////////////////////
// $Id$
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

namespace Ui {
    class RigControlFrame;
}

const int BANDLIST_TIMEOUT_DUR = 3000;

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
    QAction* readAction;
    QAction* writeAction;
    QAction* editAction;
    QAction* clearAction;

    int memNo;



private slots:
    void memoryUpdate();

    void memoryShortCutSelected();
    void readActionSelected();
    void editActionSelected();
    void writeActionSelected();
    void clearActionSelected();
signals:
    void clearActionSelected(int);

};
class TuneMemoryButton : public QObject
{
    Q_OBJECT

public:
    explicit TuneMemoryButton(QToolButton *b, RigControlFrame *rcf, int no);
    ~TuneMemoryButton();

    RigControlFrame *rigControlFrame;
    QToolButton* memButton;
    QMenu* memoryMenu;
    QAction* readAction;
    QAction* writeAction;

    int memNo;
    QString freq;
private slots:

    void readActionSelected();
    void writeActionSelected();
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
    void setTpm(int);
    void setRitEnableState(bool s);

    bool isRadioLoaded();


    void exitFreqEdit();
    void runButtonUpdate(int);
    void runButReadActSel(int buttonNumber);
    void runButWriteActSel(int buttonNumber);
    void runButEditActSel(int buttonNumber);

    void tuneButtonUpdate(int);
    void tuneButReadActSel(int buttonNumber);
    void tuneButWriteActSel(int buttonNumber);

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
    void sendFreq(QString f);


    void ritClearShortCutSelected();
    //void bandListTimeout();

    void setRadioFreq();


    void checkRigDetailsAvail();
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
    QMap<int, TuneMemoryButton *> tuneButtonMap;
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
    QString curMode;

    bool ritEnable;
    bool ritOn;
    bool ritEditOn;
    //QString curRit;



    QStringList listOfRadios;
    RadioDetails selRadioDetails;
    PubSubName selRadioName;
    QString radioName;
    QString rigAppName;
    QString radioState;

    QTimer *launchRadioSelectTimer;
    int launchRadioSelectCount;

    QString lastFreq;

    TuneMemoryButton *curTuneButton = nullptr;

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

    void initTuneMemoryButton();
    void updateTuneButtons();

    void traceMsg(QString msg);

    QString extractKhz(QString f);
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
};





#endif // RIGCONTROLFRAME_H
