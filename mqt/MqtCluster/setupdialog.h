#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include <QSettings>

#include "clustercommon.h"


class QDialogButtonBox;
class QPushButton;
class QTableView;
class StandardTableModel;


namespace Ui {
class SetupDialog;
}

const QString CLUSTER_NODE_LIST_FILE = "./Configuration/Cluster/ClusterSites.ini";

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

signals:

    void personalDataUpdated(QString, QString, QString, QString);
    void clusterListChanged();

public slots:
    QString getCurrentNodeName();

private slots:


    void addClusterNode();
    void deleteClusterNode();
    void timeToliveEditFinished();

    void saveGeneralSettings();

    void loadGeneralToSetupTab();

    void clusterListDataChanged(QStandardItem *);



    void callsignFinished(const QString&);

    void nameEditFinshed();
    void qthEditFinished();
    void locatorFinished(const QString&);
    void saveButtonPushed();

    void cancelButtonPushed();
    void runStartCmdFileChkBoxChanged(int state);
    void runEndCmdFileChkBoxChanged(int state);


private:
    Ui::SetupDialog *ui;


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

    bool runStartCmdFilesChanged;
    bool enableStartCmdFiles;
    bool runEndCmdFilesChanged;
    bool enableEndCmdFiles;

    bool personalDataChanged;

    void savePersonal();
    void readPersonal();

    QString callsign;
    QString name;
    QString locator;
    QString qth;





    void loadListClusterToView();
    void loadClusterListToModel();
    void initClusterListModel();
    void loadPersonalToSetupTab();
    void clusterListSave();
    void loadSettingsToModel(QStringList &availNodeNames, QSettings &settings);
    void closeEvent(QCloseEvent *event);
    void doCloseEvent();
};

#endif // SETUPDIALOG_H
