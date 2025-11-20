#ifndef DMKEYSEDITDLG_H
#define DMKEYSEDITDLG_H

#include <QDialog>
#include <QWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <memory>
#include "dmFKeydef.h"
#include "PubSubName.h"
#include "txKeyerFactory.h"
#include "txKeyerbase.h"



namespace Ui {
class DMKeysEditDlg;
}

struct DMKeysEditDlgConfig
{

    DMKeysEditDlgConfig(KeyerMap &configs,
                        TxKeyerCapabilities &cap,
                        const QMap<QString, QString> &rMap,
                        const QStringList &rList)
        : allKeyConfigs(configs)
        , txKeyerCap(cap)
        , radioMap(rMap)
        , listOfRadios(rList)
    {}

    KeyerMap &allKeyConfigs;
    TxKeyerCapabilities &txKeyerCap;
    const QMap<QString, QString> &radioMap;
    const QStringList &listOfRadios;





    QString fKeyFileName;
    QString minosSelectedContestName;
    TxKeyerFactory *txKeyerFactory = nullptr;
    QSharedPointer<TxKeyerBase> txKeyer;
    QString txKeyerType;
    PubSubName minosSelectedRadio;



};


class DMKeysEditDlg : public QDialog
{
    Q_OBJECT

    Ui::DMKeysEditDlg *ui;

    //Keys &keys;
    //KeyerMap &allKeyConfigs;

    //QString minosSelectedContestName;

    //PubSubName minosSelectedRadio;
    //QString minosSelectedRadioLocalName;

    //QString txKeyerType;

    void showSections();
    void showSection();
    void showDetails();
    void getDetails(const QString selectedContestName, const QString selectedRadioListName, QString wherefrom);
    void doCloseEvent();

public:
    explicit DMKeysEditDlg(QWidget *parent, const DMKeysEditDlgConfig &config);
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
    void saveCurrentSection(const QString contestName, const QString radioName, QString wherefrom);
    bool isCurrentSectionDirty(const QString selectedContestName, const QString selectedRadioListName) const;
    void clearDirtyFlag(const QString selectedContestName, const QString selectedRadioListName);
    //QString getRigKey() const;

    const DMKeysEditDlgConfig &config;

    std::vector<std::shared_ptr<KeyVal>> m_tableKeyVals; // store pointers to
    // checkboxes and rec button


    //TxKeyerFactory *txKeyerFactory = nullptr;
    //QSharedPointer<TxKeyerBase> txKeyer;
    //TxKeyerCapabilities &txKeyerCap;

    bool ignoreSectionChange = false;
    bool ignoreRadioChange = false;

    QString selectedContestName;
    QString radioListSelectedName;
    PubSubName minosSelectedRadioName;
    QString minosSelectedRadioLocalName;

    QListWidget* RadioList = nullptr;
    //const QMap<QString, QString> &radioMap;
    //const QStringList &listOfRadios;


    void showRadiosForSection();
    void showRadioListButtons(bool show);

    bool checkRadioExists(QString contestName, QString radioName, bool &radioExists);

    void setupTableRow(int row, KeyVal &k);
    void setupTableColumns();


    void logMessage(QString msg);
    QString getCurrentSelectedContest();
    QString getCurrentSelectedRadio();
    QString getCurrentSelectedItem(QListWidget *list, const QString &defaultValue);
    void selectListItem(QListWidget *list, const QString &name);
};
#endif // DMKEYSEDITDLG_H
