#ifndef TXVMRIGBUTTONDIALOG_H
#define TXVMRIGBUTTONDIALOG_H

#include <QDialog>
#include "voicekeyerbase.h"

namespace Ui {
class TxVmRigButtonDialog;
}

const int REPEAT_DUR_MIN = 0;
const int REPEAT_DUR_MAX = 180; // secs

class TxVmRigButtonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TxVmRigButtonDialog(QWidget *parent = nullptr);
    ~TxVmRigButtonDialog();

    void setVmData(VoiceKeyerParams* vmData);
private slots:
    void on_okButtonClicked();
    void on_cancelbuttonClicked();

    void onVmRepeatPauseDurEditingFinished();
    void onVmMessageDurEditingFinished();

private:
    Ui::TxVmRigButtonDialog *ui;
    VoiceKeyerParams* vmData;


    bool validateDur(QString durName, QString dur, int &dur_);
};

#endif // TXVMRIGBUTTONDIALOG_H
