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

#include "rigcontrolcommonconstants.h"
#include <QCoreApplication>
#include "serialCommonData.h"
class RotPresetData
{
    Q_DECLARE_TR_FUNCTIONS(RotPresetData)
public:
static const char * presetButtonLabels[4];


    RotPresetData(int _number, QString _name, QString _bearing);
    RotPresetData();

    int number = 0;
    QString name;
    QString bearing;

};



const QChar SHORTLOCATOR_IDENTIFIER = '#'; // add to bearing to identify that it has been calculated from short locator.
                                           // Used in bearing sent from memory and clusterframe.

// Common Rotator Constants

const QChar DEGREE_SYMBOL = QChar('\260'); // octal value
const QChar BEARING_TRUE_CHAR = 'T';
const QChar SHORTLOC_DELIMITER_START = '(';
const QChar SHORTLOC_DELIMITER_END = ')';
const int ROTATE_MOVE_TIMEOUT = 5;



// Rotator Keys

inline const QString ROTATE_CW_KEY = "Ctrl+r";
inline const QString ROTATE_CCW_KEY = "Ctrl+l";
inline const QString ROTATE_STOP_KEY = "Ctrl+s";
inline const QString ROTATE_TURN_KEY = "Ctrl+t";

inline const QString ROTATE_PRESET1 = "Ctrl+1";
inline const QString ROTATE_PRESET2 = "Ctrl+2";
inline const QString ROTATE_PRESET3 = "Ctrl+3";
inline const QString ROTATE_PRESET4 = "Ctrl+4";
inline const QString ROTATE_PRESET5 = "Ctrl+5";
inline const QString ROTATE_PRESET6 = "Ctrl+6";
inline const QString ROTATE_PRESET7 = "Ctrl+7";
inline const QString ROTATE_PRESET8 = "Ctrl+8";
inline const QString ROTATE_PRESET9 = "Ctrl+9";
inline const QString ROTATE_PRESET10 = "Ctrl+0";

inline const QStringList presetShortCutKeys = {
                                            ROTATE_PRESET1, ROTATE_PRESET2,
                                            ROTATE_PRESET3, ROTATE_PRESET4,
                                            ROTATE_PRESET5, ROTATE_PRESET6,
                                            ROTATE_PRESET7, ROTATE_PRESET8,
                                            ROTATE_PRESET9, ROTATE_PRESET10
                                        };


inline const QString ROTATE_PRESET_MENU1 = "Ctrl+Alt+1";
inline const QString ROTATE_PRESET_MENU2 = "Ctrl+Alt+2";
inline const QString ROTATE_PRESET_MENU3 = "Ctrl+Alt+3";
inline const QString ROTATE_PRESET_MENU4 = "Ctrl+Alt+4";
inline const QString ROTATE_PRESET_MENU5 = "Ctrl+Alt+5";
inline const QString ROTATE_PRESET_MENU6 = "Ctrl+Alt+6";
inline const QString ROTATE_PRESET_MENU7 = "Ctrl+Alt+7";
inline const QString ROTATE_PRESET_MENU8 = "Ctrl+Alt+8";
inline const QString ROTATE_PRESET_MENU9 = "Ctrl+Alt+9";
inline const QString ROTATE_PRESET_MENU0 = "Ctrl+Alt+0";

inline const QStringList presetMenuShortCutKeys = {
                                                        ROTATE_PRESET_MENU1, ROTATE_PRESET_MENU2,
                                                        ROTATE_PRESET_MENU3, ROTATE_PRESET_MENU4,
                                                        ROTATE_PRESET_MENU5, ROTATE_PRESET_MENU6,
                                                        ROTATE_PRESET_MENU7, ROTATE_PRESET_MENU8,
                                                        ROTATE_PRESET_MENU9, ROTATE_PRESET_MENU0
                                             };

const int COMPASS_MAX360 = 360;
const int COMPASS_MAX359 = 359; // Green Heron Rotator
const int COMPASS_HALF = 180;
const int COMPASS_NEG_HALF = -180;
const int COMPASS_MIN0 = 0;
const int COMPASS_ERROR = 999;

const int ROTATE_ENDSTOP_TOLERANCE = 0;


