///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur RadRotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef CLUSTERCLIENTFILTERDIALOG_H
#define CLUSTERCLIENTFILTERDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QVector>
#include <QCheckBox>
#include <QStringListModel>
#include <QListWidget>

#include "LoggerContest.h"
#include "clustercommon.h"

namespace Ui {
class ClusterClientFilterDialog;
}

class ClusterClientFilterDialog : public QDialog
{
    Q_OBJECT

public:

    explicit ClusterClientFilterDialog(BaseContestLog *c, ClusterClientFilterSettings &filterSettings_, QWidget *parent);

    ~ClusterClientFilterDialog();

    static int mainTabIndex;
    static int distanceTabIndex;

    bool checkBandMatch(int bandNum);
    bool checkModeMatch(int bandNum);


    void copyCallsignFilterListToListWidget();
    void copyLocatorFilterListToListWidget();

    //void setTabCurrentIndex(int i);
    //int getTabCurrentIndex();

    QStringList getCallsignFilterList();
    QStringList getLocatorFilterList();

    ClusterClientFilterSettings getFilterSettings(){return filterSettings;}
    //void setContest(BaseContestLog *c);
    //ClusterClientFilterSettings filterSettings;

    void setBandFilter(int band);
    bool getEnableHFSpotsFlag();

    void saveClusterFilterToContest();
    void setModeFilter(bool state, int mode);

    bool getBandFilterChangedFlag(){return bandfilterChanged;}
    bool getModeFilterChangedFlag(){return modefilterChanged;}
    bool getCallsignFilerChangedFlag(){return callsignfilterChanged;}
    bool getLocatorFilterChangedFlag(){return locatorfilterChanged;}
    bool getDistanceFilterChangedFlag(){return distancefilterChanged;}
    bool getSettingsChangedFlag(){return settingsChanged;}
    bool getIgnoreDistChangedFlag(){return ignoreDistChanged;}
    bool getIgnoreEmptyDistChangedFlag(){return ignoreEmptyDistChanged;}


signals:
    //void filtersChanged(bool, bool, bool, bool);


private:
    Ui::ClusterClientFilterDialog *ui;
    LoggerContestLog *ct = nullptr;
    ClusterClientFilterSettings filterSettings;

    struct distValue{
        int distance;
        bool distChanged = false;
        bool ignoreDistance = false;
        bool ignoreDistChanged = false;
        bool ignoreEmptyDistance = false;
        bool ignoreEmptyDistanceChanged = false;
    };

    QList<distValue>  hfDistanceValues;
    QList<distValue> vhfDistanceValues;
    QList<distValue> mwDistanceValues;


    QList<QCheckBox*> allBandChkBoxList;
    QList<QCheckBox*> hfBandChkBoxList;
    QList<QCheckBox*> vhfBandChkBoxList;
    QList<QCheckBox*> mwBandChkBoxList;

    QList<QCheckBox*> modeChkBoxList;


    QList<QLineEdit*> allDistanceLineEditsList;
    QList<QCheckBox*> allIgnoreDistanceChkBoxList;
    QList<QCheckBox*> allIgnoreEmptyDistanceChkBoxList;
    QList<QLabel*> allDistanceLabelsList;


    QList<QLineEdit*> hfDistanceLineEditsList;
    QList<QCheckBox*> hfIgnoreDistanceChkBoxList;
    QList<QCheckBox*> hfIgnoreEmptyDistanceChkBoxList;
    QList<QLabel*> hfDistanceLabelsList;


    QList<QLineEdit*> vhfDistanceLineEditsList;
    QList<QCheckBox*> vhfIgnoreDistanceChkBoxList;
    QList<QCheckBox*> vhfIgnoreEmptyDistanceChkBoxList;
    QList<QLabel*> vhfDistanceLabelsList;


    QList<QLineEdit*> mwDistanceLineEditsList;
    QList<QCheckBox*> mwIgnoreDistanceChkBoxList;
    QList<QCheckBox*> mwIgnoreEmptyDistanceChkBoxList;
    QList<QLabel*> mwDistanceLabelsList;


    QListWidget* callsignListWidget;
    int callsignListWidgetCurrentRow;


