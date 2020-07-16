#include "ContestApp.h"
#include "bandmapcommon.h"

#include "base_pch.h"
#include "Clusterbandmapconfigure.h"
#include "ui_Clusterbandmapconfigure.h"

ClusterBandmapConfigure::ClusterBandmapConfigure(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ClusterBandmapConfigure)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Cluster/Bandmap Configure"));

    QSettings settings;
    QByteArray geometry = settings.value("ClusterBandmpaConfigure/geometry/").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);


     distanceLineEdits /*<< ui->distanceFilter1_8MHz << ui->distanceFilter3_5MHz << ui->distanceFilter7MHz
                       << ui->distanceFilter14MHz << ui->distanceFilter21MHz << ui->distanceFilter28MHz*/
                       << ui->distanceFilter50MHz << ui->distanceFilter70MHz << ui->distanceFilter144MHz
                       << ui->distanceFilter432MHz << ui->distanceFilter1296MHz << ui->distanceFilter2300MHz
                       << ui->distanceFilter3_4GHz << ui->distanceFilter5_6GHz << ui->distanceFilter10GHz;


     ClusterFilterIdAndNames clustId;

     for (int i = 0; i < distanceLineEdits.count(); i++)
     {
         distValue distItem;
         //distItem.distance = config.value(distanceIniNames[i], DEFAULT_FILTER_DISTANCE).toInt();
         TContestApp::getContestApp() ->loggerBundle.getIntProfile( clustId.getDefaultFilterId(i), distItem.distance );

         distanceLineEdits[i]->setText(QString::number(distItem.distance));
         distItem.changed = false;
         distanceValues.append(distItem);
     }



     for (int i = 0; i < distanceLineEdits.count(); i++)
     {
         connect(distanceLineEdits[i], &QLineEdit::editingFinished, [=]() {onDistanceEditingFinished(i);});
     }

     connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onAccepted()));
     connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(onRejected()));

     //bool allowHF = false;
     //TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpAllowHF, allowHF );


      ui->hf_frame->setVisible(false);  // don't show Hf for this release

    // get addBandmapTuningTolerance

      TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpAddBandMapTuningTolerance, addBandmapTuningTolerance );


      if (addBandmapTuningTolerance < ADD_TUNING_BANDMAP_FREQ_DEFAULT_MIN_TOLERANCE || addBandmapTuningTolerance > ADD_TUNING_BANDMAP_FREQ_DEFAULT_MAX_TOLERANCE)
      {
         addBandmapTuningTolerance =  ADD_TUNING_BANDMAP_FREQ_DEFAULT_TOLERANCE;
      }

      //connect(ui->addBandmapTuningTolSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onFreqToleranceValueChanged(int)));
      ui->addBandmapTuningTolSpinBox->setRange(ADD_TUNING_BANDMAP_FREQ_DEFAULT_MIN_TOLERANCE, ADD_TUNING_BANDMAP_FREQ_DEFAULT_MAX_TOLERANCE);

      ui->addBandmapTuningTolSpinBox->setValue(addBandmapTuningTolerance);
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
            QString msg = tr("Distance Error - %1. Please enter a distance %2 to max %3").arg( distanceLineEdits[idx]->text()).arg(MIN_FILTER_DISTANCE).arg(MAX_FILTER_DISTANCE);
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


void ClusterBandmapConfigure::onFreqToleranceValueChanged(int value)
{



}
void ClusterBandmapConfigure::onAccepted()
{

    saveDistances();

    if (ui->addBandmapTuningTolSpinBox->value() != addBandmapTuningTolerance)
    {
        // changed save
        TContestApp::getContestApp()->loggerBundle.setIntProfile(elpAddBandMapTuningTolerance, ui->addBandmapTuningTolSpinBox->value());

    }
}

void ClusterBandmapConfigure::onRejected()
{
    doClose();
}

void ClusterBandmapConfigure::saveDistances()
{

    ClusterFilterIdAndNames clustId;
    for (int i = 0; i < distanceLineEdits.count(); i++)
    {
        if (distanceValues[i].changed && distanceValues[i].distance != DEFAULT_FILTER_DISTANCE)
        {
            TContestApp::getContestApp()->loggerBundle.setIntProfile(clustId.getDefaultFilterId(i), distanceValues[i].distance);

        }
    }
}


void ClusterBandmapConfigure::doClose()
{
    QSettings settings;
    settings.setValue("ClusterBandmapConfigure/geometry/", saveGeometry());

    close();
}
