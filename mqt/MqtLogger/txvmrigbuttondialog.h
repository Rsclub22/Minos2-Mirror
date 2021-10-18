#ifndef TXVMRIGBUTTONDIALOG_H
#define TXVMRIGBUTTONDIALOG_H

#include <QDialog>
#include "voicekeyerbase.h"

namespace Ui {
class TxVmRigButtonDialog;
}


class TxVmRigButtonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TxVmRigButtonDialog(QWidget *parent = nullptr);
    ~TxVmRigButtonDialog();

    void setVmData(VoiceKeyerParams* vmData);
    void setCwMessageTextBoxVisible(bool state);
private slots:
    void on_okButtonClicked();
    void on_cancelbuttonClicked();

    void onVmRepeatPauseDurEditingFinished();
    void onVmMessageDurEditingFinished();
    void on_txCwMessageEditingFinshed();
public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;
private:

    Ui::TxVmRigButtonDialog *ui;
    VoiceKeyerParams* vmData;

    bool validateDur(QString durName, QString dur, int &dur_);
    void doCloseEvent();
    bool checkLengthOfCwMessage(int length);
};

#endif // TXVMRIGBUTTONDIALOG_H
