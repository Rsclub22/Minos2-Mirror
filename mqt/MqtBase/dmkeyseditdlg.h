#ifndef DMKEYSEDITDLG_H
#define DMKEYSEDITDLG_H

#include <QDialog>

namespace Ui {
class DMKeysEditDlg;
}

// map(key contest type) of pairs (first map(key Fkey) of run message , second map(key fKey) of S&P message)
//typedef QMap<QString, QPair<QMap<QString, QString>, QMap<QString, QString> > > Keys;

// map(key "Digi" of (map(key current set name) of vector of pairs (fkey text, fkey message) ))
// QVector is 24 entries, F1-F12, F1-F12
typedef QMap<QString, QMap<QString, QVector<QPair<QString, QString> > > > Keys;

class DMKeysEditDlg : public QDialog
{
    Q_OBJECT

    Ui::DMKeysEditDlg *ui;

    Keys &keys;
    QString initialSection;
    int lastOffset = -1;

    QString baseTitle;
    QString name;
    void showSections();
    void showSection();
    void showDetails();
    void getDetails();
    void doCloseEvent();
public:
    explicit DMKeysEditDlg(QWidget *parent , QString name, Keys &keys);
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

    void on_splitter_splitterMoved(int pos, int index);

public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;

};
#endif // DMKEYSEDITDLG_H
