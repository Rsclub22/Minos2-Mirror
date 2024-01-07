#ifndef POWERMATEFRAME_H
#define POWERMATEFRAME_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <QFrame>
#include <QThread>

class ControlPowerMate;
#if defined (WIN32)

class PMThread:public QThread
{
public:
    PMThread(ControlPowerMate *p, HANDLE devH);
    virtual ~PMThread();

    bool terminated = false;
    HANDLE devHandle = 0;
    int TIMER_CAT = 100;
    ControlPowerMate *controller = nullptr;

    virtual void run() Q_DECL_OVERRIDE;

};
#endif

class ControlPowerMate: public QObject
{
    Q_OBJECT
private:
    static ControlPowerMate *cp;
    ControlPowerMate();
    ~ControlPowerMate() override;

#if defined (WIN32)

    HANDLE handleToDevice = 0;							// device handle of the PowerMate Knob

    PMThread *pmThread = nullptr;


signals:
    void dataReceived(QByteArray);

public:
    static ControlPowerMate *getControlPowerMate()
    {
        if (!cp)
        {
            cp = new ControlPowerMate();
        }
        return cp;
    }

    void start();
    void stop();

};

namespace Ui {
class PowerMateFrame;
}

class PowerMateFrame : public QFrame
{
    Q_OBJECT

public:
    explicit PowerMateFrame(QWidget *parent = nullptr);
    ~PowerMateFrame();
private:
    Ui::PowerMateFrame *ui;

#endif
private slots:
    void on_enabledCb_clicked();
};

#endif // POWERMATEFRAME_H
