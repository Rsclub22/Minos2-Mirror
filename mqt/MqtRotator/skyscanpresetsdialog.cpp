#include "skyscanpresetsdialog.h"
#include "ui_skyscanpresetsdialog.h"
#include "rotatorcommon.h"
#include "regsettings.h"

SkyScanPresetsDialog::SkyScanPresetsDialog(QWidget *parent, SkyScanData* curData_, QString editButtonType)
    : QDialog(parent)
    , ui(new Ui::SkyScanPresetsDialog)
    , curData(curData_)

{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("%1 Antenna - SkyScan Preset %2 - %3").arg(curData->getAntennaName()).arg(QString::number(curData->getNumber() + 1), editButtonType));

    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("SkyScanPresetDialog/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);


    connect(ui->presetNameLineEdit, &QLineEdit::editingFinished, this, &SkyScanPresetsDialog::presetNameEditingFinished);
    connect(ui->skyScanRotatorStartBearingUpDownButton, QOverload<int>::of(&ToolButtonUpDown::valueChanged), this, &SkyScanPresetsDialog::skyScanStartBearingToolbuttonValueChanged);
    connect(ui->skyScanRotatorEndBearingUpDownButton, QOverload<int>::of(&ToolButtonUpDown::valueChanged), this, &SkyScanPresetsDialog::skyScanEndBearingToolbuttonValueChanged);
    connect(ui->skyScanStepDegreeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SkyScanPresetsDialog::skyScanStepDegreesSpinBoxValueChanged);
    connect(ui->skyScanPauseTimeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SkyScanPresetsDialog::skyScanPauseTimeSpinBoxValueChanged);


    connect(this, &SkyScanPresetsDialog::updateEndStopType,  ui->compassDialwidget, &MinosCompass::updateEndStopType);
    connect(this, &SkyScanPresetsDialog::updateAntennaOffset,  ui->compassDialwidget, &MinosCompass::updateAntennaOffset);
    connect(this, &SkyScanPresetsDialog::updateStartBearing, ui->compassDialwidget, &MinosCompass::updateSkyScanStartBearing);
    connect(this, &SkyScanPresetsDialog::updateEndBearing, ui->compassDialwidget, &MinosCompass::updateSkyScanEndBearing);

    //remove spin box highlighting
    connect(ui->skyScanStepDegreeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onSkyScanStepDegreeSpinBoxChanged()), Qt::QueuedConnection);
    connect(ui->skyScanPauseTimeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onSkyScanPauseTimeSpinBoxChanged()), Qt::QueuedConnection);


    //connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SkyScanPresetsDialog::editAccepted);
    //connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &SkyScanPresetsDialog::editRejected);

    ui->compassDialwidget->setDoNotShowNeedle(true);



    ui->presetNameLineEdit->setText(curData->getPresetName());
    initialiseStepDegreeToolButton(curData->getStepToolButtonMin(), curData->getStepToolButtonMax(),
                                                       curData->getStepToolButtonStepInterval(), curData->getSkyScanStepDegreesValue());
    initialisePauseTimeToolButton(curData->getPauseToolButtonMin(), curData->getPauseToolButtonMax(),
                                                      curData->getPauseToolButtonStepInterval(), curData->getSkyScanPauseTimeValue());
    setStartBearingToolButton(curData->getRotatorMinAzimuth(), curData->getRotatorMaxAzimuth(), curData->getSkyScanStepDegreesValue(), curData->getSkyScanStartBearing());
    setEndBearingToolButton(curData->getRotatorMinAzimuth(), curData->getRotatorMaxAzimuth(), curData->getSkyScanStepDegreesValue(), curData->getSkyScanEndBearing());

    sendEndStopTypeToCompassDial();
    sendAntennaOffsetToCompassDial();

    displaySkyScanRotatorMinMaxAzimuth(curData->getRotatorMinAzimuth(), curData->getRotatorMaxAzimuth(), curData->getAntennaOffset(), curData->getSouthStopType(), curData->getEndStopType());

    updateSkyScanStartBearingDisplay(curData->getSkyScanStartBearing());
    updateSkyScanEndBearingDisplay(curData->getSkyScanEndBearing());


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
    doCloseEvent();
}

void SkyScanPresetsDialog::doCloseEvent()
{
    RegSettings settings;
    settings.getSettings().setValue("RotControlSetup/geometry", saveGeometry());
}


void SkyScanPresetsDialog::closeEvent (QCloseEvent *event)
{

    doCloseEvent();
    QWidget::closeEvent(event);
}

void SkyScanPresetsDialog::presetNameEditingFinished()
{
    if (curData->getPresetName() != ui->presetNameLineEdit->text().trimmed())
    {
        curData->setPresetName(ui->presetNameLineEdit->text().trimmed());
        curData->setPresetNameIsDirty(true);
    }

}


void SkyScanPresetsDialog::skyScanStartBearingToolbuttonValueChanged()
{
    if(curData->getSkyScanStartBearing() != ui->skyScanRotatorStartBearingUpDownButton->value())
    {
        curData->setSkyScanStartBearing(ui->skyScanRotatorStartBearingUpDownButton->value());
        curData->setSkyScanStartBearingIsDirty(true);
        updateSkyScanStartBearingDisplay(curData->getSkyScanStartBearing());
        //calculateAntennaDisplayBearing(curData->getSkyScanStartBearing(), ui->skyScanCompassStartBearingDisplay);
        //emit updateStartBearing(curData->getSkyScanStartBearing());
    }
}


