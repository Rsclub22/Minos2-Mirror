#include "ContestApp.h"
#include "bandmapcommon.h"

#include "base_pch.h"
#include "Clusterbandmapconfigure.h"
#include "ui_Clusterbandmapconfigure.h"

ClusterBandmapConfigure::ClusterBandmapConfigure(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::ClusterBandmapConfigure)
{
    ui->setupUi(this);
}

ClusterBandmapConfigure::~ClusterBandmapConfigure()
{
    delete ui;
}

void ClusterBandmapConfigure::initialise()
{
    distanceLineEdits << ui->distanceFilter1_8MHz << ui->distanceFilter3_5MHz << ui->distanceFilter7MHz
                      << ui->distanceFilter14MHz << ui->distanceFilter21MHz << ui->distanceFilter28MHz
                      << ui->distanceFilter50MHz << ui->distanceFilter70MHz << ui->distanceFilter144MHz
                      << ui->distanceFilter432MHz << ui->distanceFilter1296MHz << ui->distanceFilter2300MHz
                      << ui->distanceFilter3_4GHz << ui->distanceFilter5_6GHz << ui->distanceFilter10GHz;


    BandList::getBandList().loadAllBands(bands);
    ClusterFilterDefaultDistIniName defaultDistIniNames;
    defaultDistIniNames.initClusterFilterIdAndNames(bands);

    QSettings config(CLUSTER_FILTER_FILE, QSettings::IniFormat);
    config.beginGroup("Default Distance");

    for (int i = 0; i < bands.count(); i++)
    {
        QString band = bands[i].data()->uk;
        distValue distItem;
        QString dKey = defaultDistIniNames.getDefaultDistIniName(band).defaultDistanceName;
        if (dKey.isEmpty())
        {
            distItem.distance = DEFAULT_FILTER_DISTANCE;
        }
        else
        {
            distItem.distance = config.value(dKey, DEFAULT_FILTER_DISTANCE).toInt();
        }
        distItem.distLineEdit = distanceLineEdits[i];
        distItem.distLineEdit->setText(QString::number(distItem.distance));
        distItem.changed = false;
        distanceValues.insert(band, distItem);
    }


   config.endGroup();

   lessGreaterThanDistanceFlag = config.value(LESS_GREATER_THAN_DISTANCE_FLAG_INI_NAME, false).toBool();

   if (lessGreaterThanDistanceFlag)
   {
       ui->spotLessThanDistanceRadioButton->setChecked(false);
       ui->spotGreaterThanDistanceRadioButton->setChecked(true);
   }
   else
   {
       ui->spotLessThanDistanceRadioButton->setChecked(true);
       ui->spotGreaterThanDistanceRadioButton->setChecked(false);
   }

    for (int i = 0; i < distanceLineEdits.count(); i++)
    {
        connect(distanceLineEdits[i], &QLineEdit::editingFinished, this, [=]() {onDistanceEditingFinished(distanceLineEdits[i]);});
    }

    bool allowHF = false;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpAllowHF, allowHF );


    connect(ui->spotLessThanDistanceRadioButton, &QRadioButton::clicked, this, [=](){onSpotLessThanDistanceRadioButClicked();});
    connect(ui->spotGreaterThanDistanceRadioButton, &QRadioButton::clicked, this, [=](){onSpotGreaterThanDistanceRadioButClicked();});

     ui->hfFrame->setVisible(allowHF);  // don't show Hf for this release

   // get addBandmapTuningTolerance

     TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpAddBandMapTuningTolerance, addBandmapTuningTolerance );


     if (addBandmapTuningTolerance < ADD_TUNING_BANDMAP_FREQ_DEFAULT_MIN_TOLERANCE || addBandmapTuningTolerance > ADD_TUNING_BANDMAP_FREQ_DEFAULT_MAX_TOLERANCE)
     {
        addBandmapTuningTolerance =  ADD_TUNING_BANDMAP_FREQ_DEFAULT_TOLERANCE;
     }

     ui->addBandmapTuningTolSpinBox->setRange(ADD_TUNING_BANDMAP_FREQ_DEFAULT_MIN_TOLERANCE, ADD_TUNING_BANDMAP_FREQ_DEFAULT_MAX_TOLERANCE);

     ui->addBandmapTuningTolSpinBox->setValue(addBandmapTuningTolerance);

     bool operatingFreqFlag;
     TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpBandMapTurnOffOperatingFreqStrip, operatingFreqFlag );
     ui->operatingFreqChkBox->setChecked(operatingFreqFlag);

     bool followRadioModeFlag;
     TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpBandMapFollowRadioModeOperatingFreqStrip, followRadioModeFlag );
     ui->modeOperatingFreqChkBox->setChecked(followRadioModeFlag);

     bool minBFlag;
     TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpBandMapMouseInFrameDelay, minBFlag );
     ui->mouseInBcb->setChecked(minBFlag);

}
bool ClusterBandmapConfigure::check()
{
    return true;
}
void ClusterBandmapConfigure::cancel()
{

}
void ClusterBandmapConfigure::finalise()
{
    saveDistances();

    TContestApp::getContestApp()->loggerBundle.setIntProfile(elpAddBandMapTuningTolerance, ui->addBandmapTuningTolSpinBox->value());
    TContestApp::getContestApp() ->loggerBundle.setBoolProfile( elpBandMapTurnOffOperatingFreqStrip,  ui->operatingFreqChkBox->isChecked());
    TContestApp::getContestApp() ->loggerBundle.setBoolProfile( elpBandMapFollowRadioModeOperatingFreqStrip,  ui->modeOperatingFreqChkBox->isChecked());
    TContestApp::getContestApp() ->loggerBundle.setBoolProfile( elpBandMapMouseInFrameDelay,  ui->mouseInBcb->isChecked());
    TContestApp::getContestApp() ->loggerBundle.setBoolProfile( elpBandmapOldStyle, ui->oldBandmapChkBox->isChecked() );

    TContestApp::getContestApp() ->loggerBundle.flushProfile();
}
void ClusterBandmapConfigure::onDistanceEditingFinished(QLineEdit *distLineEdit)
{
    int distance = 0;
    bool ok = false;
    if (!distLineEdit->text().isEmpty())
    {
        QString band = findBandKey(distLineEdit);
        distance = distLineEdit->text().toInt(&ok);
        if (!ok || distance < MIN_FILTER_DISTANCE || distance > MAX_FILTER_DISTANCE)
        {
            QMessageBox messageBox;
            QString msg = tr("Distance Error - %1. Please enter a distance %2 to max %3").arg( distLineEdit->text()).arg(MIN_FILTER_DISTANCE).arg(MAX_FILTER_DISTANCE);
            messageBox.critical(this, tr("Distance Entry Error"), msg);
            return;
        }
        else
        {
            distanceValues[band].distance = distance;
            distanceValues[band].changed = true;
        }
    }
}

