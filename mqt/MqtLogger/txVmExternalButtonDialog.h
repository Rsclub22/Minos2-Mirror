#ifndef TXVMEXTERNALBUTTONDIALOG_H
#define TXVMEXTERNALBUTTONDIALOG_H

#include <QDialog>
#include "voicekeyerbase.h"

namespace Ui {
class TxVmExternalButtonDialog;
}

class TxVmExternalButtonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TxVmExternalButtonDialog(QWidget *parent = nullptr);
    ~TxVmExternalButtonDialog();

    void doSetVU(unsigned int rmsvol, unsigned int peakvol, unsigned int samples);
    void setVmData(VoiceKeyerParams* vmData);

private slots:
    void on_okButtonCicked();
    void on_cancelbuttonClicked();


    void onVmRepeatPauseDurEditingFinished();
    void onVmMessageDurEditingFinished();
    void on_replayButton_clicked();

    void on_recordButton_clicked();

    void on_stopButton_clicked();

    void on_recordLevel_valueChanged(double arg1);
    void on_recordSlider_valueChanged(int position);

public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;

private:
    Ui::TxVmExternalButtonDialog *ui;
    VoiceKeyerParams* vmData;
    bool inVolChange = false;

    void doCloseEvent();
    void setVolumeMults();
    bool validateDur(QString durName, QString dur, int &dur_);


};

#endif // TXVMEXTERNALBUTTONDIALOG_H
