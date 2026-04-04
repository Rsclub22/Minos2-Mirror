#ifndef TSTATSDISPFRAME_H
#define TSTATSDISPFRAME_H

#include "minospanel.h"
#include <QFrame>
#include <QDateTime>

class LoggerContestLog;

namespace Ui {
class TStatsDispFrame;
}

class TStatsDispFrame : public MinosPanel
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
