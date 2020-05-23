#include <QSignalMapper>
#include "ContestApp.h"

#include "base_pch.h"
#include "Clusterbandmapconfigure.h"
#include "ui_Clusterbandmapconfigure.h"

ClusterBandmapConfigure::ClusterBandmapConfigure(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ClusterBandmapConfigure)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Cluster/Bandmap Filter Configure"));

    QSettings settings;
    QByteArray geometry = settings.value("ClusterBandmpaConfigure/geometry/").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);


     distanceLineEdits << ui->distanceFilter1_8MHz << ui->distanceFilter3_5MHz << ui->distanceFilter7MHz
                       << ui->distanceFilter14MHz << ui->distanceFilter21MHz << ui->distanceFilter28MHz
                       << ui->distanceFilter50MHz << ui->distanceFilter70MHz << ui->distanceFilter144MHz
                       << ui->distanceFilter432MHz << ui->distanceFilter1296MHz << ui->distanceFilter2300MHz
                       << ui->distanceFilter3_4GHz << ui->distanceFilter5_6GHz << ui->distanceFilter10GHz;

     QSettings config(CLUSTER_FILTER_FILE, QSettings::IniFormat);
     config.beginGroup("distanceFilter");


     for (int i = 0; i < distanceLineEdits.count(); i++)
     {
         distValue distItem;
         distItem.distance = config.value(distanceIniNames[i], DEFAULT_FILTER_DISTANCE).toInt();
         distanceLineEdits[i]->setText(QString::number(distItem.distance));
         distItem.changed = false;
         distanceValues.append(distItem);
     }

     config.endGroup();

     QSignalMapper *signalMapper = new QSignalMapper(this);
     connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(onDistanceEditingFinished(int)));

     for (int i = 0; i < distanceLineEdits.count(); i++)
     {
         signalMapper->setMapping(distanceLineEdits[i], i);
         connect(distanceLineEdits[i], SIGNAL(editingFinished()), signalMapper, SLOT(map()));
     }

     connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onAccepted()));
     connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(onRejected()));

     bool allowHF = false;
     TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpAllowHF, allowHF );


      ui->hf_frame->setVisible(allowHF);



}

ClusterBandmapConfigure::~ClusterBandmapConfigure()
{
    delete ui;
}


void ClusterBandmapConfigure::onDistanceEditingFinished(int idx)
{
    int distance = 0;
    bool ok = false;
    if (!distanceLineEdits[idx]->text().isEmpty())
    {
        distance = distanceLineEdits[idx]->text().toInt(&ok);
        if (!ok || distance < MIN_FILTER_DISTANCE || distance > MAX_FILTER_DISTANCE)
        {
            QMessageBox messageBox;
            QString msg = tr("Distance Error - %1. Please enter a distance %2 to max %3").arg( distanceLineEdits[idx]->text().arg(MIN_FILTER_DISTANCE).arg(MAX_FILTER_DISTANCE));
            messageBox.critical(this, tr("Distance Entry Error"), msg);
            return;
        }
        else
        {
            distanceValues[idx].distance = distance;
            distanceValues[idx].changed = true;
        }
    }
}

void ClusterBandmapConfigure::onAccepted()
{

    saveDistances();
}

void ClusterBandmapConfigure::onRejected()
{
    doClose();
}

void ClusterBandmapConfigure::saveDistances()
{


    QSettings config(CLUSTER_FILTER_FILE, QSettings::IniFormat);
    config.beginGroup("distanceFilter");

    for (int i = 0; i < distanceLineEdits.count(); i++)
    {
        if (distanceValues[i].changed && distanceValues[i].distance != DEFAULT_FILTER_DISTANCE)
        {
            config.setValue(distanceIniNames[i], distanceValues[i].distance);
        }
    }

    config.endGroup();

}


void ClusterBandmapConfigure::doClose()
{
    QSettings settings;
    settings.setValue("ClusterBandmapConfigure/geometry/", saveGeometry());

    close();
}
