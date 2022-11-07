#ifndef TFORCELOGDLG_H
#define TFORCELOGDLG_H

#include "validators.h"
#include <QDialog>

namespace Ui {
class TForceLogDlg;
}
class BaseContestLog;
class TForceLogDlg : public QDialog
{
    Q_OBJECT

public:
    explicit TForceLogDlg(QWidget *parent);
    ~TForceLogDlg();

    int doexec(BaseContestLog *contest, ScreenContact &screenContact, ErrorList &errs);

private slots:
    void on_OKButton_clicked();

    void on_LocCalcButton_clicked();

    void on_CancelButton_clicked();

private:
    Ui::TForceLogDlg *ui;
};

#endif // TFORCELOGDLG_H
