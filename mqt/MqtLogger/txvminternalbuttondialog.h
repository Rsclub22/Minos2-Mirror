#ifndef TXVMINTERNALBUTTONDIALOG_H
#define TXVMINTERNALBUTTONDIALOG_H

#include <QDialog>
#include "voicekeyerbase.h"

namespace Ui {
class TxVmInternalButtonDialog;
}

class TxVmInternalButtonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TxVmInternalButtonDialog(QWidget *parent = nullptr);
    ~TxVmInternalButtonDialog();

    void doSetVU(unsigned int peakvol, unsigned int rmsvol, unsigned int samples);
    void setVmData(VoiceKeyerParams* vmData);
private slots:
    void on_okButtonCicked();
    void on_cancelbuttonClicked();


    void onVmRepeatPauseDurEditingFinished();
    void on_replayButton_clicked();

    void on_recordButton_clicked();

    void on_stopButton_clicked();

    void on_recordLevel_valueChanged(double arg1);
    void on_recordSlider_valueChanged(int position);

public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;

private:
    Ui::TxVmInternalButtonDialog *ui;
    VoiceKeyerParams* vmData;
    bool inVolChange = false;

    void doCloseEvent();
    void setVolumeMults();
    bool validateDur(QString durName, QString dur, int &dur_);
};

#endif // TXVMINTERNALBUTTONDIALOG_H
