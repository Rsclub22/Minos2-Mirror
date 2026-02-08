#ifndef RUNBUTTONSFRAME_H
#define RUNBUTTONSFRAME_H

#include <QShortcut>
#include <QGroupBox>

#include "rigcontrolframe.h"

namespace Ui {
class RunButtonsFrame;
}

class RunButtonsFrame;
class QToolButton;
class QMenu;

const int NO_RUN_BUTTON_ON = -1;
const int RUN_BUTTON_1_ON = 0;
const int RUN_BUTTON_2_ON = 1;

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
    //QAction* readAction;
    QAction* writeAction;
    //QAction* editAction;
    QAction* clearAction;

    Frequency returnFrequency;

    void setContest(BaseContestLog *);

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
    void clearActionActivated(int);
    void buttonActivated(int);


private:
    int memNo;
    static QKeySequence runButShortCut[];
    static QKeySequence runButShiftShortCut[];

};
class RunmodeController : public QObject
{
    Q_OBJECT

public:
    explicit RunmodeController();
    ~RunmodeController();

    void linkRunButtonFrame(RunButtonsFrame *prbf)
    {
        rbf = prbf;
    }
    void chkRunFreq();
    bool chkRadioFreqOnRunFreq();
    void setFreq(Frequency freq);
    void setRunModeOff();
    void setRunMode(int buttonNumber, memoryData::memData m);

    RunButtonsFrame *rbf = nullptr;
    QTimer *chkRunFreqTimer = nullptr;

    RigControlFrame *rigControl = nullptr;

    Frequency curRadioFreq;

    int runButtonOnNum = NO_RUN_BUTTON_ON;

    bool radioOffRunFreq = true;

    Frequency curRunFreq;
    QString curRunMode;

public slots:
    void on_radioFreqCheckTimer();
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

    void setCallFreq();
    void setFreq(Frequency freq);
    void logRadioSettingsChanged(QSharedPointer<RadioSettingsDialogChangeFlag>);
private:
    Ui::RunButtonsFrame *ui;
    LoggerContestLog *ct = nullptr;
    RigControlFrame *rigControl = nullptr;

    RunmodeController rmc;

    void initRunMemoryButton();
    void loadRunButtonLabels();
    memoryData::memData getCtRunMemoryData(int memoryNumber);
    void setCtRunMemoryData(int memoryNumber, memoryData::memData m);
    void clearCtRunMemoryData(int memoryNumber, memoryData::memData m);
    int otherButton(int buttonNumber);

    void setRunButtonsFrameTitle();
    bool getRadioReadOnlyFlag();

    void sandPChanged(BaseContestLog *c, bool s);
private slots:
    void runButActivated(int buttonNumber);
    void setFreqDisplay(Frequency, bool);
    void radioIsConnected(bool on);
public slots:
    void runButClearActSel(int buttonNumber);

};

#endif // RUNBUTTONSFRAME_H
