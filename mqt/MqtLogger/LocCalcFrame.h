#ifndef LOCCALCFRAME_H
#define LOCCALCFRAME_H

#include "base_pch.h"

namespace Ui {
class LocCalcFrame;
}

class LocCalcFrame : public QFrame
{
    Q_OBJECT

public:
    explicit LocCalcFrame(QWidget *parent = nullptr);
    ~LocCalcFrame() override;
    void doExec();
    void setContest(BaseContestLog *contest);

    QString S1Loc;
    QString S2Loc;
    QString Distance;

private slots:
    void on_CalcButton_clicked();

    void on_ExitButton_clicked();

    void on_CancelButton_clicked();

    void on_S1Calc_clicked();

    void on_S2Calc_clicked();

private:
    Ui::LocCalcFrame *ui;
    void doCloseEvent();
    bool handleExit( QLineEdit *Edit );

    BaseContestLog *contest;

};

#endif // LOCCALCFRAME_H
