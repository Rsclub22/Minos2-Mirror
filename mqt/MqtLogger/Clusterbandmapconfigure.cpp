#include <QSettings>
#include <QLineEdit>
#include <QMessageBox>

#include "ContestApp.h"
#include "bandmapcommon.h"
#include "rigutils.h"
#include "MinosLoggerEvents.h"
#include "Clusterbandmapconfigure.h"
#include "ui_Clusterbandmapconfigure.h"

ClusterBandmapConfigure::ClusterBandmapConfigure(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::ClusterBandmapConfigure)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

ClusterBandmapConfigure::~ClusterBandmapConfigure()
{
    delete ui;
}

void ClusterBandmapConfigure::initialise()
{
    QSettings settings;
    int curTabNo = settings.value("OptionsClusterBandmapConfigure/curTab").toInt();
    ui->ClusterBandmapConfiguretabWidget->setCurrentIndex(curTabNo);

    BandList::getBandList().loadAllBands(bands, false);
    ClusterFilterDefaultDistIniName defaultDistIniNames;
    defaultDistIniNames.initClusterFilterIdAndNames(bands);

    QSettings config(CLUSTER_FILTER_FILE, QSettings::IniFormat);
    config.beginGroup("Default Distance");

    QGridLayout *hfLayout = new QGridLayout();
    ui->HFFrame->setLayout(hfLayout);

    int hfRow = 0;
    int hfCol = 0;

    QGridLayout *vhfLayout = new QGridLayout();
    ui->VHFUHFFrame->setLayout(vhfLayout);

    int vhfRow = 0;
    int vhfCol = 0;

    for (const auto &b: qAsConst(bands))
    {
        QLineEdit *qle = new QLineEdit();
        QLabel *qlel = new QLabel();
        qlel->setText(b->uk);

        if (b->getType() == HF_BANDTYPE)
        {
            hfLayout->addWidget(qlel, hfRow, hfCol);
            hfLayout->addWidget(qle, hfRow, hfCol + 1);
            hfCol += 2;
            if (hfCol%4 == 0)
            {
                hfRow++;
                hfCol = 0;
            }
        }
        else if (b->getType() == VHF_BANDTYPE || b->getType() == MW_BANDTYPE)
        {
            vhfLayout->addWidget(qlel, vhfRow, vhfCol);
            vhfLayout->addWidget(qle, vhfRow, vhfCol + 1);
            vhfCol += 2;
            if (vhfCol%4 == 0)
            {
                vhfRow++;
                vhfCol = 0;
            }
        }

        connect(qle, &QLineEdit::editingFinished, this, [=]() {onDistanceEditingFinished(qle);});

        QString band = b->uk;
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
        distItem.distLineEdit = qle;
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

    connect(ui->spotLessThanDistanceRadioButton, &QRadioButton::clicked, this, [=](){onSpotLessThanDistanceRadioButClicked();});
    connect(ui->spotGreaterThanDistanceRadioButton, &QRadioButton::clicked, this, [=](){onSpotGreaterThanDistanceRadioButClicked();});

     ui->HFgroupBox->setVisible(true);

   // get addBandmapTuningTolerance

     TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpAddBandMapTuningTolerance, addBandmapTuningTolerance );


     if (addBandmapTuningTolerance < ADD_TUNING_BANDMAP_FREQ_DEFAULT_MIN_TOLERANCE || addBandmapTuningTolerance > ADD_TUNING_BANDMAP_FREQ_DEFAULT_MAX_TOLERANCE)
     {
        addBandmapTuningTolerance =  ADD_TUNING_BANDMAP_FREQ_DEFAULT_TOLERANCE;
     }

     ui->addBandmapTuningTolSpinBox->setRange(ADD_TUNING_BANDMAP_FREQ_DEFAULT_MIN_TOLERANCE, ADD_TUNING_BANDMAP_FREQ_DEFAULT_MAX_TOLERANCE);

     ui->addBandmapTuningTolSpinBox->setValue(addBandmapTuningTolerance);

     tuningAddMap.initialise(&TContestApp::getContestApp() ->loggerBundle, elpAddBandMapTuningEnable, ui->tuningAddMapcb);
     disableNotShown.initialise(&TContestApp::getContestApp() ->loggerBundle, elpBandMapDisableNotShown, ui->disableNotShowncb);
     disableLoggedCalls.initialise(&TContestApp::getContestApp() ->loggerBundle, elpBandMapDisableLoggedCalls, ui->disableLoggedCallscb);
     disablePlaceHolders.initialise(&TContestApp::getContestApp() ->loggerBundle, elpBandMapDisablePlaceHolders, ui->disablePlaceHolderscb);

     BandMapTurnOffOperatingFreqStrip.initialise(&TContestApp::getContestApp() ->loggerBundle, elpBandMapTurnOffOperatingFreqStrip, ui->operatingFreqChkBox);
     BandMapFollowRadioModeOperatingFreqStrip.initialise(&TContestApp::getContestApp() ->loggerBundle, elpBandMapFollowRadioModeOperatingFreqStrip, ui->modeOperatingFreqChkBox);
     BandMapMouseInFrameDelay.initialise(&TContestApp::getContestApp() ->loggerBundle, elpBandMapMouseInFrameDelay, ui->mouseInBcb);
     BandMapShowDerivedLoc.initialise(&TContestApp::getContestApp() ->loggerBundle, elpShowDerivedLoc, ui->showDerivedLocChkBox);
     BandmapOldStyle.initialise(&TContestApp::getContestApp() ->loggerBundle, elpBandmapOldStyle, ui->oldBandmapChkBox);
     BandmapInvert.initialise(&TContestApp::getContestApp() ->loggerBundle, elpBandmapInvert, ui->invertBandmap);

     QVBoxLayout *vbl = new QVBoxLayout(ui->scrollAreaWidgetContents);
     vbl->setContentsMargins(1, 1, 1, 1);
     ui->scrollAreaWidgetContents->setLayout(vbl);

     bandLimits.clear();
     for (const auto &b: qAsConst(bands))
     {
         QFrame *bFrame = new QFrame;
         QHBoxLayout *hbl = new QHBoxLayout;
         bFrame->setLayout(hbl);

         QLabel *qlel = new QLabel();
         qlel->setText(b->uk);

         hbl->addWidget(qlel);

         QLabel *sLabel = new QLabel();
         sLabel->setText(tr("Low Freq"));
         hbl->addWidget(sLabel);


         QLineEdit *qlbs = new QLineEdit();
         connect(qlbs, &QLineEdit::editingFinished, this, [=]() {
            QString freq = qlbs->text();
            valInputFreq(freq, tr(RADIO_FREQ_EDIT_ERR_MSG));
         });

         ConfigurationOption lb;
         lb.initialise(BAND_LIST_INI, BAND_LIST_SECT_FREQ_LOW, b->uk, qlbs, b->bandmapLow.convertFreqStrDispSingle() );
         bandLimits.push_back(lb);
         hbl->addWidget(qlbs);

         QLabel *eLabel = new QLabel();
         eLabel->setText(tr("High Freq"));
         hbl->addWidget(eLabel);

         QLineEdit *qlbe = new QLineEdit();
         connect(qlbe, &QLineEdit::editingFinished, this, [=]() {
            QString freq = qlbe->text();
            valInputFreq(freq, tr(RADIO_FREQ_EDIT_ERR_MSG));
         });
         ConfigurationOption hb;
         hb.initialise(BAND_LIST_INI, BAND_LIST_SECT_FREQ_HIGH, b->uk, qlbe, b->bandmapHigh.convertFreqStrDispSingle() );
         bandLimits.push_back(hb);
         hbl->addWidget(qlbe);

         vbl->addWidget(bFrame);
     }
}

