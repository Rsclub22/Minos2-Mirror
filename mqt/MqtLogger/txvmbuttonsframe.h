#ifndef TXVMBUTTONSFRAME_H
#define TXVMBUTTONSFRAME_H


#include "base_pch.h"
#include <QShortcut>
#include <QGroupBox>
#include <QObject>

namespace Ui {
class TxVmButtonsFrame;
}

class TxVmButtonsFrame;

class TxVoiceMemButton: public QObject
{


    Q_OBJECT
public:
    explicit TxVoiceMemButton(QToolButton *b, TxVmButtonsFrame *vmf, int no);
    ~TxVoiceMemButton();


    TxVmButtonsFrame *txVmButtonFrame;
    QToolButton* vmButton;
    QMenu* vmMenu;
    QShortcut* shortKey;
    QShortcut* shiftShortKey;
    QAction* runOffAction;
    QAction* readAction;
    QAction* writeAction;
    QAction* editAction;
    QAction* clearAction;

    void showButtonOnOff(bool state);

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

class TxVmButtonsFrame : public QGroupBox
{
    Q_OBJECT

public:
    explicit TxVmButtonsFrame(QWidget *parent = nullptr);
    ~TxVmButtonsFrame();

    QMap<int, TxVoiceMemButton*> txVmButtonMap;

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

private:
    Ui::TxVmButtonsFrame *ui;

    int runButtonOnNum;
    void initRunMemoryButton();
    void loadRunButtonLabels();

    void initTxVmButton();
private slots:
    void radioIsConnected(bool on);
};

#endif // TXVMBUTTONSFRAME_H
