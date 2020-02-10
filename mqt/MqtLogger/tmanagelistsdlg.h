#ifndef TMANAGELISTSDLG_H
#define TMANAGELISTSDLG_H

#include "base_pch.h"

namespace Ui {
class TManageListsDlg;
}

class TManageListsDlg : public QDialog
{
    Q_OBJECT

public:
    explicit TManageListsDlg(QWidget *parent = nullptr);
    ~TManageListsDlg();

private slots:
    void on_CloseListButton_clicked();

    void on_ExitButton_clicked();

    void on_openListButton_clicked();

    void on_moveUpButton_clicked();

    void on_moveDownButton_clicked();

private:
    Ui::TManageListsDlg *ui;
    void DrawList();
    void enableActions();
};

#endif // TMANAGELISTSDLG_H