// Status messages sent to minos logger
inline const QString ROT_STATUS_CONNECTED  = "Connected";
inline const QString ROT_STATUS_DISCONNECTED = "Disconnected";
inline const QString ROT_STATUS_ERROR = "Error";

inline const QString ROT_STATUS_ROTATE_CCW = "Rotating CCW";
inline const QString ROT_STATUS_ROTATE_CW = "Rotating CW";
inline const QString ROT_STATUS_STOP = "Stop";
inline const QString ROT_STATUS_TURN_TO = "Turning to bearing";

// Bearing Log
inline const QString BEARINGLOG_FILETYPE = ".log";

// File Name Constants

inline const QString LOCAL_ANTENNA = "Local";

QString CONFIGURATION_FILEPATH_LOGGER();
QString CONFIGURATION_FILEPATH_LOCAL();

inline const QString MINOS_ROTATOR_CONFIG_FILE = "MinosRotatorConfig.ini";
//const QString ROTATOR_DATA_FILE = "rotators.ini";
QString PST_CONFIG_FILE();

QString ANTENNA_PATH_LOGGER();
QString ANTENNA_PATH_LOCAL();
inline const QString FILENAME_AVAIL_ANTENNAS = "AvailAntenna.ini";
inline const QString FILENAME_CURRENT_ANTENNA = "CurrentAntenna.ini";

// Rotator Types
// ROT_180_180 is ROT_0_360 set to southstop.
enum endStop {ROT_NEG180_180, ROT_0_360, ROT_0_450, ROT_NEG180_540, ROT_180_180};
inline const QStringList endStopNames = (QStringList() << "ROT_NEG180_180" << "ROT_0_360" << "ROT_0_450" << "ROT_NEG180_540" << "ROT_180_180");

// Defines the South Stop Types
// S_STOPINV when a rotator is mounted inverted normal N Stop is now S Stop. Feedback is South 0 Degrees through 360 at South
//S_STOPCOMP when a rotator is mounted with a south stop, but the sensor is a compass. Rotator will rotate 180 degrees to 180, rotator type ROT_180_180
enum southStop {S_STOPOFF, S_STOPINV, S_STOPCOMP};
inline const QStringList southStopNames = (QStringList() << "S_StopOff" << "S_StopInv" << "S_StopComp");
// Overlap Status

enum overlapStat { NO_OVERLAP, NEG_OVERLAP, POS_OVERLAP};


// SkyScan Default Values
inline const int MIN_SKYSCAN_STEP_DEGREES = 5;
inline const int MAX_SKYSCAN_STEP_DEGREES = 45;
inline const int DEFAULT_SKYSCAN_STEP_DEGREES_INCREMENT = 10000;
inline const int MIN_SKYSCAN_PAUSE_MINS = 1;
inline const int MAX_SKYSCAN_PAUSE_MINS = 45;
inline const int SKYSCAN_PAUSE_STEP_INCREMENT_MINS = 10000;
inline const int DEFAULT_MIN_SKYSCAN_START_BEARING = 10000;
inline const int DEFAULT_MAX_SKYSCAN_START_BEARING = 10000;
inline const int DEFAULT_SKYSCAN_START_BEARING = 10000;
inline const int DEFAULT_SKYSCAN_NUMBER_OF_STEPS =  10000;
inline const int DEFAULT_SKYSCAN_END_BEARING = 10000;

// Pushbutton Styles

inline const QString BUTTON_ON_STYLE = QString("background-color: Sandybrown ; border-style: outset; border-width: 1px; border-color: black; min-width: 5em; padding: 3px;\n");
inline const QString BUTTON_OFF_STYLE = QString("background-color: Gainsboro ; border-style: outset; border-width: 1px; border-color: black; min-width: 5em; padding: 3px;\n");

extern const QStringList presetButtonLabels;

enum skyScanBearingStates {ROT_STOPPED, ROT_REACHED_TARGET, ROT_NEAR_TARGET, ROT_MOVING, ROT_STOPPED_MOVING};
inline const QStringList bearingStateTxt = {"ROT_STOPPED", "ROT_REACHED_TARGET", "ROT_NEAR_TARGET", "ROT_MOVING", "ROT_STOPPED_MOVING"};

