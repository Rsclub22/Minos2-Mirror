#ifndef KPMAINWINDOW_H
#define KPMAINWINDOW_H

#include "StdInReader.h"
#include <QMainWindow>
#include <QProcess>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class KPMainWindow; }
QT_END_NAMESPACE

class KPRPCServer;

class KPMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    KPMainWindow(QWidget *parent = nullptr);
    ~KPMainWindow();

private:
    Ui::KPMainWindow *ui;
    QProcess *runner = nullptr;
    QTimer CloseTimer;
    StdInReader *stdinReader = new StdInReader(this);
    KPRPCServer *kpc = nullptr;
    QString host;
    QString port;

    void runAlsaScript(const QString &alsaFileName, const QString &command);

    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

private slots:
    void on_setupBrowseButton_clicked();

    void on_started();
    void on_finished(int, QProcess::ExitStatus exitStatus);
    void on_error(QProcess::ProcessError error);

    void on_readyReadStandardError();
    void on_readyReadStandardOutput();


    void on_saveAlsaButton_clicked();

    void on_restoreAlsaButton_clicked();

    void on_outputCombo_activated(int);
    void CloseTimerTimer();
    void onNewHost(QString h, QString p);
    void onSequenceCount(qint64);
};
#endif // KPMAINWINDOW_H
