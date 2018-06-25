#ifndef FILTERFRAME_H
#define FILTERFRAME_H

#include "base_pch.h"
#include "MinosLoggerEvents.h"

extern ContList contlist[ CONTINENTS ];

namespace Ui {
class FilterFrame;
}

class FilterFrame : public QFrame
{
    Q_OBJECT

public:
    explicit FilterFrame(QWidget *parent = nullptr);
    ~FilterFrame();

    void setContest(LoggerContestLog *ct);
    void initFilters();
private slots:
    void on_WorkedCB_clicked();
    void on_UnworkedCB_clicked();
    void on_ContEU_clicked();
    void on_ContOC_clicked();
    void on_ContAS_clicked();
    void on_ContSA_clicked();
    void on_ContAF_clicked();
    void on_ContNA_clicked();
private:
    Ui::FilterFrame *ui;
    LoggerContestLog *contest = nullptr;
    bool filterClickEnabled = false;

    void saveFilters();
    void filtersChanged();

};

#endif // FILTERFRAME_H
