#include "skyscanpresetsdialog.h"
#include "ui_skyscanpresetsdialog.h"

SkyScanPresetsDialog::SkyScanPresetsDialog(QWidget *parent, SkyScanPresetData* curData_, QString editButtonType)
    : QDialog(parent)
    , ui(new Ui::SkyScanPresetsDialog)
    , curData(curData_)

{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("%1 Antenna - SkyScan Preset %2 - %3").arg(curData->getAntennaName()).arg(QString::number(curData->getNumber() + 1), editButtonType));

    connect(ui->skyScanRotatorStartBearingUpDownButton, QOverload<int>::of(&ToolButtonUpDown::valueChanged), this, &SkyScanPresetsDialog::skyScanStartBearingToolbuttonValueChanged);
    connect(ui->skyScanRotatorEndBearingUpDownButton, QOverload<int>::of(&ToolButtonUpDown::valueChanged), this, &SkyScanPresetsDialog::skyScanEndBearingToolbuttonValueChanged);
    connect(ui->skyScanStepDegreeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SkyScanPresetsDialog::skyScanStepDegreesSpinBoxValueChanged);
    connect(ui->skyScanPauseTimeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SkyScanPresetsDialog::skyScanPauseTimeSpinBoxValueChanged);

    connect(this, &SkyScanPresetsDialog::updateAntennaOffset, ui->compassDialwidget, &MinosCompass::updateSkyScanEndBearing);
    connect(this, &SkyScanPresetsDialog::updateEndStopType,  ui->compassDialwidget, &MinosCompass::updateEndStopType);
    connect(this, &SkyScanPresetsDialog::updateAntennaOffset,  ui->compassDialwidget, &MinosCompass::updateAntennaOffset);
    connect(this, &SkyScanPresetsDialog::updateStartBearing, ui->compassDialwidget, &MinosCompass::updateSkyScanStartBearing);
    connect(this, &SkyScanPresetsDialog::updateEndBearing, ui->compassDialwidget, &MinosCompass::updateSkyScanEndBearing);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SkyScanPresetsDialog::editAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &SkyScanPresetsDialog::editRejected);

    ui->compassDialwidget->setDoNotShowNeedle(true);

    sendEndStopTypeToCompassDial();
    skyScanDisplayRotatorMinAzMaxAz(curData->getRotatorMinAzimuth(), curData->getRotatorMaxAzimath(), curData->getAntennaOffset(), curData->getSouthStopType(), curData->getEndStopType());

}

SkyScanPresetsDialog::~SkyScanPresetsDialog()
{
    delete ui;
}


void SkyScanPresetsDialog::editAccepted()
{

}


void SkyScanPresetsDialog::editRejected()
{

}


void SkyScanPresetsDialog::skyScanStartBearingToolbuttonValueChanged()
{
    if(curData->getSkyScanStartBearing() != ui->skyScanRotatorStartBearingUpDownButton->value())
    {
        curData->setSkyScanStartBearing(ui->skyScanRotatorStartBearingUpDownButton->value());
        curData->setSkyScanStartBearingIsDirty(true);
        calculateAntennaDisplayBearing(curData->getSkyScanStartBearing(), ui->skyScanCompassStartBearingDisplay);
        emit updateStartBearing(curData->getSkyScanStartBearing());
    }
}


void SkyScanPresetsDialog::skyScanEndBearingToolbuttonValueChanged()
{
    if(curData->getSkyScanEndBearing() != ui->skyScanRotatorEndBearingUpDownButton->value())
    {
        curData->setSkyScanEndBearing(ui->skyScanRotatorEndBearingUpDownButton->value());
        curData->setSkyScanEndBearingIsDirty(true);
        calculateAntennaDisplayBearing(curData->getSkyScanEndBearing(), ui->skyScanCompassEndBearingDisplay);
        emit updateEndBearing(curData->getSkyScanEndBearing());
    }
}

void SkyScanPresetsDialog::skyScanStepDegreesSpinBoxValueChanged()
{

}


void SkyScanPresetsDialog::skyScanPauseTimeSpinBoxValueChanged()
{

}

void SkyScanPresetsDialog::sendEndStopTypeToCompassDial()
{
    emit updateEndStopType(curData->getEndStopType());
}

void SkyScanPresetsDialog::initialiseStepDegreeToolButton(int minStep, int maxStep, int stepInterval, int initialValue)
{
    ui->skyScanStepDegreeSpinBox->setRange(minStep, maxStep);
    ui->skyScanStepDegreeSpinBox->setSingleStep(stepInterval);
    ui->skyScanStepDegreeSpinBox->setValue(initialValue);

}

