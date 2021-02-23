#ifndef CLUSTERBANDMAPCONFIGURE_H
#define CLUSTERBANDMAPCONFIGURE_H

#include <QFrame>
#include "clustercommon.h"

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

private slots:
    void onDistanceEditingFinished(QLineEdit *distLineEdit);
    void onFreqToleranceValueChanged(int value);
    void onSpotLessThanDistanceRadioButClicked();
    void onSpotGreaterThanDistanceRadioButClicked();
private:
    Ui::ClusterBandmapConfigure *ui;

    struct distValue{
        QLineEdit *distLineEdit;
        int distance;
        bool changed;
    };

    QList<QLineEdit*> distanceLineEdits;
    //QList<distValue> distanceValues;

    QVector<QSharedPointer<BandInfo> > bands;
    QMap<QString, distValue> distanceValues;

    int addBandmapTuningTolerance;
    bool lessGreaterThanDistanceFlag = false;   // less than = false, greater than = true;

    void saveDistances();
    QString findBandKey(QLineEdit *distLineEdit);
};

#endif // CLUSTERBANDMAPCONFIGURE_H
