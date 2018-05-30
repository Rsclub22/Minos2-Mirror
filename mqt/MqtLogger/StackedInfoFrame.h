#ifndef STACKEDINFOFRAME_H
#define STACKEDINFOFRAME_H

#include "base_pch.h"
#include "MinosLoggerEvents.h"

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

    int splitterHandleWidth;
    LoggerContestLog *contest;
    bool filterClickEnabled;

    void initFilters();
    void saveFilters();
    void filtersChanged();
private slots:
    void on_ScrollToDistrict( const QString &qth, BaseContestLog* );
    void on_ScrollToCountry( const QString &csCs, BaseContestLog* );

    void on_WorkedCB_clicked();
    void on_UnworkedCB_clicked();
    void on_ContEU_clicked();
    void on_ContOC_clicked();
    void on_ContAS_clicked();
    void on_ContSA_clicked();
    void on_ContAF_clicked();
    void on_ContNA_clicked();

    void on_StackedMults_currentChanged(int arg1);
    void on_FontChanged();
    void on_infoCombo_currentIndexChanged(int arg1);

    void onFiltersChanged(BaseContestLog *ct);
    void onUpdateStats(BaseContestLog *ct);
    void onUpdateMemories(BaseContestLog *ct);
    void onRefreshMults(BaseContestLog *ct);

public slots:
    void setContest(LoggerContestLog *contest);
    void refreshMults(LoggerContestLog *contest);
};

#endif // STACKEDINFOFRAME_H
