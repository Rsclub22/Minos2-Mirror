#ifndef DMKEYSEDITDLG_H
#define DMKEYSEDITDLG_H

#include <QDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include "dmFKeydef.h"
#include "PubSubName.h"
#include "txKeyerFactory.h"
#include "txKeyerbase.h"



namespace Ui {
class DMKeysEditDlg;
}




class DMKeysEditDlg : public QDialog
{
    Q_OBJECT

    Ui::DMKeysEditDlg *ui;

    //Keys &keys;
    KeyerMap &allKeyConfigs;

    QString minosSelectedContestName;
    QString selectedContestName;
    PubSubName minosSelectedRadio;
    QString minosSelectedRadioLocalName;
    QString radioListSelectedName;
    QString txKeyerType;

    void showSections();
    void showSection();
    void showDetails();
    void getDetails();
    void doCloseEvent();

public:
    explicit DMKeysEditDlg(QWidget *parent, QString fKeyFileName, QString minosSelectedContestName_, KeyerMap &allKeyConfigs, TxKeyerFactory *txKeyerFactory_,
                           QSharedPointer<TxKeyerBase> txKeyer_, QString txKeyerType, PubSubName minosSelectedRadio_, const QMap<QString, QString> &radioMap_, const QStringList &listOfRadios_);
    ~DMKeysEditDlg() override;

    int exec() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_NewSectionButton_clicked();

    void on_CopyButton_clicked();

    void on_DeleteButton_clicked();

    void on_CancelButton_clicked();

    void on_OKButton_clicked();

    void on_SectionsList_itemSelectionChanged();

    void onRadioListItemSelectionChanged();

    void on_renameButton_clicked();

    void on_settingsSplitter_splitterMoved(int pos, int index);

    void on_upButton_clicked();

    void on_downButton_clicked();

    void onOptionsTableSelectionChanged(const QItemSelection &selected, const QItemSelection &);
    void on_addRadioButton_clicked();
    void on_deleteRadioButton_clicked();
    void onTxKeyerCommonConfigPbClicked();


public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;

private:
    void saveCurrentSection();
    bool isCurrentSectionDirty() const;
    void clearDirtyFlag();
    //QString getRigKey() const;

    TxKeyerFactory *txKeyerFactory = nullptr;
    QSharedPointer<TxKeyerBase> txKeyer;

    bool ignoreSectionChange = false;
    QListWidget* RadioList = nullptr;
    const QMap<QString, QString> &radioMap;
    const QStringList &listOfRadios;


    void showRadiosForSection();
    void showRadioListButtons(bool show);

    bool checkRadioExists(QString contestName, QString radioName, bool &radioExists);

    void setupTableRow(int row, KeyVal &k);
    void setupTableColumns();


};
#endif // DMKEYSEDITDLG_H
