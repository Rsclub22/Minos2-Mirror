#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "base_pch.h"

#include "vucallback.h"
#include "rrsoundsys.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void volcallback(unsigned int rmsvol, unsigned int peakvol, unsigned int samples);
private slots:
    void onStdInRead(QString);

    void on_startRecButton_clicked();

    void on_stopRecButton_clicked();

    void on_baseFileBrowse_clicked();

    void inChannelCB_currentTextChanged(const QString &arg1);

    void outChannelCB_currentTextChanged(const QString &arg1);

    void on_baseFilename_editingFinished();

    void on_rotInterval_editingFinished();

    void on_recordLevel_valueChanged(double arg1);

    void on_recordSlider_valueChanged(int value);

    void on_recordMono_stateChanged(int arg1);

    void on_autostartCb_stateChanged(int);

    void onCloseTimer();

    void on_notify(AnalysePubSubNotify an, const QString from);
    void on_contestLinkCB_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;
    RtAudioSoundSystem rass;

    QDateTime tstart;
    QDateTime tend;
    bool started = false;
    bool stopped = false;

    StdInReader stdinReader;
    QTimer closeTimer;

    bool closing = false;
    bool inVolChange = false;

    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;
    void setVolumeMults();
};
extern MainWindow *mainWindow;
#endif // MAINWINDOW_H
