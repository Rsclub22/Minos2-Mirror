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

class MMTTYFrame : public QFrame
{
    Q_OBJECT

public:
    explicit MMTTYFrame(QWidget *parent, bool twoTone, QLineEdit *sendEdit, QString fname);
    ~MMTTYFrame();

    void sendCharacters(const QString &, int carrier);
    void sendMode(QString);

    void closeFrame();

    void msgEventFilter(MSG *msg, long *result );

private:
    Ui::MMTTYFrame *ui;
    QString fname;
    QWidget *t = nullptr;

    QLineEdit *sendEdit = nullptr;
    bool twoTone = false;

    QProcess *rttyProcess = nullptr;
    QString rttyEngine;
    QStringList rttyEngineOpts;

    HWND mttyHWnd;
    bool txState = false;
    bool twoToneActive = false;
    bool mmttyActive = false;
    int carrier = 0;
    int bpskSpeed = 31;
    int rttySpeed = 45;

    HWND getTempHwnd()
    {
        return reinterpret_cast<HWND>(t->winId());
    }
    WId getTempId()
    {
        return t->winId();
    }

    void runRttyEngine(QString app, QStringList opts);

    void createProcess();
private slots:

    void onSendCharacters(QString, int);
    void onSetSpeeds(int b, int m);

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
