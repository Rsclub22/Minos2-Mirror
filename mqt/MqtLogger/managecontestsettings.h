#ifndef MANAGECONTESTSETTINGS_H
#define MANAGECONTESTSETTINGS_H

#include <QDialog>

#include "PubSubName.h"
#include "qcoreapplication.h"

class ContestDetails;

enum eCSettings { ecsStation, ecsEntry, ecsQTH, ecsSection, ecsRadio, ecsRotator,
       ecsMainOp, ecsSecondOp, ecsScreenLayout,
       ecsLogSet, ecsStartApps,
       ecsMaxVal
} ;

class ContestSettings
{
   Q_DECLARE_TR_FUNCTIONS(ContestSettings)

public:
    QString contestName;
    QString stationBundle;
    QString entryBundle;
    QString QTHBundle;

    QString section;

    PubSubName radio;
    PubSubName rotator;

    QString mainOp;
    QString secondOp;

    QString screenLayout;

    QString logSet;
    QString appSet;

    QStringList getHeaders() const;
    QStringList getValues() const;
    bool operator <(const ContestSettings &) const;

    QString getVal(eCSettings s) const;
    void setVal(eCSettings s, QString val);
private:
    QString headerName(eCSettings s) const;
};
namespace Ui {
class ManageContestSettings;
}
class SettingsBundle;
class ManageContestSettings : public QDialog
{
    Q_OBJECT

    Ui::ManageContestSettings *ui;
    ContestDetails *parentDetails = nullptr;
    ContestSettings *settings = nullptr;
    QString geoString = QString("ContestSettings");
    bool supressSelect = false;

    void showSettings();
    void showSetting();
    void showDetails();
    void getDetails();
    void doCloseEvent();
public:
    explicit ManageContestSettings(ContestDetails *parent , QString cv);
    void ShowCurrentSectionOnly();
    ~ManageContestSettings() override;

    static QMap<QString, ContestSettings> allSettings;
    static const QString defaultContestSettings;
    static QStringList getSettingsList();
    static void getAllSettings();
    static void saveAllSettings();

    int exec() override;
    QString currentValue;

    static ContestSettings * getCurrentSettings(QString &cname);
    static void getSettings(QString cname, ContestDetails *parentDetails);
    bool getNewName(QString &Value);
    int save();
    int edit();
private slots:
    void on_CancelButton_clicked();

    void on_OKButton_clicked();

    void on_SettingsList_itemSelectionChanged();

    void on_settingsSplitter_splitterMoved(int pos, int index);

    void on_deleteButton_clicked();

    void on_renameButton_clicked();

public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;

};
#endif // MANAGECONTESTSETTINGS_H
