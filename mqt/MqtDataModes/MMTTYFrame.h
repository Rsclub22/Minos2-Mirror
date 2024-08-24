#ifndef MMTTYFRAME_H
#define MMTTYFRAME_H

#include "frequency.h"
#include <QFrame>
#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>

#include <QProcess>
#include <QSharedMemory>

namespace Ui {
class MMTTYFrame;
}
class EngineWindow;
class MMTTYFrame : public QFrame
{
    Q_OBJECT

public:
    explicit MMTTYFrame(EngineWindow *parent, bool twoTone, QLineEdit *sendEdit, QString fname, QString name);
    ~MMTTYFrame();

    void sendCharacters(const QString &, int markf);
    void sendMode(QString);

    void closeFrame();

    void msgEventFilter(MSG *msg, long *result );

private:
    Ui::MMTTYFrame *ui;
    EngineWindow *engineWindow;
    QString fname;
    QWidget *t = nullptr;

    QLineEdit *sendEdit = nullptr;
    bool twoTone = false;

    QProcess *rttyProcess = nullptr;
    QString rttyEngine;
    QStringList rttyEngineOpts;

    HWND mttyHWnd;
    bool txState = false;
    bool active = false;

    int markFrequency = 0;
    QString bpskSpeed;
    QString rttySpeed;

    WId getTempId()
    {
        // winId should be the HWND of the widget
        return t->winId();
    }

    void runRttyEngine(QString app, QStringList opts);

    void createProcess();
private slots:

    void onSendCharacters(QString, int);
    void onSetSpeeds(QString b, QString m);

    void onRigModeFreq(QString, Frequency);
    void on_finished(int err, QProcess::ExitStatus exitStatus);
    void on_error(QProcess::ProcessError error);
    void on_readyReadStandardError();
    void on_readyReadStandardOutput();
    void on_started();

signals:
    void txChanged(bool);

};

#endif // MMTTYFRAME_H
