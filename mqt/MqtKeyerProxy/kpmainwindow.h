#ifndef KPMAINWINDOW_H
#define KPMAINWINDOW_H

#include "CommandReader.h"
#include "vudata.h"
#include <QMainWindow>
#include <QProcess>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class KPMainWindow; }
QT_END_NAMESPACE

class KPRPCServer;
class QComboBox;
class SerialPTT;
class QLabel;

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
    QSharedPointer<CommandReader> commandReader = QSharedPointer<CommandReader>(new CommandReader(this));
    KPRPCServer *kpc = nullptr;

    QString oldHost;
    QString oldPort;
    QString oldRate;

    bool pttState = false;

    SerialPTT *sp = nullptr;
    QLabel *sbLabel = nullptr;
    bool inPTTConfig = false;

    QString alsaStore = "store";
    QString alsaRestore = "restore";

    void runAlsaScript(const QString &alsaFileName, const QString &command);

    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

    void fillPortsInfo(QComboBox *comportSel);
    void saveSerialSettings();
    bool openSerialPort();
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
    void onNewHost(QString h, QString p, QString s);
    void onSequenceCount(qint64);
    void doSetVU(vudata);
    void doPTT(bool);
    void on_spCombo_activated(int index);
    void on_dtr_clicked();
    void on_rts_clicked();
    void on_serialPTTEnable_stateChanged(int arg1);
    void on_pushButton_clicked();
};
#endif // KPMAINWINDOW_H
