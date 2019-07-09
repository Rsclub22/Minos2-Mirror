/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef ROTATORCOMMON_H
#define ROTATORCOMMON_H

#include "base_pch.h"

class RotPresetData
{

public:

    RotPresetData(int _number, QString _name, QString _bearing);
    RotPresetData();

    int number = 0;
    QString name;
    QString bearing;

};


const QChar SHORTLOCATOR_IDENTIFIER = '#'; // add to bearing to identify that it has been calculated from short locator.
                                           // Used in bearing sent from memory and clusterframe.

// Common Rotator Constants

const QChar DEGREE_SYMBOL = 0260; // octal value
const QChar BEARING_TRUE_CHAR = 'T';
const QChar SHORTLOC_DELIMITER_START = '(';
const QChar SHORTLOC_DELIMITER_END = ')';
const int ROTATE_MOVE_TIMEOUT = 5;



// Rotator Keys

const QString ROTATE_CW_KEY = "Ctrl+r";
const QString ROTATE_CCW_KEY = "Ctrl+l";
const QString ROTATE_STOP_KEY = "Ctrl+s";
const QString ROTATE_TURN_KEY = "Ctrl+t";

const QString ROTATE_PRESET1 = "Ctrl+1";
const QString ROTATE_PRESET2 = "Ctrl+2";
const QString ROTATE_PRESET3 = "Ctrl+3";
const QString ROTATE_PRESET4 = "Ctrl+4";
const QString ROTATE_PRESET5 = "Ctrl+5";
const QString ROTATE_PRESET6 = "Ctrl+6";
const QString ROTATE_PRESET7 = "Ctrl+7";
const QString ROTATE_PRESET8 = "Ctrl+8";
const QString ROTATE_PRESET9 = "Ctrl+9";
const QString ROTATE_PRESET10 = "Ctrl+0";

const QStringList presetShortCutKeys = {
                                            ROTATE_PRESET1, ROTATE_PRESET2,
                                            ROTATE_PRESET3, ROTATE_PRESET4,
                                            ROTATE_PRESET5, ROTATE_PRESET6,
                                            ROTATE_PRESET7, ROTATE_PRESET8,
                                            ROTATE_PRESET9, ROTATE_PRESET10
                                        };


const QString ROTATE_PRESET_MENU1 = "Ctrl+Alt+1";
const QString ROTATE_PRESET_MENU2 = "Ctrl+Alt+2";
const QString ROTATE_PRESET_MENU3 = "Ctrl+Alt+3";
const QString ROTATE_PRESET_MENU4 = "Ctrl+Alt+4";
const QString ROTATE_PRESET_MENU5 = "Ctrl+Alt+5";
const QString ROTATE_PRESET_MENU6 = "Ctrl+Alt+6";
const QString ROTATE_PRESET_MENU7 = "Ctrl+Alt+7";
const QString ROTATE_PRESET_MENU8 = "Ctrl+Alt+8";
const QString ROTATE_PRESET_MENU9 = "Ctrl+Alt+9";
const QString ROTATE_PRESET_MENU0 = "Ctrl+Alt+0";

const QStringList presetMenuShortCutKeys = {
                                                        ROTATE_PRESET_MENU1, ROTATE_PRESET_MENU2,
                                                        ROTATE_PRESET_MENU3, ROTATE_PRESET_MENU4,
                                                        ROTATE_PRESET_MENU5, ROTATE_PRESET_MENU6,
                                                        ROTATE_PRESET_MENU7, ROTATE_PRESET_MENU8,
                                                        ROTATE_PRESET_MENU9, ROTATE_PRESET_MENU0
                                             };

const int COMPASS_MAX360 = 360;
const int COMPASS_HALF = 180;
const int COMPASS_NEG_HALF = -180;
const int COMPASS_MIN0 = 0;
const int COMPASS_ERROR = 999;

const int ROTATE_ENDSTOP_TOLERANCE = 0;


// Status messages sent to minos logger
const QString ROT_STATUS_CONNECTED  = "Connected";
const QString ROT_STATUS_DISCONNECTED = "Disconnected";
const QString ROT_STATUS_ERROR = "Error";

const QString ROT_STATUS_ROTATE_CCW = "<font color='Green'>Rotating CCW</font>";
const QString ROT_STATUS_ROTATE_CW = "<font color='Green'>Rotating CW</font>";
const QString ROT_STATUS_STOP = "Stop";
const QString ROT_STATUS_TURN_TO = "<font color='Green'>Turning to bearing</font>";

const QString ROT_ACTIVE = "Rot_Active";
const QString ROT_NOT_ACTIVE = "Rot_Not_Active";

// Bearing Log
const QString BEARINGLOG_FILETYPE = ".log";

// File Name Constants

const QString LOCAL_ANTENNA = "Local";

const QString CONFIGURATION_FILEPATH_LOGGER = "./Configuration/";
const QString CONFIGURATION_FILEPATH_LOCAL = "./Configuration/";

const QString MINOS_ROTATOR_CONFIG_FILE = "MinosRotatorConfig.ini";
const QString ROTATOR_DATA_FILE = "rotators.ini";

const QString ANTENNA_PATH_LOGGER = "./Configuration/Antenna/";
const QString ANTENNA_PATH_LOCAL = "./Configuration/Antenna/";
const QString FILENAME_AVAIL_ANTENNAS = "AvailAntenna.ini";
const QString FILENAME_CURRENT_ANTENNA = "CurrentAntenna.ini";

// Rotator Types
// ROT_180_180 is ROT_0_360 set to southstop.
enum endStop {ROT_NEG180_180, ROT_0_360, ROT_0_450, ROT_NEG180_540, ROT_180_180};
const QStringList endStopNames = (QStringList() << "ROT_NEG180_180" << "ROT_0_360" << "ROT_0_450" << "ROT_NEG180_540" << "ROT_180_180");

// Defines the South Stop Types
// S_STOPINV when a rotator is mounted inverted normal N Stop is now S Stop. Feedback is South 0 Degrees through 360 at South
//S_STOPCOMP when a rotator is mounted with a south stop, but the sensor is a compass. Rotator will rotate 180 degrees to 180, rotator type ROT_180_180
enum southStop {S_STOPOFF, S_STOPINV, S_STOPCOMP};
const QStringList southStopNames = (QStringList() << "S_StopOff" << "S_StopInv" << "S_StopComp");
// Overlap Status

enum overlapStat { NO_OVERLAP, NEG_OVERLAP, POS_OVERLAP};

// Pushbutton Styles

const QString BUTTON_ON_STYLE = QString("background-color: Sandybrown ; border-style: outset; border-width: 1px; border-color: black; min-width: 5em; padding: 3px;\n");
const QString BUTTON_OFF_STYLE = QString("background-color: Gainsboro ; border-style: outset; border-width: 1px; border-color: black; min-width: 5em; padding: 3px;\n");

const QStringList presetButtonLabels = {"&Read", "&New", "&Edit", "&Clear"};

#endif // ROTATORCOMMON_H
