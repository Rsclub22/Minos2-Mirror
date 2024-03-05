#ifndef CLUSTERBANDMAPCONFIGURE_H
#define CLUSTERBANDMAPCONFIGURE_H

#include <QFrame>
#include "clustercommon.h"
#include "ConfigurationOption.h"

namespace Ui {
class ClusterBandmapConfigure;
}



class ClusterBandmapConfigure : public QFrame
{
    Q_OBJECT

public:
    explicit ClusterBandmapConfigure(QWidget *parent = nullptr);
    ~ClusterBandmapConfigure();

    void initialise();
    void finalise();

    bool check();
    void cancel();
private slots:
    void onDistanceEditingFinished(QLineEdit *distLineEdit);
    void onFreqToleranceValueChanged(int value);
    void onSpotLessThanDistanceRadioButClicked();
    void onSpotGreaterThanDistanceRadioButClicked();
    void on_ClusterBandmapConfiguretabWidget_currentChanged(int index);
private:
    Ui::ClusterBandmapConfigure *ui;

    ConfigurationOption tuningAddMapCW;
    ConfigurationOption tuningAddMapData;
    ConfigurationOption tuningAddMapPhone;

    ConfigurationOption tuningTolCW;
    ConfigurationOption tuningTolData;
    ConfigurationOption tuningTolPhone;

    ConfigurationOption disableNotShown;
    ConfigurationOption disableLoggedCalls;
    ConfigurationOption disablePlaceHolders;

    ConfigurationOption BandMapTurnOffOperatingFreqStrip;
    ConfigurationOption BandMapFollowRadioModeOperatingFreqStrip;
    ConfigurationOption BandMapMouseInFrameDelay;
    ConfigurationOption BandmapOldStyle;
    ConfigurationOption BandMapShowDerivedLoc;
    ConfigurationOption BandmapInvert;

    struct distValue{
        QLineEdit *distLineEdit;
        int distance;
        bool changed;
    };

    QVector<QSharedPointer<BandInfo> > bands;
    QMap<QString, distValue> distanceValues;

    bool lessGreaterThanDistanceFlag = false;   // less than = false, greater than = true;

    QVector<ConfigurationOption> bandLimits;

    void saveDistances();
    QString findBandKey(QLineEdit *distLineEdit);
};

#endif // CLUSTERBANDMAPCONFIGURE_H
