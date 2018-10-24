#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include <QSettings>




class QDialogButtonBox;
class QPushButton;
class QTableView;
class StandardTableModel;


namespace Ui {
class SetupDialog;
}

const QString CLUSTER_SETTINGS_FILE = "./Configuration/Cluster/ClusterSettings.ini";
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

signals:
    void newTTLValue(QString);

public slots:
    QString getCurrentNodeName();
private slots:


    void addClusterNode();
    void deleteClusterNode();
    void timeToliveEditFinished();

    void saveGeneralSettings();
    void readGeneralSettings();
    void loadGeneralToSetupTab();

    void clusterListDataChanged(QStandardItem *);

    void callsignEditFinished();
    void nameEditFinshed();
    void qthEditFinished();
    void locatorEditFinished();
    void saveButtonPushed();

    void cancelButtonPushed();
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

    bool personelDataChanged;

    void savePersonel();
    void readPersonel();

    QString callsign;
    QString name;
    QString locator;
    QString qth;





    void loadListClusterToView();
    void loadClusterListToModel();
    void initClusterListModel();
    void loadPersonelToSetupTab();
    void clusterListSave();
    void loadSettingsToModel(QStringList &availNodeNames, QSettings &settings);
};

#endif // SETUPDIALOG_H
