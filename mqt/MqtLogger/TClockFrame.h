#ifndef TCLOCKFRAME_H
#define TCLOCKFRAME_H

#include <QFrame>

class BaseContestLog;

namespace Ui {
class TClockFrame;
}

class TClockFrame : public QFrame
{
    Q_OBJECT

public:
    explicit TClockFrame(QWidget *parent = nullptr);
    ~TClockFrame();
    void setContest(BaseContestLog *contest);

private:
    Ui::TClockFrame *ui;
    BaseContestLog *contest;

private slots:
    void RecheckTimerTimer( );
};

#endif // TCLOCKFRAME_H
