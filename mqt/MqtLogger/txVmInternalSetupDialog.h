#ifndef TXVMINTERNALSETUPDIALOG_H
#define TXVMINTERNALSETUPDIALOG_H


#include <QDialog>
#include "txKeyerfactory.h"
#include "txkeyerCommonConstants.h"

extern const char * indevKey;
extern const char * outdevKey;


namespace Ui {
class txVmInternalSetupDialog;
}

using namespace TxKeyerCommon;

class txVmInternalSetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit txVmInternalSetupDialog(TxKeyerCapabilities voiceCap_, int maxNumButtons_, int nb, QWidget *parent = nullptr);
    ~txVmInternalSetupDialog();

    int getNumButtons(){return numButtons;}


private slots:

    void inChannelCB_currentTextChanged(const QString &arg1);
    void outChannelCB_currentTextChanged(const QString &arg1);

    void onNumButtonsValueChanged(int num);

public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;

private:

    Ui::txVmInternalSetupDialog *ui;

    TxKeyerCapabilities voiceCap;

    int numButtons = MININUM_BUTTONS;
    int maxNumButtons = MAXIMUM_BUTTONS;

    void initSetup();
    void doCloseEvent();
};

#endif // TXVMINTERNALSETUPDIALOG_H
