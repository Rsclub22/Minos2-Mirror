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
    explicit BandmapClientFilterDialog(QWidget *parent = nullptr);
    ~BandmapClientFilterDialog();

    bool checkBandMatch(int bandNum);
    bool checkModeMatch(int bandNum);

    void copyModeFiltersToDialog();
    void setContest(BaseContestLog *c);
    BandmapClientFilterSettings filterSettings;

    void saveBandmapFilterToContest();


signals:
    void filtersChanged(bool);


private:
    Ui::BandmapClientFilterDialog *ui;
    LoggerContestLog *ct = nullptr;

    QList<QCheckBox*> modeChkBoxList;

    bool modeButtonState;

    QString contestUuid;

    void initCheckFilterTab();

    void clearModes();
    void setModes();
    void restoreModes();

    void closeEvent(QCloseEvent *event);

    bool modeFiltersChanged();
    void copyModeFiltersToFilterSettings();

    void doCloseEvent();

private slots:

    void modeButtonSelected();

    void filtersAccepted();
    void filtersRejected();


};

#endif // BANDMAPCLIENTFILTERDIALOG_H
