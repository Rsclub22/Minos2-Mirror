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
    ~TManageListsDlg() override;

private slots:
    void on_CloseListButton_clicked();

    void on_ExitButton_clicked();

    void on_openListButton_clicked();

    void on_moveUpButton_clicked();

    void on_moveDownButton_clicked();

    void on_ListsListBox_itemSelectionChanged();

public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;

private:
    Ui::TManageListsDlg *ui;
    void DrawList();
    void enableActions();
    void doCloseEvent();

};

#endif // TMANAGELISTSDLG_H
