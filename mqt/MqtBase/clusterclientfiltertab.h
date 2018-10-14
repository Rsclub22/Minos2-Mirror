#ifndef CLUSTERCLIENTFILTERTAB_H
#define CLUSTERCLIENTFILTERTAB_H

#include <QTabWidget>
#include <QVector>
#include <QCheckBox>
#include "clustercommon.h"



namespace Ui {
class ClusterClientFilterTab;
}

const unsigned int vhfBandMasks[] = {_50M, _70M, _144M, _432M};
const unsigned int mWaveBandMasks[] = {_1296M, _2300M, _3_4G, _5_6G, _10G};
const unsigned int modeMasks[] = {CWMODE, PHONEMODE, RTTYMODE, PSKMODE, MGMMODE};

class ClusterClientFilterTab : public QTabWidget
{
    Q_OBJECT

public:
    explicit ClusterClientFilterTab(QWidget *parent = nullptr);
    ~ClusterClientFilterTab();

    unsigned int getBandFilterMask();
    unsigned int getModeFilterMask();

private:
    Ui::ClusterClientFilterTab *ui;
    QList<QCheckBox*> vhfChkBoxList;
    QList<QCheckBox*> mWaveChkBoxList;
    QList<QCheckBox*> modeChkBoxList;

    unsigned int bandFilterMask;
    unsigned int modeFilterMask;

    bool vhfButtonState;
    bool mWaveButtonState;
    bool modeButtonState;

    void initCheckFilterTab();
    void clearVHFBands();
    void setVHFBands();
    void clearMWaveBands();
    void setMWaveBands();
    void clearModes();
    void setModes();
    void clearAllFilters();


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
