#ifndef TSETTINGSEDITDLG_H
#define TSETTINGSEDITDLG_H

#include "base_pch.h"

namespace Ui {
class TSettingsEditDlg;
}

class SettingsBundle;
class TSettingsEditDlg : public QDialog
{
    Q_OBJECT

    Ui::TSettingsEditDlg *ui;

    SettingsBundle *bundle;
    bool currSectionOnly;
    QString initialSection;

    QString baseTitle;
    void showSections(QString currSection);
    void showSection();
    void showDetails();
    void getDetails();
    void doCloseEvent();
public:
    explicit TSettingsEditDlg(QWidget *parent , SettingsBundle *bundle);
    void ShowCurrentSectionOnly();
    ~TSettingsEditDlg() override;

    int exec() override;

private slots:
    void on_NewSectionButton_clicked();

    void on_CopyButton_clicked();

    void on_DeleteButton_clicked();

    void on_CancelButton_clicked();

    void on_OKButton_clicked();

    void on_SectionsList_itemSelectionChanged();
    void on_renameButton_clicked();

    void on_splitter_splitterMoved(int pos, int index);

public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;

};
#endif // TSETTINGSEDITDLG_H
