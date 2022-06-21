#ifndef TXVMEXTERNALBUTTONDIALOG_H
#define TXVMEXTERNALBUTTONDIALOG_H

#include <QDialog>
#include "SliderSpinner.h"
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

    void onKeyerConfig(QString key, QString val);

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void volsChanged();

    void compressionChanged();

    void on_doFilter_stateChanged(int arg1);

    void on_doCompression_stateChanged(int arg1);

public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;

private:
    Ui::TxVmExternalButtonDialog *ui;

    SliderSpinner *recordFrame = nullptr;
    SliderSpinner *replayFrame = nullptr;
    SliderSpinner *passthroughFrame = nullptr;

    SliderSpinner *windowFrame = nullptr;
    SliderSpinner *thresholdFrame = nullptr;
    SliderSpinner *ratioFrame = nullptr;
    SliderSpinner *attackFrame = nullptr;
    SliderSpinner *releaseFrame = nullptr;
    SliderSpinner *makeUpGainFrame = nullptr;


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
