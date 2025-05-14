/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2025
//
//
/////////////////////////////////////////////////////////////////////////////


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
    explicit SkyScanPresetsDialog(QWidget *parent, SkyScanData* _curData, QString editButtonType);
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

    void onSkyScanStepDegreeSpinBoxChanged();
    void onSkyScanPauseTimeSpinBoxChanged();
private:
    Ui::SkyScanPresetsDialog *ui;



    SkyScanData* curData = nullptr;


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
    void accept() override;
};

#endif // SKYSCANPRESETSDIALOG_H
