#ifndef CLUSTERCLIENTFILTERTAB_H
#define CLUSTERCLIENTFILTERTAB_H

#include <QTabWidget>
#include <QVector>
#include <QCheckBox>
#include "clustercommon.h"



namespace Ui {
class ClusterClientFilterTab;
}


class ClusterClientFilterTab : public QTabWidget
{
    Q_OBJECT

public:
    explicit ClusterClientFilterTab(QWidget *parent = nullptr);
    ~ClusterClientFilterTab();

    unsigned int getBandFilterMask();
    unsigned int getModeFilterMask();

    void copyBandFilterMaskToEdit();
    void copyModeFilterMaskToEdit();

private:
    Ui::ClusterClientFilterTab *ui;
    QList<QCheckBox*> vhfChkBoxList;
    QList<QCheckBox*> mWaveChkBoxList;
    QList<QCheckBox*> modeChkBoxList;

    unsigned int bandFilterMask;
    unsigned int editBandFilterMask;
    unsigned int modeFilterMask;
    unsigned int editModeFilterMask;

    bool vhfButtonState;
    bool mWaveButtonState;
    bool modeButtonState;

    bool filterTabChanged;

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
};

#endif // CLUSTERCLIENTFILTERTAB_H
