#ifndef TXVMBUTTONDIALOG_H
#define TXVMBUTTONDIALOG_H

#include <QDialog>
#include "voicekeyerbase.h"

namespace Ui {
class TxVmButtonDialog;
}

const int REPEAT_DUR_MIN = 0;
const int REPEAT_DUR_MAX = 180; // secs

class TxVmButtonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TxVmButtonDialog(QWidget *parent = nullptr);
    ~TxVmButtonDialog();

    void setVmData(VoiceKeyerParams* vmData);
private slots:
    void on_okButton_clicked();
    void on_cancelbutton_clicked();
    void onVmRepeatDurEditingFinished();
private:
    Ui::TxVmButtonDialog *ui;
    VoiceKeyerParams* vmData;

    bool validateRepeatDur(QString dur, int &dur_);
};

#endif // TXVMBUTTONDIALOG_H
