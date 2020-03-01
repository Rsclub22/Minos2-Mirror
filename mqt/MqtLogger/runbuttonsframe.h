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

    RunButtonsFrame *rigControlFrame;
    QToolButton* memButton;
    QMenu* memoryMenu;
    QShortcut* shortKey;
    QShortcut* shiftShortKey;
    QAction* runOffAction;
    QAction* readAction;
    QAction* writeAction;
    QAction* editAction;
    QAction* clearAction;

    QString returnFrequency;

    void setContest(BaseContestLog *);

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

    void setFreq(QString freq);
private:
    Ui::RunButtonsFrame *ui;
    LoggerContestLog *ct = nullptr;
    RigControlFrame *rigControl = nullptr;

    int runButtonOnNum = NO_RUN_BUTTON_ON;

    bool runButtonOnFlag;
    bool radioOffRunFreq;

    bool oldRadioOffRunFreq = false;  // used by on_ChkRunFreq()

    QString curRunFreq;
    QTimer *chkRunFreqTimer;


    void initRunMemoryButton();
    void loadRunButtonLabels();
    memoryData::memData getRunMemoryData(int memoryNumber);
    void setRunMemoryData(int memoryNumber, memoryData::memData m);
    bool chkRadioFreqOnRunFreq();
    int otherButton(int buttonNumber);

signals:
    void sendRunOnFlag(QString, bool);
    void sendRunOffFreqFlag(QString, bool);
private slots:
    void runButActivated(int buttonNumber);
    void on_ChkRunFreq();
public slots:
    void runButClearActSel(int buttonNumber);

};

#endif // RUNBUTTONSFRAME_H
