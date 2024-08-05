/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Winkeyer Control
//
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2024
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////
#ifndef WINKEYERSETUPDIALOG_H
#define WINKEYERSETUPDIALOG_H

#include <QDialog>
#include "winKeyerCommon.h"



namespace Ui {
class WinKeyerSetupDialog;
}

class WinKeyerSetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WinKeyerSetupDialog(QWidget *parent = nullptr);
    ~WinKeyerSetupDialog();
    bool isSetupDirty(){return setupIsDirty;}
    QSharedPointer<WinkeyerStateStorage>getSetupData(){ return setupWinkeyStateStoragePtr;}

private slots:

    void onWeightLineEditEditingFinished();
    void onTailTimeLineEditEditingFinished();
    void onLeadTimeLineEditEditingFinished();
    void onFirstCharExtenLineEditEditingFinished();
    void onkeyCompLineEditEditingFinished();

    void onPaddleswapStateChanged();
    void onAutoSpaceStateChanged();
    void onCtSpacingStateChanged();
    void onEnablePttStateChanged();
    void onSerialEchoStateChanged();
    void onPadddleEchoStateChanged();
    void onPaddleMuteStateChanged();
    void onMinWpmLineEditEditingFinished();
    void onMaxWpmLineEditEditingFinished();
    void onComportSelCurrentIndexChanged();
    void onPaddleWdChkBoxStateChanged();
    void onKeyerModelSelectCurrentIndexChanged();

    void SpeedPotLockRadButClicked();
    void onSidetoneComobselIndexedChanged();
    void onHangTimeComboSelIndexedChanged();
private:
    Ui::WinKeyerSetupDialog *ui;

    QSharedPointer<WinkeyerState> setupWinkeyerStatePtr;
    QSharedPointer<WinkeyerStateStorage> setupWinkeyStateStoragePtr;

    bool setupIsDirty = false;

    void loadAvailableComports();
    void fillKeyerModes();
    void fillSidetoneFreq();
    void fillHangtime();


    void loadSettingsToDialog();

    void setComportComboComboSel(QString comport);
    void setSidetoneComboSel(QString freq);
    void onDefaultsPushbuttonClicked();
    void connectSignals();
    void disconnectSignals();
};

#endif // WINKEYERSETUPDIALOG_H
