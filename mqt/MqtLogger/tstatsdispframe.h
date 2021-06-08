#ifndef TSTATSDISPFRAME_H
#define TSTATSDISPFRAME_H

#include "base_pch.h"

class LoggerContestLog;

namespace Ui {
class TStatsDispFrame;
}

class TStatsDispFrame : public QFrame
{
    Q_OBJECT

public:
    explicit TStatsDispFrame(QWidget *parent = nullptr);
    ~TStatsDispFrame();

    void reInitialiseStats();
    void setContest(LoggerContestLog *ct );
    void setBand(QString band);

private slots:
    void on_P1Edit_valueChanged(int arg1);
    void on_P2Edit_valueChanged(int arg1);
    void RecheckTimerTimer( );

private:
    Ui::TStatsDispFrame *ui;
    LoggerContestLog *ct;
    QDateTime lastCheckTime;
    bool suppressPeriodChange;
};

#endif // TSTATSDISPFRAME_H