QString getBearingStateTxt(skyScanBearingStates state);

QString convertBearingToString(int bearing);

class srotParams
{
public:


  static void copyRot(srotParams* srce, srotParams &dest)
  {

      dest.antennaName = srce->antennaName;
      dest.antennaNumber = srce->antennaNumber;
      dest.configLabel = srce->configLabel;
      dest.comport = srce->comport;
      dest.rotatorModel = srce->rotatorModel;
      dest.rotatorManufacturer = srce->rotatorManufacturer;
      dest.rotatorModelName = srce->rotatorModelName;
      dest.rotatorModelNumber = srce->rotatorModelNumber;
      dest.pollInterval = srce->pollInterval;
      dest.min_azimuth = srce->min_azimuth;
      dest.max_azimuth = srce->max_azimuth;
      dest.endStopType = srce->endStopType;
      dest.rotatorCWEndStop = srce->rotatorCWEndStop;
      dest.rotatorCCWEndStop = srce->rotatorCCWEndStop;
      dest.rotType = srce->rotType;
      dest.min_elevation = srce->min_elevation;
      dest.max_elevation = srce->max_elevation;
      dest.southStopType = srce->southStopType;
      dest.overRunFlag = srce->overRunFlag;
      dest.supportCwCcwCmd = srce->supportCwCcwCmd;
      dest.simCwCcwCmd = srce->simCwCcwCmd;
      dest.antennaOffset = srce->antennaOffset;
      dest.moving = srce->moving;
      dest.showCompassDialFlag = srce->showCompassDialFlag;
      dest.portType = srce->portType;
      dest.advancedCommsFlag = srce->advancedCommsFlag;
      dest.networkAdd = srce->networkAdd;
      dest.networkPort = srce->networkPort;
      //dest.maxBaudRate = srce->maxBaudRate;
      //dest.minBaudRate = srce->minBaudRate;
      dest.baudrate = srce->baudrate;
      dest.parity = srce->parity;
      dest.stopbits = srce->stopbits;
      dest.databits = srce->databits;
      dest.handshake = srce->handshake;
      //dest.enableRot = srce->enableRot;
      dest.forceDtr = srce->forceDtr;
      dest.forceRts = srce->forceRts;
      dest.nearStopTolerance = srce->nearStopTolerance;


    }


  QString antennaName;
  QString antennaNumber;
  QString configLabel;
  QString comport;
  QString rotatorModel;
  QString rotatorManufacturer;
  QString rotatorModelName;
  int rotatorModelNumber = 0;
  int rotatorCWEndStop = COMPASS_MIN0;  // actual rotator endstops
  int rotatorCCWEndStop= COMPASS_MAX360;
  endStop rotType = ROT_0_360;          // actual rotator type

  QString pollInterval = ROT_DEFAULT_POLLINTERVAL;

  int min_azimuth = 0;                   // working endstops
  int max_azimuth = 0;
  endStop endStopType = ROT_0_360;      //working endstop type

  int min_elevation = 0;
  int max_elevation = 0;
  southStop southStopType = S_STOPOFF;
  bool overRunFlag = false;
  bool supportCwCcwCmd = true;
  bool simCwCcwCmd = false;
  int antennaOffset = 0;
  bool moving = false;
  bool showCompassDialFlag;
  int portType = 0;
  bool advancedCommsFlag;
  QString networkAdd;
  QString networkPort;
  //int maxBaudRate = 0;
  //int minBaudRate = 0;
  int baudrate = 0;
  serialCommonData::serialParityCodes parity = serialCommonData::serialParityCodes::MINOS_PARITY_NONE;
  int stopbits = 0;
  int databits = 0;
  serialCommonData::s_handshakeCodes handshake = serialCommonData::s_handshakeCodes::HANDSHAKE_NONE;
  serialCommonData::s_forceLinesCodes forceDtr = serialCommonData::s_forceLinesCodes::FORCE_LINE_NONE;
  serialCommonData::s_forceLinesCodes forceRts = serialCommonData::s_forceLinesCodes::FORCE_LINE_NONE;
  //bool enableRot = false;

  int nearStopTolerance = 0;


};



#endif // ROTATORCOMMON_H
