#ifndef CLUSTERBANDMAPCONFIGURE_H
#define CLUSTERBANDMAPCONFIGURE_H

#include <QDialog>
#include "clustercommon.h"

namespace Ui {
class ClusterBandmapConfigure;
}



class ClusterBandmapConfigure : public QDialog
{
    Q_OBJECT

public:
    explicit ClusterBandmapConfigure(QWidget *parent = nullptr);
    ~ClusterBandmapConfigure();

private slots:
    void onDistanceEditingFinished(QLineEdit *distLineEdit);
    void onAccepted();
    void onRejected();
    void onFreqToleranceValueChanged(int value);
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


    void doClose();
    void saveDistances();
    QString findBandKey(QLineEdit *distLineEdit);
};

#endif // CLUSTERBANDMAPCONFIGURE_H
