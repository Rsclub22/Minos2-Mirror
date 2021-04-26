#ifndef TXVMINTERNALSETUPDIALOG_H
#define TXVMINTERNALSETUPDIALOG_H


#include <QDialog>
#include "voicekeyerbase.h"
#include "voicekeyerfactory.h"

namespace Ui {
class txVmInternalSetupDialog;
}
const int MAXNUM_BUTTONS = 8;
const int MININUM_BUTTONS = 2;

class txVmInternalSetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit txVmInternalSetupDialog(VoiceKeyerCapabilities voiceCap_, QWidget *parent = nullptr);
    ~txVmInternalSetupDialog();



    void setVmCommonParamsData(VoiceKeyerCommonParams *vmCommonParams_);

private slots:

    void inChannelCB_currentTextChanged(const QString &arg1);
    void outChannelCB_currentTextChanged(const QString &arg1);

    void onNumButtonsValueChanged(int num);

public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;

private:

    Ui::txVmInternalSetupDialog *ui;

    VoiceKeyerCapabilities voiceCap;
    VoiceKeyerCommonParams *vmCommonParams;

    void initSetup();
    void doCloseEvent();
};

#endif // TXVMINTERNALSETUPDIALOG_H
