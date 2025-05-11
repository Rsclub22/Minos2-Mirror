#ifndef SKYSCANPRESETSDIALOG_H
#define SKYSCANPRESETSDIALOG_H

#include <QDialog>
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




    void initialiseStepDegreeToolButton(int minStep, int maxStep, int stepInterval, int initialValue);
    void initialisePauseTimeToolButton(int minTime, int maxTime, int timeInterval, int initialValue);



    void initialiseRotatorMinMaxAzimuth(int minAz, int maxAz);
    void initialiseAntennaOffset(int antennaOffset_);
    void setStartBearingToolButton(int minRotatorBearing, int maxRotatorBearing, int stepInterval, int value);
    void setEndBearingToolButton(int minRotatorBearing, int maxRotatorBearing, int stepInterval, int value);

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
private:
    Ui::SkyScanPresetsDialog *ui;



    SkyScanPresetData* curData = nullptr;


    int rotatorMinAz = 0;
    int rotatorMaxAz = 360;
    int antennaOffset = 0;



    void setStartAntennaBearingDisplay(int rotatorStartBearing);
    void setEndAntennaBearingDisplay(int rotatorStartBearing);
    void calculateAntennaDisplayBearing(int rotatorBearing, QLabel *displayAntennaBearingLabel);
    void sendEndStopTypeToCompassDial();
    void skyScanDisplayRotatorMinAzMaxAz(int minAz, int maxAz, int antennaOffset, southStop southStopType, endStop endStopType);
};

#endif // SKYSCANPRESETSDIALOG_H
