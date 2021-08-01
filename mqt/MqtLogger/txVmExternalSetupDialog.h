#ifndef TXVMEXTERNALSETUPDIALOG_H
#define TXVMEXTERNALSETUPDIALOG_H

#include <QDialog>
#include "voicekeyerbase.h"
#include "voicekeyerfactory.h"

namespace Ui {
class TxVmExternalSetupDialog;
}

class TxVmExternalSetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TxVmExternalSetupDialog(VoiceKeyerCapabilities voiceCap_, QWidget *parent = nullptr);
    ~TxVmExternalSetupDialog();

    void setVmCommonParamsData(VoiceKeyerCommonParams *vmCommonParams_);

private:
    Ui::TxVmExternalSetupDialog *ui;

    VoiceKeyerCapabilities voiceCap;
    VoiceKeyerCommonParams *vmCommonParams;

    void initSetup();
    void doCloseEvent();
private slots:

//    void inChannelCB_currentTextChanged(const QString &arg1);
//    void outChannelCB_currentTextChanged(const QString &arg1);

//    void onNumButtonsValueChanged(int num);

public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;

};

#endif // TXVMEXTERNALSETUPDIALOG_H
