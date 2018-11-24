/////////////////////////////////////////////////////////////////////////////
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

#include "clustercommon.h"

namespace Ui {
class ClusterClientFilterDialog;
}

class ClusterClientFilterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ClusterClientFilterDialog(QWidget *parent = nullptr);
    ~ClusterClientFilterDialog();

    unsigned int getBandFilterMask();
    unsigned int getModeFilterMask();

    void copyBandFilterMaskToEdit();
    void copyModeFilterMaskToEdit();
    void copyCallsignFilterListToListWidget();
    void copyLocatorFilterListToListWidget();

    void setTabCurrentIndex(int i);
    int getTabCurrentIndex();

    QStringList getCallsignFilterList();
    QStringList getLocatorFilterList();


    void setContest(BaseContestLog *c);

signals:
    void filtersChanged(int);


private:
    Ui::ClusterClientFilterDialog *ui;
    BaseContestLog *ct = nullptr;


    QList<QCheckBox*> bandChkBoxList;
    //QList<QCheckBox*> mWaveChkBoxList;
    QList<QCheckBox*> modeChkBoxList;


    //QStringList callsignFilterList;
    QListWidget* callsignListWidget;
    int callsignListWidgetCurrentRow;


    //QStringList locatorFilterList;
    QListWidget* locatorListWidget;
    int locatorListWidgetCurrentRow;

    //unsigned int bandFilterMask;
    QList<bool> editBandFilter;

    //unsigned int editBandFilterMask;
    //unsigned int modeFilterMask;
    //unsigned int editModeFilterMask;
    QList<bool> editModeFilter;

    ClusterClientFilterSettings filterSettings;

    bool vhfButtonState;
    bool mWaveButtonState;
    bool modeButtonState;


    bool bandTabChanged;
    bool modeChanged;
    bool callsignEditChanged;
    bool locatorEditChanged;

    void initCheckFilterTab();
    void clearVHFBands();
    void setVHFBands();
    void restoreVHFBands();
    void clearMWaveBands();
    void setMWaveBands();
    void restoreMWBands();
    void clearModes();
    void setModes();
    void restoreModes();
    void clearAllFilters();


    void loadBandSettings(unsigned int bandMask);
    void loadModeSettings(unsigned int modeMask);
    void restoreTabSettings();

    void closeEvent(QCloseEvent *event);

    bool searchItem(QString text, QListWidget *listWidget);



    void saveFilterToFile(QStringList listOfFilters, QString type);
    void importFilterToWidgetList(QStringList &listOfFilters, QString type);
    QStringList getItemsTextFromListWidget(QListWidget *lw);


private slots:
    void bandChecked(int checkBoxNum);
    //void mWaveChecked(int checkBoxNum);
    void modeChecked(int checkBoxNum);

    void vhfButtonSelected();
    void mWaveButtonSelected();
    void modeButtonSelected();
    void clearAllButtonSelected();


    void filtersAccepted();
    void filtersRejected();


    void callsignAddClicked();
    void callsignDelClicked();
    void callsignCurrentRowChanged(int currentRow);
    void locatorCurrentRowChanged(int currentRow);
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
};

#endif // CLUSTERCLIENTFILTERDIALOG_H
