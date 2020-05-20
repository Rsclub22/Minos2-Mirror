#include "Clusterbandmapconfigure.h"
#include "ui_Clusterbandmapconfigure.h"

ClusterBandmapConfigure::ClusterBandmapConfigure(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ClusterBandmapConfigure)
{
    ui->setupUi(this);
}

ClusterBandmapConfigure::~ClusterBandmapConfigure()
{
    delete ui;
}
