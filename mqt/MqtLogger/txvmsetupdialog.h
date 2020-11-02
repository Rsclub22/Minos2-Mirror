#ifndef TXVMSETUPDIALOG_H
#define TXVMSETUPDIALOG_H

#include <QDialog>
#include "voicekeyerbase.h"
#include "voicekeyerfactory.h"

namespace Ui {
class TxVmSetupDialog;
}

const int MAXNUM_BUTTONS = 8;
const int MININUM_BUTTONS = 2;

class TxVmSetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TxVmSetupDialog(VoiceKeyerCapabilities voiceCap_, QWidget *parent = nullptr);
    ~TxVmSetupDialog();


    void setVmCommonParamsData(VoiceKeyerCommonParams *vmCommonParams_);
private:

    Ui::TxVmSetupDialog *ui;

    VoiceKeyerCapabilities voiceCap;




    void initSetup();

};

#endif // TXVMSETUPDIALOG_H
