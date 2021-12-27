#ifndef KEYERMAIN_H
#define KEYERMAIN_H

#include "base_pch.h"
#include <QComboBox>
#include <QMainWindow>
#include <QProcess>
#include "KeyerJson.h"
#include "VKMixer.h"

namespace Ui {
class KeyerMain;
}

class KeyerMain : public QMainWindow
{
    Q_OBJECT
    Ui::KeyerMain *ui;

public:
    KeyerJson masterConfig;

    explicit KeyerMain(QWidget *parent = nullptr);
    virtual ~KeyerMain() override;

    void setLines(bool PTTOut, bool PTTIn, bool L1, bool L2, int lmode);

    void doConfig(QString);
    bool writeConfig(bool force);
private slots:

    void CaptionTimerTimer();

    void lineTimerTimer();


    void on_recordButton_clicked();

    void on_playButton_clicked();

    void on_stopButton_clicked();

    void on_AutoRepeatCheckBox_clicked();

    void on_PipCheckBox_clicked();

    void on_delayEdit_valueChanged(const QString &arg1);

    void on_tuneButton_clicked();

    void on_twoToneButton_clicked();

    void on_aboutButton_clicked();

    void on_setupBrowseButton_clicked();

    void on_started();
    void on_finished(int, QProcess::ExitStatus exitStatus);
    void on_error(QProcess::ProcessError error);

    void on_readyReadStandardError();
    void on_readyReadStandardOutput();


    void on_saveAlsaButton_clicked();

    void on_restoreAlsaButton_clicked();

    void on_recordSlider_valueChanged(int position);

    void on_replaySlider_valueChanged(int position);

    void on_passThroughSlider_valueChanged(int position);

    void on_recordValue_valueChanged(double arg1);

    void on_replayValue_valueChanged(double arg1);

    void on_passThroughValue_valueChanged(double arg1);

    void onPTT(bool);

    void doSetVU(unsigned int rmsvol , unsigned int peakvol, unsigned int samples );
    void lcallback( bool pPTT, bool pPTTRef, bool pL1Ref, bool pL2Ref, int lmode );

    void doSliders(int, int, int);

    void on_messageName_editingFinished();

    void on_keyCombo_currentIndexChanged(int index);

private:
    void syncSetLines();
    QTimer lineTimer;
    QTimer CaptionTimer;

    StdInReader *stdinReader = new StdInReader(this);

    bool PTT;
    bool PTTRef;
    bool L1Ref;
    bool L2Ref;
    int linesMode;

    bool recordWait;
    bool recording;

    bool inVolChange;

    unsigned int rmsvol= 0;
    unsigned int peakvol = 0;
    unsigned int samples = 0;

    QProcess *runner;

    void setVolumeMults();


    void runAlsaScript(const QString &alsaFileName, const QString &command);

    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;
};
extern KeyerMain *keyerMain;
#endif // KEYERMAIN_H
