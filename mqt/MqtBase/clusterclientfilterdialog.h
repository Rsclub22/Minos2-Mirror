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



    void setTabCurrentIndex(int i);
    int getTabCurrentIndex();

signals:
    void filtersChanged();


private:
    Ui::ClusterClientFilterDialog *ui;
    QList<QCheckBox*> vhfChkBoxList;
    QList<QCheckBox*> mWaveChkBoxList;
    QList<QCheckBox*> modeChkBoxList;

    QStringList callsignFilterList;
    QStringList callsignFilterListTemp;
    QStringList locatorFilterList;
    QStringList locatorFilterListTemp;

    unsigned int bandFilterMask;
    unsigned int editBandFilterMask;
    unsigned int modeFilterMask;
    unsigned int editModeFilterMask;

    bool vhfButtonState;
    bool mWaveButtonState;
    bool modeButtonState;


    bool filterTabChanged;
    bool callsignEditChanged;
    bool locatorEditChanged;

    void initCheckFilterTab();
    void clearVHFBands();
    void setVHFBands();
    void clearMWaveBands();
    void setMWaveBands();
    void clearModes();
    void setModes();
    void clearAllFilters();


    void loadBandSettings(unsigned int bandMask);
    void loadModeSettings(unsigned int modeMask);
    void restoreTabSettings();

    void closeEvent(QCloseEvent *event);

private slots:
    void vhfChecked(int checkBoxNum);
    void mWaveChecked(int checkBoxNum);
    void modeChecked(int checkBoxNum);

    void vhfButtonSelected();
    void mWaveButtonSelected();
    void modeButtonSelected();
    void clearAllButtonSelected();


    void filtersAccepted();
    void filtersRejected();

    void callsignEditFinished();
    void locatorEditFinished();
};

#endif // CLUSTERCLIENTFILTERDIALOG_H
