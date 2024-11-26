///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur RadRotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
//
//
/////////////////////////////////////////////////////////////////////////////


#ifndef BANDMAPCLIENTFILTERDIALOG_H
#define BANDMAPCLIENTFILTERDIALOG_H

#include <QDialog>
#include "clustercommon.h"


namespace Ui {
class BandmapClientFilterDialog;
}

class BandmapClientFilterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BandmapClientFilterDialog(BandmapClientFilterSettings &filterSettings, const QString &title, QWidget *parent);
    ~BandmapClientFilterDialog();

    static int mainTabIndex;

    void copyModeFiltersToDialog();

    bool getSettingsChangedFlag(){return settingsChanged;}
    BandmapClientFilterSettings getFilterSettings(){return filterSettings;}

    void loadDistanceFilterEditBox();
    void loadIgnoreDistanceChkBoxState();
    void loadIgnoreEmptyDistanceValuesChkBoxState();

private:
    Ui::BandmapClientFilterDialog *ui;

    QList<QCheckBox*> modeChkBoxList;
    QMap<QString, QCheckBox*> modeCheckBoxes;


    bool distanceChanged;
    bool distanceChkBoxChanged;
    bool distanceEmptyChkBoxChanged;
    bool settingsChanged;

    BandmapClientFilterSettings filterSettings;

    void initCheckFilterTab(const QString &t);

    void clearModes();
    void setModes();
    void restoreModes();

    void closeEvent(QCloseEvent *event);

    bool modeFiltersChanged();
    void copyModeFiltersToFilterSettings();

    void doCloseEvent();

    void loadSettingsToDialogBox();
    bool areAnyModesSet();
private slots:

    void modeButtonSelected();

    void filtersAccepted();
    void filtersRejected();


    void onDistanceEditFinished();
    void onIgnoreDistanceChkBoxStateChanged(int state);
    void onIgnoreEmptyDistanceValuesChkBoxStateChanged(int state);
    void setFilterTabCurrentIndex(int idx);
    int getFilterTabCurrentIndex();
    void onFilterTabIndexChanged(int idx);
};

#endif // BANDMAPCLIENTFILTERDIALOG_H