QString ClusterBandmapConfigure::findBandKey(QLineEdit *distLineEdit)
{
    for (auto &b:bands)
    {
        QString band = b.data()->uk;
        if (distanceValues.value(band).distLineEdit == distLineEdit)
        {
            return band;
        }
    }

    return "";
}

void ClusterBandmapConfigure::onFreqToleranceValueChanged(int /*value*/)
{



}
void ClusterBandmapConfigure::saveDistances()
{

    ClusterFilterDefaultDistIniName defaultDistIniNames;
    defaultDistIniNames.initClusterFilterIdAndNames(bands);

    QSettings config(CLUSTER_FILTER_FILE, QSettings::IniFormat);
    config.beginGroup("Default Distance");

    for (auto &b:bands)
    {
        QString band = b.data()->uk;
        if (distanceValues.value(band).changed && distanceValues.value(band).distance != DEFAULT_FILTER_DISTANCE)
        {
            config.setValue(defaultDistIniNames.getDefaultDistIniName(band).defaultDistanceName, distanceValues.value(band).distance);
        }
    }

    config.endGroup();

    config.setValue(LESS_GREATER_THAN_DISTANCE_FLAG_INI_NAME, lessGreaterThanDistanceFlag);

}

void ClusterBandmapConfigure::onSpotLessThanDistanceRadioButClicked()
{
    lessGreaterThanDistanceFlag = false;
}

void ClusterBandmapConfigure::onSpotGreaterThanDistanceRadioButClicked()
{
    lessGreaterThanDistanceFlag = true;
}
