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
    void onDistanceEditingFinished(int idx);
    void on_OKButton_clicked();
    void on_CancelButton_clicked();
private:
    Ui::ClusterBandmapConfigure *ui;

    struct distValue{
        int distance;
        bool changed;
    };

    QList<QLineEdit*> distanceLineEdits;
    QList<distValue> distanceValues;


    void doClose();
    void saveDistances();
};

#endif // CLUSTERBANDMAPCONFIGURE_H
