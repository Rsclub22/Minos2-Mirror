#ifndef TXVMRIGSETUPDIALOG_H
#define TXVMRIGSETUPDIALOG_H

#include <QDialog>
#include "voicekeyerbase.h"
#include "voicekeyerfactory.h"

namespace Ui {
class TxVmRigSetupDialog;
}

const int MAXNUM_BUTTONS = 8;
const int MININUM_BUTTONS = 2;

class TxVmRigSetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TxVmRigSetupDialog(VoiceKeyerCapabilities voiceCap_, int nb, QWidget *parent = nullptr);
    ~TxVmRigSetupDialog();

    int getNumButtons(){return numButtons;}

    bool getCatPttForEomState();
    bool getSetCwModeAndRestoreState();
private slots:

    void onNumButtonsValueChanged(int num);

public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;

private:

    Ui::TxVmRigSetupDialog *ui;

    VoiceKeyerCapabilities voiceCap;

    int numButtons = MININUM_BUTTONS;

    void initSetup();
    void doCloseEvent();
};

#endif // TXVMRIGSETUPDIALOG_H
