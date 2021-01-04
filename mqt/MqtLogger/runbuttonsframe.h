#ifndef RUNBUTTONSFRAME_H
#define RUNBUTTONSFRAME_H

#include "base_pch.h"
#include <QShortcut>
#include <QGroupBox>

#include "rigcontrolframe.h"

namespace Ui {
class RunButtonsFrame;
}

class RunButtonsFrame;

class RunMemoryButton : public QObject
{
    Q_OBJECT

public:
    explicit RunMemoryButton(QToolButton *b, RunButtonsFrame *rcf, int no);
    ~RunMemoryButton();

    RunButtonsFrame *runButtonsFrame;
    QToolButton* memButton;
    QMenu* memoryMenu;
    QShortcut* shortKey;
    QShortcut* shiftShortKey;
    QAction* runOffAction;
    QAction* readAction;
    QAction* writeAction;
    QAction* editAction;
    QAction* clearAction;

    Frequency returnFrequency;

    void setContest(BaseContestLog *);

    int getMemNo(){return memNo;}

    void showButtonOnOff(bool state);
    void showRunToolButtonOffFreq();
    void showRunToolButtonOnFreq();

private slots:
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

class RunButtonsFrame : public QGroupBox
{
    Q_OBJECT

public:
    explicit RunButtonsFrame(QWidget *parent = nullptr);
    ~RunButtonsFrame();

    QMap<int, RunMemoryButton *> runButtonMap;

    void runButtonUpdate(int);
    void runButReadActSel(int buttonNumber);
    void runButWriteActSel(int buttonNumber);
    void runButEditActSel(int buttonNumber);
    void runButOffActionSelected(int buttonNumber);
    void setRunButtonActive(int buttonNumber);
    void runModeOff(int buttonNumber);
    void switchRunButton(int buttonNumber);
    void setRunFreq(int buttonNumber);
    void setRunButtonText(int buttonNumber);

    void setContest(BaseContestLog *c);
    void setRigControl(RigControlFrame *);

    void setFreq(Frequency freq);
private:
    Ui::RunButtonsFrame *ui;
    LoggerContestLog *ct = nullptr;
    RigControlFrame *rigControl = nullptr;

    Frequency curRadioFreq;

    int runButtonOnNum;

    bool radioOffRunFreq = false;

    bool oldRadioOffRunFreq = false;  // used by on_ChkRunFreq()

    Frequency curRunFreq;
    QString curRunMode;
    QTimer *chkRunFreqTimer;


    void initRunMemoryButton();
    void loadRunButtonLabels();
    memoryData::memData getRunMemoryData(int memoryNumber);
    void setRunMemoryData(int memoryNumber, memoryData::memData m);
    bool chkRadioFreqOnRunFreq();
    int otherButton(int buttonNumber);

    void chkRunFreq();
signals:
    void sendRunOnFlag(Frequency, QString mode, bool);
    void sendRunOffFreqFlag(Frequency, bool);
private slots:
    void runButActivated(int buttonNumber);
    void on_radioFreqCheckTimer();
    void setFreqDisplay(Frequency, bool);
    void radioIsConnected(bool on);
public slots:
    void runButClearActSel(int buttonNumber);

};

#endif // RUNBUTTONSFRAME_H
