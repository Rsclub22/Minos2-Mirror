#ifndef TXVMBUTTONDIALOG_H
#define TXVMBUTTONDIALOG_H

#include <QDialog>
#include "voicekeyerbase.h"

namespace Ui {
class TxVmButtonDialog;
}

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
private:
    Ui::TxVmButtonDialog *ui;
    VoiceKeyerParams* vmData;

};

#endif // TXVMBUTTONDIALOG_H
