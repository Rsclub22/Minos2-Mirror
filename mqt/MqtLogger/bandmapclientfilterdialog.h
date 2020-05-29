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
#include "LoggerContest.h"
#include "clustercommon.h"


namespace Ui {
class BandmapClientFilterDialog;
}

class BandmapClientFilterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BandmapClientFilterDialog(BaseContestLog *c, BandmapClientFilterSettings filterSettings, QWidget *parent = nullptr);
    ~BandmapClientFilterDialog();

    bool checkBandMatch(int bandNum);
    bool checkModeMatch(int bandNum);

    void copyModeFiltersToDialog();
    void setContest(BaseContestLog *c);

    void saveBandmapFilterToContest();

    bool getSettingsChangedFlag(){return settingsChanged;}
    BandmapClientFilterSettings getFilterSettings(){return filterSettings;}

    void setModeFilter(bool state, int mode);
    void loadDistanceFilterEditBox();
    void loadIgnoreDistanceChkBoxState();
    void loadIgnoreEmptyDistanceValuesChkBoxState();

signals:
    //void filtersChanged(bool);


private:
    Ui::BandmapClientFilterDialog *ui;
    LoggerContestLog *ct = nullptr;

    QList<QCheckBox*> modeChkBoxList;

    bool modeButtonState;
    bool distanceChanged;
    bool distanceChkBoxChanged;
    bool distanceEmptyChkBoxChanged;
    bool settingsChanged;

    QString contestUuid;
    QString distanceIniName;

    BandmapClientFilterSettings filterSettings;

    void initCheckFilterTab();

    void clearModes();
    void setModes();
    void restoreModes();

    void closeEvent(QCloseEvent *event);

    bool modeFiltersChanged();
    void copyModeFiltersToFilterSettings();

    void doCloseEvent();


    void getCurrentBandDistanceIniName();

    void loadSettingsToDialogBox();
private slots:

    void modeButtonSelected();

    void filtersAccepted();
    void filtersRejected();


    void onDistanceEditFinished();
    void onIgnoreDistanceChkBoxStateChanged(int state);
    void onIgnoreEmptyDistanceValuesChkBoxStateChanged(int state);
};

#endif // BANDMAPCLIENTFILTERDIALOG_H