void SkyScanPresetsDialog::updateSkyScanStartBearingDisplay(int bearing)
{
    emit updateStartBearing(bearing); // send to compass display
    calculateAntennaDisplayBearing(bearing, ui->skyScanCompassStartBearingDisplay);

}


void SkyScanPresetsDialog::skyScanEndBearingToolbuttonValueChanged()
{
    if(curData->getSkyScanEndBearing() != ui->skyScanRotatorEndBearingUpDownButton->value())
    {
        curData->setSkyScanEndBearing(ui->skyScanRotatorEndBearingUpDownButton->value());
        curData->setSkyScanEndBearingIsDirty(true);
        updateSkyScanEndBearingDisplay(curData->getSkyScanEndBearing());
        //calculateAntennaDisplayBearing(curData->getSkyScanEndBearing(), ui->skyScanCompassEndBearingDisplay);
        //emit updateEndBearing(curData->getSkyScanEndBearing());
    }
}

void SkyScanPresetsDialog::updateSkyScanEndBearingDisplay(int bearing)
{
    emit updateEndBearing(bearing); // send to compass display
    calculateAntennaDisplayBearing(bearing, ui->skyScanCompassEndBearingDisplay);

}

void SkyScanPresetsDialog::skyScanStepDegreesSpinBoxValueChanged()
{
    if (curData->getSkyScanStepDegreesValue() != ui->skyScanStepDegreeSpinBox->value())
    {
        curData->setSkyScanStepDegreesValue(ui->skyScanStepDegreeSpinBox->value());
        curData->setSkyScanStepDegreesValueIsDirty(true);
        clearSkyScanStartEndBearings();
    }
}


void SkyScanPresetsDialog::clearSkyScanStartEndBearings()
{
    int brg = 0;
    curData->setSkyScanStartBearing(brg);
    ui->skyScanRotatorStartBearingUpDownButton->setValue(brg);
    updateSkyScanStartBearingDisplay(curData->getSkyScanStartBearing());
    curData->setSkyScanStartBearingIsDirty(true);
    curData->setSkyScanEndBearing(brg);
    ui->skyScanRotatorEndBearingUpDownButton->setValue(brg);
    updateSkyScanEndBearingDisplay(curData->getSkyScanEndBearing());
    curData->setSkyScanEndBearingIsDirty(true);
}


void SkyScanPresetsDialog::skyScanPauseTimeSpinBoxValueChanged()
{
    if (curData->getSkyScanPauseTimeValue() != ui->skyScanPauseTimeSpinBox->value())
    {
        curData->setSkyScanPauseTimeValue(ui->skyScanPauseTimeSpinBox->value());
        curData->setSkyScanPauseTimeValueDirty(true);
    }
}

void SkyScanPresetsDialog::sendEndStopTypeToCompassDial()
{
    emit updateEndStopType(curData->getEndStopType());
}

void SkyScanPresetsDialog::sendAntennaOffsetToCompassDial()
{
    emit updateAntennaOffset(curData->getAntennaOffset());
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

void SkyScanPresetsDialog::onSkyScanStepDegreeSpinBoxChanged() // slot
{
    ui->skyScanStepDegreeSpinBox->findChild<QLineEdit*>()->deselect();
}


void SkyScanPresetsDialog::onSkyScanPauseTimeSpinBoxChanged() // slot
{
    ui->skyScanPauseTimeSpinBox->findChild<QLineEdit*>()->deselect();
}

void SkyScanPresetsDialog::setStartBearingToolButton(int minRotatorBearing, int maxRotatorBearing, int stepInterval, int value)
{
    ui->skyScanRotatorStartBearingUpDownButton->setRange(minRotatorBearing, maxRotatorBearing);
    ui->skyScanRotatorStartBearingUpDownButton->setStep(stepInterval);
    ui->skyScanRotatorStartBearingUpDownButton->setValue(value);
    emit updateStartBearing(curData->getSkyScanStartBearing());
}

void SkyScanPresetsDialog::setEndBearingToolButton(int minRotatorBearing, int maxRotatorBearing, int stepInterval, int value)
{
    ui->skyScanRotatorEndBearingUpDownButton->setRange(minRotatorBearing, maxRotatorBearing);
    ui->skyScanRotatorEndBearingUpDownButton->setStep(stepInterval);
    ui->skyScanRotatorEndBearingUpDownButton->setValue(value);
    emit updateStartBearing(curData->getSkyScanEndBearing());

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
    int curBearingWithOffset = rotatorBearing + curData->getAntennaOffset();


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







void SkyScanPresetsDialog::displaySkyScanRotatorMinMaxAzimuth(int minAz, int maxAz, int antennaOffset, enum southStop southStopType, enum endStop endStopType)
{

    skyScanRotatorDisplayLabels displayLabels;
    displayLabels.minAzimuthLabel = ui->skyScanRotatorDisplayMinAz;
    displayLabels.maxAzimuthLabel = ui->skyScanRotatorDisplayMaxAz;
    displayLabels.antennaOffsetLabel = ui->antennaOffsetDisplay;
    displayLabels.rotatorStopLabel = ui->rotatorStopLabel;
    displayLabels.southStopDisplayLabel = ui->southStopDisplay;

    skyScanDisplayRotatorMinAzMaxAz(minAz, maxAz, antennaOffset,  southStopType, endStopType, displayLabels);

}




