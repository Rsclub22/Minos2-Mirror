#ifndef STACKEDINFOFRAME_H
#define STACKEDINFOFRAME_H

#include "base_pch.h"
#include "RigMemoryFrame.h"
#include "TClockFrame.h"
#include "districtframe.h"
#include "dxccframe.h"
#include "locTreeFrame.h"
#include "locframe.h"
#include "tstatsdispframe.h"
#include "FilterFrame.h"

namespace Ui {
class StackedInfoFrame;
}
extern ContList contlist[ CONTINENTS ];
extern bool showWorked;
extern bool showUnworked ;
class StackedInfoFrame : public QFrame
{
    Q_OBJECT

public:
    explicit StackedInfoFrame(QWidget *parent = nullptr, int instance = 0);
    ~StackedInfoFrame();

private:
    Ui::StackedInfoFrame *ui;

    int stackInstance = 0;

    LoggerContestLog *contest;

    TClockFrame *clockFrame = nullptr;
    DXCCFrame *dxccFrame = nullptr;
    DistrictFrame *districtFrame = nullptr;
    FilterFrame *filterFrame = nullptr;
    RigMemoryFrame *rigMemFrame = nullptr;
    LocFrame *locFrame = nullptr;
    LocTreeFrame *locTreeFrame = nullptr;
    TStatsDispFrame *statsFrame = nullptr;

    QFrame *currStackFrame = nullptr;

private slots:
    void on_ScrollToDistrict( const QString &qth, BaseContestLog* );
    void on_ScrollToCountry( const QString &csCs, BaseContestLog* );

    void on_FontChanged();
    void on_infoCombo_currentIndexChanged(int arg1);

    void onFiltersChanged(BaseContestLog *ct);
    void onUpdateStats(BaseContestLog *ct);
    void onUpdateMemories(BaseContestLog *ct);
    void onRefreshStackMults(BaseContestLog *ct);

public slots:
    void setContest(LoggerContestLog *contest);
    //void refreshMults(LoggerContestLog *contest);
};

#endif // STACKEDINFOFRAME_H
