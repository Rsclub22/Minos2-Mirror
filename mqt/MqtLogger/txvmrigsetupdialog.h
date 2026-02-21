#ifndef TXVMRIGSETUPDIALOG_H
#define TXVMRIGSETUPDIALOG_H

#include <QDialog>
#include "txKeyerFactory.h"
#include "txkeyerCommonConstants.h"


namespace Ui {
class TxVmRigSetupDialog;
}

using namespace TxKeyerCommon;

class TxVmRigSetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TxVmRigSetupDialog(TxKeyerCapabilities voiceCap_, int maxNumButtons_, int nb, QWidget *parent = nullptr);
    ~TxVmRigSetupDialog();

    int getNumButtons(){return numButtons;}

    bool getCatPttForEomState();
    bool getSetCwModeAndRestoreState();
    void setSwitchToCwVisible(bool visible);
    void setSwitchToCwChecked(bool checked);
    void setSetupRadioGroupBoxTitle(QString selectedRadioName);
    void setMaxNumOfButtons(int maxNumButtons);
    void setMaxNumOfButtonsLabel(int maxNumButtons);

    void setPttEomGroupBoxVisible(bool visible);
    void setEomRadioButtons(TxKeyerCommon::KeyerEomTypes eomType);
    TxKeyerCommon::KeyerEomTypes getSelectedEomType();


    void setButtonWidgetsVisible(bool visible);
private slots:

    void onNumButtonsValueChanged(int num);



public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;

private:

    Ui::TxVmRigSetupDialog *ui;

    TxKeyerCapabilities voiceCap;

    int numButtons = MINIMUM_BUTTONS;
    int maxNumButtons = MAXIMUM_BUTTONS;


    void initSetup();
    void doCloseEvent();


};

#endif // TXVMRIGSETUPDIALOG_H
