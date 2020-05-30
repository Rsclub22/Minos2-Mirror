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

    bool checkBandMatch(int bandNum);
    bool checkModeMatch(int bandNum);

    void copyBandFiltersToDialog();
    void copyModeFiltersToDialog();
    void copyCallsignFilterListToListWidget();
    void copyLocatorFilterListToListWidget();

    void setTabCurrentIndex(int i);
    int getTabCurrentIndex();

    QStringList getCallsignFilterList();
    QStringList getLocatorFilterList();

    ClusterClientFilterSettings getFilterSettings(){return filterSettings;}
    //void setContest(BaseContestLog *c);
    //ClusterClientFilterSettings filterSettings;

    void setBandFilter(int band);
    bool getEnableHFSpotsFlag();

    void saveClusterFilterToContest();
    void setModeFilter(bool state, int mode);
signals:
    void filtersChanged(bool, bool, bool, bool);


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

    QList<distValue>  distanceValues;

    QList<QCheckBox*> bandChkBoxList;
    QList<QCheckBox*> modeChkBoxList;
    QList<QLineEdit*> distanceLineEditsList;
    QList<QCheckBox*> ignoreDistanceChkBoxList;
    QList<QCheckBox*> ignoreEmptyDistanceChkBoxList;
    QList<QLabel*> distanceLabelsList;


    QListWidget* callsignListWidget;
    int callsignListWidgetCurrentRow;


    QListWidget* locatorListWidget;
    int locatorListWidgetCurrentRow;


    bool vhfButtonState;
    bool mWaveButtonState;
    bool modeButtonState;

    QString contestUuid;

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



    void restoreTabSettings();

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
    void copyModeFiltersToFilterSettings();


    void doCloseEvent();
    void enableDistanceFields();
    void setDefaultDistValues(int start, int end, bool status);
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
    void onUhfClearAllIgnorePbClicked();
    void onVhfClearAllIgnorePbClicked();

    void onUhfSetAllIgnorePbClicked();
    void onVhfSetAllIgnorePbClicked();

    void onUhfSetDefDistPbClicked();
    void onVhfSetDefDistPbClicked();

    void onIgnoreDistanceChecked(int);
    void onDistanceEditingFinished(int);
    void onIgnoreEmptyDistanceChecked(int idx);

    void onVhfSetAllEmptyPbClicked();
    void onUhfSetAllEmptyPbClicked();

    void onVhfClearAllEmptyDistPbClicked();
    void onUhfClearAllEmptyDistPbClicked();
    void onBandChkBoxChecked(int idx);
};

#endif // CLUSTERCLIENTFILTERDIALOG_H
