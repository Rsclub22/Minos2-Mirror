#ifndef TXVMEXTERNALBUTTONDIALOG_H
#define TXVMEXTERNALBUTTONDIALOG_H

#include <QDialog>
#include "voicekeyerbase.h"
#include "CompressorParams.h"

namespace Ui {
class TxVmExternalButtonDialog;
}

class TxVmExternalButtonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TxVmExternalButtonDialog(QWidget *parent = nullptr);
    ~TxVmExternalButtonDialog();

    void setVmData(VoiceKeyerParams* vmData);

private slots:
    void on_replayButton_clicked();

    void on_recordButton_clicked();

    void on_stopButton_clicked();

    void on_recordValue_valueChanged(double arg1);
    void on_recordSlider_valueChanged(int position);

    void onKeyerConfig(QString key, QString val);

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_replayValue_valueChanged(double arg1);

    void on_replaySlider_valueChanged(int value);

    void on_passThroughValue_valueChanged(double arg1);

    void on_passThroughSlider_valueChanged(int value);

    void on_windowSlider_valueChanged(int);

    void on_thresholdSlider_valueChanged(int );

    void on_ratioSlider_valueChanged(int );

    void on_attackSlider_valueChanged(int);

    void on_releaseSlider_valueChanged(int );

    void on_makeUpGainSlider_valueChanged(int);

public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;

private:
    Ui::TxVmExternalButtonDialog *ui;
    VoiceKeyerParams* vmData;
    int inVolChangeCount = 0;

    void doCloseEvent();
    bool validateDur(QString durName, QString dur, int &dur_);

    CompressorParams compParams;

    void getCompSliders();
    void setCompSliders();

    void pubSliders();
};

#endif // TXVMEXTERNALBUTTONDIALOG_H
