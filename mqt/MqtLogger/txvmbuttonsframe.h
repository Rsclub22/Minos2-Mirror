#ifndef TXVMBUTTONSFRAME_H
#define TXVMBUTTONSFRAME_H

#include <QShortcut>
#include <QGroupBox>
#include <QKeyEvent>

#include "PubSubValue.h"
#include "radiodetails.h"
#include "voicekeyerbase.h"
#include "voicekeyerfactory.h"

class QToolButton;
class QMenu;
class BaseContestLog;

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

    QAction* newAction;
    QAction* editAction;

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

    void setPttEnabled(bool state, PubSubName psn);

    void setPttType(int type, PubSubName psn);
    void setVoiceMemAvail(bool avail, PubSubName psn);
    void setCwMemType(int cwMemType, PubSubName psn);
    void setPttState(bool state);

    void setSelectedRadio(PubSubName selectedRadio);
    void setRadioIsConnected(bool connected);
    void setRadioName(const QString radName);
    int getCwMemType(PubSubName psn);
    void setMode(const QString m);
    void setContest(BaseContestLog *);
signals:

    void pttStatus(bool);
    void sendRadioMode(QString m);

private:
    Ui::TxVmButtonsFrame *ui;

    QSharedPointer<VoiceKeyerBase> txVoiceKeyer;
    VoiceKeyerFactory* voiceKeyerFactory;

    BaseContestLog *ct = nullptr;

    QTimer *extKeyerConnectTimer = nullptr;
    //QList<QShortcut *> shortCutKeyList;

    QShortcut *stopButtonShortcut;
    QString voiceKeyerType;


    QTimer* msgDurTimer;
    QTimer* repeatPauseTimer;
    int buttonNumSent ;

    bool usePttForEomFlag = false;

    QList<QToolButton*> voiceMemButtonList;

    QList<VoiceKeyerParams> vmKeyParamList;

    QMap<PubSubName, RadioDetails> allRadioDetails;

    PubSubName selectedRadio;
    QString curMode;
    QString savedMode;
    bool radioConnected;
    bool pttState;

    bool notifyComboChange = true;
    void initTxVmButtonFrame();



    void setRunButtonText(const int buttonNumber, const QString name);
    void setVoiceNumMemButtonsVisible(int);
    void clearButtonLabels();
    void startVMMsg(int buttonNumber);
    void createKeyer(QString voiceKeyerName);
    void setPttStatusIndicatorOnOff(bool on);
    bool isVoiceMemAvail(PubSubName psn);
    bool isCwMemTypeAvail(PubSubName psn);
    void setAvailIndicatorVisible(bool visible);
    void setAvailIndicatorOnOff(bool on);
    void setAvailIndicatorForRadioOnOff(PubSubName radName);
    void setRepeatIndicatorVisible(bool visible);
    void setRepeatIndicatorForMessageOnOff(bool state);
    void setFrameState(QString voiceKeyerName);
    void setRepeatIndicatorOnOff(bool on);
    void pttStopMessage(bool state);


    void updateVoiceMemAvailStateAndCwType();
    void setTXStatusVisible(bool visible);
    void sendModeToRadio(const QString m);
private slots:

    void onVoiceKeyerSelect(int idx);
    void onVmSetupClicked();
    void onRepeatPauseTimerTimeout();
    void onVmStopClicked();
    void onMsgDurTimerTimeout();
    void fKey(BaseContestLog *c, int e);


    void onRemoteConfigChanged();
    void onRemoteKeyerStarted(int key);
    void onRemoteKeyerStopped();
    void on_pipCb_stateChanged(int arg1);
    void onExtConnectTimer();
};

#endif // TXVMBUTTONSFRAME_H
