#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include <QSettings>
#include "cutils.h"


class QDialogButtonBox;
class QPushButton;
class QTableView;
class StandardTableModel;


namespace Ui {
class SetupDialog;
}

const int ClusterListNumCols = 4;
const int NameColNum = 0;
const int AddressColNum = 1;
const int PortColNum = 2;
const int PasswdColNum = 3;

class SetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetupDialog(QWidget *parent = nullptr);
    ~SetupDialog();


    QString getUserName();
    QString getUserCallsign();
    QString getUserLocator();
    QString getUserQth();


    bool doesClusterNameExist(QString nodeName);
    QStringList getClusterInfo(QString clusterName);
    QStringList getListOfClusterNames();

    QStringList clusterInfo;
    QStringList clusterNameList;



    void saveCurrentNodeName(QString nodeName);
    void readGeneralSettings();
    QString getTimeToLive();

    void setTabNum(int num);

    bool getRunStartFileFlag();
    bool getRunEndFileFlag();

    bool getRemoveRepeatSpotFilterFlag(){return removeRepeatSpotFilterFlag;}
    int getRemoveRepeatSpotsFreqDelta(){return removeRepeatSpotsFreqDelta;}
    int getRemoveRepeatSpotsWithinTime(){return removeRepeatSpotsWithinTime;}

    void createDefaultGeneralSettingsFile();
    void readPersonal();
    void loadGeneralToSetupTab();
    void loadPersonalToSetupTab();

    bool getSendToDXClusterEnabled();
    //bool getBandFilterOnSaveFlag(){return bandFilterOnSaveFlag;}

    //bool getStartEndScriptOnSaveFlag(){return startEndScriptOnSaveFlag;}

signals:

    void personalDataUpdated(QString, QString, QString, QString);
    void clusterListChanged();
    void sendSpotToTxEnabled(bool);

public slots:
    QString getCurrentNodeName();

private slots:


    void addClusterNode();
    void deleteClusterNode();
    void timeToliveEditFinished();

    void saveGeneralSettings();



    void clusterListDataChanged(QStandardItem *);



    void callsignFinished(const QString&);

    void nameEditFinshed();
    void qthEditFinished();
    void locatorFinished(const QString&);
    void saveButtonPushed();

    void cancelButtonPushed();
    //void runStartCmdFileChkBoxChanged(int state);
    //void runEndCmdFileChkBoxChanged(int state);


    void sendSpotsToDXClusterChkBoxChanged(int state);
    //void onSaveBandFilterChkBoxClicked(int state);
    void onQrzCheckBoxChkBoxClicked(int state);
    void repeatSpotsCheckboxChanged();
    void repeatSpotFreqDeltaEditingFinished();
    void repeatSpotWithinTimeEditingFinished();
private:
    Ui::SetupDialog *ui;
    UpperCaseValidator ucValidator;


    QStringList availNodeNames;
    QStandardItemModel *clusterListModel;
    QTableView *clustersListView;

    QDialogButtonBox *buttonBox;

    QPushButton *clustersListSaveButton;
    QPushButton *clustersListAddButton;
    QPushButton *clustersListDeleteButton;

    int numClusterNodes;
    bool listDataChanged;
    QString timeToLive;
    bool timeToLiveChanged;

    //bool runStartCmdFilesChanged;
    //bool enableStartCmdFiles;
    //bool runEndCmdFilesChanged;
    //bool enableEndCmdFiles;

    bool sendSpotToDXCluster;
    bool sendSpotsToDXClusterChanged;

    bool personalDataChanged;

    bool removeRepeatSpotFilterFlag = false;
    bool removeRepeatSpotFilterFlagChanged = false;

    int removeRepeatSpotsFreqDelta = 200; // Hz
    bool removeRepeatSpotsFreqDeltaChanged = false;

    int removeRepeatSpotsWithinTime = 10; // mins
    bool removeRepeatSpotsWithinTimeChanged = false;

    //bool bandFilterOnSaveFlag;
    //bool bandFilterOnSaveChanged;

    //bool startEndScriptOnSaveFlag;
    //bool startEndScriptOnSaveChanged;

    bool useQrzForQraFlag;
    bool useQrzForQraChanged;

    void savePersonal();


    QString callsign;
    QString name;
    QString locator;
    QString qth;





    void loadListClusterToView();
    void loadClusterListToModel();
    void initClusterListModel();

    void clusterListSave();
    void loadSettingsToModel(QStringList &availNodeNames, QSettings &settings);
    void closeEvent(QCloseEvent *event);
    void doCloseEvent();

    void setRemoveSpotItemsDisabled(bool disabled);
};

#endif // SETUPDIALOG_H