bool ClusterBandmapConfigure::check()
{
    bool valid = true;
    for (const auto &bl : qAsConst(bandLimits))
    {
        QString freq = bl.sValue();
        if (!valInputFreq(freq, tr(RADIO_FREQ_EDIT_ERR_MSG)))
        {
            valid = false;
        }
    }
    return valid;
}
void ClusterBandmapConfigure::cancel()
{

}
void ClusterBandmapConfigure::finalise()
{
    saveDistances();

    TContestApp::getContestApp()->loggerBundle.setIntProfile(elpAddBandMapTuningTolerance, ui->addBandmapTuningTolSpinBox->value());

    tuningAddMap.finalise();
    disableNotShown.finalise();
    disableLoggedCalls.finalise();
    disablePlaceHolders.finalise();

    BandMapTurnOffOperatingFreqStrip.finalise();
    BandMapFollowRadioModeOperatingFreqStrip.finalise();
    BandMapMouseInFrameDelay.finalise();
    BandMapShowDerivedLoc.finalise();
    BandmapOldStyle.finalise();
    BandmapInvert.finalise();

    TContestApp::getContestApp() ->loggerBundle.flushProfile();

    bool limitsChanged = false;
    for (const auto &bl : qAsConst(bandLimits))
    {
        if (bl.finalise())
        {
            // validate...
            limitsChanged = true;
        }
    }
    if (limitsChanged)
    {
        MinosLoggerEvents::sendBandmapLimitsChanged();
    }
}


void ClusterBandmapConfigure::on_ClusterBandmapConfiguretabWidget_currentChanged(int index)
{
    QSettings settings;
    settings.setValue("OptionsClusterBandmapConfigure/curTab", index);
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
    for (const auto &b: qAsConst(bands))
    {
        QString band = b->uk;
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

    for (const auto &b: qAsConst(bands))
    {
        QString band = b->uk;
        if (distanceValues.value(band).changed && distanceValues.value(band).distance != DEFAULT_FILTER_DISTANCE)
        {
            DefaultDistanceIniName s = defaultDistIniNames.getDefaultDistIniName(band);
            //QString s1 = defaultDistIniNames.getDefaultDistIniName(band).defaultDistanceName;
            //int d = distanceValues.value(band).distance;
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
