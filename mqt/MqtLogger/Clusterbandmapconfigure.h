#ifndef CLUSTERBANDMAPCONFIGURE_H
#define CLUSTERBANDMAPCONFIGURE_H

#include <QDialog>

namespace Ui {
class ClusterBandmapConfigure;
}

class ClusterBandmapConfigure : public QDialog
{
    Q_OBJECT

public:
    explicit ClusterBandmapConfigure(QWidget *parent = nullptr);
    ~ClusterBandmapConfigure();

private:
    Ui::ClusterBandmapConfigure *ui;
};

#endif // CLUSTERBANDMAPCONFIGURE_H