    QListWidget* locatorListWidget;
    int locatorListWidgetCurrentRow;


    bool vhfButtonState;
    bool mWaveButtonState;
    bool modeButtonState;

    QString contestUuid;

    bool bandfilterChanged = false;
    bool modefilterChanged = false;
    bool callsignfilterChanged = false;
    bool locatorfilterChanged = false;
    bool distancefilterChanged = false;
    bool ignoreDistChanged = false;
    bool ignoreEmptyDistChanged = false;
    bool settingsChanged = false;


//    bool enableHFSpots;

    void initCheckFilterTab();
    void clearVHFBands();
    void setVHFBands();
    void restoreBands();
    void clearMWaveBands();
    void setMWaveBands();
    void restoreMWBands();
    void clearModes();
    void setModes();
    void restoreModes();
    void clearAllFilters();





    void closeEvent(QCloseEvent *event);

    bool searchItem(QString text, QListWidget *listWidget);



    void saveFilterToFile(QStringList listOfFilters, QString type);
    void importFilterToWidgetList(QStringList &listOfFilters, QString type);
    QStringList getItemsTextFromListWidget(QListWidget *lw);


    bool bandFiltersChanged();
    bool modeFiltersChanged();
    bool callsignFiltersChanged();
    bool locatorFiltersChanged();

    void copyBandFiltersToFilterSettings();
    void copyBandFiltersToDialog();
    void copyModeFiltersToFilterSettings();


    void doCloseEvent();
    void enableDistanceFields();
    void setDefaultDistValues(int start, int end, bool status);


    void setFilterTabCurrentIndex(int idx);
    int getFilterTabCurrentIndex();
    void setDistanceFilterTabCurrentIndex(int idx);
    int getDistanceFilterTabCurrentIndex();

    void setHFVisible(bool state);
    bool hfDistanceValuesChanged();
    bool vhfDistanceValuesChanged();
    bool mwDistanceValuesChanged();
    bool hfIgnoreDistanceChanged();
    bool vhfIgnoreDistanceChanged();
    bool mwIgnoreDistanceChanged();
    bool hfIgnoreEmptyDistanceChanged();
    bool vhfIgnoreEmptyDistanceChanged();
    bool mwIgnoreEmptyDistanceChanged();
private slots:
    //void bandChecked(int checkBoxNum);
    //void modeChecked(int checkBoxNum);

    void vhfButtonSelected();
    void mWaveButtonSelected();
    void modeButtonSelected();
    void clearAllButtonSelected();


    void filtersAccepted();
    void filtersRejected();


    void callsignAddClicked();
    void callsignDelClicked();
    //void callsignCurrentRowChanged(int currentRow);
    //void locatorCurrentRowChanged(int currentRow);
    void callsignEditClicked();
    void locatorAddClicked();
    void locatorEditClicked();
    void locatorDelClicked();
    void onCallsignListSave();
    void onCallsignListImport();
    void onLocatorListSave();
    void onLocatorListImport();
    void callsignDelAllClicked();
    void locatorDelAllClicked();

    void onVhfClearAllIgnorePbClicked();


    void onVhfSetAllIgnorePbClicked();

    void onHfSetDefDistPbClicked();
    void onVhfSetDefDistPbClicked();
    void onMwSetDefDistPbClicked();

    void onIgnoreDistanceChecked(int);
    void onDistanceEditingFinished(int);
    void onIgnoreEmptyDistanceChecked(int idx);

    void onVhfSetAllEmptyPbClicked();
    void onUhfSetAllEmptyPbClicked();

    void onVhfClearAllEmptyDistPbClicked();

    void onBandChkBoxChecked(int idx);
    void onFilterTabIndexChanged(int idx);
    void onDistanceFilterTabIndexChanged(int idx);


    void onMwSetAllIgnorePbClicked();
    void onHfSetAllIgnorePbClicked();
    void onMwClearAllIgnorePbClicked();
    void onHfClearAllIgnorePbClicked();
    void onMwSetAllEmptyPbClicked();
    void onHfSetAllEmptyPbClicked();
    void onMwClearAllEmptyDistPbClicked();
    void onHfClearAllEmptyDistPbClicked();
};

#endif // CLUSTERCLIENTFILTERDIALOG_H
