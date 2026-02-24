#ifndef STACKEDINFOFRAME_H
#define STACKEDINFOFRAME_H

#include "RigMemoryFrame.h"
#include "TClockFrame.h"
#include "districtframe.h"
#include "dxccframe.h"
#include "locTreeFrame.h"
#include "locframe.h"
#include "tstatsdispframe.h"
#include "auxentries.h"

namespace Ui {
class StackedInfoFrame;
}
extern bool showWorked;
extern bool showUnworked ;

class StackedInfoFrame : public QFrame
{
    Q_OBJECT

public:
    TSingleLogFrame *tslf;

    explicit StackedInfoFrame(QWidget *parent, int instance, TSingleLogFrame *t);
    ~StackedInfoFrame();

    void setCurrentFrameType(QString);

private:
    Ui::StackedInfoFrame *ui;

    int stackInstance = 0;
    LoggerContestLog *contest = nullptr;

    TClockFrame *clockFrame = nullptr;
    DXCCFrame *dxccFrame = nullptr;
    DistrictFrame *districtFrame = nullptr;
    RigMemoryFrame *rigMemFrame = nullptr;
    LocFrame *locFrame = nullptr;
    LocTreeFrame *locTreeFrame = nullptr;
    TStatsDispFrame *statsFrame = nullptr;

    QFrame *currStackFrame = nullptr;

    bool showAuxHeaders = true;

    void setTabVisibility();

    void stackMargins();
private slots:
    void on_ScrollToDistrict( const QString &qth, BaseContestLog* );
    void on_ScrollToCountry( const QString &csCs, BaseContestLog* );

    void onInfoComboCurrentIndexChanged(int arg1);

    void onFiltersChanged(BaseContestLog *ct);
    void onUpdateStats(BaseContestLog *ct);
    void onUpdateMemories(BaseContestLog *ct);
    void onRefreshStackMults(BaseContestLog *ct);

    void clearContestInFrame(BaseContestLog *ct);
    void onContestBandChanged(BaseContestLog *ct);
    void on_currentTabChangedSlot(int index);

    void on_ShowAuxHeaders();
public slots:
    void setContest(LoggerContestLog *contest);
};

#endif // STACKEDINFOFRAME_H
