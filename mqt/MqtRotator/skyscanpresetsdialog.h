#ifndef SKYSCANPRESETSDIALOG_H
#define SKYSCANPRESETSDIALOG_H

#include <QDialog>
#include <QSettings>
#include "rotatorcommon.h"

namespace Ui {
class SkyScanPresetsDialog;
}

class SkyScanPresetsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SkyScanPresetsDialog(QWidget *parent, SkyScanPresetData* _curData, QString editButtonType);
    ~SkyScanPresetsDialog();





signals:

    void updateAntennaOffset(int offset);
    void updateEndStopType(int endStopType_);
    void updateStartBearing(int brg);
    void updateEndBearing(int brg);

private slots:
    void editAccepted();
    void editRejected();
    void skyScanStartBearingToolbuttonValueChanged();
    void skyScanEndBearingToolbuttonValueChanged();
    void skyScanStepDegreesSpinBoxValueChanged();

    void skyScanPauseTimeSpinBoxValueChanged();
    void presetNameEditingFinished();

private:
    Ui::SkyScanPresetsDialog *ui;



    SkyScanPresetData* curData = nullptr;


    void initialiseStepDegreeToolButton(int minStep, int maxStep, int stepInterval, int initialValue);
    void initialisePauseTimeToolButton(int minTime, int maxTime, int timeInterval, int initialValue);


    void setStartBearingToolButton(int minRotatorBearing, int maxRotatorBearing, int stepInterval, int value);
    void setEndBearingToolButton(int minRotatorBearing, int maxRotatorBearing, int stepInterval, int value);

    void setStartAntennaBearingDisplay(int rotatorStartBearing);
    void setEndAntennaBearingDisplay(int rotatorStartBearing);
    void calculateAntennaDisplayBearing(int rotatorBearing, QLabel *displayAntennaBearingLabel);
    void sendEndStopTypeToCompassDial();
    void updateSkyScanStartBearingDisplay(int bearing);
    void updateSkyScanEndBearingDisplay(int bearing);
    void sendAntennaOffsetToCompassDial();
    void displaySkyScanRotatorMinMaxAzimuth(int minAz, int maxAz, int antennaOffset, southStop southStopType, endStop endStopType);
    void closeEvent(QCloseEvent *event) override;
    void doCloseEvent();
    void clearSkyScanStartEndBearings();
};

#endif // SKYSCANPRESETSDIALOG_H
