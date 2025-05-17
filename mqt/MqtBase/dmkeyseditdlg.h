#ifndef DMKEYSEDITDLG_H
#define DMKEYSEDITDLG_H

#include <QDialog>
#include <QItemSelection>
#include "dmFKeydef.h"

namespace Ui {
class DMKeysEditDlg;
}


class DMKeysEditDlg : public QDialog
{
    Q_OBJECT

    Ui::DMKeysEditDlg *ui;

    Keys &keys;

    QString name;
    void showSections();
    void showSection();
    void showDetails();
    void getDetails();
    void doCloseEvent();
public:
    explicit DMKeysEditDlg(QWidget *parent , QString fKeyFileName, QString name, Keys &keys);
    ~DMKeysEditDlg() override;

    int exec() override;

private slots:
    void on_NewSectionButton_clicked();

    void on_CopyButton_clicked();

    void on_DeleteButton_clicked();

    void on_CancelButton_clicked();

    void on_OKButton_clicked();

    void on_SectionsList_itemSelectionChanged();
    void on_renameButton_clicked();

    void on_settingsSplitter_splitterMoved(int pos, int index);

    void on_upButton_clicked();

    void on_downButton_clicked();

    void onOptionsTableSelectionChanged(const QItemSelection &selected, const QItemSelection &);
public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;

};
#endif // DMKEYSEDITDLG_H
