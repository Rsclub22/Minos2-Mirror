#ifndef KEYERMAIN_H
#define KEYERMAIN_H

#include "base_pch.h"
#include <QComboBox>
#include <QMainWindow>
#include <QProcess>

#include "VKMixer.h"

namespace Ui {
class KeyerMain;
}

class KeyerMain : public QMainWindow
{
    Q_OBJECT
    Ui::KeyerMain *ui;

public:
    explicit KeyerMain(QWidget *parent = nullptr);
    virtual ~KeyerMain() override;

    void setLines(bool PTTOut, bool PTTIn, bool L1, bool L2, int lmode);
    void volcallback(unsigned int rmsvol , unsigned int peakvol, unsigned int samples );

private slots:

    void onStdInRead(QString);

    void CaptionTimerTimer();

    void LineTimerTimer();


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

private:
    void syncSetLines();
    QTimer LineTimer;
    QTimer CaptionTimer;

    StdInReader stdinReader;

    bool PTT;
    bool PTTRef;
    bool L1Ref;
    bool L2Ref;
    int linesMode;

    bool recordWait;
    bool recording;

    bool inVolChange;

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