void SkyScanPresetsDialog::initialisePauseTimeToolButton(int minTime, int maxTime, int timeInterval, int initialValue)
{
    ui->skyScanPauseTimeSpinBox->setRange(minTime, maxTime);
    ui->skyScanPauseTimeSpinBox->setSingleStep(timeInterval);
    ui->skyScanPauseTimeSpinBox->setValue(initialValue);
}

void SkyScanPresetsDialog::setStartBearingToolButton(int minRotatorBearing, int maxRotatorBearing, int stepInterval, int value)
{
    ui->skyScanRotatorStartBearingUpDownButton->setRange(minRotatorBearing, maxRotatorBearing);
    ui->skyScanRotatorStartBearingUpDownButton->setStep(stepInterval);
    ui->skyScanRotatorStartBearingUpDownButton->setValue(value);
}

void SkyScanPresetsDialog::setEndBearingToolButton(int minRotatorBearing, int maxRotatorBearing, int stepInterval, int value)
{
    ui->skyScanRotatorEndBearingUpDownButton->setRange(minRotatorBearing, maxRotatorBearing);
    ui->skyScanRotatorEndBearingUpDownButton->setStep(stepInterval);
    ui->skyScanRotatorEndBearingUpDownButton->setValue(value);

}

void SkyScanPresetsDialog::setStartAntennaBearingDisplay(int rotatorStartBearing)
{
    calculateAntennaDisplayBearing(rotatorStartBearing, ui->skyScanCompassStartBearingDisplay);

}

void SkyScanPresetsDialog::setEndAntennaBearingDisplay(int rotatorEndBearing)
{
    calculateAntennaDisplayBearing(rotatorEndBearing, ui->skyScanCompassEndBearingDisplay);
}

void SkyScanPresetsDialog::calculateAntennaDisplayBearing(int rotatorBearing, QLabel* displayAntennaBearingLabel)
{
    int curBearingWithOffset = rotatorBearing + antennaOffset;


    //int displayBearing = curBearingWithOffset;


    if (curBearingWithOffset > COMPASS_MAX360)
    {
        curBearingWithOffset = curBearingWithOffset - COMPASS_MAX360;
    }
    else if (curBearingWithOffset < COMPASS_MIN0)
    {
        curBearingWithOffset = COMPASS_MAX360 + curBearingWithOffset;
    }

    QString compassStartBearing = QString::number(curBearingWithOffset).rightJustified(3, '0');
    displayAntennaBearingLabel->setText(compassStartBearing);
}

void SkyScanPresetsDialog::initialiseRotatorMinMaxAzimuth(int minAz, int maxAz)
{
    rotatorMinAz = minAz;
    rotatorMaxAz = maxAz;

}


void SkyScanPresetsDialog::initialiseAntennaOffset(int antennaOffset_)
{
    antennaOffset = antennaOffset_;
}


void SkyScanPresetsDialog::skyScanDisplayRotatorMinAzMaxAz(int minAz, int maxAz, int antennaOffset, enum southStop southStopType, enum endStop endStopType)
{
    Q_UNUSED(endStopType)

    QString minAzStr = QString::number(minAz).rightJustified(3, '0'); //convertBearingToString(minAz);
    QString maxAzStr = QString::number(maxAz).rightJustified(3, '0'); //convertBearingToString(maxAz);

    ui->skyScanRotatorDisplayMinAz->setText(minAzStr);
    ui->skyScanRotatorDisplayMaxAz->setText(maxAzStr);
    ui->antennaOffsetDisplay->setText(QString::number(antennaOffset));

 /*   QString sStopDisplay = "South ";

    if (southStopType == S_STOPOFF)
    {
        ui->rotatorStopLabel->setVisible(false);
        ui->southStopDisplay->setVisible(false);


    }
    else if (southStopType == S_STOPINV)
    {
        sStopDisplay.append("180 - 180");
        ui->southStopDisplay->setText(sStopDisplay);
        ui->southStopDisplay->setVisible(true);
        ui->rotatorStopLabel->setVisible(true);

    }
    else if (southStopType == S_STOP_COMPASS_SENSOR)
    {

        sStopDisplay.append("-180 - 180 Compass Sensor");
        ui->southStopDisplay->setText(sStopDisplay);
        ui->southStopDisplay->setVisible(true);
        ui->rotatorStopLabel->setVisible(true);
    }
*/
}


