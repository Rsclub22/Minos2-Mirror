#ifndef TXVMBUTTONSFRAME_H
#define TXVMBUTTONSFRAME_H


#include "base_pch.h"
#include <QShortcut>
#include <QGroupBox>
#include <QObject>
#include "voicekeyerbase.h"
#include "voicekeyerfactory.h"
#include "txvmbuttondialog.h"
#include "txvmsetupdialog.h"
#include "radiodetails.h"
#include "rigcontrolframe.h"

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


    TxVmButtonsFrame *txVmButtonsFrame;
    QToolButton* vmButton;
    QMenu* vmMenu;
    QShortcut* shortKey;
    QShortcut* shiftShortKey;
    QAction* readAction;
    QAction* newAction;
    QAction* editAction;
    QAction* setupAction;


    void showButtonOnOff(bool state);

private slots:
    void memoryShortCutSelected();
    void readActionSelected();
    void editActionSelected();
    void writeActionSelected();

    void buttonSelected();


signals:

    void buttonActivated(int);


private:
    int memNo;


};


const QString VM_BUTTON_ON_STYLE = QString("background-color: orange ; color:black ; border-style: outset; border-width: 1px; border-color: black;\n");
const QString VM_BUTTON_OFF_STYLE = QString("background-color: Gainsboro ; color:black ; border-style: outset; border-width: 1px; border-color: black;\n");


class TxVmButtonsFrame : public QGroupBox
{
    Q_OBJECT

public:
    explicit TxVmButtonsFrame(QWidget *parent = nullptr);
    ~TxVmButtonsFrame();

    QMap<int, TxVoiceMemButton*> txVmButtonMap;





    void readActionSel(int buttonNumber);
    void readActionSelected(int buttonNumber);
    void editActionSelected(int buttonNumber);
    void writeActionSelected(int buttonNumber);




    void setRadioLoaded();
    void onSetPttEnabled(bool state, PubSubName psn);
    void onSetPttType(int type, PubSubName psn);
    void onSetPttState(bool state);
    void setRigControl(RigControlFrame *rc);
private:
    Ui::TxVmButtonsFrame *ui;


    VoiceKeyerBase* txVoiceKeyer;
    VoiceKeyerFactory* voiceKeyerFactory;

    QList<QShortcut *> shortCutKeyList;

    VoiceKeyerCommonParams vmCommonParams;

    QString voiceKeyerType;

    QTimer* msgDurTimer;
    QTimer* repeatPauseTimer;
    int buttonNumSent;

    QList<QToolButton*> voiceMemButtonList;

    QList<VoiceKeyerParams> vmKeyParamList;

    QMap<PubSubName, RadioDetails> allRadioDetails;

    bool radioConnected;
    bool radioLoaded;
    bool pttState;
    RigControlFrame *rigControl;

    void initTxVmButton();

    void setRunButtonText(const int buttonNumber, const QString name);
    void saveVmCommonParams(VoiceKeyerCommonParams &vmCommonParams);
    void readVmCommonParams(VoiceKeyerCommonParams &vmCommonParams);
    void setVoiceNumMemButtonsVisible(int);
    void clearButtonLabels();
    void startVMMsg(int buttonNumber);
private slots:
    void onRadioIsConnected(bool on);
    void onVoiceKeyerSelect(int idx);
    void onVmSetupClicked();
    void onRepeatPauseTimerTimeout();
    void onVmStopClicked();
    void onMsgDurTimerTimeout();
};

#endif // TXVMBUTTONSFRAME_H
